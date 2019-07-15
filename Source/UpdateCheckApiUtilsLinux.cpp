//=====================================================================
// Copyright(c) 2018-2019, Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \brief Linux + Mac implementation of the UpdateCheckApi Utilities.
//=====================================================================

#include "UpdateCheckApiUtils.h"

// C++:
#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>

#include <climits>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace UpdateCheckApiUtils
{

static const char* STR_ERROR_FAILED_TO_READ_OUTPUT = "Error: failed to read output";

static const char* LINUX_TEMP_DIRECTORY_ENV_VARIABLE_NAME = "TMPDIR";
static const char* LINUX_TEMP_DIRECTORY_DEFAULT_PATH = "/tmp";

bool GetTempDirectory(std::string& tempDir)
{
    const char* tmpPtr = getenv(LINUX_TEMP_DIRECTORY_ENV_VARIABLE_NAME);
    if (tmpPtr != nullptr)
    {
        tempDir = tmpPtr;
    }
    else
    {
#if defined(P_tmpdir)
        // P_tmpdir is a macro defined in stdio.h.
        tempDir = P_tmpdir;
#endif
        if (tempDir.empty())
        {
            // Use default path.
            tempDir = LINUX_TEMP_DIRECTORY_DEFAULT_PATH;
        }
    }

    return true;
}

// Returns true iff the function succeeded. Buffer is set to true iff the
// process is alive.
static bool IsProcessAlive(pid_t processId, bool& buffer)
{
    int rc = kill(processId, 0);
    buffer = (rc == 0);
    return true;
}

// Waits for a process to terminate, with a limit of timeoutMsec Milliseconds.
static bool WaitForProcessToTerminate(pid_t processId, unsigned long timeoutMsec, long* pExitCode, bool child)
{
    bool    theProcessExited = false;
    pid_t   waitPid;
    int     status;

    // Special (easy) case where the windows version has specified a timeout of INFINITE (-1)
    // In this case, we perform a blocking wait for the termination
    if (timeoutMsec == ULONG_MAX)
    {
        waitPid = waitpid(processId, &status, 0);
        theProcessExited = ((-1 != waitPid) && WIFEXITED(status));
    }
    else
    {
        // Sleep for timeoutMsec milliseconds, then check for the existance of /proc/<pid>
        // There is a pitfall here: pid recycling.  This won't happen if timeoutMsec is
        // reasonable, but if it is long enough for a whole 32-bits worth of new processes to
        // have been created during the sleep period, we will report the result incorrectly.
        // Unlikely, but possible.
        timespec toSleep;
        long accumulatedWaitTimeNanoseconds = 0;
        long nanoSecondsInSingleWait = 0;
        long timeoutNanoseconds = timeoutMsec * 1000 * 1000;

        // pay attention to the possibility of overflow for a 32-bit long
        // basically, don't convert the number into nanoseconds and then clean it up.
        nanoSecondsInSingleWait = std::min<long>(50 * 1000 * 1000, timeoutNanoseconds);
        toSleep.tv_sec = 0;
        toSleep.tv_nsec = nanoSecondsInSingleWait;

        while (false == theProcessExited && accumulatedWaitTimeNanoseconds < timeoutNanoseconds)
        {
            (void)nanosleep(&toSleep, NULL);

            if (child)
            {
                // And now do a non-blocking wait
                waitPid = waitpid(processId, &status, WNOHANG);

                // 0 means child exists
                theProcessExited = (waitPid != 0);
            }
            else
            {
                IsProcessAlive(processId, theProcessExited);
                theProcessExited = !theProcessExited;
            }

            accumulatedWaitTimeNanoseconds += nanoSecondsInSingleWait;
        }
    }

    // Unfortunately, we cannot detect the child process exit code on Linux.
    // So we lie
    if (pExitCode != NULL)
    {
        *pExitCode = 0;
    }

    return theProcessExited;
}

// Terminates a process running on the local machine.
static bool TerminateProcess(pid_t processId, bool isGracefulShutdownRequired)
{
    bool isTerminated = false;

    if (isGracefulShutdownRequired)
    {
        int rcKill = kill(processId, SIGTERM);

        if (rcKill == 0)
        {
            const unsigned long timeout = 2000;
            isTerminated = WaitForProcessToTerminate(processId, timeout, NULL, true);
        }
    }

    if (!isTerminated)
    {
        kill(processId, SIGKILL);
        waitpid(processId, NULL, 0);
        isTerminated = true;
    }

    return isTerminated;
}

// This is the data structure that is being used to communicate with the spawned process.
struct popen2_data_t
{
    pid_t m_childPid;
    int   m_fromChildChannel;
    int   m_toChildChannel;
};

// This is an alternative implementation of popen() that provides the caller
// with a bidirectional communication channel to the spawned process.
static bool popen2(const char* pCmd, popen2_data_t& childInfo)
{
    bool ret = false;
    pid_t p;
    int pipe_stdin[2], pipe_stdout[2];

    if (pCmd != nullptr && !pipe(pipe_stdin) && !pipe(pipe_stdout))
    {
        // Create the child process.
        p = fork();

        if (p > -1)
        {
            if (p == 0)
            {
                // This is the child process. Execute the command.
                close(pipe_stdin[1]);
                dup2(pipe_stdin[0], 0);
                close(pipe_stdout[0]);
                dup2(pipe_stdout[1], 1);

                execl("/bin/sh", "sh", "-c", pCmd, NULL);
                perror("execl");
                _exit(99);
            }

            // Set the output.
            childInfo.m_childPid = p;
            childInfo.m_toChildChannel = pipe_stdin[1];
            childInfo.m_fromChildChannel = pipe_stdout[0];

            // We are done.
            ret = true;
        }
    }

    return ret;
}

// Executes the command in a different process and captures its output.
// This routine blocks, but, using the cancelSignal flag, it allows
// the caller to terminate the command's execution.
bool ExecAndGrabOutput(const char* cmd, const bool& cancelSignal, std::string& cmdOutput)
{
    // The default buffer size.
    const size_t BUFF_SIZE = 65536;
    bool ret = false;

    // Clear the output buffer.
    cmdOutput.clear();

    if (cmd != nullptr)
    {
        // Launch the command.
        popen2_data_t procData;

        if (popen2(cmd, procData))
        {
            // Prepare the temporary buffer.
            char tmpBuffer[BUFF_SIZE];
            memset(tmpBuffer, 0, BUFF_SIZE);

            while (true)
            {
                // The timeout interval in milliseconds.
                unsigned SLEEP_INTERVAL_MS = 100;

                if (cancelSignal || WaitForProcessToTerminate(procData.m_childPid, SLEEP_INTERVAL_MS, NULL, true))
                {
                    // If the the build cancel signal was set, or the CLI has finished
                    // running (or we have an error waiting), stop waiting.
                    break;
                }
            }

            if (cancelSignal)
            {
                TerminateProcess(procData.m_childPid, 0);
            }
            else
            {
                // Grab the command's output.
                // Set this operation to be non-blocking in case that we have no
                // pending data.
                fcntl(procData.m_fromChildChannel, F_SETFL, O_NONBLOCK);
                ret = (read(procData.m_fromChildChannel, tmpBuffer, BUFF_SIZE) != -1);

                if (!ret)
                {
                    cmdOutput = STR_ERROR_FAILED_TO_READ_OUTPUT;
                }
                else
                {
                    // Assign the output buffer.
                    cmdOutput += tmpBuffer;
                }
            }

            ret = true;

            // Close the child's output stream handle.
            close(procData.m_fromChildChannel);
        }
    }

    return ret;
}

} // namespace UpdateCheckApiUtils

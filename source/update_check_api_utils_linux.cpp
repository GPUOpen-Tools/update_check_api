//=====================================================================
/// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Linux + Mac implementation of the UpdateCheckApi Utilities.
//=====================================================================

#include "update_check_api_utils.h"

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
    static const char* kStringErrorFailedToReadOutput = "Error: failed to read output";

    static const char* kLinuxTempDirectoryEnvVariableName = "TMPDIR";
    static const char* kLinuxTempDirectoryDefaultPath     = "/tmp";

    bool GetTempDirectory(std::string& temp_dir)
    {
        const char* temp_ptr = getenv(kLinuxTempDirectoryEnvVariableName);
        if (temp_ptr != nullptr)
        {
            temp_dir = temp_ptr;
        }
        else
        {
#if defined(P_tmpdir)
            // P_tmpdir is a macro defined in stdio.h.
            temp_dir = P_tmpdir;
#endif
            if (temp_dir.empty())
            {
                // Use default path.
                temp_dir = kLinuxTempDirectoryDefaultPath;
            }
        }

        return true;
    }

    // Returns true iff the function succeeded. Buffer is set to true iff the
    // process is alive.
    static bool IsProcessAlive(pid_t process_id, bool& buffer)
    {
        int return_code = kill(process_id, 0);
        buffer          = (return_code == 0);
        return true;
    }

    // Waits for a process to terminate, with a limit of timeout_msec Milliseconds.
    static bool WaitForProcessToTerminate(pid_t process_id, unsigned long timeout_msec, long* exit_code, bool child)
    {
        bool  the_process_exited = false;
        pid_t wait_pid;
        int   status;

        // Special (easy) case where the windows version has specified a timeout of INFINITE (-1)
        // In this case, we perform a blocking wait for the termination
        if (timeout_msec == ULONG_MAX)
        {
            wait_pid           = waitpid(process_id, &status, 0);
            the_process_exited = ((-1 != wait_pid) && WIFEXITED(status));
        }
        else
        {
            // Sleep for timeout_msec milliseconds, then check for the existance of /proc/<pid>
            // There is a pitfall here: pid recycling.  This won't happen if timeout_msec is
            // reasonable, but if it is long enough for a whole 32-bits worth of new processes to
            // have been created during the sleep period, we will report the result incorrectly.
            // Unlikely, but possible.
            timespec to_sleep;
            long     accumulated_wait_time_nanoseconds = 0;
            long     nanoseconds_in_single_wait        = 0;
            long     timeout_nanoseconds               = timeout_msec * 1000 * 1000;

            // pay attention to the possibility of overflow for a 32-bit long
            // basically, don't convert the number into nanoseconds and then clean it up.
            nanoseconds_in_single_wait = std::min<long>(50 * 1000 * 1000, timeout_nanoseconds);
            to_sleep.tv_sec            = 0;
            to_sleep.tv_nsec           = nanoseconds_in_single_wait;

            while (false == the_process_exited && accumulated_wait_time_nanoseconds < timeout_nanoseconds)
            {
                (void)nanosleep(&to_sleep, NULL);

                if (child)
                {
                    // And now do a non-blocking wait
                    wait_pid = waitpid(process_id, &status, WNOHANG);

                    // 0 means child exists
                    the_process_exited = (wait_pid != 0);
                }
                else
                {
                    IsProcessAlive(process_id, the_process_exited);
                    the_process_exited = !the_process_exited;
                }

                accumulated_wait_time_nanoseconds += nanoseconds_in_single_wait;
            }
        }

        // Unfortunately, we cannot detect the child process exit code on Linux.
        // So we lie
        if (exit_code != NULL)
        {
            *exit_code = 0;
        }

        return the_process_exited;
    }

    // Terminates a process running on the local machine.
    static bool TerminateProcess(pid_t process_id, bool is_graceful_shutdown_required)
    {
        bool is_terminated = false;

        if (is_graceful_shutdown_required)
        {
            int return_code_kill = kill(process_id, SIGTERM);

            if (return_code_kill == 0)
            {
                const unsigned long timeout = 2000;
                is_terminated               = WaitForProcessToTerminate(process_id, timeout, NULL, true);
            }
        }

        if (!is_terminated)
        {
            kill(process_id, SIGKILL);
            waitpid(process_id, NULL, 0);
            is_terminated = true;
        }

        return is_terminated;
    }

    /// This is the data structure that is being used to communicate with the spawned process.
    struct popen2_data_t
    {
        pid_t child_pid;           ///< child process ID
        int   from_child_channel;  ///< pipe to communicate from the child
        int   to_child_channel;    ///< pipe to communicate to the child
    };

    /// This is an alternative implementation of popen() that provides the caller
    /// with a bidirectional communication channel to the spawned process.
    static bool popen2(const char* command, popen2_data_t& child_info)
    {
        bool  ret = false;
        pid_t child_pid;
        int   pipe_stdin[2], pipe_stdout[2];

        if (command != nullptr && !pipe(pipe_stdin) && !pipe(pipe_stdout))
        {
            // Create the child process.
            child_pid = fork();

            if (child_pid > -1)
            {
                if (child_pid == 0)
                {
                    // This is the child process. Execute the command.
                    close(pipe_stdin[1]);
                    dup2(pipe_stdin[0], 0);
                    close(pipe_stdout[0]);
                    dup2(pipe_stdout[1], 1);

                    execl("/bin/sh", "sh", "-c", command, NULL);
                    perror("execl");
                    _exit(99);
                }

                // Set the output.
                child_info.child_pid          = child_pid;
                child_info.to_child_channel   = pipe_stdin[1];
                child_info.from_child_channel = pipe_stdout[0];

                // We are done.
                ret = true;
            }
        }

        return ret;
    }

    // Executes the command in a different process and captures its output.
    // This routine blocks, but, using the cancelSignal flag, it allows
    // the caller to terminate the command's execution.
    bool ExecAndGrabOutput(const char* cmd, const bool& cancel_signal, std::string& cmd_output)
    {
        // The default buffer size.
        const size_t kBufferSize = 65536;
        bool         ret         = false;

        // Clear the output buffer.
        cmd_output.clear();

        if (cmd != nullptr)
        {
            // Launch the command.
            popen2_data_t proc_data;

            if (popen2(cmd, proc_data))
            {
                // Prepare the temporary buffer.
                char temp_buffer[kBufferSize];
                memset(temp_buffer, 0, kBufferSize);

                while (true)
                {
                    // The timeout interval in milliseconds.
                    const unsigned kSleepIntervalMilliseconds = 100;

                    if (cancel_signal || WaitForProcessToTerminate(proc_data.child_pid, kSleepIntervalMilliseconds, NULL, true))
                    {
                        // If the the build cancel signal was set, or the CLI has finished
                        // running (or we have an error waiting), stop waiting.
                        break;
                    }
                }

                if (cancel_signal)
                {
                    TerminateProcess(proc_data.child_pid, 0);
                }
                else
                {
                    // Grab the command's output.
                    // Set this operation to be non-blocking in case that we have no
                    // pending data.
                    fcntl(proc_data.from_child_channel, F_SETFL, O_NONBLOCK);
                    ret = (read(proc_data.from_child_channel, temp_buffer, kBufferSize) != -1);

                    if (!ret)
                    {
                        cmd_output = kStringErrorFailedToReadOutput;
                    }
                    else
                    {
                        // Assign the output buffer.
                        cmd_output += temp_buffer;
                    }
                }

                ret = true;

                // Close the child's output stream handle.
                close(proc_data.from_child_channel);
            }
        }

        return ret;
    }

}  // namespace UpdateCheckApiUtils

//=============================================================================
// Copyright(c) 2018-2019, Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \brief Windows implementation of the UpdateCheckApi Utilities.
//=============================================================================

#include "UpdateCheckApiUtils.h"

// C++:
#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>
#include <Shlwapi.h>

namespace UpdateCheckApiUtils
{
static const char* STR_TEMP_FILE_PREFIX = "tmpAMDToolsUpdateCheck_";
static const char* STR_TEMP_FILE_EXTENSION = ".txt";
static const char* STR_ERROR_FAILED_TO_GET_TEMP_DIR = "Error: failed to get Temp directory for downloading file.";
static const char* STR_ERROR_FAILED_TO_LAUNCH_COMMAND = "Error: failed to launch the command.";

static const char* WINDOWS_TEMP_DIRECTORY_DEFAULT_PATH = "C:/Windows/Temp";

std::string GetTempDirectory()
{
    std::string strTempDir;
    const DWORD maxPathLen = MAX_PATH + 1;
    char stringBuffer[maxPathLen];

    // Get a temp directory from Windows.
#ifdef _UNICODE
    wchar_t wstringBuffer[maxPathLen];
    DWORD gotTempPath = GetTempPath(maxPathLen, wstringBuffer);

    assert(gotTempPath != 0);
    if (gotTempPath != 0)
    {
        int mbLength = WideCharToMultiByte(CP_UTF8, 0, wstringBuffer, -1, stringBuffer, maxPathLen * sizeof(char), NULL, NULL);
        assert(mbLength != 0);
        if (mbLength != 0)
        {
            strTempDir = stringBuffer;
        }
    }
#else
    DWORD gotTempPath = GetTempPath(maxPathLen, stringBuffer);
    assert(gotTempPath != 0);
    if (gotTempPath != 0)
    {
        strTempDir = stringBuffer;
    }
#endif // !_UNICODE

    // If that didn't work to get a user-specific path, then use a default Temp dir.
    if (strTempDir.size() == 0)
    {
        strTempDir = WINDOWS_TEMP_DIRECTORY_DEFAULT_PATH;
    }

    return strTempDir;
}

bool ExecAndGrabOutput(const char* cmd, const bool& cancelSignal, std::string& cmdOutput)
{
    bool ret = false;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Clear the output string.
    cmdOutput.clear();

    // Generate a unique file name for the temp file.
#ifdef _UNICODE
    std::wstringstream tmpFileName;
    std::wstringstream tmpFilePath;
#else
    std::stringstream tmpFileName;
    std::stringstream tmpFilePath;
#endif
    tmpFileName << STR_TEMP_FILE_PREFIX << __rdtsc() << STR_TEMP_FILE_EXTENSION;

    // Get Windows TEMP directory:
    static const int bufferLength = 261;
#ifdef _UNICODE
    WCHAR stringBuffer[bufferLength];
#else
    char stringBuffer[bufferLength];
#endif
    DWORD result = GetEnvironmentVariable(TEXT("TEMP"), stringBuffer, bufferLength);
    if (0 == result)
    {
        cmdOutput = STR_ERROR_FAILED_TO_GET_TEMP_DIR;
    }
    else
    {
        tmpFilePath << stringBuffer << "/" << tmpFileName.str();

        if (cmd != NULL)
        {
            HANDLE h = CreateFile(tmpFilePath.str().c_str(),
                FILE_APPEND_DATA,
                FILE_SHARE_WRITE | FILE_SHARE_READ,
                &sa,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            PROCESS_INFORMATION pi;
            STARTUPINFO si;
            BOOL wasProcessCreated = FALSE;
            DWORD flags = CREATE_NO_WINDOW;

            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            ZeroMemory(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags |= STARTF_USESTDHANDLES;
            si.hStdInput = NULL;
            si.hStdError = h;
            si.hStdOutput = h;

#ifdef _UNICODE
            int cmdLen = MultiByteToWideChar(CP_ACP, 0, cmd, -1, NULL, 0);

            WCHAR* pCmdLine = new WCHAR[cmdLen];
            MultiByteToWideChar(CP_ACP, 0, cmd, -1, pCmdLine, cmdLen);
#else
            char* pCmdLine = new char[strlen(cmd) + 1];
            memcpy(pCmdLine, cmd, strlen(cmd));
            pCmdLine[strlen(cmd)] = NULL;
#endif
            wasProcessCreated = CreateProcess(NULL, pCmdLine, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);

            delete[] pCmdLine;

            if (TRUE == wasProcessCreated)
            {
                // The timeout interval in milliseconds.
                unsigned int SLEEP_INTERVAL_MS = 100;

                while (WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, SLEEP_INTERVAL_MS))
                {
                    if (cancelSignal)
                    {
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        CloseHandle(h);
                    }
                }

                if (!cancelSignal)
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    CloseHandle(h);
                }

                // Read the temp file
                if (TRUE == PathFileExists(tmpFilePath.str().c_str()))
                {
                    // Read the command's output.
                    std::fstream tmpFileStream;

                    tmpFileStream.open(tmpFilePath.str(), std::ifstream::in);

                    if (tmpFileStream.is_open())
                    {
                        std::stringstream tmpCmdOutput;
                        tmpCmdOutput << tmpFileStream.rdbuf();
                        tmpFileStream.close();

                        cmdOutput = tmpCmdOutput.str();
                    }
                }

                ret = true;
            }
            else
            {
                cmdOutput = STR_ERROR_FAILED_TO_LAUNCH_COMMAND;
            }

            // Delete the temporary file.
            DeleteFile(tmpFilePath.str().c_str());
        }
    }

    return ret;
}

} // namespace UpdateCheckApiUtils

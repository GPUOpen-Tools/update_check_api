//=============================================================================
/// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Windows implementation of the UpdateCheckApi Utilities.
//=============================================================================

#include "update_check_api_utils.h"
#include "update_check_api_strings.h"

#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>
#include <Shlwapi.h>

namespace UpdateCheckApiUtils
{
    static const char* kStringTempFilePrefix             = "tmpAMDToolsUpdateCheck_";
    static const char* kStringTempFileExtension          = ".txt";
    static const char* kStringErrorFailedToLaunchCommand = "Error: failed to launch the command.";

    bool GetTempDirectory(std::string& temp_dir)
    {
        const DWORD kMaxPathLength = MAX_PATH + 1;
        char        string_buffer[kMaxPathLength];
        bool        got_temp_dir = false;

        // Get a temp directory from Windows.
#ifdef _UNICODE
        wchar_t w_string_buffer[kMaxPathLength];
        DWORD   dw_got_temp_path = GetTempPath(kMaxPathLength, w_string_buffer);

        got_temp_dir = (dw_got_temp_path != 0);
        assert(got_temp_dir);
        if (got_temp_dir)
        {
            int mb_length = WideCharToMultiByte(CP_UTF8, 0, w_string_buffer, -1, string_buffer, kMaxPathLength * sizeof(char), NULL, NULL);
            assert(mb_length != 0);
            if (mb_length != 0)
            {
                temp_dir = string_buffer;
            }
        }
#else
        DWORD got_temp_path = GetTempPath(kMaxPathLength, string_buffer);
        got_temp_dir        = (got_temp_path != 0);
        assert(got_temp_dir);
        if (got_temp_dir)
        {
            temp_dir = string_buffer;
        }
#endif  // !_UNICODE

        // If GetTempPath was successful, then make sure that path actually exists and is writeable,
        // otherwise skip ahead and report the error condition.
        if (got_temp_dir)
        {
            // Test if directory exists and has write access.
            struct stat info;

            if (stat(temp_dir.c_str(), &info) != 0)
            {
                // Path does not exist.
                got_temp_dir = false;
            }
            else
            {
                // Make sure it is a directory and has Read & Write permissions.
                if (((info.st_mode & S_IFDIR) == S_IFDIR) && ((info.st_mode & S_IREAD) == S_IREAD) && ((info.st_mode & S_IWRITE) == S_IWRITE))
                {
                    got_temp_dir = true;
                }
                else
                {
                    // It is either not a directory, or is missing read or write permissions.
                    got_temp_dir = false;
                }
            }
        }

        return got_temp_dir;
    }

    bool ExecAndGrabOutput(const char* cmd, const bool& cancel_signal, std::string& cmd_output)
    {
        bool                return_value = false;
        SECURITY_ATTRIBUTES sa;
        sa.nLength              = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle       = TRUE;

        // Clear the output string.
        cmd_output.clear();

        // Generate a unique file name for the temp file.
#ifdef _UNICODE
        std::wstringstream temp_file_name;
        std::wstringstream temp_file_path;
#else
        std::stringstream temp_file_name;
        std::stringstream temp_file_path;
#endif
        temp_file_name << kStringTempFilePrefix << __rdtsc() << kStringTempFileExtension;

        std::string temp_dir;
        if (!GetTempDirectory(temp_dir))
        {
            cmd_output = kStringErrorUnableToFindTempDirectory;
        }
        else
        {
#ifdef _UNICODE

            int    temp_dir_length = MultiByteToWideChar(CP_ACP, 0, temp_dir.c_str(), -1, NULL, 0);
            WCHAR* temp_dir_w      = new WCHAR[temp_dir_length];
            MultiByteToWideChar(CP_ACP, 0, temp_dir.c_str(), -1, temp_dir_w, temp_dir_length);
            temp_file_path << temp_dir_w << "/" << temp_file_name.str();
            delete[] temp_dir_w;
#else
            temp_file_path << temp_dir << "/" << temp_file_name.str();
#endif  // !_UNICODE

            if (cmd != NULL)
            {
                HANDLE h = CreateFile(
                    temp_file_path.str().c_str(), FILE_APPEND_DATA, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                PROCESS_INFORMATION pi;
                STARTUPINFO         si;
                BOOL                was_process_created = FALSE;
                DWORD               flags               = CREATE_NO_WINDOW;

                ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
                ZeroMemory(&si, sizeof(STARTUPINFO));
                si.cb = sizeof(STARTUPINFO);
                si.dwFlags |= STARTF_USESTDHANDLES;
                si.hStdInput  = NULL;
                si.hStdError  = h;
                si.hStdOutput = h;

#ifdef _UNICODE
                int cmd_len = MultiByteToWideChar(CP_ACP, 0, cmd, -1, NULL, 0);

                WCHAR* cmd_line = new WCHAR[cmd_len];
                MultiByteToWideChar(CP_ACP, 0, cmd, -1, cmd_line, cmd_len);
#else
                char* cmd_line = new char[strlen(cmd) + 1];
                memcpy(cmd_line, cmd, strlen(cmd));
                cmd_line[strlen(cmd)] = NULL;
#endif
                was_process_created = CreateProcess(NULL, cmd_line, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);

                delete[] cmd_line;

                if (TRUE == was_process_created)
                {
                    // The timeout interval in milliseconds.
                    unsigned int SLEEP_INTERVAL_MS = 100;

                    while (WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, SLEEP_INTERVAL_MS))
                    {
                        if (cancel_signal)
                        {
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);
                            CloseHandle(h);
                        }
                    }

                    if (!cancel_signal)
                    {
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        CloseHandle(h);
                    }

                    // Read the temp file
                    if (TRUE == PathFileExists(temp_file_path.str().c_str()))
                    {
                        // Read the command's output.
                        std::fstream temp_file_stream;

                        temp_file_stream.open(temp_file_path.str(), std::ifstream::in);

                        if (temp_file_stream.is_open())
                        {
                            std::stringstream temp_cmd_output;
                            temp_cmd_output << temp_file_stream.rdbuf();
                            temp_file_stream.close();

                            cmd_output = temp_cmd_output.str();
                        }
                    }

                    return_value = true;
                }
                else
                {
                    cmd_output = kStringErrorFailedToLaunchCommand;
                }

                // Delete the temporary file.
                DeleteFile(temp_file_path.str().c_str());
            }
        }

        return return_value;
    }

}  // namespace UpdateCheckApiUtils

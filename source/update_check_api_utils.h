//=============================================================================
/// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Utility functions that have OS-dependent implementations.
//=============================================================================
#ifndef UPDATECHECKAPI_UPDATE_CHECK_API_UTILS_H_
#define UPDATECHECKAPI_UPDATE_CHECK_API_UTILS_H_

#include <string>

namespace UpdateCheckApiUtils
{
    /// @brief Retrieves the temporary directory.
    ///
    /// Files belong in this directory if they are expected to only exist for
    /// the duration that the application is running. Guaranteed to not return
    /// an empty string.
    ///
    /// @param [out] temp_dir The path to the temporary directory.
    ///
    /// @return true if a temp directory was obtained.
    bool GetTempDirectory(std::string& temp_dir);

    /// @brief Executes a supplied command line and captures the output text.
    ///
    /// This is a synchronous method, but has support for canceling the process if it is
    /// taking longer than desired. Returns true if the process executes
    /// successfully even if there is no output.
    ///
    /// @param [in]  cmd           The command to execute.
    /// @param [in]  cancel_signal Reference to a boolean that can be set to true to abort the command execution.
    /// @param [out] cmd_output    The output of executing the supplied command.
    ///
    /// @return true if successful; false otherwise.
    bool ExecAndGrabOutput(const char* cmd, const bool& cancel_signal, std::string& cmd_output);
}

#endif  // UPDATECHECKAPI_UPDATE_CHECK_API_UTILS_H_

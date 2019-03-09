//=============================================================================
// Copyright (c) 2018-2019 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file UpdateCheckApiUtils.h
/// \brief Utility functions that have OS-dependent implementations.
//=============================================================================

#ifndef _UPDATECHECKAPI_UPDATECHECKAPIUTILS_H_
#define _UPDATECHECKAPI_UPDATECHECKAPIUTILS_H_

#include <string>

namespace UpdateCheckApiUtils
{
    // Retrieves the temporary directory. Files belong in this directory if they
    // are expected to only exist for the duration that the application is
    // running. Guaranteed to not return an empty string.
    std::string GetTempDirectory();

    // Executes a supplied command line and captures the output text. This is a
    // synchronous method, but has support for canceling the process if it is
    // taking longer than desired. Returns true if the process executes
    // successfully even if there is no output.
    bool ExecAndGrabOutput(const char* cmd, const bool& cancelSignal, std::string& cmdOutput);
}

#endif  // _UPDATECHECKAPI_UPDATECHECKAPIUTILS_H_


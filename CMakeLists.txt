#=================================================================
# Copyright 2018-2019 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
# CMakeList.txt : CMake project for UpdateCheckApi, include source and define
# project specific logic here. The UpdateCheckApi is just a set a of source
# code that other projects can include, it is not a buildable project on its
# own.

cmake_minimum_required (VERSION 3.0)

# Root to the UpdateCheckApi directory.
set (UPDATECHECKAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Note: json.hpp file was originally downloaded from https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp
set (JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Ext/json-3.2.0)

if(WIN32)
    set(OS_SUFFIX Win32)
    set(UPDATECHECKAPI_OS_LIBS Shlwapi)
    set(AMDTOOLSDOWNLOADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/AMDToolsDownloader/Windows/AMDToolsDownloader.exe CACHE INTERNAL "")
elseif(UNIX AND NOT APPLE)
    set(OS_SUFFIX Linux)
    set(UPDATECHECKAPI_OS_LIBS rt -pthread)
    set(AMDTOOLSDOWNLOADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/AMDToolsDownloader/Linux/AMDToolsDownloader CACHE INTERNAL "")
elseif(APPLE)
    # Intentionally using the same Linux code on APPLE.
    set(OS_SUFFIX Linux)
    set(UPDATECHECKAPI_OS_LIBS -pthread)
    set(AMDTOOLSDOWNLOADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/AMDToolsDownloader/Mac/AMDToolsDownloader CACHE INTERNAL "")
else()
    message(FATAL_ERROR "The current platform is not supported by UpdateCheckApi.")
endif()

# Set a list of all source files.
set(UPDATECHECKAPI_SRC
    ${UPDATECHECKAPI_DIR}/Source/UpdateCheckApi.cpp
    ${UPDATECHECKAPI_DIR}/Source/UpdateCheckApiUtils${OS_SUFFIX}.cpp
    ${JSON_DIR}/json.hpp
    CACHE INTERNAL "")

# Set a list of Qt source files.
set(UPDATECHECKAPI_QT_SRC
    ${UPDATECHECKAPI_DIR}/Source/UpdateCheckThread.cpp
    ${UPDATECHECKAPI_DIR}/Source/UpdateCheckResultsDialog.cpp
    CACHE INTERNAL "")

# Set a list of all header files.
set(UPDATECHECKAPI_INC
    ${UPDATECHECKAPI_DIR}/Include/UpdateCheckApi.h
    ${UPDATECHECKAPI_DIR}/Include/UpdateCheckApiStrings.h
    ${UPDATECHECKAPI_DIR}/Include/UpdateCheckApiUtils.h
    CACHE INTERNAL "")

# Set a list of Qt header files.
set(UPDATECHECKAPI_QT_INC
    ${UPDATECHECKAPI_DIR}/Include/UpdateCheckThread.h
    ${UPDATECHECKAPI_DIR}/Include/UpdateCheckResultsDialog.h
    CACHE INTERNAL "")

# Set a list of Qt UI files.
set (UPDATECHECKAPI_QT_UI
    ${UPDATECHECKAPI_DIR}/Source/UpdateCheckResultsDialog.ui
    CACHE INTERNAL "")

# Set a list of all include directories.
set(UPDATECHECKAPI_INC_DIRS
    ${UPDATECHECKAPI_DIR}/Include
    ${UPDATECHECKAPI_DIR}/Ext
    CACHE INTERNAL "")

# Set a list of all additional library directories.
set (UPDATECHECKAPI_LIB_DIRS
    CACHE INTERNAL "")

# Set a list of additonal libraries to link.
set (UPDATECHECKAPI_LIBS ${UPDATECHECKAPI_OS_LIBS} CACHE INTERNAL "")

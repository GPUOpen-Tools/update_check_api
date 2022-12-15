#=================================================================
# Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
# CMakeList.txt : CMake project for UpdateCheckApi, include source and define
# project specific logic here. The UpdateCheckApi is just a set a of source
# code that other projects can include, it is not a buildable project on its
# own.

cmake_minimum_required (VERSION 3.10)

# Root to the UpdateCheckApi directory.
set (UPDATECHECKAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Note: json.hpp file was originally downloaded from https://github.com/nlohmann/json/releases/download/v3.9.1/json.hpp
set (JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/json-3.9.1)

if(WIN32)
    set(OS_SUFFIX Win32)
    set(OS_SUFFIX_LOWER win32)
    set(UPDATECHECKAPI_OS_LIBS Shlwapi)
    set(RTDA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rtda/windows/rtda.exe CACHE INTERNAL "")
elseif(UNIX AND NOT APPLE)
    set(OS_SUFFIX Linux)
    set(OS_SUFFIX_LOWER linux)
    set(UPDATECHECKAPI_OS_LIBS rt -pthread)
    set(RTDA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rtda/linux/rtda CACHE INTERNAL "")
elseif(APPLE)
    # Intentionally using the same Linux code on APPLE.
    set(OS_SUFFIX Linux)
    set(OS_SUFFIX_LOWER linux)
    set(UPDATECHECKAPI_OS_LIBS -pthread)
    set(RTDA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rtda/macos/rtda CACHE INTERNAL "")
else()
    message(FATAL_ERROR "The current platform is not supported by UpdateCheckApi.")
endif()

# Set a list of all source files.
set(UPDATECHECKAPI_SRC
    ${UPDATECHECKAPI_DIR}/source/update_check_api.cpp
    ${UPDATECHECKAPI_DIR}/source/update_check_api_utils_${OS_SUFFIX_LOWER}.cpp
    ${JSON_DIR}/json.hpp
    CACHE INTERNAL "")

# Set a list of Qt source files.
set(UPDATECHECKAPI_QT_SRC
    ${UPDATECHECKAPI_DIR}/source/update_check_thread.cpp
    ${UPDATECHECKAPI_DIR}/source/update_check_results_dialog.cpp
    CACHE INTERNAL "")

# Set a list of all header files.
set(UPDATECHECKAPI_INC
    ${UPDATECHECKAPI_DIR}/source/update_check_api.h
    ${UPDATECHECKAPI_DIR}/source/update_check_api_strings.h
    ${UPDATECHECKAPI_DIR}/source/update_check_api_utils.h
    CACHE INTERNAL "")

# Set a list of Qt header files.
set(UPDATECHECKAPI_QT_INC
    ${UPDATECHECKAPI_DIR}/source/update_check_thread.h
    ${UPDATECHECKAPI_DIR}/source/update_check_results_dialog.h
    CACHE INTERNAL "")

# Set a list of Qt UI files.
set (UPDATECHECKAPI_QT_UI
    ${UPDATECHECKAPI_DIR}/source/update_check_results_dialog.ui
    CACHE INTERNAL "")

# Set a list of all include directories.
set(UPDATECHECKAPI_INC_DIRS
    ${UPDATECHECKAPI_DIR}/source
    CACHE INTERNAL "")

# Set a list of all additional library directories.
set (UPDATECHECKAPI_LIB_DIRS
    CACHE INTERNAL "")

# Set a list of additonal libraries to link.
set (UPDATECHECKAPI_LIBS ${UPDATECHECKAPI_OS_LIBS} CACHE INTERNAL "")

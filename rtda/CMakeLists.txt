#=================================================================
# Copyright 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
# CMakeList.txt : CMake project to compile radeon_tools_download_assistant.go
#
cmake_minimum_required(VERSION 3.10)

# Make a target that other projects can add as a dependency.
add_custom_target(rtda)

# Set the platform-dependent name of the binary.
if(WIN32)
    set (RTDA_BINARY rtda.exe)
elseif (LINUX OR APPLE)
    set (RTDA_BINARY rtda)
endif()

# Set path to compiled rtda binary.
# This is cached so that projects depending on this binary
# can copy it into their local output directory.
set(RTDA_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/${RTDA_BINARY}
    CACHE INTERNAL "")

# The command which actually compiles the binary.
add_custom_command(TARGET rtda PRE_BUILD
    COMMAND go build
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/radeon_tools_download_assistant.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    BYPRODUCTS ${RTDA_PATH}
    COMMENT "'go build' rtda into ${CMAKE_CURRENT_SOURCE_DIR}"
)

#=================================================================
# Copyright 2018-2019 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
# CMakeList.txt : CMake project to compile AMDToolsDownloader.go
#
cmake_minimum_required(VERSION 2.8.11)

# Make a target that other projects can add as a dependency.
add_custom_target(AMDToolsDownloader)

# Set the platform-dependent name of the binary.
if(WIN32)
set (AMDTOOLSDOWNLOADER_BINARY AMDToolsDownloader.exe)
elseif (LINUX OR APPLE)
set (AMDTOOLSDOWNLOADER_BINARY AMDToolsDownloader)
endif()

# Set path to compiled AMDToolsDownloader binary.
# This is cached so that projects depending on this binary
# can copy it into their local output directory.
set(AMDTOOLSDOWNLOADER_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/${AMDTOOLSDOWNLOADER_BINARY}
    CACHE INTERNAL "")

# The command which actually compiles the binary.
add_custom_command(TARGET AMDToolsDownloader PRE_BUILD
    COMMAND go build
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/AMDToolsDownloader.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    BYPRODUCTS ${AMDTOOLSDOWNLOADER_PATH}
    COMMENT "'go build' AMDToolsDownloader into ${CMAKE_CURRENT_SOURCE_DIR}"
)

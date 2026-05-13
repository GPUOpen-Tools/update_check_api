#=================================================================
# Copyright (c) 2018-2026 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
# CMakeList.txt : CMake project for UpdateCheckApi.

cmake_minimum_required (VERSION 3.23)
project(UpdateCheckApi)

option(UPDATE_CHECK_API_HEADLESS "If ON, do not build components that require Qt." OFF)

if (NOT UPDATE_CHECK_API_HEADLESS)
    if (NOT TARGET Qt::Widgets)
        if (NOT PROJECT_IS_TOP_LEVEL)
            message(WARNING "Qt is not configured. Did you mean to set UPDATE_CHECK_API_HEADLESS?")
        endif()
        find_package(Qt6 COMPONENTS REQUIRED Core Gui Widgets)
    endif()
endif()

if (NOT UPDATE_CHECK_API_HEADLESS)
    # Build Qt .ui MOC files for the library.
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# Note: json.hpp file was originally downloaded from https://github.com/nlohmann/json/releases/download/v3.9.1/json.hpp
set (JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/json-3.9.1)

if(WIN32)
    set(OS_SUFFIX Win32)
    set(OS_SUFFIX_LOWER win32)
    set(UPDATE_CHECK_API_OS_LIBS Shlwapi)
    set(RTDA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rtda/windows/rtda.exe CACHE INTERNAL "")
elseif(UNIX AND NOT APPLE)
    set(OS_SUFFIX Linux)
    set(OS_SUFFIX_LOWER linux)
    set(UPDATE_CHECK_API_OS_LIBS rt -pthread)
    set(RTDA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rtda/linux/rtda CACHE INTERNAL "")
elseif(APPLE)
    # Intentionally using the same Linux code on APPLE.
    set(OS_SUFFIX Linux)
    set(OS_SUFFIX_LOWER linux)
    set(UPDATE_CHECK_API_OS_LIBS -pthread)
    set(RTDA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rtda/macos/rtda CACHE INTERNAL "")
else()
    message(FATAL_ERROR "The current platform is not supported by UpdateCheckApi.")
endif()

# Root to the UpdateCheckApi directory.
set (UPDATE_CHECK_API_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Set a list of all source files.
set(UPDATE_CHECK_API_SRC
    ${UPDATE_CHECK_API_DIR}/source/update_check_api.cpp
    ${UPDATE_CHECK_API_DIR}/source/update_check_api_utils_${OS_SUFFIX_LOWER}.cpp
    ${JSON_DIR}/json.hpp)

# Set a list of Qt source files.
set(UPDATE_CHECK_API_QT_SRC
    ${UPDATE_CHECK_API_DIR}/source/update_check_thread.cpp
    ${UPDATE_CHECK_API_DIR}/source/update_check_results_dialog.cpp)

# Set a list of all header files.
set(UPDATE_CHECK_API_INC
    ${UPDATE_CHECK_API_DIR}/source/update_check_api.h
    ${UPDATE_CHECK_API_DIR}/source/update_check_api_strings.h
    ${UPDATE_CHECK_API_DIR}/source/update_check_api_utils.h)

# Set a list of Qt header files.
set(UPDATE_CHECK_API_QT_INC
    ${UPDATE_CHECK_API_DIR}/source/update_check_thread.h
    ${UPDATE_CHECK_API_DIR}/source/update_check_results_dialog.h)

# Set a list of Qt UI files.
set (UPDATE_CHECK_API_QT_UI
    ${UPDATE_CHECK_API_DIR}/source/update_check_results_dialog.ui)

add_library(update_check_api STATIC)
add_library(AMD::update_check_api ALIAS update_check_api)

# Define sources, headers, and libraries for all builds.
target_sources(update_check_api
    PRIVATE ${UPDATE_CHECK_API_SRC}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/source"
        FILES ${UPDATE_CHECK_API_INC})
target_link_libraries(update_check_api PRIVATE ${UPDATE_CHECK_API_OS_LIBS})

# Add sources, headers, and libraries for non-headless builds.
if (NOT UPDATE_CHECK_API_HEADLESS)
    add_library(update_check_api_qt STATIC)
    add_library(AMD::update_check_api_qt ALIAS update_check_api_qt)
    target_sources(update_check_api_qt
        PRIVATE ${UPDATE_CHECK_API_QT_SRC}
        PUBLIC
            FILE_SET HEADERS
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/source"
            FILES ${UPDATE_CHECK_API_QT_INC})
    target_link_libraries(update_check_api_qt PUBLIC AMD::update_check_api Qt::Widgets)
endif()

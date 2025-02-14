# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.15)

project(KDToolBars VERSION 0.1.0)

include(FeatureSummary)
include(GNUInstallDirs)

option(KDToolBars_QT6 "Build against Qt 6" OFF)
option(KDToolBars_TESTS "Build tests" ON)
option(KDToolBars_STATIC "Build statically" OFF)
option(KDToolBars_DOCS "Build the API documentation" OFF)
option(KDToolBars_EXAMPLES "Build examples" ON)
option(KDToolBars_DEVELOPER_MODE "Developer Mode" OFF)
option(KDToolBars_WERROR "Compile with -Werror" OFF)
option(KDToolBars_ENABLE_SANITIZERS "Compile with ASAN and UBSAN" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/ECM/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/KDAB/modules")

# Set a default build type if none was specified
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git" OR KDToolBars_DEVELOPER_MODE)
    set(default_build_type "Debug")
endif()

if(KDToolBars_ENABLE_SANITIZERS)
    # For CI usage, but feel free to add MSVC support
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

if(KDToolBars_DEVELOPER_MODE)
    set(KDToolBars_WERROR ON)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to ${default_build_type} as none was specified.")
    set(CMAKE_BUILD_TYPE
        "${default_build_type}"
        CACHE STRING "Choose the type of build." FORCE
    )

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(KDToolBars_QT6)
    set(QT_VERSION_MAJOR 6)
    set(KDTOOLBARS_LIBRARY_QTID "-qt6")
else()
    set(QT_VERSION_MAJOR 5)
    set(KDTOOLBARS_LIBRARY_QTID "")
endif()

find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Widgets
    REQUIRED
)
include(KDQtInstallPaths) # to set QT_INSTALL_FOO variables

# Always build the test harness in developer-mode
if(KDToolBars_DEVELOPER_MODE)
    set(KDToolBars_TESTS ON)
endif()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(KDToolBars_IS_ROOT_PROJECT TRUE)

    message(STATUS "Building KDToolBars ${KDToolBars_VERSION} in ${CMAKE_BUILD_TYPE} mode. "
                   "Installing to ${CMAKE_INSTALL_PREFIX}"
    )

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
else()
    # Always disable tests, examples, docs when used as a submodule
    set(KDToolBars_IS_ROOT_PROJECT FALSE)
    set(KDToolBars_TESTS FALSE)
    set(KDToolBars_EXAMPLES FALSE)
    set(KDToolBars_DOCS FALSE)
endif()

if(KDToolBars_STATIC)
    set(KDTOOLBARS_LIBRARY_MODE "STATIC")
else()
    set(KDTOOLBARS_LIBRARY_MODE "SHARED")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Default to hidden visibility for symbols
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(src)

if(KDToolBars_EXAMPLES)
    add_subdirectory(examples)
endif()

if(KDToolBars_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(KDToolBars_DOCS)
    add_subdirectory(docs) # needs to go last, in case there are build source files
endif()

if(KDToolBars_IS_ROOT_PROJECT)
    # Add uninstall target (not for submodules since parent projects typically have uninstall too)
    include(ECMUninstallTarget)
endif()

if(KDToolBars_IS_ROOT_PROJECT)
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

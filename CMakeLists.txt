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

include(GNUInstallDirs)

option(KDTOOLBARS_QT6 "Build against Qt 6" OFF)
option(KDTOOLBARS_TESTS "Build tests" ON)
option(KDTOOLBARS_STATIC "Build statically" OFF)
option(KDTOOLBARS_EXAMPLES "Build examples" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(KDTOOLBARS_QT6)
    set(Qt_VERSION_MAJOR 6)
else()
    set(Qt_VERSION_MAJOR 5)
endif()

if(KDTOOLBARS_STATIC)
    set(KDTOOLBARS_LIBRARY_MODE "STATIC")
else()
    set(KDTOOLBARS_LIBRARY_MODE "SHARED")
endif()

find_package(
    Qt${Qt_VERSION_MAJOR}
    COMPONENTS Widgets
    REQUIRED
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(src)

if(KDTOOLBARS_EXAMPLES)
    add_subdirectory(examples)
endif()

if(KDTOOLBARS_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

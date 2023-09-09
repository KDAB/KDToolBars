# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.1.0)

project(KDToolBars VERSION 0.1.0)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(
    Qt5
    COMPONENTS Widgets
    REQUIRED
)

option(KDTOOLBARS_TESTS "Build tests" ON)
option(KDTOOLBARS_EXAMPLES "Build examples" ON)

add_subdirectory(src)

if(KDTOOLBARS_EXAMPLES)
    add_subdirectory(example)
endif()

if(KDTOOLBARS_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

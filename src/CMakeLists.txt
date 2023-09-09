# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
set(KDTOOLBARS_SRC
    toolbar.cpp
    toolbar.h
    toolbarseparator.cpp
    toolbarseparator.h
    toolbarlayout.cpp
    toolbarlayout.h
    toolbartraylayout.cpp
    toolbartraylayout.h
    toolbarcontainerlayout.cpp
    toolbarcontainerlayout.h
    mainwindow.cpp
    mainwindow.h
)

set(KDTOOLBARS_RESOURCES
    kdtoolbars_resources.qrc
)

add_library(kdtoolbars ${KDTOOLBARS_SRC} ${KDTOOLBARS_RESOURCES})

target_link_libraries(kdtoolbars PUBLIC Qt5::Widgets Qt5::WidgetsPrivate)

target_include_directories(kdtoolbars PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
find_package(
    Qt5
    COMPONENTS Test
    REQUIRED
)

add_executable(tst_toolbar tst_toolbar.cpp)
target_link_libraries(tst_toolbar kdtoolbars Qt5::Test)
add_test(NAME tst_toolbar COMMAND $<TARGET_FILE:tst_toolbar>)

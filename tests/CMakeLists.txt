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

# Create a test with the specified name
function(add_kdtoolbars_test name sources)
    set(TARGET_NAME test_kdtoolbars_${name})
    add_executable(${TARGET_NAME} ${sources})

    target_link_libraries(${TARGET_NAME} kdtoolbars Qt5::Test)

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
endfunction()

add_kdtoolbars_test(toolbar tst_toolbar.cpp)
add_kdtoolbars_test(mainwindow tst_mainwindow.cpp)

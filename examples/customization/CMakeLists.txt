# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
add_executable(customization-example main.cpp test.qrc)

target_link_libraries(customization-example PRIVATE kdtoolbars)

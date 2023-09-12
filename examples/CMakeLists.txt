# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
add_subdirectory(simple)
add_subdirectory(customization)

find_package(KDDockWidgets QUIET)
if(KDDockWidgets_FOUND)
    add_subdirectory(kddw-integration)
endif()

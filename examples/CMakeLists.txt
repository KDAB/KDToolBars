# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
add_subdirectory(simple)
add_subdirectory(customization)

set(KDDockWidgetsPackageName KDDockWidgets${KDTOOLBARS_LIBRARY_QTID})
find_package(${KDDockWidgetsPackageName} CONFIG)
set(KDDockWidgets_FOUND ${${KDDockWidgetsPackageName}_FOUND})
if(KDDockWidgets_FOUND)
    add_subdirectory(kddw-integration)
endif()

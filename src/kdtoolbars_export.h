/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#if defined(KDTOOLBARS_STATICLIB)
#define KDTOOLBARS_EXPORT
#else
#if defined(BUILDING_KDTOOLBARS_LIBRARY)
#define KDTOOLBARS_EXPORT Q_DECL_EXPORT
#else
#define KDTOOLBARS_EXPORT Q_DECL_IMPORT
#endif
#endif

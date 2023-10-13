/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QMouseEvent>
#include <QHoverEvent>
#include <QDropEvent>

namespace KDToolBars::Qt5Qt6Compat {

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

template<typename EventT>
inline
    typename std::enable_if_t<std::is_base_of_v<QMouseEvent, EventT>
                                  || std::is_base_of_v<QHoverEvent, EventT>
                                  || std::is_base_of_v<QDropEvent, EventT>,
                              QPoint>
    eventPos(const EventT *e)
{
    return e->pos();
}

inline QPoint eventGlobalPos(const QMouseEvent *e)
{
    return e->globalPos();
}

#else

template<typename EventT>
inline
    typename std::enable_if_t<std::is_base_of_v<QSinglePointEvent, EventT>
                                  || std::is_base_of_v<QDropEvent, EventT>,
                              QPoint>
    eventPos(const EventT *e)
{
    return e->position().toPoint();
}

inline QPoint eventGlobalPos(const QSinglePointEvent *e)
{
    return e->globalPosition().toPoint();
}

#endif

}

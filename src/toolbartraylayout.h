/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "mainwindow.h"
#include "toolbar_p.h"

#include <QLayoutItem>

#include <optional>

namespace KDToolBars {

class ToolBar;
class ToolBarContainerLayout;

struct ToolBarTrayLayoutState
{
    struct Item
    {
        int pos;
        QString objectName;
        bool isHidden;
        bool isFloating;
        QPoint floatingPos;
        ToolBarState toolBarState;
    };
    struct Row
    {
        std::vector<Item> items;
    };
    std::vector<Row> rows;

    void save(QDataStream &stream) const;
    bool load(QDataStream &stream);
};

class ToolBarTrayLayout
{
public:
    explicit ToolBarTrayLayout(
        ToolBarTray tray, Qt::Orientation orientation, ToolBarContainerLayout *parent);
    ~ToolBarTrayLayout();

    int count() const;
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    void setGeometry(const QRect &rect);
    QSize sizeHint() const;
    QSize minimumSize() const;
    void invalidate();

    void insertToolBar(ToolBar *before, ToolBar *toolbar);
    void insertToolBarBreak(ToolBar *before);

    void moveToolBar(ToolBar *toolbar, const QPoint &pos);
    void adjustToolBarRow(const ToolBar *toolbar);
    bool hoverToolBar(ToolBar *toolbar);

    int rowCount() const;
    Qt::Orientation orientation() const
    {
        return m_orientation;
    }

    ToolBarTray tray() const
    {
        return m_tray;
    }

    bool hasToolBar(const ToolBar *toolbar) const;

    ToolBarTrayLayoutState state() const;
    void applyState(const ToolBarTrayLayoutState &state, const std::vector<ToolBar *> &toolbars, const std::vector<QAction *> &actions);

private:
    struct Item
    {
        QLayoutItem *widgetItem;
        int pos;
        int size;
    };
    struct ItemPath
    {
        int row;
        int index;
    };
    struct Row
    {
        QVector<Item> items;
        int pos;
        QSize sizeHint;
        QSize minimumSize;

        int dockedCount() const;
    };
    std::optional<ItemPath> FindItem(const QWidget *widget) const;
    Item *item(const ItemPath &path);
    QLayoutItem *layoutItem(const ItemPath &path);
    std::tuple<QLayoutItem *, bool> TakeLayoutItem(const ItemPath &path);
    QVector<Item> adjustRow(const Row &row, const ToolBar *pivot) const;
    int adjustItemSizes(QVector<Item> &items, int availableSize) const;
    void updateRowSizes() const;
    void doLayout();

    int pick(const QSize &size) const
    {
        return m_orientation == Qt::Horizontal ? size.width() : size.height();
    }
    int &rpick(QSize &size) const
    {
        return m_orientation == Qt::Horizontal ? size.rwidth() : size.rheight();
    }
    int perp(const QSize &size) const
    {
        return m_orientation == Qt::Horizontal ? size.height() : size.width();
    }
    int &rperp(QSize &size) const
    {
        return m_orientation == Qt::Horizontal ? size.rheight() : size.rwidth();
    }
    int pick(const QPoint &pos) const
    {
        return m_orientation == Qt::Horizontal ? pos.x() : pos.y();
    }
    int &rpick(QPoint &pos) const
    {
        return m_orientation == Qt::Horizontal ? pos.rx() : pos.ry();
    }
    int perp(const QPoint &pos) const
    {
        return m_orientation == Qt::Horizontal ? pos.y() : pos.x();
    }
    int &rperp(QPoint &pos) const
    {
        return m_orientation == Qt::Horizontal ? pos.ry() : pos.rx();
    }

    ToolBarContainerLayout *m_parent;
    ToolBarTray m_tray;
    Qt::Orientation m_orientation;
    QMargins m_contentsMargins;
    QVector<Row> m_rows;
    QRect m_contentsRect;
    bool m_dirty = true;
};

} // namespace KDToolBars

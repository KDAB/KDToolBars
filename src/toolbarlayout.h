/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kdtoolbars_export.h"

#include <QLayout>

namespace KDToolBars {

class ToolBar;

struct ToolBarLayoutState
{
    std::vector<int> rowBreaks;

    void save(QDataStream &stream) const;
    bool load(QDataStream &stream);
};

class KDTOOLBARS_EXPORT ToolBarLayout : public QLayout
{
    Q_OBJECT
public:
    explicit ToolBarLayout(ToolBar *toolbar, QWidget *parent = nullptr);
    ~ToolBarLayout() override;

    int columns() const;
    void setColumns(int columns);

    void setMinimumSize(const QSize &size);

    int titleHeight() const;
    QRect titleArea() const;

    int handleExtent() const;
    QRect handleArea() const;

    enum ToolBarWidgetType {
        StandardButton,
        Separator,
        CustomWidget,
    };
    void insertWidget(int index, QWidget *widget, ToolBarWidgetType type);
    void setCloseButton(QWidget *widget);

    QSize adjustToWidth(int width);
    QSize adjustToHeight(int height);

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    Qt::Orientations expandingDirections() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void invalidate() override;

    // find where to place the drop indicator when hovering over this layout
    struct DropSite
    {
        int itemIndex = 0;
        QPoint topLeft;
        int size = 0; // width if m_layoutType == LayoutType::Vertical, height otherwise
    };
    DropSite findDropSite(const QPoint &pos) const;

    // margins for contents excluding the title bar and handle
    QMargins innerContentsMargins() const;
    QMargins innerContentsMargins(bool floating, Qt::Orientation dockedOrientation) const;

    QSize dockedContentsSize(Qt::Orientation orientation) const;

    ToolBarLayoutState state() const;
    void applyState(const ToolBarLayoutState &state);

private:
    int titleHeight(bool floating) const;
    int handleExtent(bool floating) const;

    struct DynamicLayout
    {
        QSize minimumSize;
        std::vector<int> rowBreaks;
    };

    enum class LayoutType {
        Horizontal,
        Vertical,
        Columns,
        Dynamic
    };
    LayoutType layoutType() const;
    LayoutType layoutType(bool floating, Qt::Orientation dockedOrientation) const;

    void updateGeometries() const;
    void layoutRows(const std::vector<int> &rowBreaks) const;
    void initializeDynamicLayouts() const;
    const DynamicLayout *preferredLayoutForSize(const QSize &availableSize) const;

    ToolBar *m_toolbar;
    QVector<QLayoutItem *> m_items;
    QLayoutItem *m_closeButton = nullptr;
    struct ItemRow
    {
        int height; // width if m_layoutType == LayoutType::Vertical
        struct Item
        {
            QLayoutItem *item;
            QRect geometry;
        };
        std::vector<Item> items;
    };
    mutable QVector<ItemRow> m_itemRows;
    mutable QSize m_contentsSize;
    mutable bool m_dirty = true;
    int m_columns = 1; // only used if m_layoutType == LayoutType::Columns
    mutable std::vector<DynamicLayout> m_dynamicLayouts;
    mutable std::vector<int> m_rowBreaks;
    QRect m_geometry;
    QSize m_minimumSize;
};

} // namespace KDToolBars

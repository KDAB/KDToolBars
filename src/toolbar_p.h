/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "toolbar.h"
#include "toolbarlayout.h"

#include <QMimeData>

#include <unordered_map>

namespace KDToolBars {

class DropIndicator;

struct ToolBarState
{
    struct Action
    {
        bool isSeparator;
        QString objectName;
    };
    std::vector<Action> actions;
    ToolBarLayoutState layoutState;

    void save(QDataStream &stream) const;
    bool load(QDataStream &stream);
};

class ToolbarActionMimeData : public QMimeData
{
    Q_OBJECT
public:
    QAction *action;
};

class ToolBar::Private : public QObject
{
public:
    enum class Margin {
        Left,
        Right,
        Top,
        Bottom,
        None
    };

    explicit Private(ToolBarOptions options, ToolBar *toolbar);

    void init();

    Margin marginAt(const QPoint &p) const;
    bool resizeStart(const QPoint &p);
    void resizeEnd();
    void dragMargin(const QPoint &p);

    void actionEvent(QActionEvent *event);
    bool mousePressEvent(const QMouseEvent *me);
    bool mouseMoveEvent(const QMouseEvent *me);
    bool mouseReleaseEvent(const QMouseEvent *me);
    bool mouseDoubleClickEvent(const QMouseEvent *me);
    bool hoverMoveEvent(const QHoverEvent *me);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);

    QRect titleArea() const;
    QRect handleArea() const;

    // update window flags when the toolbar is docked or undocked
    void setWindowState(bool floating, const QPoint &pos = {});

    // toolbar is docked and is being dragged by the handle
    bool isMoving() const;

    void undock(const QPoint &pos);
    void dock();
    void offsetDragPosition(const QPoint &offset);

    QSize dockedSize() const;

    bool isResizing() const
    {
        return m_resizeMargin != Margin::None;
    }

    bool isResizable() const
    {
        return !m_columnLayout;
    }

    bool eventFilter(QObject *watched, QEvent *event) override;

    QWidget *widgetForAction(QAction *action) const;
    QAction *actionForWidget(QWidget *widget) const;

    struct ActionWidget
    {
        ToolBarLayout::ToolBarWidgetType type;
        QWidget *widget;
    };
    ActionWidget createWidgetForAction(QAction *action);

    QRect actionRect(QAction *action) const;
    bool updateDropIndicatorGeometry(QPoint pos);
    bool canCustomize() const;

    ToolBarState state() const;
    void applyState(const ToolBarState &state, const std::vector<QAction *> &actions);

    ToolBar *const q;
    ToolBarOptions m_options;
    bool m_columnLayout = false;
    ToolBarLayout *m_layout = nullptr;
    QToolButton *m_closeButton = nullptr;
    std::unordered_map<QAction *, ActionWidget> m_actionWidgets;
    bool m_isDragging = false;
    QPoint m_dragPos;
    QPoint m_initialDragPos;
    Margin m_resizeMargin = Margin::None;
    QSize m_iconSize;
    bool m_explicitIconSize = false;
    Qt::ToolButtonStyle m_toolButtonStyle = Qt::ToolButtonIconOnly;
    bool m_explicitToolButtonStyle = false;
    Qt::Orientation m_dockedOrientation = Qt::Horizontal;
    ToolBarTrays m_allowedTrays = ToolBarTray::All;
    DropIndicator *m_dropIndicator = nullptr;
};

} // namespace KDToolBars

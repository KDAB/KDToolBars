/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "mainwindow.h"

#include <QFrame>

class QTimer;
class QImage;
class QWidgetAction;
class QToolButton;
class QStyleOption;

namespace KDToolBars {

enum class ToolBarOption {
    None = 0,
    IsCustom = 1, // created while customizing toolbars
};
Q_DECLARE_FLAGS(ToolBarOptions, ToolBarOption);
Q_DECLARE_OPERATORS_FOR_FLAGS(ToolBarOptions);

class ToolBar : public QFrame
{
    Q_OBJECT
public:
    explicit ToolBar(ToolBarOptions options = ToolBarOption::None, QWidget *parent = nullptr);
    explicit ToolBar(QWidget *parent = nullptr);
    ~ToolBar() override;

    ToolBarOptions options() const;

    bool columnLayout() const;
    void setColumnLayout(bool columnLayout);

    int columns() const;
    void setColumns(int columns);

    // Set spacing between toolbar buttons
    void setSpacing(int spacing);

    QAction *addSeparator();
    void clear();

    bool isFloating() const;

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    Qt::ToolButtonStyle toolButtonStyle() const;
    void setToolButtonStyle(Qt::ToolButtonStyle style);

    void setDockedOrientation(Qt::Orientation orientation);
    Qt::Orientation dockedOrientation() const;

    void setAllowedTrays(ToolBarTrays trays);
    ToolBarTrays allowedTrays() const;

    QToolButton *closeButton() const;

    virtual bool canBeReset() const;
    virtual void reset();

signals:
    void iconSizeChanged(const QSize &size);
    void toolButtonStyleChanged(const Qt::ToolButtonStyle style);
    void isFloatingChanged(bool isFloating);
    void actionsCustomized();

protected:
    void paintEvent(QPaintEvent *event) override;
    void actionEvent(QActionEvent *event) override;
    bool event(QEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void updateIconSize(const QSize &size);
    void updateToolButtonStyle(Qt::ToolButtonStyle style);

    void initStyleOption(QStyleOption *option);

    class Private;
    Private *d;

    friend class ToolBarTrayLayout;
    friend class MainWindow;
};

} // namespace KDToolBars

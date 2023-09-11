/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QMainWindow>

namespace KDToolBars {

class ToolBarContainerLayout;
class ToolBar;

enum class ToolBarTray {
    None = 0,
    Top = 1 << 0,
    Left = 1 << 1,
    Right = 1 << 2,
    Bottom = 1 << 3,
    All = Top | Left | Right | Bottom
};
Q_DECLARE_FLAGS(ToolBarTrays, ToolBarTray);
Q_DECLARE_OPERATORS_FOR_FLAGS(ToolBarTrays);

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~MainWindow() override;

    void setCentralWidget(QWidget *widget);

    void addToolBar(ToolBar *toolbar);
    void addToolBar(ToolBarTray tray, ToolBar *toolbar);
    void insertToolBar(ToolBar *before, ToolBar *toolbar);
    void addToolBarBreak(ToolBarTray tray = ToolBarTray::Top);
    void insertToolBarBreak(ToolBar *before);
    void removeToolBar(ToolBar *toolbar);

    QByteArray saveToolBarState() const;
    bool restoreToolBarState(const QByteArray &state);

    bool isCustomizingToolBars() const;
    void setCustomizingToolBars(bool customizing);

signals:
    void customizingToolBarsChanged(bool customizing);

private:
    friend class ToolBar;

    class Private;
    Private *d;
};

} // namespace KDToolBars

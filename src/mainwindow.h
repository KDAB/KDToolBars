/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QMainWindow>

namespace KDToolBars {

class CToolBarCustomizeDlg;
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

    ToolBarTray toolBarTray(const ToolBar *toolbar) const;

    int toolBarCount() const;
    ToolBar *toolBarAt(int index) const;

    QByteArray saveToolBarState() const;
    bool restoreToolBarState(const QByteArray &state);

    bool isCustomizingToolBars() const;
    void customizeToolBars();

signals:
    void customizingToolBarsChanged(bool customizing);
    void toolBarAboutToBeInserted(const ToolBar *toolbar, int index);
    void toolBarInserted(const ToolBar *toolbar);
    void toolBarAboutToBeRemoved(const ToolBar *toolbar, int index);
    void toolBarRemoved();

private:
    friend class ToolBar;
    friend class ToolBarCustomizationDialog;

    class Private;
    Private *d;
};

} // namespace KDToolBars

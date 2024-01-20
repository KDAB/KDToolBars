/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <memory>

#include <QDialog>
#include <QPointer>

class QTabWidget;
class QListView;
class QCheckBox;

namespace KDToolBars {

class MainWindow;
class ToolBarListModel;

class ToolBarCustomizationDialog : public QDialog
{
public:
    explicit ToolBarCustomizationDialog(MainWindow *manager, QWidget *parent = nullptr);
    ~ToolBarCustomizationDialog() override;

private:
    void setupUi();

    QPointer<MainWindow> m_mainWindow;
    ToolBarListModel *m_toolBarModel;
    QTabWidget *m_tabWidget;
    QListView *m_toolbarList;
    QPushButton *m_reset;
    QPushButton *m_resetAll;
    QPushButton *m_newToolBar;
    QPushButton *m_renameToolBar;
    QPushButton *m_deleteToolBar;
    QCheckBox *m_showToolTips;
};

}

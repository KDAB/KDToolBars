/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "mainwindow.h"

namespace KDToolBars {

class ToolBarContainerLayout;

class MainWindow::Private
{
public:
    explicit Private(MainWindow *mainWindow);

    void setCustomizingToolBars(bool customizing);

    MainWindow *const q;
    QWidget *m_container;
    ToolBarContainerLayout *m_layout;
    bool m_customizingToolBars = false;
};

} // namespace KDToolBars

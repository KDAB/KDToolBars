#pragma once

#include "mainwindow.h"

namespace KDToolBars {

class ToolBarContainerLayout;

class MainWindow::Private
{
public:
    explicit Private(MainWindow *mainWindow);

    MainWindow *const q;
    QWidget *m_container;
    ToolBarContainerLayout *m_layout;
};

} // namespace KDToolBars

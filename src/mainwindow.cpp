/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "mainwindow.h"

#include "mainwindow_p.h"
#include "toolbarcontainerlayout.h"
#include "toolbar.h"
#include "toolbartraylayout.h"

using namespace KDToolBars;

MainWindow::Private::Private(MainWindow *mainWindow)
    : q(mainWindow)
    , m_container(new QWidget(mainWindow))
    , m_layout(new ToolBarContainerLayout(m_container))
{
    m_layout->setContentsMargins(0, 0, 0, 0);

    connect(m_layout, &ToolBarContainerLayout::toolBarAboutToBeInserted, q, &MainWindow::toolBarAboutToBeInserted);
    connect(m_layout, &ToolBarContainerLayout::toolBarInserted, q, &MainWindow::toolBarInserted);
    connect(m_layout, &ToolBarContainerLayout::toolBarAboutToBeRemoved, q, &MainWindow::toolBarAboutToBeRemoved);
    connect(m_layout, &ToolBarContainerLayout::toolBarRemoved, q, &MainWindow::toolBarRemoved);
}

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , d(new Private(this))
{
    QMainWindow::setCentralWidget(d->m_container);
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::setCentralWidget(QWidget *widget)
{
    d->m_layout->setCentralWidget(widget);
}

void MainWindow::addToolBar(ToolBar *toolbar)
{
    addToolBar(ToolBarTray::Top, toolbar);
}

void MainWindow::addToolBar(ToolBarTray tray, ToolBar *toolbar)
{
    disconnect(this, &QMainWindow::iconSizeChanged, toolbar, &ToolBar::updateIconSize);
    disconnect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &ToolBar::updateToolButtonStyle);
    d->m_layout->removeToolBar(toolbar);

    toolbar->updateIconSize(iconSize());
    toolbar->updateToolButtonStyle(toolButtonStyle());
    connect(this, &QMainWindow::iconSizeChanged, toolbar, &ToolBar::updateIconSize);
    connect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &ToolBar::updateToolButtonStyle);

    d->m_layout->addToolBar(tray, toolbar);
}

void MainWindow::insertToolBar(ToolBar *before, ToolBar *toolbar)
{
    disconnect(this, &QMainWindow::iconSizeChanged, toolbar, &ToolBar::updateIconSize);
    disconnect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &ToolBar::updateToolButtonStyle);
    d->m_layout->removeToolBar(toolbar);

    toolbar->updateIconSize(iconSize());
    toolbar->updateToolButtonStyle(toolButtonStyle());
    connect(this, &QMainWindow::iconSizeChanged, toolbar, &ToolBar::updateIconSize);
    connect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &ToolBar::updateToolButtonStyle);

    d->m_layout->insertToolBar(before, toolbar);
}

void MainWindow::addToolBarBreak(ToolBarTray tray)
{
    d->m_layout->addToolBarBreak(tray);
}

void MainWindow::insertToolBarBreak(ToolBar *before)
{
    d->m_layout->insertToolBarBreak(before);
}

void MainWindow::removeToolBar(ToolBar *toolbar)
{
    disconnect(this, &QMainWindow::iconSizeChanged, toolbar, &ToolBar::updateIconSize);
    disconnect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &ToolBar::updateToolButtonStyle);
    d->m_layout->removeToolBar(toolbar);

    toolbar->hide();
}

ToolBarTray MainWindow::toolBarTray(const ToolBar *toolbar) const
{
    auto *tray = d->m_layout->toolBarTray(toolbar);
    return tray != nullptr ? tray->tray() : ToolBarTray::None;
}

int MainWindow::toolBarCount() const
{
    return d->m_layout->toolBarCount();
}

ToolBar *MainWindow::toolBarAt(int index) const
{
    return d->m_layout->toolBarAt(index);
}

QByteArray MainWindow::saveToolBarState() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    d->m_layout->saveState(stream);
    return data;
}

bool MainWindow::restoreToolBarState(const QByteArray &state)
{
    QByteArray data = state;
    QDataStream stream(&data, QIODevice::ReadOnly);
    return d->m_layout->restoreState(stream);
}

bool MainWindow::isCustomizingToolBars() const
{
    return d->m_customizingToolBars;
}

void MainWindow::setCustomizingToolBars(bool customizing)
{
    if (d->m_customizingToolBars == customizing)
        return;
    d->m_customizingToolBars = customizing;
    emit customizingToolBarsChanged(customizing);
}

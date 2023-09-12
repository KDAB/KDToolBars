/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <kdtoolbars/toolbar.h>
#include <kdtoolbars/mainwindow.h>

#include <QTest>
#include <QSignalSpy>

using namespace KDToolBars;

Q_DECLARE_METATYPE(const ToolBar *)

class TestMainWindow : public QObject
{
    Q_OBJECT
private slots:
    void testSimple();
};

void TestMainWindow::testSimple()
{
    MainWindow mw;

    QCOMPARE(mw.toolBarCount(), 0);

    qRegisterMetaType<const ToolBar *>();
    QSignalSpy insertedSpy(&mw, &MainWindow::toolBarInserted);
    QSignalSpy removedSpy(&mw, &MainWindow::toolBarRemoved);

    // add one toolbar
    ToolBar tb1;

    mw.addToolBar(&tb1);
    QCOMPARE(mw.toolBarCount(), 1);
    QCOMPARE(mw.toolBarAt(0), &tb1);
    QCOMPARE(insertedSpy.count(), 1);

    // should have been added to the top tray by default
    QCOMPARE(mw.toolBarTray(&tb1), ToolBarTray::Top);

    // add another toolbar
    ToolBar tb2;
    mw.addToolBar(ToolBarTray::Left, &tb2);
    QCOMPARE(mw.toolBarCount(), 2);
    QCOMPARE(mw.toolBarAt(1), &tb2);
    QCOMPARE(mw.toolBarTray(&tb2), ToolBarTray::Left);
    QCOMPARE(insertedSpy.count(), 2);

    // should still have 2 toolbars if we add the first one again
    mw.addToolBar(ToolBarTray::Bottom, &tb1);
    QCOMPARE(mw.toolBarCount(), 2);
    QCOMPARE(mw.toolBarTray(&tb1), ToolBarTray::Bottom);
    QCOMPARE(mw.toolBarTray(&tb2), ToolBarTray::Left);

    // toolbar was removed then added again
    QCOMPARE(insertedSpy.count(), 3);
    QCOMPARE(removedSpy.count(), 1);

    // remove one toolbar
    mw.removeToolBar(&tb1);
    QCOMPARE(mw.toolBarCount(), 1);
    QCOMPARE(mw.toolBarTray(&tb1), ToolBarTray::None);
    QCOMPARE(mw.toolBarAt(0), &tb2);
    QCOMPARE(mw.toolBarTray(&tb2), ToolBarTray::Left);
    QCOMPARE(removedSpy.count(), 2);
}

QTEST_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"

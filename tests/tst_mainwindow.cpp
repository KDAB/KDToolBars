/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <kdtoolbars/toolbar.h>
#include <kdtoolbars/mainwindow.h>

#include <QAction>
#include <QTest>
#include <QSignalSpy>

using namespace KDToolBars;

Q_DECLARE_METATYPE(const ToolBar *)

class TestMainWindow : public QObject
{
    Q_OBJECT
private slots:
    void testSimple();
    void testSaveState();
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

void TestMainWindow::testSaveState()
{
    MainWindow mw;

    ToolBar tb;
    tb.setObjectName("test-toolbar"); // needs unique object name for serialization

    // add toolbar to the bottom tray
    mw.addToolBar(ToolBarTray::Bottom, &tb);
    QCOMPARE(mw.toolBarCount(), 1);
    QCOMPARE(mw.toolBarTray(&tb), ToolBarTray::Bottom);

    // save state
    auto state = mw.saveToolBarState();

    // move toolbar to the right tray
    mw.addToolBar(ToolBarTray::Right, &tb);
    QCOMPARE(mw.toolBarCount(), 1);
    QCOMPARE(mw.toolBarTray(&tb), ToolBarTray::Right);

    // restore state, toolbar should move back to the bottom tray
    mw.restoreToolBarState(state);
    QCOMPARE(mw.toolBarCount(), 1);
    QCOMPARE(mw.toolBarTray(&tb), ToolBarTray::Bottom);

    // add two actions to the toolbar
    QAction a1;
    a1.setObjectName("test-action-1"); // needs unique object name for serialization

    QAction a2;
    a2.setObjectName("test-action-2");

    tb.addAction(&a1);
    tb.addAction(&a2);
    QCOMPARE(tb.actions(), QList<QAction *>({ &a1, &a2 }));

    // save state
    state = mw.saveToolBarState();

    // swap the action positions
    tb.insertAction(&a1, &a2);
    QCOMPARE(tb.actions(), QList<QAction *>({ &a2, &a1 }));

    // restore state, actions should be back to the initial position
    mw.restoreToolBarState(state);
    QCOMPARE(tb.actions(), QList<QAction *>({ &a1, &a2 }));

    // remove one of the actions
    tb.removeAction(&a1);
    QCOMPARE(tb.actions(), QList<QAction *>({ &a2 }));

    // restore state, removed action should be back
    mw.restoreToolBarState(state);
    QCOMPARE(tb.actions(), QList<QAction *>({ &a1, &a2 }));
}

QTEST_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"

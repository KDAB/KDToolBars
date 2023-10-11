/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <kdtoolbars/mainwindow.h>
#include <kdtoolbars/toolbar.h>

#include <kddockwidgets/qtwidgets/DockWidget.h>
#include <kddockwidgets/qtwidgets/MainWindow.h>

#include <QApplication>
#include <QAction>
#include <QLabel>

KDToolBars::ToolBar *makeToolBar(const char *name, std::initializer_list<const char *> icons, QWidget *parent = nullptr)
{
    auto *toolbar = new KDToolBars::ToolBar(KDToolBars::ToolBarOption::None, parent);
    toolbar->setWindowTitle(name);
    for (const auto *iconName : icons) {
        if (iconName) {
            auto *action = new QAction(QIcon(QStringLiteral(":/%1").arg(iconName)), iconName, parent);
            toolbar->addAction(action);
        } else {
            toolbar->addSeparator();
        }
    }
    return toolbar;
}

class TestWindow : public KDToolBars::MainWindow
{
public:
    explicit TestWindow(QWidget *parent = nullptr)
        : KDToolBars::MainWindow(parent)
        , m_dockWidgetWindow(new KDDockWidgets::QtWidgets::MainWindow(
              "KDDW_MainWindow", KDDockWidgets::MainWindowOption_HasCentralWidget, this))
    {
        setWindowTitle(tr("KDToolBars example"));

        setCentralWidget(m_dockWidgetWindow);

        createCentralWidget();
        createToolBars();
        createDockWidgets();
    }

private:
    void createToolBars()
    {
        auto *tb1 = makeToolBar("toolbar 1", { "coffee", "globe", nullptr, "sun", "moon", nullptr, "cloud", "cloud-rain" });
        auto *tb2 = makeToolBar("toolbar 2", { "feather", "upload", "download" });
        auto *tb3 = makeToolBar("toolbar 3", { "file", "folder", "star", nullptr, "arrow-left", "arrow-up", "arrow-down", "arrow-right" });
        auto *tb4 = makeToolBar("toolbar 4", { "music", "image", "video", "file-text" });

        addToolBar(tb1);
        addToolBar(tb2);
        addToolBar(tb3);
        insertToolBarBreak(tb3);
        addToolBar(tb4);
    }

    void createDockWidgets()
    {
        auto *dock = new KDDockWidgets::QtWidgets::DockWidget(QStringLiteral("KDDW_Dock"));
        dock->setWidget(new QLabel(tr("Dock Widget"), this));
        m_dockWidgetWindow->addDockWidget(dock, KDDockWidgets::Location_OnRight);
    }

    void createCentralWidget()
    {
        auto *centralWidget = new QLabel(tr("Central Widget"), this);
        m_dockWidgetWindow->setPersistentCentralWidget(centralWidget);
    }

    KDDockWidgets::QtWidgets::MainWindow *m_dockWidgetWindow;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    TestWindow w;
    w.resize(1500, 400);
    w.show();

    return app.exec();
}

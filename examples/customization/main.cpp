/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <kdtoolbars/mainwindow.h>
#include <kdtoolbars/toolbar.h>

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>

using namespace KDToolBars;

ToolBar *makeToolBar(const char *name, std::initializer_list<const char *> icons, bool customizable, QWidget *parent = nullptr)
{
    static int toolbarId = 0;
    ToolBarOptions options = ToolBarOption::None;
    if (customizable)
        options |= ToolBarOption::IsCustomizable;
    auto *toolbar = new ToolBar(options, parent);
    toolbar->setWindowTitle(name);
    toolbar->setObjectName(QString::number(toolbarId));
    ++toolbarId;
    for (const auto *iconName : icons) {
        if (iconName) {
            static int actionId = 0;
            auto *action = new QAction(QIcon(QStringLiteral(":/%1").arg(iconName)), iconName, parent);
            action->setObjectName(QString::number(actionId));
            ++actionId;
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
    {
        setWindowTitle(tr("KDToolBars example"));

        createCentralWidget();
        createToolBars();
    }

private:
    void createToolBars()
    {
        auto *tb1 = makeToolBar("toolbar 1", { "coffee", "globe", nullptr, "sun", "moon", nullptr, "cloud", "cloud-rain" }, true);
        auto *tb2 = makeToolBar("toolbar 2", { "feather", "upload", "download" }, true);
        auto *tb3 = makeToolBar("toolbar 3", { "file", "folder", "star", nullptr, "arrow-left", "arrow-up", "arrow-down", "arrow-right" }, true);
        auto *tb4 = makeToolBar("toolbar 4", { "music", "image", "video", "file-text" }, false);

        addToolBar(tb1);
        insertToolBar(tb1, tb2);
        addToolBarBreak();
        addToolBar(tb3);
        addToolBar(tb4);
        insertToolBarBreak(tb4);
    }

    void createCentralWidget()
    {
        auto *centralWidget = new QWidget(this);

        auto p = centralWidget->palette();
        p.setColor(QPalette::Window, Qt::cyan);
        centralWidget->setPalette(p);
        centralWidget->setAutoFillBackground(true);

        auto *layout = new QVBoxLayout(centralWidget);

        auto *iconSize = new QCheckBox(tr("Large icons"), centralWidget);
        layout->addWidget(iconSize);
        connect(iconSize, &QCheckBox::toggled, this, [this](bool checked) {
            const QSize size = checked ? QSize(64, 64) : QSize(32, 32);
            setIconSize(size);
        });

        auto *buttonStyle = new QCheckBox(tr("Button style"), centralWidget);
        layout->addWidget(buttonStyle);
        connect(buttonStyle, &QCheckBox::toggled, centralWidget, [this](bool checked) {
            const Qt::ToolButtonStyle style = checked ? Qt::ToolButtonTextOnly : Qt::ToolButtonIconOnly;
            setToolButtonStyle(style);
        });

        // create buttons to save/restore toolbar state
        {
            auto *buttonLayout = new QHBoxLayout;
            layout->addLayout(buttonLayout);

            auto *saveState = new QPushButton(tr("Save state"), this);
            buttonLayout->addWidget(saveState);
            connect(saveState, &QAbstractButton::clicked, this, [this] {
                m_lastState = saveToolBarState();
            });

            auto *restoreState = new QPushButton(tr("Restore state"), this);
            buttonLayout->addWidget(restoreState);
            connect(restoreState, &QAbstractButton::clicked, this, [this] {
                if (!m_lastState.isNull())
                    restoreToolBarState(m_lastState);
            });

            auto *customize = new QPushButton(tr("Customize"), this);
            buttonLayout->addWidget(customize);
            connect(customize, &QAbstractButton::clicked, this, &MainWindow::customizeToolBars);

            buttonLayout->addStretch();
        }

        layout->addStretch();

        setCentralWidget(centralWidget);
    }

    QByteArray m_lastState;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TestWindow w;
    w.resize(1500, 400);
    w.show();

    return app.exec();
}

#include "toolbar.h"
#include "toolbartraylayout.h"
#include "toolbarcontainerlayout.h"
#include "mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QCheckBox>

using namespace KDToolBars;

ToolBar *makeToolBar(const char *name, std::initializer_list<const char *> icons, QWidget *parent = nullptr)
{
    auto *toolbar = new ToolBar(name, parent);
    for (const auto *iconName : icons) {
        if (iconName) {
            auto *action = new QAction(QIcon(QStringLiteral(":/%1").arg(iconName)), iconName, parent);
            toolbar->addAction(action);
        } else {
            toolbar->addSeparator();
        }
    }
    toolbar->setIconSize(QSize(48, 48));
    return toolbar;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto *tb1 = makeToolBar("toolbar 1", { "gvim.png", "firefox.png", "gerbera.png", nullptr, "orca.png", "gimp.png", "firefox.png" });
    auto *tb2 = makeToolBar("toolbar 2", { "orca.png", "gimp.png" });
    auto *tb3 = makeToolBar("toolbar 3", { "firefox.png", "firefox.png", "firefox.png" });
    auto *tb4 = makeToolBar("toolbar 4", { "gvim.png", "gvim.png", "gvim.png" });

    MainWindow mw;
    mw.setWindowTitle(QObject::tr("KDToolBars example"));

    mw.addToolBar(tb1);
    mw.insertToolBar(tb1, tb2);
    mw.addToolBarBreak();
    mw.addToolBar(tb3);
    mw.addToolBar(tb4);
    mw.insertToolBarBreak(tb4);

    auto *centralWidget = new QWidget(&mw);

    auto p = centralWidget->palette();
    p.setColor(QPalette::Window, Qt::cyan);
    centralWidget->setPalette(p);
    centralWidget->setAutoFillBackground(true);

    auto *layout = new QVBoxLayout(centralWidget);

    auto *iconSize = new QCheckBox(QObject::tr("Large icons"), centralWidget);
    layout->addWidget(iconSize);

    QObject::connect(iconSize, &QCheckBox::toggled, &mw, [&mw](bool checked) {
        const QSize size = checked ? QSize(64, 64) : QSize(32, 32);
        mw.setIconSize(size);
    });

    auto *button_style = new QCheckBox(QObject::tr("Button style"), centralWidget);
    layout->addWidget(button_style);

    QObject::connect(button_style, &QCheckBox::toggled, centralWidget, [&mw](bool checked) {
        const Qt::ToolButtonStyle style = checked ? Qt::ToolButtonTextOnly : Qt::ToolButtonIconOnly;
        mw.setToolButtonStyle(style);
    });

    layout->addStretch();

    mw.setCentralWidget(centralWidget);

    mw.resize(1500, 400);
    mw.show();

    return app.exec();
}

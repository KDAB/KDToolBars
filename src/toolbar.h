#pragma once

#include "mainwindow.h"
#include "toolbarlayout.h"

#include <QFrame>

class QTimer;
class QImage;
class QWidgetAction;
class QToolButton;
class QStyleOption;

namespace KDToolBars {

class ToolBar : public QFrame
{
    Q_OBJECT
public:
    explicit ToolBar(const QString &title, QWidget *parent = nullptr);
    explicit ToolBar(QWidget *parent = nullptr);
    ~ToolBar() override;

    bool columnLayout() const;
    void setColumnLayout(bool columnLayout);

    int columns() const;
    void setColumns(int columns);

    // Set spacing between toolbar buttons
    void setSpacing(int spacing);

    QWidget *widgetForAction(const QAction *action) const;

    QAction *addSeparator();
    void clear();

    bool isFloating() const;

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    Qt::ToolButtonStyle toolButtonStyle() const;
    void setToolButtonStyle(Qt::ToolButtonStyle style);

    void setDockedOrientation(Qt::Orientation orientation);
    Qt::Orientation dockedOrientation() const;

    void setAllowedTrays(ToolBarTrays trays);
    ToolBarTrays allowedTrays() const;

    QToolButton *closeButton() const;

signals:
    void iconSizeChanged(const QSize &size);
    void toolButtonStyleChanged(const Qt::ToolButtonStyle style);
    void topLevelChanged(bool is_top_level);

protected:
    void paintEvent(QPaintEvent *event) override;
    void actionEvent(QActionEvent *event) override;
    bool event(QEvent *event) override;

    void updateIconSize(const QSize &size);
    void updateToolButtonStyle(Qt::ToolButtonStyle style);

    void initStyleOption(QStyleOption *option);
    struct ActionWidget {
        ToolBarLayout::ToolBarWidgetType type;
        QWidget *widget;
    };
    ActionWidget createWidgetForAction(QAction *action);

    class Private;
    Private *d;

    friend class ToolBarTrayLayout;
    friend class MainWindow;
};

} // namespace KDToolBars

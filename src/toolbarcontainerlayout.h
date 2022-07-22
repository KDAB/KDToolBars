#pragma once

#include "mainwindow.h"

#include <QLayout>

#include <array>
#include <optional>
#include <unordered_map>

namespace KDToolBars {

class ToolBar;
class ToolBarTrayLayout;

class ToolBarContainerLayout : public QLayout
{
    Q_OBJECT
public:
    explicit ToolBarContainerLayout(QWidget *parent);
    ~ToolBarContainerLayout() override;

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    Qt::Orientations expandingDirections() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void invalidate() override;

    void setCentralWidget(QWidget *widget);

    void addToolBar(ToolBarTray tray, ToolBar *toolbar);
    void insertToolBar(ToolBar *before, ToolBar *toolbar);
    void addToolBarBreak(ToolBarTray tray);
    void insertToolBarBreak(ToolBar *before);
    void removeToolBar(ToolBar *toolbar);

    void moveToolBar(ToolBar *toolbar, const QPoint &pos);
    void adjustToolBarRow(const ToolBar *toolbar);
    void hoverToolBar(ToolBar *toolbar);

    ToolBarTrayLayout *toolBarTray(const ToolBar *toolbar) const;

    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream);

private:
    enum TrayPosition { TopTray,
                        LeftTray,
                        RightTray,
                        BottomTray,
                        TrayCount };
    int trayIndex(ToolBarTray tray) const;
    std::array<ToolBarTrayLayout *, TrayCount> m_trays;
    std::unordered_map<const ToolBar *, ToolBarTrayLayout *> m_toolbarTray;
    QLayoutItem *m_centralWidget = nullptr;

    template<typename TraySizeGetterT, typename WidgetSizeGetterT>
    QSize layoutSize(TraySizeGetterT traySize, WidgetSizeGetterT widgetSize) const;

    ToolBarTrayLayout *topTray() const
    {
        return m_trays[TopTray];
    }
    ToolBarTrayLayout *leftTray() const
    {
        return m_trays[LeftTray];
    }
    ToolBarTrayLayout *rightTray() const
    {
        return m_trays[RightTray];
    }
    ToolBarTrayLayout *bottomTray() const
    {
        return m_trays[BottomTray];
    }
};

} // namespace KDToolBars

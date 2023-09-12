/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

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

    int toolBarCount() const;
    ToolBar *toolBarAt(int index) const;

    ToolBarTrayLayout *toolBarTray(const ToolBar *toolbar) const;

    bool eventFilter(QObject *watched, QEvent *event) override;

    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream);

signals:
    void toolBarAboutToBeInserted(const ToolBar *toolbar, int index);
    void toolBarInserted(const ToolBar *toolbar);
    void toolBarAboutToBeRemoved(const ToolBar *toolbar, int index);
    void toolBarRemoved();

private:
    friend class ToolBarTrayLayout;
    enum TrayPosition {
        TopTray,
        LeftTray,
        RightTray,
        BottomTray,
        TrayCount
    };
    void insertToolBar(ToolBarTrayLayout *trayLayout, ToolBar *before, ToolBar *toolbar);
    int trayIndex(ToolBarTray tray) const;
    std::array<ToolBarTrayLayout *, TrayCount> m_trays;
    std::vector<ToolBar *> m_toolbars;
    std::unordered_map<const ToolBar *, ToolBarTrayLayout *> m_toolbarTray;
    QLayoutItem *m_centralWidget = nullptr;
    std::unique_ptr<QWidget> m_actionContainer;

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

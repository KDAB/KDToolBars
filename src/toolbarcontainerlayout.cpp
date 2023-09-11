/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbarcontainerlayout.h"

#include "toolbar.h"
#include "toolbartraylayout.h"

#include <QtWidgets/private/qlayout_p.h>

using namespace KDToolBars;

namespace {
constexpr int kLayoutVersionMarker = 1;
}

ToolBarContainerLayout::ToolBarContainerLayout(QWidget *parent)
    : QLayout(parent)
{
    m_trays[TopTray] = new ToolBarTrayLayout(ToolBarTray::Top, Qt::Horizontal, this);
    m_trays[LeftTray] = new ToolBarTrayLayout(ToolBarTray::Left, Qt::Vertical, this);
    m_trays[RightTray] = new ToolBarTrayLayout(ToolBarTray::Right, Qt::Vertical, this);
    m_trays[BottomTray] = new ToolBarTrayLayout(ToolBarTray::Bottom, Qt::Horizontal, this);
}

ToolBarContainerLayout::~ToolBarContainerLayout()
{
    qDeleteAll(m_trays);
}

void ToolBarContainerLayout::addItem(QLayoutItem *item)
{
    qWarning("ToolBarTrayContainerLayout::addItem: Please use addToolBar instead");
}

int ToolBarContainerLayout::count() const
{
    auto count = std::accumulate(m_trays.begin(), m_trays.end(), 0, [](int count, const auto &tray) {
        return count + tray->count();
    });
    if (m_centralWidget != nullptr)
        ++count;
    return count;
}

QLayoutItem *ToolBarContainerLayout::itemAt(int index) const
{
    if (index >= 0) {
        for (const auto &tray : m_trays) {
            const auto trayCount = tray->count();
            if (index < trayCount)
                return tray->itemAt(index);
            index -= trayCount;
        }
    }
    return index == 0 ? m_centralWidget : nullptr;
}

QLayoutItem *ToolBarContainerLayout::takeAt(int index)
{
    if (index >= 0) {
        for (const auto &tray : m_trays) {
            const auto trayCount = tray->count();
            if (index < trayCount)
                return tray->takeAt(index);
            index -= trayCount;
        }
    }
    if (index == 0) {
        auto *item = m_centralWidget;
        m_centralWidget = nullptr;
        return item;
    }
    return nullptr;
}

Qt::Orientations ToolBarContainerLayout::expandingDirections() const
{
    return {};
}

void ToolBarContainerLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);

    auto contentsRect = this->contentsRect();

    const auto topHeight = topTray()->sizeHint().height();
    const auto bottomHeight = bottomTray()->sizeHint().height();

    const auto leftWidth = leftTray()->sizeHint().width();
    const auto rightWidth = rightTray()->sizeHint().width();

    const auto centerWidth = contentsRect.width() - (leftWidth + rightWidth);
    const auto centerHeight = contentsRect.height() - (topHeight + bottomHeight);

    // top tray
    auto topRect = contentsRect;
    topRect.setHeight(topHeight);
    topTray()->setGeometry(topRect);

    // left tray
    auto leftRect = contentsRect;
    leftRect.setTop(contentsRect.top() + topHeight);
    leftRect.setWidth(leftWidth);
    leftRect.setHeight(centerHeight);
    leftTray()->setGeometry(leftRect);

    // right tray
    auto rightRect = contentsRect;
    rightRect.setLeft(contentsRect.left() + contentsRect.width() - rightWidth);
    rightRect.setTop(contentsRect.top() + topHeight);
    rightRect.setHeight(centerHeight);
    rightTray()->setGeometry(rightRect);

    // bottom tray
    auto bottomRect = contentsRect;
    bottomRect.setTop(contentsRect.top() + topHeight + centerHeight);
    bottomTray()->setGeometry(bottomRect);

    if (m_centralWidget != nullptr) {
        const auto pos = contentsRect.topLeft() + QPoint(leftWidth, topHeight);
        const auto size = QSize(centerWidth, centerHeight);
        m_centralWidget->setGeometry(QRect(pos, size));
    }
}

QSize ToolBarContainerLayout::sizeHint() const
{
    return layoutSize(&ToolBarTrayLayout::sizeHint, &QLayoutItem::sizeHint);
}

QSize ToolBarContainerLayout::minimumSize() const
{
    return layoutSize(&ToolBarTrayLayout::minimumSize, &QLayoutItem::minimumSize);
}

template<typename TraySizeGetterT, typename WidgetSizeGetterT>
QSize ToolBarContainerLayout::layoutSize(
    TraySizeGetterT traySize, WidgetSizeGetterT widgetSize) const
{
    const auto topSize = (topTray()->*traySize)();
    const auto leftSize = (leftTray()->*traySize)();
    const auto rightSize = (rightTray()->*traySize)();
    const auto bottomSize = (bottomTray()->*traySize)();
    const auto centralWidgetSize =
        m_centralWidget != nullptr ? (m_centralWidget->*widgetSize)() : QSize(0, 0);
    const auto width = std::max(
        { topSize.width(), leftSize.width() + centralWidgetSize.width() + rightSize.width(),
          bottomSize.width() });
    const auto height =
        topSize.height() + std::max({ leftSize.height(), centralWidgetSize.height(), rightSize.height() }) + bottomSize.height();
    return QSize(width, height).grownBy(contentsMargins());
}

void ToolBarContainerLayout::invalidate()
{
    for (auto &tray : m_trays)
        tray->invalidate();
    QLayout::invalidate();
}

void ToolBarContainerLayout::setCentralWidget(QWidget *widget)
{
    if (m_centralWidget != nullptr) {
        auto *oldWidget = m_centralWidget->widget();
        if (oldWidget != nullptr) {
            oldWidget->hide();
            removeWidget(oldWidget);
        }
    }
    if (widget != nullptr) {
        addChildWidget(widget);
        m_centralWidget = QLayoutPrivate::createWidgetItem(this, widget);
    } else {
        delete m_centralWidget;
        m_centralWidget = nullptr;
    }
    invalidate();
}

void ToolBarContainerLayout::addToolBar(ToolBarTray tray, ToolBar *toolbar)
{
    const auto trayIndex = this->trayIndex(tray);
    if (trayIndex == -1)
        return;

    emit toolBarAboutToBeInserted(toolbar, m_toolbars.size());

    addChildWidget(toolbar);

    m_toolbars.push_back(toolbar);
    auto *trayLayout = m_trays[trayIndex];
    m_toolbarTray[toolbar] = trayLayout;
    trayLayout->insertToolBar(nullptr, toolbar);

    invalidate();

    emit toolBarInserted(toolbar);
}

void ToolBarContainerLayout::insertToolBar(ToolBar *before, ToolBar *toolbar)
{
    emit toolBarAboutToBeInserted(toolbar, m_toolbars.size());

    addChildWidget(toolbar);

    m_toolbars.push_back(toolbar);
    auto *trayLayout = toolBarTray(before);
    Q_ASSERT(trayLayout);
    m_toolbarTray[toolbar] = trayLayout;
    trayLayout->insertToolBar(before, toolbar);

    invalidate();

    emit toolBarInserted(toolbar);
}

void ToolBarContainerLayout::addToolBarBreak(ToolBarTray tray)
{
    const auto trayIndex = this->trayIndex(tray);
    if (trayIndex == -1)
        return;

    m_trays[trayIndex]->insertToolBarBreak(nullptr);

    invalidate();
}

void ToolBarContainerLayout::insertToolBarBreak(ToolBar *before)
{
    auto *trayLayout = toolBarTray(before);
    Q_ASSERT(trayLayout);
    trayLayout->insertToolBarBreak(before);

    invalidate();
}

void ToolBarContainerLayout::moveToolBar(ToolBar *toolbar, const QPoint &pos)
{
    auto *tray = toolBarTray(toolbar);
    if (tray == nullptr)
        return;
    tray->moveToolBar(toolbar, pos);
    invalidate();
}

void ToolBarContainerLayout::adjustToolBarRow(const ToolBar *toolbar)
{
    auto *tray = toolBarTray(toolbar);
    if (tray == nullptr)
        return;
    tray->adjustToolBarRow(toolbar);
    invalidate();
}

void ToolBarContainerLayout::hoverToolBar(ToolBar *toolbar)
{
    bool docked = false;
    for (auto *tray : m_trays) {
        if (!toolbar->allowedTrays().testFlag(tray->tray()))
            continue;
        if (tray->hoverToolBar(toolbar)) {
            m_toolbarTray[toolbar] = tray; // toolbar may have docked on another tray
            docked = true;
            break;
        }
    }
    if (docked)
        invalidate();
}

ToolBarTrayLayout *ToolBarContainerLayout::toolBarTray(const ToolBar *toolbar) const
{
    auto it = m_toolbarTray.find(toolbar);
    if (it == m_toolbarTray.end())
        return nullptr;
    return it->second;
}

void ToolBarContainerLayout::removeToolBar(ToolBar *toolbar)
{
    auto it = std::find(m_toolbars.begin(), m_toolbars.end(), toolbar);
    if (it == m_toolbars.end())
        return;

    const auto index = static_cast<int>(std::distance(m_toolbars.begin(), it));
    emit toolBarAboutToBeRemoved(toolbar, index);

    removeWidget(toolbar);

    m_toolbars.erase(it);
    m_toolbarTray.erase(toolbar);

    emit toolBarRemoved();
}

int ToolBarContainerLayout::trayIndex(ToolBarTray tray) const
{
    switch (tray) {
    case ToolBarTray::Top:
        return TopTray;
    case ToolBarTray::Left:
        return LeftTray;
    case ToolBarTray::Right:
        return RightTray;
    case ToolBarTray::Bottom:
        return BottomTray;
    default:
        break;
    }
    return -1;
}

void ToolBarContainerLayout::saveState(QDataStream &stream) const
{
    stream << kLayoutVersionMarker;
    const auto count = std::accumulate(
        m_trays.begin(), m_trays.end(), 0,
        [](int count, const auto &tray) { return count + tray->count(); });
    stream << count;
    for (const auto *tray : m_trays)
        tray->state().save(stream);
}

bool ToolBarContainerLayout::restoreState(QDataStream &stream)
{
    int version, toolbarCount;
    stream >> version;
    stream >> toolbarCount;
    if (stream.status() != QDataStream::Ok || version != kLayoutVersionMarker)
        return false;

    // load tray states
    std::array<ToolBarTrayLayoutState, TrayCount> trayStates;
    for (auto &state : trayStates) {
        if (!state.load(stream))
            return false;
    }

    // apply tray states
    std::vector<ToolBar *> toolbars;
    toolbars.reserve(m_toolbarTray.size());
    std::transform(
        m_toolbarTray.begin(), m_toolbarTray.end(), std::back_inserter(toolbars),
        [](const auto &item) { return const_cast<ToolBar *>(item.first); });

    for (size_t i = 0; i < TrayCount; ++i)
        m_trays[i]->applyState(trayStates[i], toolbars);

    // clear toolbar tray map
    decltype(m_toolbarTray) oldToolbarTray;
    std::swap(oldToolbarTray, m_toolbarTray);

    // add back any toolbar that wasn't restored
    for (auto *toolbar : toolbars) {
        const auto found = std::any_of(
            m_trays.begin(), m_trays.end(), [toolbar](auto *tray) { return tray->hasToolBar(toolbar); });
        if (!found) {
            auto *tray = oldToolbarTray[toolbar];
            tray->insertToolBar(nullptr, toolbar);
        }
    }

    // reinitialize the toolbar -> tray map
    for (auto *toolbar : toolbars) {
        auto it = std::find_if(
            m_trays.begin(), m_trays.end(), [toolbar](auto *tray) { return tray->hasToolBar(toolbar); });
        Q_ASSERT(it != m_trays.end());
        m_toolbarTray[toolbar] = *it;
    }

    invalidate();

    return true;
}

int ToolBarContainerLayout::toolBarCount() const
{
    return static_cast<int>(m_toolbars.size());
}

ToolBar *ToolBarContainerLayout::toolBarAt(int index) const
{
    if (index < 0 || index >= m_toolbars.size())
        return nullptr;
    return m_toolbars[index];
}

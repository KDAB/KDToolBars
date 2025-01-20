/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbartraylayout.h"

#include "toolbar.h"
#include "toolbar_p.h"
#include "toolbarcontainerlayout.h"

#include <QtWidgets/private/qlayout_p.h>

using namespace KDToolBars;

// clazy:excludeall=detaching-member

ToolBarTrayLayout::ToolBarTrayLayout(
    ToolBarTray tray, Qt::Orientation orientation, ToolBarContainerLayout *parent)
    : m_parent(parent)
    , m_tray(tray)
    , m_orientation(orientation)
{
}

ToolBarTrayLayout::~ToolBarTrayLayout() = default;

int ToolBarTrayLayout::count() const
{
    return std::accumulate(m_rows.begin(), m_rows.end(), 0, [](int count, const auto &row) {
        return count + row.items.count();
    });
}

QLayoutItem *ToolBarTrayLayout::itemAt(int index) const
{
    if (index >= 0) {
        for (const auto &row : m_rows) {
            auto &items = row.items;
            const auto rowCount = items.count();
            if (index < rowCount)
                return items[index].widgetItem;
            index -= rowCount;
        }
    }
    return nullptr;
}

QLayoutItem *ToolBarTrayLayout::takeAt(int index)
{
    if (index >= 0) {
        for (auto it = m_rows.begin(); it != m_rows.end(); ++it) {
            auto &items = it->items;
            const auto rowCount = items.count();
            if (index < rowCount) {
                auto *item = items.takeAt(index).widgetItem;
                if (items.empty())
                    m_rows.erase(it);
                return item;
            }
            index -= rowCount;
        }
    }
    return nullptr;
}

QSize ToolBarTrayLayout::sizeHint() const
{
    if (m_dirty)
        updateRowSizes();
    auto size = QSize(0, 0);
    for (const auto &row : m_rows) {
        rpick(size) = std::max(pick(size), pick(row.sizeHint));
        rperp(size) += perp(row.sizeHint);
    }
    return size.grownBy(m_contentsMargins);
}

QSize ToolBarTrayLayout::minimumSize() const
{
    if (m_dirty)
        updateRowSizes();
    auto size = QSize(0, 0);
    for (const auto &row : m_rows) {
        rpick(size) = std::max(pick(size), pick(row.minimumSize));
        rperp(size) += perp(row.minimumSize);
    }
    return size.grownBy(m_contentsMargins);
}

void ToolBarTrayLayout::invalidate()
{
    m_dirty = true;
}

void ToolBarTrayLayout::setGeometry(QRect rect)
{
    m_contentsRect = rect.marginsRemoved(m_contentsMargins);
    doLayout();
}

void ToolBarTrayLayout::doLayout()
{
    if (m_dirty)
        updateRowSizes();

    const auto topLeft = m_contentsRect.topLeft();

    for (const auto &row : std::as_const(m_rows)) {
        QVector<Item> items;

        // find a toolbar that's being dragged in this row
        auto *movingToolbar = [&row]() {
            const auto &items = row.items;
            auto it = std::find_if(items.begin(), items.end(), [](const auto &item) {
                auto *tb = qobject_cast<ToolBar *>(item.widgetItem->widget());
                Q_ASSERT(tb != nullptr);
                return tb->d->isMoving();
            });
            return it != items.end() ? static_cast<ToolBar *>(it->widgetItem->widget()) : nullptr;
        }();
        if (movingToolbar) {
            // if one of the toolbars in this row is being dragged, adjust the position of the other
            // toolbars accordingly
            items = adjustRow(row, movingToolbar);
        } else {
            items = row.items;
            std::stable_sort(items.begin(), items.end(), [](const auto &lhs, const auto &rhs) {
                return lhs.pos < rhs.pos;
            });

            const auto availableSize = pick(m_contentsRect.size());
            auto used = adjustItemSizes(items, availableSize);

            // adjust positions
            auto start = 0;
            for (auto &item : items) {
                used -= item.size;
                item.pos = std::max(std::min(item.pos, (availableSize - used) - item.size), start);
                start = item.pos + item.size;
            }
        }

        for (const auto &item : std::as_const(items)) {
            auto *widgetItem = item.widgetItem;
            auto itemSize = widgetItem->sizeHint();
            rpick(itemSize) = item.size;
            QPoint pos;
            rpick(pos) = item.pos;
            rperp(pos) = row.pos;
            widgetItem->setGeometry(QRect(topLeft + pos, itemSize));
        }
    }
}

void ToolBarTrayLayout::moveToolBar(ToolBar *toolbar, QPoint pos)
{
    if (toolbar->isFloating())
        return;
    auto itemPath = findItem(toolbar);
    if (!itemPath)
        return;

    if (m_dirty)
        updateRowSizes();

    const auto topLeft = m_contentsRect.topLeft();
    const auto availableSize = m_contentsRect.size();

    const auto *widgetItem = layoutItem(*itemPath);
    const auto itemSize = widgetItem->sizeHint();
    const auto itemMinimumSize = widgetItem->minimumSize();

    int newPos = pick(pos) - pick(topLeft);

    const auto halfSize = perp(itemSize) / 2;
    const auto center = (perp(pos) - perp(topLeft)) + halfSize;

    bool topRowCreated = false;
    bool bottomRowCreated = false;
    int newRow = -1;

    // HACK: figure out position of the cursor based on the desired toolbar position
    // TODO: find a cleaner way to do this
    const auto cursorPos = (pos - toolbar->pos()) + toolbar->mapToGlobal(toolbar->d->m_dragPos);

    bool unplug = [this, &cursorPos, &topLeft, &availableSize] {
        const auto localCursorPos = m_parent->parentWidget()->mapFromGlobal(cursorPos);
        constexpr auto kDockMargin = 8;
        return pick(localCursorPos) < pick(topLeft) - kDockMargin || pick(localCursorPos) > pick(topLeft) + pick(availableSize) + kDockMargin;
    }();

    if (!unplug) {
        // moving up, should we create another top row?
        if (center < 0) {
            const auto createRow = [this, &itemPath] {
                // find top-most non-empty row
                Q_ASSERT(!m_rows.empty());
                auto it = std::find_if(
                    m_rows.begin(), m_rows.end(), [](const Row &row) { return row.dockedCount() > 0; });
                Q_ASSERT(it != m_rows.end());
                const auto index = static_cast<int>(std::distance(m_rows.begin(), it));
                if (index != itemPath->row)
                    return true;
                return it->dockedCount() > 1;
            }();
            if (createRow) {
                m_rows.insert(0, {});
                itemPath->row++;
                newRow = 0;
                topRowCreated = true;
            } else {
                // unplug if fully above the top-most row
                unplug = perp(pos) < perp(topLeft) - perp(itemSize);
            }
        }
    }

    // moving down, should we create another bottom row?
    if (newRow == -1 && !unplug) {
        int totalSize = std::accumulate(
            m_rows.begin(), m_rows.end(), 0,
            [this](int size, const Row &row) { return perp(row.sizeHint) + size; });
        if (center > totalSize) {
            const auto createRow = [this, &itemPath] {
                // find bottom-most non-empty row
                Q_ASSERT(!m_rows.empty());
                auto it = std::find_if(
                    m_rows.rbegin(), m_rows.rend(), [](const Row &row) { return row.dockedCount() > 0; });
                Q_ASSERT(it != m_rows.rend());
                const auto index = static_cast<int>(std::distance(m_rows.begin(), std::next(it).base()));
                if (index != itemPath->row)
                    return true;
                return it->dockedCount() > 1;
            }();
            if (createRow) {
                m_rows.push_back({});
                newRow = m_rows.count() - 1;
                bottomRowCreated = true;
            } else {
                // unplug if fully below the bottom-most row
                unplug = perp(pos) > perp(topLeft) + totalSize;
            }
        }
    }

    // should we move the toolbar to another row?
    if (newRow == -1 && !unplug) {
        for (int i = 0, count = m_rows.count(); i < count; ++i) {
            const auto &row = m_rows[i];
            if (center >= row.pos && center < row.pos + perp(row.sizeHint)) {
                newRow = i;
                break;
            }
        }
    }

    if (unplug) {
        const auto pos = cursorPos - toolbar->d->m_initialDragPos;
        toolbar->d->undock(pos);
    } else {
        newPos = std::max(std::min(newPos, pick(availableSize) - pick(itemSize)), 0);
        const bool changeRow = [this, toolbar, &itemMinimumSize, &itemPath, newRow] {
            if (newRow == -1)
                return false;
            if (newRow == itemPath->row)
                return false;
            // check if the new row is already full
            const auto availableSize = pick(m_contentsRect.size()) - pick(m_rows[newRow].minimumSize);
            const auto dockedSize = pick(itemMinimumSize);
            return dockedSize <= availableSize;
        }();
        if (changeRow) {
            // move item to another row
            auto [item, rowRemoved] = TakeLayoutItem(*itemPath);
            const bool movingDown = newRow > itemPath->row;
            if (rowRemoved && movingDown)
                --newRow;
            Q_ASSERT(newRow >= 0 && newRow < m_rows.count());
            m_rows[newRow].items.append(Item { item, newPos });

            const auto bottomUp = m_tray == ToolBarTray::Bottom || m_tray == ToolBarTray::Right;

            // offset drag position if we're creating or removing the top row
            int offset = 0;
            if (!bottomUp) {
                if (topRowCreated)
                    offset = -1;
                else if (rowRemoved && movingDown)
                    offset = 1;
            } else {
                if (bottomRowCreated)
                    offset = 1;
                else if (rowRemoved && !movingDown)
                    offset = -1;
            }
            if (offset != 0) {
                offset *= perp(itemSize);
                toolbar->d->offsetDragPosition(
                    m_orientation == Qt::Horizontal ? QPoint(0, offset) : QPoint(offset, 0));
            }
        } else {
            auto &item = m_rows[itemPath->row].items[itemPath->index];
            item.pos = newPos;
        }
    }
}

void ToolBarTrayLayout::adjustToolBarRow(const ToolBar *toolbar)
{
    const auto itemPath = findItem(toolbar);
    if (!itemPath)
        return;
    const auto row = itemPath->row;
    m_rows[row].items = adjustRow(m_rows[row], toolbar);
}

void ToolBarTrayLayout::insertToolBar(ToolBar *before, ToolBar *toolbar)
{
    auto *item = QLayoutPrivate::createWidgetItem(m_parent, toolbar);

    // find where to place the toolbar
    auto [items, index] = [this, before, toolbar]() -> std::tuple<QVector<Item> &, int> {
        if (before == nullptr) {
            // append toolbar to the end of the last row
            if (m_rows.empty())
                m_rows.push_back({});
            auto &items = m_rows.back().items;
            return { items, items.count() };
        } else {
            // find where to insert it
            const auto path = findItem(before);
            Q_ASSERT(path);
            auto &items = m_rows[path->row].items;
            return { items, path->index };
        }
    }();

    // position it after other items in the row
    auto pos = std::accumulate(
        items.begin(), items.begin() + index, 0,
        [this](int pos, const auto &item) { return pos + pick(item.widgetItem->sizeHint()); });

    items.insert(index, { item, pos });

    // offset position of the remainder items in the row
    auto size = pick(item->sizeHint());
    for (auto it = std::next(items.begin(), index + 1); it != items.end(); ++it)
        it->pos += size;

    toolbar->setDockedOrientation(m_orientation);

    // dock the toolbar
    toolbar->d->setWindowState(false, {});
}

void ToolBarTrayLayout::insertToolBarBreak(ToolBar *before)
{
    if (before == nullptr) {
        if (m_rows.empty() || m_rows.back().items.empty())
            return;
        // add another row
        m_rows.push_back({});
        return;
    }

    const auto path = findItem(before);
    Q_ASSERT(path);
    // no need to insert a break if this is already the first item in the row
    if (path->index == 0)
        return;

    const auto items = m_rows[path->row].items;

    QVector<Item> leftItems(items.begin(), items.begin() + path->index);
    Q_ASSERT(!leftItems.isEmpty());

    QVector<Item> rightItems(items.begin() + path->index, items.end());
    Q_ASSERT(!rightItems.isEmpty());
    const int offset = rightItems.front().pos;
    for (auto &item : rightItems)
        item.pos -= offset;

    m_rows.remove(path->row);
    m_rows.insert(path->row, Row { std::move(rightItems), 0, {} });
    m_rows.insert(path->row, Row { std::move(leftItems), 0, {} });
}

void ToolBarTrayLayout::updateRowSizes() const
{
    if (!m_dirty)
        return;
    auto *that = const_cast<ToolBarTrayLayout *>(this);
    int pos = 0;
    for (auto &row : that->m_rows) {
        auto sizeHint = QSize(0, 0);
        auto minimumSize = QSize(0, 0);
        auto &items = row.items;
        for (const auto &item : items) {
            const auto itemSizeHint = item.widgetItem->sizeHint();
            rpick(sizeHint) += pick(itemSizeHint);
            rperp(sizeHint) = std::max(perp(sizeHint), perp(itemSizeHint));
            const auto itemMinimumSize = item.widgetItem->minimumSize();
            rpick(minimumSize) += pick(itemMinimumSize);
            rperp(minimumSize) = std::max(perp(minimumSize), perp(itemMinimumSize));
        }
        row.sizeHint = sizeHint;
        row.minimumSize = minimumSize;
        row.pos = pos;
        pos += perp(sizeHint);
    }
    that->m_dirty = false;
}

std::optional<ToolBarTrayLayout::ItemPath> ToolBarTrayLayout::findItem(
    const QWidget *widget) const
{
    for (int row = 0, count = m_rows.count(); row < count; ++row) {
        auto &items = m_rows[row].items;
        auto it = std::find_if(items.begin(), items.end(), [widget](auto &item) {
            return item.widgetItem->widget() == widget;
        });
        if (it != items.end()) {
            auto index = static_cast<int>(std::distance(items.begin(), it));
            return ItemPath { row, index };
        }
    }
    return std::nullopt;
}

ToolBarTrayLayout::Item *ToolBarTrayLayout::item(ItemPath path)
{
    if (path.row < 0 || path.row >= m_rows.count())
        return nullptr;
    auto &items = m_rows[path.row].items;
    if (path.index < 0 || path.index >= items.count())
        return nullptr;
    return &items[path.index];
}

QLayoutItem *ToolBarTrayLayout::layoutItem(ItemPath path)
{
    const auto *item = this->item(path);
    return item != nullptr ? item->widgetItem : nullptr;
}

std::tuple<QLayoutItem *, bool> ToolBarTrayLayout::TakeLayoutItem(ItemPath path)
{
    Q_ASSERT(path.row >= 0 && path.row < m_rows.count());
    auto &items = m_rows[path.row].items;
    Q_ASSERT(path.index >= 0 && path.index < items.count());
    auto *item = items.takeAt(path.index).widgetItem;
    const auto removeRow = items.empty();
    if (removeRow)
        m_rows.remove(path.row);
    return { item, removeRow };
}

QVector<ToolBarTrayLayout::Item> ToolBarTrayLayout::adjustRow(
    const Row &row, const ToolBar *pivot) const
{
    const auto &items = row.items;

    auto sortedItems = items;
    std::stable_sort(sortedItems.begin(), sortedItems.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.pos < rhs.pos;
    });
    auto pivotIt = std::find_if(sortedItems.begin(), sortedItems.end(), [pivot](const auto &item) {
        return item.widgetItem->widget() == pivot;
    });
    Q_ASSERT(pivotIt != sortedItems.end());

    // adjust item sizes so that the available space is not exceeded
    const auto availableSize = pick(m_contentsRect.size());
    adjustItemSizes(sortedItems, availableSize);

    // adjust position of the pivot toolbar
    auto &pivotItem = *pivotIt;
    const auto pivotItemSize = pivotItem.size;

    auto usedSizeBefore = std::accumulate(
        sortedItems.begin(), pivotIt, 0,
        [this](int size, const auto &item) { return size + item.size; });
    pivotItem.pos = std::max(usedSizeBefore, pivotItem.pos);

    auto usedSizeAfter = std::accumulate(
        std::next(pivotIt), sortedItems.end(), 0,
        [this](int size, const auto &item) { return size + item.size; });
    pivotItem.pos = std::max(
        std::min(pick(m_contentsRect.size()) - usedSizeAfter - pivotItemSize, pivotItem.pos), 0);

    const auto pivotIndex = static_cast<int>(std::distance(sortedItems.begin(), pivotIt));
    auto adjustRange = [this, &sortedItems](int from, int to, int left, int right, int usedSize) {
        for (int i = from; i <= to; ++i) {
            auto &item = sortedItems[i];
            usedSize -= item.size;
            item.pos = std::max(std::min(item.pos, (right - usedSize) - item.size), left);
            left = item.pos + item.size;
        }
    };

    // adjust position of toolbars before the pivot toolbar so they don't overlap
    adjustRange(0, pivotIndex - 1, 0, pivotItem.pos, usedSizeBefore);

    // adjust positions of toolbars after the pivot toolbar so they don't overlap
    const auto startAfter = pivotItem.pos + pivotItemSize;
    adjustRange(
        pivotIndex + 1, sortedItems.size() - 1, startAfter, availableSize, usedSizeAfter);

    return sortedItems;
}

int ToolBarTrayLayout::adjustItemSizes(QVector<Item> &items, int availableSize) const
{
    const auto minimumSize = std::accumulate(
        items.begin(), items.end(), 0,
        [this](int size, const auto &item) { return size + pick(item.widgetItem->minimumSize()); });
    // adjust item sizes so that the available space is not exceeded
    auto extra = std::max(0, availableSize - minimumSize);
    auto used = 0;
    for (auto &item : items) {
        const auto *widgetItem = item.widgetItem;
        const auto itemMinimumSize = pick(widgetItem->minimumSize());
        const auto itemExtra = std::min(pick(widgetItem->sizeHint()) - itemMinimumSize, extra);
        item.size = itemMinimumSize + itemExtra;
        Q_ASSERT(item.size <= pick(widgetItem->sizeHint()));
        extra -= itemExtra;
        used += item.size;
    }
    return used;
}

bool ToolBarTrayLayout::hoverToolBar(ToolBar *toolbar)
{
    if (!toolbar->isFloating())
        return false;

    const auto bottomUp = m_tray == ToolBarTray::Bottom || m_tray == ToolBarTray::Right;

    // figure out the geometry of the toolbar if it were docked
    auto dockedRect = [this, toolbar] {
        const auto *layout = qobject_cast<ToolBarLayout *>(toolbar->layout());
        Q_ASSERT(layout);
        auto r = toolbar->contentsRect().marginsRemoved(layout->innerContentsMargins());
        r.setSize(layout->dockedContentsSize(m_orientation));
        r = r.marginsAdded(layout->innerContentsMargins(false, m_orientation))
                .marginsAdded(toolbar->contentsMargins());
        r.moveTopLeft(m_parent->parentWidget()->mapFromGlobal(toolbar->mapToGlobal(r.topLeft())));
        return r;
    }();

    const auto contentsTopLeft = m_contentsRect.topLeft();

    const auto canDock = [this, toolbar, &contentsTopLeft, &dockedRect, bottomUp] {
        // HACK: figure out position of the cursor based on the current toolbar position
        // TODO: find a cleaner way to do this
        auto cursorPos = toolbar->pos() + toolbar->d->m_dragPos;
        cursorPos = m_parent->parentWidget()->mapFromGlobal(cursorPos);

        // is the cursor within the tray?
        if (pick(cursorPos) < pick(contentsTopLeft) || pick(cursorPos) > pick(contentsTopLeft) + pick(m_contentsRect.size()))
            return false;

        // is the docked toolbar within the tray?
        constexpr auto kEmptyTraySize = 4;
        auto totalSize = std::accumulate(
            m_rows.begin(), m_rows.end(), 0,
            [this](int size, const Row &row) { return size + perp(row.sizeHint); });
        totalSize = std::max(totalSize, kEmptyTraySize);

        auto pos = perp(dockedRect.topLeft());
        if (bottomUp)
            pos += perp(dockedRect.size());
        const auto contentsPos = perp(contentsTopLeft);

        return pos >= contentsPos && pos < contentsPos + totalSize;
    }();
    if (!canDock)
        return false;

    auto *layoutItem = [this, toolbar] {
        auto *tray = m_parent->toolBarTray(toolbar);
        auto itemPath = tray->findItem(toolbar);
        Q_ASSERT(itemPath);
        auto [layoutItem, rowRemoved] = tray->TakeLayoutItem(*itemPath);
        return layoutItem;
    }();

    // we may have stolen the toolbar from another tray, so update its orientation
    toolbar->setDockedOrientation(m_orientation);

    int plugRow = 0;
    auto pos = perp(dockedRect.topLeft()) - perp(contentsTopLeft);
    if (bottomUp)
        pos += perp(dockedRect.size());
    for (int i = 0, count = m_rows.count(); i < count; ++i) {
        const auto &row = m_rows[i];
        const auto size = perp(row.sizeHint);
        if (pos >= row.pos && pos < row.pos + size) {
            plugRow = i;
            if (pos > row.pos + (size / 2))
                ++plugRow;
            break;
        }
    }

    // create a new row for the item
    m_rows.insert(plugRow, {});
    auto &items = m_rows[plugRow].items;

    items.append(Item { layoutItem, pick(dockedRect.topLeft()) - pick(contentsTopLeft) });

    toolbar->d->dock();

    return true;
}

int ToolBarTrayLayout::rowCount() const
{
    return m_rows.count();
}

int ToolBarTrayLayout::Row::dockedCount() const
{
    return std::count_if(items.begin(), items.end(), [](Item item) {
        auto *tb = qobject_cast<ToolBar *>(item.widgetItem->widget());
        Q_ASSERT(tb);
        return !tb->isHidden() && !tb->isFloating();
    });
}

bool ToolBarTrayLayout::hasToolBar(const ToolBar *toolbar) const
{
    return findItem(toolbar) != std::nullopt;
}

ToolBarTrayLayoutState ToolBarTrayLayout::state() const
{
    ToolBarTrayLayoutState state;
    state.rows.reserve(m_rows.count());
    for (const auto &row : m_rows) {
        const auto &items = row.items;
        ToolBarTrayLayoutState::Row rowState;
        rowState.items.reserve(items.count());
        for (const auto &item : items) {
            const auto *tb = qobject_cast<ToolBar *>(item.widgetItem->widget());
            Q_ASSERT(tb);
            const bool isCustom = tb->options() & ToolBarOption::IsCustom;
            if (!isCustom && tb->objectName().isEmpty()) {
                qWarning("Object name is not set for toolbar, won't be properly restored!");
            }
            ToolBarTrayLayoutState::Item itemState;
            itemState.isCustom = isCustom;
            itemState.pos = item.pos;
            itemState.objectName = isCustom ? tb->windowTitle() : tb->objectName();
            itemState.isHidden = tb->isHidden();
            itemState.isFloating = tb->isFloating();
            itemState.floatingPos = tb->isFloating() ? tb->pos() : QPoint();
            itemState.toolBarState = tb->d->state();
            rowState.items.push_back(std::move(itemState));
        }
        state.rows.push_back(std::move(rowState));
    }
    return state;
}

void ToolBarTrayLayout::applyState(
    const ToolBarTrayLayoutState &state, const std::vector<ToolBar *> &toolbars, const std::vector<QAction *> &actions)
{
    // remove all current toolbars
    for (auto &row : m_rows) {
        for (auto &item : row.items)
            delete item.widgetItem;
    }
    m_rows.clear();

    for (const auto &rowState : state.rows) {
        QVector<Item> items;
        for (auto &itemState : rowState.items) {
            ToolBar *toolbar = [this, &itemState, &toolbars]() -> ToolBar * {
                if (itemState.isCustom) {
                    auto toolbar = new ToolBar(ToolBarOption::IsCustom);
                    toolbar->setWindowTitle(itemState.objectName);

                    m_parent->addChildWidget(toolbar);
                    m_parent->m_toolbars.push_back(toolbar); // TODO: rethink this
                    m_parent->m_toolbarTray[toolbar] = this;

                    return toolbar;
                }

                const auto &objectName = itemState.objectName;
                if (objectName.isEmpty())
                    return nullptr;
                auto it = std::find_if(toolbars.begin(), toolbars.end(), [&objectName](auto *tb) {
                    return tb->objectName() == objectName;
                });
                return it != toolbars.end() ? *it : nullptr;
            }();
            if (toolbar == nullptr)
                continue;
            auto *widgetItem = QLayoutPrivate::createWidgetItem(m_parent, toolbar);

            Item item;
            item.widgetItem = widgetItem;
            item.pos = itemState.pos;
            items.append(item);

            toolbar->setDockedOrientation(m_orientation);
            toolbar->setVisible(!itemState.isHidden);
            toolbar->d->applyState(itemState.toolBarState, actions);
            toolbar->d->setWindowState(itemState.isFloating);
            if (itemState.isFloating) {
                toolbar->move(itemState.floatingPos);
            }
        }
        if (!items.isEmpty()) {
            Row row;
            row.items = std::move(items);
            m_rows.append(std::move(row));
        }
    }
}

void ToolBarTrayLayoutState::save(QDataStream &stream) const
{
    stream << static_cast<int>(rows.size());
    for (const auto &row : rows) {
        const auto &items = row.items;
        stream << static_cast<int>(items.size());
        for (const auto &item : items) {
            stream << item.isCustom;
            stream << item.pos;
            stream << item.objectName;
            stream << item.isHidden;
            stream << item.isFloating;
            stream << item.floatingPos;
            item.toolBarState.save(stream);
        }
    }
}

bool ToolBarTrayLayoutState::load(QDataStream &stream)
{
    rows.clear();
    int rowCount;
    stream >> rowCount;
    rows.reserve(rowCount);
    for (int i = 0; i < rowCount; ++i) {
        Row row;
        int itemCount;
        stream >> itemCount;
        row.items.reserve(itemCount);
        for (int j = 0; j < itemCount; ++j) {
            Item item;
            stream >> item.isCustom;
            stream >> item.pos;
            stream >> item.objectName;
            stream >> item.isHidden;
            stream >> item.isFloating;
            stream >> item.floatingPos;
            item.toolBarState.load(stream);
            row.items.push_back(std::move(item));
        }
        rows.push_back(std::move(row));
    }
    return stream.status() == QDataStream::Ok;
}

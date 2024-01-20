/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbarlayout.h"

#include "toolbar.h"
#include "toolbarseparator.h"

#include <QStyleOptionToolButton>

#include <QtWidgets/private/qlayout_p.h>

#include <optional>

using namespace KDToolBars;

namespace {

bool isSeparator(QLayoutItem *item)
{
    return qobject_cast<ToolBarSeparator *>(item->widget()) != nullptr;
}

} // namespace

ToolBarLayout::ToolBarLayout(ToolBar *toolbar, QWidget *parent)
    : QLayout(parent)
    , m_toolbar(toolbar)
{
}

ToolBarLayout::~ToolBarLayout()
{
    while (!m_items.isEmpty())
        delete m_items.takeFirst();
    delete m_closeButton;
}

void ToolBarLayout::invalidate()
{
    m_dirty = true;
    QLayout::invalidate();
}

void ToolBarLayout::addItem(QLayoutItem *item)
{
    m_rowBreaks.clear();
    m_items.append(item);
    invalidate();
}

int ToolBarLayout::count() const
{
    int count = m_items.count();
    if (m_closeButton != nullptr)
        ++count;
    return count;
}

QLayoutItem *ToolBarLayout::itemAt(int index) const
{
    if (index < 0)
        return nullptr;
    if (index < m_items.count())
        return m_items[index];
    if (index == m_items.count())
        return m_closeButton;
    return nullptr;
}

QLayoutItem *ToolBarLayout::takeAt(int index)
{
    auto *item = [this, index]() -> QLayoutItem * {
        if (index < 0)
            return nullptr;
        if (index < m_items.count()) {
            m_rowBreaks.clear();
            return m_items.takeAt(index);
        }
        if (index == m_items.count()) {
            auto *item = m_closeButton;
            m_closeButton = nullptr;
            return item;
        }
        return nullptr;
    }();
    invalidate();
    return item;
}

Qt::Orientations ToolBarLayout::expandingDirections() const
{
    return {};
}

QSize ToolBarLayout::sizeHint() const
{
    if (m_dirty)
        updateGeometries();
    const auto sizeHint =
        m_contentsSize.expandedTo(m_minimumSize).grownBy(innerContentsMargins());
    return sizeHint;
}

QSize ToolBarLayout::minimumSize() const
{
    if (m_dirty)
        updateGeometries();
    return m_minimumSize.grownBy(innerContentsMargins());
}

void ToolBarLayout::updateGeometries() const
{
    if (!m_dirty)
        return;

    // initialize item geometries and size hint

    m_itemRows.clear();

    if (!m_items.empty()) {
        switch (layoutType()) {
        case LayoutType::Vertical: {
            int rowWidth = 0;
            for (auto *item : std::as_const(m_items))
                rowWidth = std::max(rowWidth, item->sizeHint().width());

            QPoint pos;
            ItemRow row;
            int count = 0;
            row.height = rowWidth;
            for (auto *item : std::as_const(m_items)) {
                count++;
                auto *separator = qobject_cast<ToolBarSeparator *>(item->widget());
                if (separator != nullptr)
                    separator->setOrientation(Qt::Vertical);
                row.items.push_back(
                    ItemRow::Item { item, QRect(pos, QSize(rowWidth, item->sizeHint().height())) });
                // add space between rows
                int spaceBetween = (count < m_items.size()) ? spacing() : 0;
                pos.ry() += item->sizeHint().height() + spaceBetween;
            }
            m_itemRows.push_back(std::move(row));
            m_contentsSize = QSize(rowWidth, pos.y());
            break;
        }
        case LayoutType::Horizontal: {
            int rowHeight = 0;
            for (auto *item : std::as_const(m_items))
                rowHeight = std::max(rowHeight, item->sizeHint().height());
            QPoint pos;
            ItemRow row;
            row.height = rowHeight;
            int count = 0;
            for (auto *item : std::as_const(m_items)) {
                count++;
                auto *separator = qobject_cast<ToolBarSeparator *>(item->widget());
                if (separator != nullptr)
                    separator->setOrientation(Qt::Horizontal);
                row.items.push_back(
                    ItemRow::Item { item, QRect(pos, QSize(item->sizeHint().width(), rowHeight)) });
                int spaceBetween = (count < m_items.size()) ? spacing() : 0;
                pos.rx() += item->sizeHint().width() + spaceBetween;
            }
            m_itemRows.push_back(std::move(row));
            m_contentsSize = QSize(pos.x(), rowHeight);
            break;
        }
        case LayoutType::Columns: {
            std::vector<int> rowBreaks;
            int column = 0;
            for (int i = 0, itemCount = static_cast<int>(m_items.size()); i < itemCount; ++i) {
                if (column == m_columns || isSeparator(m_items[i]) || (i > 0 && isSeparator(m_items[i - 1]))) {
                    rowBreaks.push_back(i);
                    column = 0;
                }
                ++column;
            }
            rowBreaks.push_back(static_cast<int>(m_items.size()));
            layoutRows(rowBreaks);
            break;
        }
        case LayoutType::Dynamic: {
            initializeDynamicLayouts();
            if (m_rowBreaks.empty())
                m_rowBreaks = m_dynamicLayouts.front().rowBreaks;
            if (!m_rowBreaks.empty())
                layoutRows(m_rowBreaks);
            break;
        }
        }
    }

    m_dirty = false;
}


void ToolBarLayout::layoutRows(const std::vector<int> &rowBreaks) const
{
    // figure out maximum row width
    int rowStart = 0;
    int maxRowWidth = 0;
    for (auto rowEnd : rowBreaks) {
        int rowWidth = 0;
        Q_ASSERT(rowEnd > rowStart);
        Q_ASSERT(rowStart >= 0 && rowEnd <= m_items.count());
        int numItems = 0;
        for (int i = rowStart; i < rowEnd; ++i) {
            rowWidth += m_items[i]->sizeHint().width();
            numItems++;
        }

        // add room for spacing in between items
        rowWidth += spacing() * (numItems - 1);
        maxRowWidth = std::max(maxRowWidth, rowWidth);
        rowStart = rowEnd;
    }
    // lay out items in rows
    rowStart = 0;
    int rowIndex = 0;
    QPoint rowPos;
    for (auto rowEnd : rowBreaks) {
        int rowHeight = 0;
        rowIndex++;
        for (int i = rowStart; i < rowEnd; ++i) {
            rowHeight = std::max(rowHeight, m_items[i]->sizeHint().height());
        }

        auto pos = rowPos;
        ItemRow row;
        row.height = rowHeight;
        for (int i = rowStart; i < rowEnd; ++i) {
            auto *item = m_items[i];
            auto size = QSize(item->sizeHint().width(), rowHeight);
            auto *separator = qobject_cast<ToolBarSeparator *>(item->widget());
            if (separator != nullptr) {
                if (i == rowStart && rowStart == rowEnd - 1) {
                    // row with a single separator spanning the full width, make separator horizontal
                    separator->setOrientation(Qt::Vertical);
                    size = QSize(maxRowWidth, item->sizeHint().height());
                } else {
                    separator->setOrientation(Qt::Horizontal);
                }
            }
            row.items.push_back(ItemRow::Item { item, QRect(pos, size) });
            // add room for spacing between items
            int spaceBetween = (i < rowEnd - 1) ? spacing() : 0;
            pos.rx() += item->sizeHint().width() + spaceBetween;
        }
        m_itemRows.push_back(std::move(row));
        // add room for spacing in between rows
        int spaceBetween = (rowIndex < rowBreaks.size()) ? spacing() : 0;
        rowPos.ry() += rowHeight + spaceBetween;
        rowStart = rowEnd;
    }

    m_contentsSize = QSize(maxRowWidth, rowPos.y());
}

void ToolBarLayout::setGeometry(const QRect &geometry)
{
    QLayout::setGeometry(geometry);

    if (m_dirty)
        updateGeometries();

    m_geometry = geometry;

    const auto contentsRect = geometry.marginsRemoved(innerContentsMargins());
    const auto contentsTopLeft = contentsRect.topLeft();
    for (const auto &row : std::as_const(m_itemRows)) {
        for (auto &item : row.items)
            item.item->setGeometry(item.geometry.translated(contentsTopLeft));
    }

    // KDAB_TODO: hide invisible widgets and show visible ones, as done by
    // QToolBarLayout
    for (auto *item : std::as_const(m_items)) {
        if (auto *widget = item->widget(); widget->isHidden())
            widget->show();
    }

    if (m_closeButton) {
        if (auto *widget = m_closeButton->widget()) {
            if (m_toolbar->isFloating()) {
                int left, top, right, bottom;
                getContentsMargins(&left, &top, &right, &bottom);
                const auto titleHeight = this->titleHeight();
                const auto size = widget->sizeHint();
                const auto topLeft = QPoint(
                    geometry.right() - right - size.width(), top + (titleHeight - size.height()) / 2);
                widget->setGeometry(QRect(topLeft, size));
                widget->show();
            } else {
                widget->hide();
            }
        }
    }
}

int ToolBarLayout::columns() const
{
    return m_columns;
}

void ToolBarLayout::setColumns(int columns)
{
    if (columns == m_columns)
        return;
    m_columns = columns;
    invalidate();
}

void ToolBarLayout::setMinimumSize(const QSize &size)
{
    m_minimumSize = size;
}

void ToolBarLayout::insertWidget(int index, QWidget *widget, ToolBarWidgetType type)
{
    addChildWidget(widget);
    auto *item = QLayoutPrivate::createWidgetItem(this, widget);
    if (type == ToolBarWidgetType::StandardButton)
        item->setAlignment(Qt::AlignJustify);
    m_rowBreaks.clear();
    m_items.insert(index, item);
    invalidate();
}

void ToolBarLayout::setCloseButton(QWidget *widget)
{
    if (m_closeButton != nullptr) {
        auto *oldWidget = m_closeButton->widget();
        if (oldWidget != nullptr) {
            oldWidget->hide();
            removeWidget(oldWidget);
        }
    }
    if (widget != nullptr) {
        addChildWidget(widget);
        m_closeButton = QLayoutPrivate::createWidgetItem(this, widget);
    } else {
        delete m_closeButton;
        m_closeButton = nullptr;
    }
    invalidate();
}

void ToolBarLayout::initializeDynamicLayouts() const
{
    struct Item
    {
        QSize size;
        bool isSeparator;
    };
    std::vector<Item> items;
    items.reserve(m_items.size());
    std::transform(m_items.begin(), m_items.end(), std::back_inserter(items), [](QLayoutItem *item) {
        return Item { item->sizeHint(), isSeparator(item) };
    });

    const auto itemCount = static_cast<int>(items.size());

    m_dynamicLayouts.clear();

    // dynamic programming: find minimum width of the layout if items starting out
    // at some index are laid out in n rows
    struct Layout
    {
        std::vector<int> rowBreaks;
        int width;
    };
    std::vector<std::vector<std::optional<Layout>>> cache(
        itemCount, std::vector<std::optional<Layout>>(itemCount + 1, std::nullopt));
    for (int rows = 1; rows <= itemCount; ++rows) {
        for (int startItem = itemCount - rows; startItem >= 0; --startItem) {
            std::optional<Layout> result = std::nullopt;
            const auto itemCount = items.size();
            if (rows == 1) {
                int rowWidth = 0;
                const auto startsWithSeparator = items[startItem].isSeparator;
                bool firstItem = true;
                for (int i = startItem; i < items.size(); ++i) {
                    if (!(startsWithSeparator && i == startItem)) {
                        rowWidth += items[i].size.width();
                        if (!firstItem)
                            rowWidth += spacing();
                        firstItem = false;
                    }
                }
                std::vector<int> rowBreaks;
                if (items[startItem].isSeparator)
                    rowBreaks.push_back(startItem + 1);
                rowBreaks.push_back(static_cast<int>(itemCount));
                result = Layout { rowBreaks, rowWidth };
            } else {
                int rowWidth = 0;
                const auto startsWithSeparator = items[startItem].isSeparator;
                bool firstItem = true;
                for (int i = startItem; i < itemCount - rows + 1; ++i) {
                    if (!(startsWithSeparator && i == startItem)) {
                        rowWidth += items[i].size.width();
                        if (!firstItem)
                            rowWidth += spacing();
                        firstItem = false;
                    }
                    // don't break line immediately after a separator
                    if (items[i].isSeparator)
                        continue;
                    const auto nextRow = cache[i + 1][rows - 1];
                    if (!nextRow)
                        continue;
                    if (!result || std::max(rowWidth, nextRow->width) <= result->width) {
                        std::vector<int> rowBreaks;
                        if (startsWithSeparator)
                            rowBreaks.push_back(startItem + 1);
                        rowBreaks.push_back(i + 1);
                        rowBreaks.insert(
                            rowBreaks.end(), nextRow->rowBreaks.begin(), nextRow->rowBreaks.end());
                        result = Layout { std::move(rowBreaks), std::max(rowWidth, nextRow->width) };
                    }
                }
            }
            cache[startItem][rows] = result;
        }
    }

    // now find out where to insert row breaks so we have rows with approximately
    // the same width
    for (int rows = 1; rows <= itemCount; ++rows) {
        auto layout = cache[0][rows];
        if (!layout)
            continue;

        auto &rowBreaks = layout->rowBreaks;
        int minimumHeight = 0;
        int rowStart = 0;
        bool first = true;
        for (auto rowEnd : rowBreaks) {
            if (!first)
                minimumHeight += spacing();
            else
                first = false;
            int rowHeight = 0;
            for (int i = rowStart; i < rowEnd; ++i) {
                rowHeight = std::max(rowHeight, items[i].size.height());
            }
            minimumHeight += rowHeight;
            rowStart = rowEnd;
        }
        m_dynamicLayouts.push_back(
            DynamicLayout { QSize(layout->width, minimumHeight), std::move(rowBreaks) });
    }
}

const ToolBarLayout::DynamicLayout *ToolBarLayout::preferredLayoutForSize(
    const QSize &availableSize) const
{
    if (m_items.empty())
        return nullptr;
    Q_ASSERT(!m_dynamicLayouts.empty());
    const auto itemCount = static_cast<int>(m_items.size());
    for (size_t i = m_dynamicLayouts.size() - 1; i > 0; --i) {
        const auto &layout = m_dynamicLayouts[i];
        const auto height = layout.minimumSize.height();
        if (height <= availableSize.height())
            return &layout;
    }
    return &m_dynamicLayouts.front();
}

ToolBarLayout::DropSite ToolBarLayout::findDropSite(const QPoint &layoutPos) const
{
    if (m_dirty)
        updateGeometries();

    const auto layoutType = this->layoutType();

    const auto contentsRect = m_geometry.marginsRemoved(innerContentsMargins());
    if (!contentsRect.contains(layoutPos)) {
        // cursor is outside the contents area
        return DropSite { -1, {}, 0 };
    }

    const auto contentsTopLeft = contentsRect.topLeft();
    if (m_itemRows.empty()) {
        // layout is empty, insert it into the first position
        return DropSite { 0, contentsTopLeft, m_minimumSize.height() };
    }

    const auto pos = layoutPos - contentsTopLeft;

    // find on which row we'll insert the new item
    const auto rowIndex = [this, layoutType, &pos]() -> int {
        auto coord = layoutType != LayoutType::Vertical ? pos.y() : pos.x();
        if (coord < 0)
            return 0;
        for (int i = 0, rowCount = static_cast<int>(m_itemRows.size()); i < rowCount; ++i) {
            if (coord < m_itemRows[i].height)
                return i;
            coord -= m_itemRows[i].height;
        }
        return m_itemRows.size() - 1;
    }();

    auto itemIndex = [this](const QLayoutItem *item) {
        auto it = std::find(m_items.begin(), m_items.end(), item);
        Q_ASSERT(it != m_items.end());
        return static_cast<int>(std::distance(m_items.begin(), it));
    };

    auto coord = [this, layoutType](const QPoint &pos) {
        return layoutType != LayoutType::Vertical ? pos.x() : pos.y();
    };

    // find position within row to insert the item

    const auto &row = m_itemRows[rowIndex];
    const auto &items = row.items;

    // find item in row that's closest to pos
    const auto it = std::min_element(
        items.begin(), items.end(), [pos, coord](const ItemRow::Item &lhs, const ItemRow::Item &rhs) {
            auto distance = [pos, coord](const ItemRow::Item &item) {
                return std::abs(coord(pos) - coord(item.geometry.center()));
            };
            return distance(lhs) < distance(rhs);
        });

    if (coord(pos) < coord(it->geometry.center())) {
        // place it on the left side of item
        return DropSite { itemIndex(it->item), it->geometry.topLeft() + contentsTopLeft,
                          row.height };
    } else {
        // place it on the right side of item
        auto next = std::next(it);
        if (next != items.end()) {
            // place it on the left side of next item
            return DropSite { itemIndex(next->item), next->geometry.topLeft() + contentsTopLeft,
                              row.height };
        } else {
            // place it after the last item in the row
            auto topLeft = it->geometry.topLeft();
            if (layoutType == LayoutType::Vertical)
                topLeft += QPoint(0, it->geometry.height());
            else
                topLeft += QPoint(it->geometry.width(), 0);
            return DropSite { itemIndex(it->item) + 1, topLeft + contentsTopLeft, row.height };
        }
    }
}

int ToolBarLayout::titleHeight() const
{
    return titleHeight(m_toolbar->isFloating());
}

int ToolBarLayout::titleHeight(bool floating) const
{
    if (!floating)
        return 0;
    constexpr auto kTitleMargin = 3;
    constexpr auto kTitleBarButtonMargin = 2;
    const QSize closeSize =
        m_closeButton != nullptr ? m_closeButton->widget()->sizeHint() : QSize(0, 0);
    const QFontMetrics titleFontMetrics = m_toolbar->fontMetrics();
    return std::max(
        closeSize.height() + 2 * kTitleBarButtonMargin,
        titleFontMetrics.height() + 2 * kTitleMargin);
}

QRect ToolBarLayout::titleArea() const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    const auto titleHeight = this->titleHeight();
    return QRect(QPoint(left, top), QSize(m_geometry.width() - (left + right), titleHeight));
}

int ToolBarLayout::handleExtent() const
{
    return handleExtent(m_toolbar->isFloating());
}

int ToolBarLayout::handleExtent(bool floating) const
{
    if (floating)
        return 0;
    QStyleOption opt;
    opt.initFrom(m_toolbar);
    return m_toolbar->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, m_toolbar);
}

QRect ToolBarLayout::handleArea() const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    const auto handleExtent = this->handleExtent();
    switch (layoutType()) {
    case LayoutType::Vertical:
    case LayoutType::Columns:
        return QRect(QPoint(left, top), QSize(m_geometry.width() - (left + right), handleExtent));
    case LayoutType::Horizontal:
    case LayoutType::Dynamic:
        return QRect(QPoint(left, top), QSize(handleExtent, m_geometry.height() - (top + bottom)));
    }
    Q_UNREACHABLE();
    return {};
}

QSize ToolBarLayout::adjustToWidth(int width)
{
    if (m_dirty)
        initializeDynamicLayouts();
    if (m_dynamicLayouts.empty())
        return QSize(0, 0);
    const auto &layout = [this, width]() -> const DynamicLayout & {
        Q_ASSERT(!m_dynamicLayouts.empty());
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        int availableWidth = width - (left + right);
        for (int i = 0; i < m_dynamicLayouts.size() - 1; ++i) {
            const auto &layout = m_dynamicLayouts[i];
            const auto &size = layout.minimumSize;
            if (size.width() <= availableWidth)
                return layout;
        }
        return m_dynamicLayouts.back();
    }();
    if (layout.rowBreaks != m_rowBreaks) {
        m_rowBreaks = layout.rowBreaks;
        invalidate();
    }
    const auto contentsSize = layout.minimumSize;
    return contentsSize.expandedTo(m_minimumSize).grownBy(innerContentsMargins());
}

QSize ToolBarLayout::adjustToHeight(int height)
{
    if (m_dirty)
        initializeDynamicLayouts();
    if (m_dynamicLayouts.empty())
        return QSize(0, 0);
    const auto &layout = [this, height]() -> const DynamicLayout & {
        Q_ASSERT(!m_dynamicLayouts.empty());
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        int availableHeight = height - (top + bottom + titleHeight());
        for (int i = m_dynamicLayouts.size() - 1; i > 0; --i) {
            const auto &layout = m_dynamicLayouts[i];
            const auto &size = layout.minimumSize;
            if (size.height() <= availableHeight)
                return layout;
        }
        return m_dynamicLayouts.front();
    }();
    if (layout.rowBreaks != m_rowBreaks) {
        m_rowBreaks = layout.rowBreaks;
        invalidate();
    }
    const auto contentsSize = layout.minimumSize;
    return contentsSize.expandedTo(m_minimumSize).grownBy(innerContentsMargins());
}

QMargins ToolBarLayout::innerContentsMargins() const
{
    return innerContentsMargins(m_toolbar->isFloating(), m_toolbar->dockedOrientation());
}

QMargins ToolBarLayout::innerContentsMargins(
    bool floating, Qt::Orientation dockedOrientation) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    if (floating) {
        top += titleHeight(floating);
    } else {
        switch (layoutType(floating, dockedOrientation)) {
        case LayoutType::Vertical:
        case LayoutType::Columns:
            top += handleExtent(floating);
            break;
        case LayoutType::Horizontal:
        case LayoutType::Dynamic:
            left += handleExtent(floating);
            break;
        }
    }
    return QMargins(left, top, right, bottom);
}

ToolBarLayout::LayoutType ToolBarLayout::layoutType() const
{
    return layoutType(m_toolbar->isFloating(), m_toolbar->dockedOrientation());
}

ToolBarLayout::LayoutType ToolBarLayout::layoutType(
    bool floating, Qt::Orientation dockedOrientation) const
{
    if (m_toolbar->columnLayout())
        return ToolBarLayout::LayoutType::Columns;
    if (floating)
        return ToolBarLayout::LayoutType::Dynamic;
    if (dockedOrientation == Qt::Horizontal)
        return ToolBarLayout::LayoutType::Horizontal;
    return ToolBarLayout::LayoutType::Vertical;
}

QSize ToolBarLayout::dockedContentsSize(Qt::Orientation orientation) const
{
    if (m_toolbar->columnLayout()) {
        int width = 0, height = 0;
        bool firstRow = true;
        int column = 0, rowWidth = 0, rowHeight = 0;
        for (auto *item : m_items) {
            const auto isSeparator = ::isSeparator(item);
            const auto breakRow = column == m_columns || isSeparator;
            if (breakRow) {
                width = std::max(width, rowWidth);
                height += rowHeight;
                if (!firstRow)
                    height += spacing();
                firstRow = false;
                column = 0;
                rowWidth = rowHeight = 0;
            }
            const auto &size = item->sizeHint();
            if (!isSeparator) {
                rowWidth += size.width();
                if (column > 0)
                    rowWidth += spacing();
                rowHeight = std::max(rowHeight, size.height());
                ++column;
            } else {
                // horizontal separator
                height += size.height() + spacing();
            }
        }
        if (column > 0) {
            width = std::max(width, rowWidth);
            height += rowHeight;
            if (!firstRow)
                height += spacing();
        }
        return QSize(width, height);
    }

    switch (orientation) {
    case Qt::Vertical: {
        int width = 0, height = 0;
        for (auto *item : std::as_const(m_items)) {
            const auto &size = item->sizeHint();
            width = std::max(width, size.width());
            height += size.height();
        }
        height += spacing() * (m_items.count() - 1);
        return QSize(width, height);
    }
    case Qt::Horizontal: {
        int width = 0, height = 0;
        for (auto *item : std::as_const(m_items)) {
            const auto &size = item->sizeHint();
            height = std::max(height, size.height());
            width += size.width();
        }
        width += spacing() * (m_items.count() - 1);
        return QSize(width, height);
    }
    }
    Q_UNREACHABLE();
    return {};
}

ToolBarLayoutState ToolBarLayout::state() const
{
    ToolBarLayoutState state;
    state.rowBreaks = m_rowBreaks;
    return state;
}

void ToolBarLayout::applyState(const ToolBarLayoutState &state)
{
    const auto &rowBreaks = state.rowBreaks;
    if (rowBreaks.empty())
        return;
    const auto valid = std::all_of(rowBreaks.begin(), rowBreaks.end(), [this](int rowEnd) {
        return rowEnd > 0 && rowEnd <= m_items.count();
    });
    if (!valid)
        return;
    m_rowBreaks = rowBreaks;
    invalidate();
}

void ToolBarLayoutState::save(QDataStream &stream) const
{
    stream << static_cast<int>(rowBreaks.size());
    for (auto rowEnd : rowBreaks)
        stream << rowEnd;
}

bool ToolBarLayoutState::load(QDataStream &stream)
{
    rowBreaks.clear();
    int count;
    stream >> count;
    rowBreaks.reserve(count);
    for (int i = 0; i < count; ++i) {
        int rowEnd;
        stream >> rowEnd;
        rowBreaks.push_back(rowEnd);
    }
    return stream.status() == QDataStream::Ok;
}

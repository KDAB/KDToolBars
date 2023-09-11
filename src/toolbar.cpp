/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbar.h"

#include "toolbar_p.h"
#include "toolbarlayout.h"
#include "toolbarseparator.h"
#include "toolbarcontainerlayout.h"
#include "mainwindow.h"
#include "mainwindow_p.h"

#include <QActionEvent>
#include <QApplication>
#include <QDrag>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionToolBar>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>

using namespace KDToolBars;

static void initKDToolBarsResources()
{
    Q_INIT_RESOURCE(kdtoolbars_resources);
}

namespace {
MainWindow *mainWindow(const ToolBar *tb)
{
    auto *w = tb->parentWidget();
    while (w != nullptr) {
        if (auto *mw = qobject_cast<MainWindow *>(w))
            return mw;
        w = w->parentWidget();
    }
    return nullptr;
}

QIcon buttonIcon(const QString &iconName)
{
    QIcon icon;
    icon.addFile(QStringLiteral(":/img/%1.png").arg(iconName));
    icon.addFile(QStringLiteral(":/img/%1-1.5x.png").arg(iconName));
    icon.addFile(QStringLiteral(":/img/%1-2x.png").arg(iconName));
    return icon;
}
} // namespace

namespace KDToolBars {

class DropIndicator : public QWidget
{
public:
    explicit DropIndicator(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        // TODO: not sure how this should look like, just paint a black rectangle for now
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);
    }
};

}

ToolBar::Private::Private(ToolBar *toolbar)
    : q(toolbar)
{
}

void ToolBar::Private::init()
{
    q->setFrameStyle(QFrame::Panel | QFrame::Raised);
    q->setLineWidth(1);

    q->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    q->setAttribute(Qt::WA_Hover);
#if 0
    // Make it a native window because we want a persistent window handle that stays "alive" after the
    // widget is docked/undocked while being dragged, otherwise we lose mouse grab
    q->setAttribute(Qt::WA_NativeWindow);
#endif

    auto *style = q->style();
    int e = style->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, q);
    m_iconSize = QSize(e, e);

    m_layout = new ToolBarLayout(q, q);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    auto updateMinimumHeight = [this] {
        // Set the minimum height to the height of a tool button
        QStyleOptionToolButton opt;
        opt.initFrom(q);
        opt.rect.setSize(m_iconSize);
        const auto buttonSize = q->style()->sizeFromContents(QStyle::CT_ToolButton, &opt, m_iconSize);
        const auto margins = m_layout->contentsMargins();
        const auto height = buttonSize.height() + margins.top() + margins.bottom();
        q->setMinimumHeight(height);
        m_layout->setMinimumSize(buttonSize);
    };
    QObject::connect(q, &ToolBar::iconSizeChanged, q, updateMinimumHeight);
    updateMinimumHeight();

    // Resize toolbar when icon size changes
    QObject::connect(q, &ToolBar::iconSizeChanged, q, [this] {
        if (q->isFloating()) {
            // Postpone resizing because we want it to happen after the buttons have been resized
            QTimer::singleShot(
                0, q, [this] { q->resize(m_layout->sizeHint().grownBy(q->contentsMargins())); });
        }
    });

    // Add the close button

    m_closeButton = new QToolButton(q);
    m_closeButton->setAutoRaise(true);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIcon(buttonIcon(QStringLiteral("close")));
    QObject::connect(m_closeButton, &QAbstractButton::clicked, q, &QWidget::close);
    m_layout->setCloseButton(m_closeButton);

    m_dropIndicator = new DropIndicator(q);
    m_dropIndicator->hide();

    q->setAcceptDrops(true);
}

ToolBar::Private::Margin ToolBar::Private::marginAt(const QPoint &p) const
{
    constexpr auto kFrameMargin = 4;
    if (p.x() < kFrameMargin)
        return Margin::Left;
    if (p.x() > q->width() - kFrameMargin)
        return Margin::Right;
    if (p.y() < kFrameMargin)
        return Margin::Top;
    if (p.y() > q->height() - kFrameMargin)
        return Margin::Bottom;
    return Margin::None;
}

bool ToolBar::Private::resizeStart(const QPoint &p)
{
    if (isResizing())
        return false;
    if (!q->isFloating() || !isResizable())
        return false;
    const auto margin = marginAt(p);
    if (margin == Margin::None)
        return false;
    m_resizeMargin = margin;
    return true;
}

void ToolBar::Private::resizeEnd()
{
    m_resizeMargin = Margin::None;
    q->setCursor(Qt::ArrowCursor);
}

void ToolBar::Private::dragMargin(const QPoint &p)
{
    auto geometry = q->geometry();
    const auto contentsMargins = q->contentsMargins();
    switch (m_resizeMargin) {
    case Margin::Left: {
        const int width = geometry.right() - p.x() + 1;
        const auto contents_size = m_layout->adjustToWidth(width);
        const auto updated_size = contents_size.grownBy(contentsMargins);
        if (updated_size.width() != geometry.width()) {
            const auto offset = geometry.width() - updated_size.width();
            q->move(geometry.x() + offset, geometry.y());
        }
        break;
    }
    case Margin::Right: {
        const int width = p.x() - geometry.left() + 1;
        m_layout->adjustToWidth(width);
        break;
    }
    case Margin::Top: {
        const auto height = geometry.bottom() - p.y() + 1;
        const auto contents_size = m_layout->adjustToHeight(height);
        const auto updated_size = contents_size.grownBy(contentsMargins);
        if (updated_size.height() != geometry.height()) {
            const auto offset = geometry.height() - updated_size.height();
            q->move(geometry.x(), geometry.y() + offset);
        }
        break;
    }
    case Margin::Bottom: {
        const auto height = p.y() - geometry.top() + 1;
        m_layout->adjustToHeight(height);
        break;
    }
    case Margin::None:
        Q_ASSERT(false);
        break;
    }
}

QWidget *ToolBar::Private::widgetForAction(QAction *action) const
{
    auto it = m_actionWidgets.find(action);
    return it != m_actionWidgets.end() ? it->second.widget : nullptr;
}

QAction *ToolBar::Private::actionForWidget(QWidget *widget) const
{
    auto it = std::find_if(m_actionWidgets.begin(), m_actionWidgets.end(),
                           [widget](const auto &item) {
                               return item.second.widget == widget;
                           });
    return it != m_actionWidgets.end() ? it->first : nullptr;
}

void ToolBar::Private::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();
    switch (event->type()) {
    case QEvent::ActionAdded: {
        int index = m_layout->count() - 1; // count includes the close button
        if (event->before()) {
            index = m_layout->indexOf(widgetForAction(event->before()));
            Q_ASSERT(index != -1);
        }
        auto item = createWidgetForAction(action);
        m_layout->insertWidget(index, item.widget, item.type);
        m_actionWidgets[action] = item;
        break;
    }
    case QEvent::ActionChanged: {
        m_layout->invalidate();
        break;
    }
    case QEvent::ActionRemoved: {
        auto it = m_actionWidgets.find(action);
        Q_ASSERT(it != m_actionWidgets.end());
        const auto item = it->second;
        m_layout->removeWidget(item.widget);
        m_actionWidgets.erase(it);
        if (item.type == ToolBarLayout::ToolBarWidgetType::CustomWidget) {
            if (auto *widgetAction = qobject_cast<QWidgetAction *>(action))
                widgetAction->releaseWidget(item.widget);
        } else {
            delete item.widget;
        }
        break;
    }
    default:
        break;
    }
}

bool ToolBar::Private::mousePressEvent(const QMouseEvent *me)
{
    if (me->button() != Qt::LeftButton)
        return false;

    // start resize?
    if (resizeStart(me->pos())) {
        return true;
    }

    // start drag?
    const bool startDrag = [this, me] {
        if (q->isFloating())
            return titleArea().contains(me->pos());
        else
            return handleArea().contains(me->pos());
    }();
    if (startDrag) {
        m_isDragging = true;
        m_dragPos = m_initialDragPos = me->pos();
        qApp->installEventFilter(this);
        q->grabMouse(Qt::SizeAllCursor);
        return true;
    }

    return false;
}

bool ToolBar::Private::mouseMoveEvent(const QMouseEvent *me)
{
    if (q->isFloating() && isResizing()) {
        dragMargin(me->globalPos());
        return true;
    }

    if (m_isDragging) {
        auto *mw = mainWindow(q);
        Q_ASSERT(mw);
        auto *layout = mw->d->m_layout;
        if (q->isFloating()) {
            const QPoint pos = me->globalPos() - m_dragPos;
            q->move(pos);
            layout->hoverToolBar(q);
        } else {
            const QPoint delta = me->globalPos() - q->mapToGlobal(m_dragPos);
            layout->moveToolBar(q, q->pos() + delta);
        }
        return true;
    }

    return false;
}

bool ToolBar::Private::mouseReleaseEvent(const QMouseEvent *me)
{
    if (isResizing()) {
        resizeEnd();
        return true;
    }

    if (m_isDragging) {
        q->releaseMouse();
        qApp->removeEventFilter(this);
        if (!q->isFloating()) {
            auto *mw = mainWindow(q);
            Q_ASSERT(mw);
            mw->d->m_layout->adjustToolBarRow(q);
        }
        m_isDragging = false;

        return true;
    }

    return false;
}

bool ToolBar::Private::mouseDoubleClickEvent(const QMouseEvent *me)
{
    if (me->button() != Qt::LeftButton)
        return false;
    const auto shouldDock = q->isFloating() && titleArea().contains(me->pos());
    if (shouldDock) {
        dock();
        return true;
    }

    return false;
}

bool ToolBar::Private::hoverMoveEvent(const QHoverEvent *he)
{
    if (!isResizing()) {
        const auto cursor = [this, he] {
            if (q->isFloating() && isResizable()) {
                const auto margin = marginAt(he->pos());
                switch (margin) {
                case ToolBar::Private::Margin::Left:
                case ToolBar::Private::Margin::Right:
                    return Qt::SizeHorCursor;
                case ToolBar::Private::Margin::Top:
                case ToolBar::Private::Margin::Bottom:
                    return Qt::SizeVerCursor;
                default:
                    break;
                }
            } else {
                if (m_isDragging || handleArea().contains(he->pos()))
                    return Qt::SizeAllCursor;
            }

            return Qt::ArrowCursor;
        }();
        q->setCursor(cursor);
    }
    return true;
}

void ToolBar::Private::dragEnterEvent(QDragEnterEvent *e)
{
    if (!canCustomize())
        return;
    if (!qobject_cast<const ToolbarActionMimeData *>(e->mimeData()))
        return;
    m_dropIndicator->show();
    updateDropIndicatorGeometry(e->pos());
    m_dropIndicator->raise();
    e->acceptProposedAction();
}

void ToolBar::Private::dragMoveEvent(QDragMoveEvent *e)
{
    if (!qobject_cast<const ToolbarActionMimeData *>(e->mimeData()))
        return;
    if (updateDropIndicatorGeometry(e->pos())) {
        e->acceptProposedAction();
    } else {
        e->ignore();
    }
}

QRect ToolBar::Private::actionRect(QAction *action) const
{
    const QWidget *w = widgetForAction(action);
    Q_ASSERT(w);
    return QRect(w->mapTo(q, QPoint(0, 0)), w->size());
}

bool ToolBar::Private::updateDropIndicatorGeometry(QPoint pos)
{
    const auto drop_site = m_layout->findDropSite(pos);
    if (drop_site.itemIndex == -1) {
        m_dropIndicator->hide();
        return false;
    }

    const QRect geometry = [this, drop_site] {
        QPoint position = drop_site.topLeft;
        constexpr auto kMargin = 4;
        constexpr auto kWidth = 2;
        const auto vertical =
            q->isFloating() || q->columnLayout() || q->dockedOrientation() == Qt::Horizontal;
        if (vertical) {
            return QRect(
                position + QPoint(-kWidth / 2, kMargin), QSize(kWidth, drop_site.size - 2 * kMargin));
        } else {
            return QRect(position + QPoint(kMargin, kWidth / 2), QSize(drop_site.size - 2 * kMargin, 2));
        }
    }();
    m_dropIndicator->show();
    m_dropIndicator->setGeometry(geometry);
    m_dropIndicator->raise();

    return true;
}

void ToolBar::Private::dragLeaveEvent(QDragLeaveEvent *)
{
    m_dropIndicator->hide();
}

void ToolBar::Private::dropEvent(QDropEvent *e)
{
    m_dropIndicator->hide();

    auto *sourceToolbar = qobject_cast<ToolBar *>(e->source());
    if (!sourceToolbar)
        return;

    auto *data = qobject_cast<const ToolbarActionMimeData *>(e->mimeData());
    if (!data)
        return;
    QAction *actionToInsert = data->action;

    const auto dropSite = m_layout->findDropSite(e->pos());
    const auto position = dropSite.itemIndex;

    const auto actions = q->actions();
    QAction *beforeAction = position < actions.count() ? actions[position] : nullptr;

    if (e->dropAction() == Qt::MoveAction) {
        if (beforeAction != actionToInsert) {
            sourceToolbar->removeAction(actionToInsert);
            emit sourceToolbar->actionsCustomized();

            q->insertAction(beforeAction, actionToInsert);
            emit q->actionsCustomized();
        }
    } else {
        q->insertAction(beforeAction, actionToInsert);
        emit q->actionsCustomized();
    }

    e->acceptProposedAction();
}

bool ToolBar::Private::canCustomize() const
{
    auto *mw = mainWindow(q);
    Q_ASSERT(mw);
    return mw->isCustomizingToolBars();
}

QRect ToolBar::Private::titleArea() const
{
    return m_layout->titleArea().translated(q->contentsRect().topLeft());
}

QRect ToolBar::Private::handleArea() const
{
    return m_layout->handleArea().translated(q->contentsRect().topLeft());
}

bool ToolBar::Private::isMoving() const
{
    return !q->isFloating() && m_isDragging;
}

void ToolBar::Private::undock(const QPoint &pos)
{
    if (q->isFloating())
        return;
    setWindowState(true, pos);
}

void ToolBar::Private::dock()
{
    if (!q->isFloating())
        return;
    setWindowState(false, {});
}

void ToolBar::Private::setWindowState(bool floating, const QPoint &pos)
{
    if (m_isDragging) {
        q->releaseMouse();
    }

    const auto wasVisible = !q->isHidden();
    const auto prevMargins = m_layout->innerContentsMargins();
    q->hide();

    Qt::WindowFlags flags = floating ? Qt::Tool : Qt::Widget;
    flags |= Qt::FramelessWindowHint;
    if (floating)
        flags |= Qt::X11BypassWindowManagerHint;
    q->setWindowFlags(flags);

    m_layout->invalidate();

    const auto marginsTopLeft = [](const QMargins &margins) {
        return QPoint(margins.left(), margins.top());
    };
    const auto marginsSize = [](const QMargins &margins) {
        return QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    };
    const auto margins = m_layout->innerContentsMargins();
    if (floating) {
        const auto marginOffset = marginsTopLeft(margins) - marginsTopLeft(prevMargins);
        q->setGeometry(QRect(pos - marginOffset, q->sizeHint()));
    }
    if (m_isDragging) {
        m_dragPos = m_initialDragPos + marginsTopLeft(margins) - marginsTopLeft(prevMargins);
        m_initialDragPos = m_dragPos;
    }

    if (wasVisible) {
        q->show();
        q->activateWindow();
    }

    if (m_isDragging) {
        q->grabMouse(Qt::SizeAllCursor);
    }

    emit q->isFloatingChanged(floating);
}

void ToolBar::Private::offsetDragPosition(const QPoint &offset)
{
    m_dragPos += offset;
}

QSize ToolBar::Private::dockedSize() const
{
    return m_layout->dockedContentsSize(m_dockedOrientation)
        .grownBy(m_layout->innerContentsMargins(false, m_dockedOrientation))
        .grownBy(q->contentsMargins());
}

bool ToolBar::Private::eventFilter(QObject *watched, QEvent *event)
{
    if (m_isDragging) {
        if (event->type() == QEvent::Enter) {
            // Sometimes toolbar buttons may get highlighted while we're dragging toolbars.
            // This usually happens when the toolbar is getting docked or undocked and we
            // temporarily lose mouse grab.
            //
            // Prevent this from happening by filtering out QEvent::Enter events on child
            // widgets while dragging a toolbar.
            const auto isToolbarChild = [this, watched] {
                auto *w = qobject_cast<QWidget *>(watched);
                if (w == nullptr)
                    return false;
                auto *p = w->parentWidget();
                while (p != nullptr) {
                    if (qobject_cast<ToolBar *>(p) != nullptr)
                        return true;
                    p = p->parentWidget();
                }
                return false;
            }();
            return isToolbarChild;
        }
        return false;
    }

    QWidget *sourceWidget = qobject_cast<QWidget *>(watched);
    if (!sourceWidget)
        return false;

    QAction *sourceAction = actionForWidget(sourceWidget);
    if (!sourceAction)
        return false;

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (canCustomize() && me->button() == Qt::LeftButton) {
            // We're customizing toolbars, start dragging this action
            QDrag *drag = new QDrag(q);
            {
                QPixmap iconPixmap(sourceWidget->size());
                QPainter painter(&iconPixmap);
                sourceWidget->render(&painter);
                drag->setPixmap(iconPixmap);
            }
            auto *data = new ToolbarActionMimeData;
            data->action = sourceAction;
            drag->setMimeData(data);
            const Qt::DropAction dropAction = drag->exec(Qt::MoveAction | Qt::CopyAction);
            if (dropAction == Qt::IgnoreAction) {
                // Action was dropped outside a toolbar, delete it
                q->removeAction(sourceAction);
                emit q->actionsCustomized();
            }
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove: {
        // Ignore mouse events while customizing toolbars
        if (canCustomize())
            return true;
        break;
    }
    case QEvent::ToolTip: {
        // Don't display tooltips while customizing toolbars
        if (canCustomize())
            return true;
        break;
    }
    default:
        break;
    }

    return false;
}

ToolBar::Private::ActionWidget ToolBar::Private::createWidgetForAction(QAction *action)
{
    // separator
    if (action->isSeparator()) {
        auto *separator = new ToolBarSeparator(q);
        return { ToolBarLayout::ToolBarWidgetType::Separator, separator };
    }

    // custom widget
    if (auto *widgetAction = qobject_cast<QWidgetAction *>(action)) {
        if (auto *widget = widgetAction->requestWidget(q)) {
            widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
            return { ToolBarLayout::ToolBarWidgetType::CustomWidget, widget };
        }
    }

    // standard button
    auto *button = new QToolButton(q);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setIconSize(m_iconSize);
    QObject::connect(q, &ToolBar::iconSizeChanged, button, &QToolButton::setIconSize);
    button->setToolButtonStyle(m_toolButtonStyle);
    QObject::connect(q, &ToolBar::toolButtonStyleChanged, button, &QToolButton::setToolButtonStyle);
    button->setDefaultAction(action);
    return { ToolBarLayout::ToolBarWidgetType::StandardButton, button };
}

ToolBar::ToolBar(const QString &title, QWidget *parent)
    : QFrame(parent)
    , d(new Private(this))
{
    initKDToolBarsResources();

    setWindowTitle(title);
    d->init();
}

ToolBar::~ToolBar()
{
    delete d;
}

void ToolBar::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter p(this);
    QStyle *style = this->style();

    if (isFloating()) {
        // paint title bar
        const auto titleArea = d->titleArea();

        static const QColor captionColor = QColor(156, 182, 209);

        const auto active = window()->isActiveWindow();
        p.setPen(palette().window().color().darker(130));
        p.setBrush(captionColor);
        p.drawRect(titleArea.adjusted(0, 1, -1, -3));

        auto textRect = titleArea;
        textRect.setWidth(textRect.width() - d->m_closeButton->width());

        QStyleOptionDockWidget opt;
        opt.initFrom(this);
        opt.title = windowTitle();
        opt.rect = textRect;

        QFont f = p.font();
        f.setBold(true);
        p.setFont(f);

        style->drawControl(QStyle::CE_DockWidgetTitle, &opt, &p, this);
    } else {
        // paint handle
        QStyleOption opt;
        opt.initFrom(this);
        opt.rect = d->handleArea();
        if (d->m_dockedOrientation == Qt::Horizontal)
            opt.state = QStyle::State_Horizontal;
        style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
    }
}

void ToolBar::actionEvent(QActionEvent *event)
{
    d->actionEvent(event);
}

bool ToolBar::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        const auto *me = static_cast<QMouseEvent *>(event);
        if (d->mousePressEvent(me))
            return true;
        break;
    }
    case QEvent::MouseMove: {
        const auto *me = static_cast<QMouseEvent *>(event);
        if (d->mouseMoveEvent(me))
            return true;
        break;
    }
    case QEvent::MouseButtonRelease: {
        const auto *me = static_cast<QMouseEvent *>(event);
        if (d->mouseReleaseEvent(me))
            return true;
        break;
    }
    case QEvent::MouseButtonDblClick: {
        const auto *me = static_cast<QMouseEvent *>(event);
        if (d->mouseDoubleClickEvent(me))
            return true;
        break;
    }
    case QEvent::HoverMove: {
        const auto *he = static_cast<QHoverEvent *>(event);
        if (d->hoverMoveEvent(he))
            return true;
        break;
    }
    default:
        break;
    }
    return QWidget::event(event);
}

void ToolBar::childEvent(QChildEvent *e)
{
    QObject *child = e->child();
    if (e->type() == QEvent::ChildAdded && child->isWidgetType()) {
        child->installEventFilter(d);
    }
    QFrame::childEvent(e);
}

void ToolBar::dragEnterEvent(QDragEnterEvent *e)
{
    d->dragEnterEvent(e);
}

void ToolBar::dragMoveEvent(QDragMoveEvent *e)
{
    d->dragMoveEvent(e);
}

void ToolBar::dragLeaveEvent(QDragLeaveEvent *e)
{
    d->dragLeaveEvent(e);
}

void ToolBar::dropEvent(QDropEvent *e)
{
    d->dropEvent(e);
}

bool ToolBar::columnLayout() const
{
    return d->m_columnLayout;
}

void ToolBar::setColumnLayout(bool columnLayout)
{
    if (columnLayout == d->m_columnLayout)
        return;
    d->m_columnLayout = columnLayout;
    d->m_layout->invalidate();
}

int ToolBar::columns() const
{
    return d->m_layout->columns();
}

void ToolBar::setColumns(int columns)
{
    d->m_layout->setColumns(columns);
}

void ToolBar::setSpacing(int spacing)
{
    d->m_layout->setSpacing(spacing);
}

void ToolBar::clear()
{
    const auto actions = this->actions();
    for (QAction *action : actions)
        removeAction(action);
}

QAction *ToolBar::addSeparator()
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

bool ToolBar::isFloating() const
{
    return isWindow();
}

QSize ToolBar::iconSize() const
{
    return d->m_iconSize;
}

void ToolBar::setIconSize(const QSize &iconSize)
{
    d->m_explicitIconSize = true;
    QSize size = iconSize;
    if (size.isNull()) {
        int e = style()->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, this);
        size = QSize(e, e);
    }
    if (size == d->m_iconSize)
        return;
    d->m_iconSize = size;
    emit iconSizeChanged(size);
}

Qt::ToolButtonStyle ToolBar::toolButtonStyle() const
{
    return d->m_toolButtonStyle;
}

void ToolBar::setToolButtonStyle(Qt::ToolButtonStyle style)
{
    d->m_explicitToolButtonStyle = true;
    if (style == d->m_toolButtonStyle)
        return;
    d->m_toolButtonStyle = style;
    emit toolButtonStyleChanged(style);
}

void ToolBar::setDockedOrientation(Qt::Orientation orientation)
{
    if (d->m_dockedOrientation == orientation)
        return;
    d->m_dockedOrientation = orientation;
    d->m_layout->invalidate();
}

Qt::Orientation ToolBar::dockedOrientation() const
{
    return d->m_dockedOrientation;
}

void ToolBar::setAllowedTrays(ToolBarTrays trays)
{
    d->m_allowedTrays = trays;
}

ToolBarTrays ToolBar::allowedTrays() const
{
    return d->m_allowedTrays;
}

QToolButton *ToolBar::closeButton() const
{
    return d->m_closeButton;
}

void ToolBar::updateIconSize(const QSize &size)
{
    if (d->m_explicitIconSize)
        return;
    setIconSize(size);
    d->m_explicitIconSize = false;
}

void ToolBar::updateToolButtonStyle(Qt::ToolButtonStyle style)
{
    if (d->m_explicitToolButtonStyle)
        return;
    setToolButtonStyle(style);
    d->m_explicitToolButtonStyle = false;
}

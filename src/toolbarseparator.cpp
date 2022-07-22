#include "toolbarseparator.h"

#include <QStyleOption>
#include <QPainter>

ToolBarSeparator::ToolBarSeparator(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

ToolBarSeparator::~ToolBarSeparator() = default;

void ToolBarSeparator::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation)
        return;
    m_orientation = orientation;
    if (orientation == Qt::Horizontal)
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    else
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    update();
}

QSize ToolBarSeparator::sizeHint() const
{
    QStyleOption opt;
    initStyleOption(&opt);
    const int extent = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, &opt, parentWidget());
    return QSize(extent, extent);
}

void ToolBarSeparator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt;
    initStyleOption(&opt);
    opt.palette.setColor(QPalette::Window, QColor(151, 151, 151));
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p, parentWidget());
}

void ToolBarSeparator::initStyleOption(QStyleOption *option) const
{
    option->initFrom(this);
    if (m_orientation == Qt::Horizontal)
        option->state |= QStyle::State_Horizontal;
}

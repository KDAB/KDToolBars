#pragma once

#include <QWidget>

class QStyleOption;

class ToolBarSeparator : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBarSeparator(QWidget *parent = nullptr);
    ~ToolBarSeparator() override;

    void setOrientation(Qt::Orientation orientation);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initStyleOption(QStyleOption *option) const;

    Qt::Orientation m_orientation = Qt::Horizontal;
};

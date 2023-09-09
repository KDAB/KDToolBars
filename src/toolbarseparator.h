/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QWidget>

class QStyleOption;

namespace KDToolBars {

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

} // namespace KDToolBars

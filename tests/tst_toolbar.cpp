/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbar.h"

#include <QAction>
#include <QProxyStyle>
#include <QTest>
#include <QToolButton>

using namespace KDToolBars;

namespace {

constexpr auto kToolButtonMargin = 4;
constexpr auto kSeparatorSize = 31;

class TestProxyStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override
    {
        switch (type) {
        case CT_ToolButton:
            return QSize(size.width() + kToolButtonMargin, size.height() + kToolButtonMargin);
        default:
            break;
        }
        return QProxyStyle::sizeFromContents(type, option, size, widget);
    }

    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override
    {
        switch (metric) {
        case PM_ToolBarSeparatorExtent:
            return kSeparatorSize;
        default:
            break;
        }
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

} // namespace

class TestToolBars : public QObject
{
    Q_OBJECT
public slots:
    void initTestCase()
    {
        qApp->setStyle(new TestProxyStyle);
    }

private slots:
    void testSimple();
    void testFloatingLayout();
};

void TestToolBars::testSimple()
{
    auto tb = new ToolBar(QString());

    // first item in the layout is the close button
    QCOMPARE(tb->layout()->count(), 1);
    QCOMPARE(tb->layout()->itemAt(0)->widget(), tb->closeButton());

    // add one action
    auto *action = new QAction(tb);
    tb->addAction(action);
    QCOMPARE(tb->layout()->count(), 2);

    // remove one action
    tb->removeAction(action);
    QCOMPARE(tb->layout()->count(), 1);

    delete tb;
}

void TestToolBars::testFloatingLayout()
{
    constexpr auto kIconSize = 37;
    constexpr auto kCloseButtonIconSize = 23;
    constexpr auto kSpacing = 7;
    constexpr auto kLayoutContentsMargin = 13;
    constexpr auto kButtonCount = 8;

    auto tb = new ToolBar("Test");
    tb->setIconSize(QSize(kIconSize, kIconSize));
    tb->closeButton()->setIconSize(QSize(kCloseButtonIconSize, kCloseButtonIconSize));
    tb->layout()->setContentsMargins(kLayoutContentsMargin, kLayoutContentsMargin, kLayoutContentsMargin, kLayoutContentsMargin);
    tb->setSpacing(kSpacing);

    // create a toolbar with some buttons
    for (int i = 0; i < kButtonCount; ++i)
        tb->addAction(new QAction(tb));
    QCOMPARE(tb->layout()->count(), kButtonCount + 1);

    tb->show();

    const auto expectedSize = [](int rows, int columns) {
        constexpr auto kButtonSize = kIconSize + kToolButtonMargin;
        constexpr auto kCloseButtonMargin = 2; // hardcoded in ToolBarLayout::titleHeight
        constexpr auto kTitleBarHeight = kCloseButtonIconSize + kToolButtonMargin + 2 * kCloseButtonMargin;

        const auto width = columns * kButtonSize + (columns - 1) * kSpacing + 2 * kLayoutContentsMargin;
        const auto height = rows * kButtonSize + (rows - 1) * kSpacing + kTitleBarHeight + 2 * kLayoutContentsMargin;

        return QSize(width, height);
    };

    // are the icons laid out in a single row?
    tb->resize(1000, 100);
    QCOMPARE(tb->layout()->sizeHint(), expectedSize(1, kButtonCount));

    // increase height to a bit more than the required height for two rows
    tb->resize(1000, expectedSize(2, kButtonCount / 2).height() + 40);

    // are the icons laid out in two rows?
    QCOMPARE(tb->layout()->sizeHint(), expectedSize(2, kButtonCount / 2));

    // set column layout
    tb->setColumns(2);
    tb->setColumnLayout(true);
    QCOMPARE(tb->layout()->sizeHint(), expectedSize(kButtonCount / tb->columns(), tb->columns()));

    // create a toolbar with a separator between the buttons
    tb->clear();
    tb->setColumnLayout(false);
    for (int i = 0; i < kButtonCount / 2; ++i)
        tb->addAction(new QAction(tb));
    tb->addSeparator();
    for (int i = 0; i < kButtonCount / 2; ++i)
        tb->addAction(new QAction(tb));
    QCOMPARE(tb->layout()->count(), kButtonCount + 2);

    // are the icons laid out in a single row with a vertical separator?
    tb->resize(1000, 100);
    QCOMPARE(tb->layout()->sizeHint(), expectedSize(1, kButtonCount) + QSize(kSeparatorSize + kSpacing, 0));

    // increase height to a bit more than the required height for two rows
    tb->resize(1000, expectedSize(2, kButtonCount / 2).height() + 40);

    // are the icons laid out in two rows with an horizontal separator between them?
    QCOMPARE(tb->layout()->sizeHint(), expectedSize(2, kButtonCount / 2) + QSize(0, kSeparatorSize + kSpacing));

    delete tb;
}

QTEST_MAIN(TestToolBars)
#include "tst_toolbar.moc"

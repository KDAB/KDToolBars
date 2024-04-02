/*
  This file is part of KDToolBars.

  SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbarcustomizationdialog.h"

#include "mainwindow.h"
#include "mainwindow_p.h"
#include "toolbar.h"

#include <QAbstractListModel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDebug>

namespace KDToolBars {

class ToolBarListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        ToolBarRole = Qt::UserRole + 1
    };

    explicit ToolBarListModel(MainWindow *manager, QObject *parent = nullptr)
        : QAbstractListModel(parent)
        , m_mainWindow(manager)
    {
        connect(manager, &MainWindow::toolBarAboutToBeInserted, this, [this](const ToolBar *, int row) {
            beginInsertRows({}, row, row);
        });
        connect(manager, &MainWindow::toolBarInserted, this, &ToolBarListModel::endInsertRows);
        connect(manager, &MainWindow::toolBarAboutToBeRemoved, this, [this](const ToolBar *, int row) { beginRemoveRows({}, row, row); });
        connect(manager, &MainWindow::toolBarRemoved, this, &ToolBarListModel::endRemoveRows);
        // TODO: should handle title/visibility changes
    }

    int rowCount(const QModelIndex &parent) const override
    {
        return parent.isValid() ? 0 : m_mainWindow->toolBarCount();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!checkIndex(index))
            return {};
        auto toolbar = m_mainWindow->toolBarAt(index.row());
        switch (role) {
        case Qt::DisplayRole:
            return toolbar->windowTitle();
        case Qt::CheckStateRole:
            return toolbar->isVisible() ? Qt::Checked : Qt::Unchecked;
        case ToolBarRole:
            return QVariant::fromValue(toolbar);
        default:
            break;
        }
        return {};
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!checkIndex(index) || role != Qt::CheckStateRole)
            return false;
        auto toolbar = m_mainWindow->toolBarAt(index.row());
        toolbar->setVisible(value.toInt() == Qt::Checked);
        emit dataChanged(index, index, { role });
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return QAbstractListModel::flags(index) | Qt::ItemFlag::ItemIsUserCheckable;
    }

private:
    MainWindow *m_mainWindow;
};

ToolBarCustomizationDialog::ToolBarCustomizationDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_toolBarModel(new ToolBarListModel(mainWindow, this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    setupUi();

    auto *sortModel = new QSortFilterProxyModel(this);
    sortModel->setSourceModel(m_toolBarModel);
    sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    sortModel->sort(0); // sort on the first (and only) column
    m_toolbarList->setModel(sortModel);

    connect(m_reset, &QPushButton::clicked, this, [this, sortModel] {
        const QModelIndex index = sortModel->mapToSource(m_toolbarList->currentIndex());
        if (!index.isValid())
            return;
        const auto name = index.data(Qt::DisplayRole).toString();
        const auto button = QMessageBox::question(
            this, {},
            tr("All your changes will be lost! Do you really want to reset the toolbar '%1'?")
                .arg(name),
            QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::Yes) {
            auto toolbar = index.data(ToolBarListModel::ToolBarRole).value<ToolBar *>();
            toolbar->reset();
        }
    });
    connect(m_resetAll, &QPushButton::clicked, this, [this] {
        const auto button = QMessageBox::question(
            this, {},
            tr("All your changes will be lost! Do you really want to reset all toolbars?"),
            QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::Yes) {
            const int count = m_mainWindow->toolBarCount();
            for (int i = 0; i < count; ++i) {
                auto toolbar = m_mainWindow->toolBarAt(i);
                toolbar->reset();
            }
        }
    });
    connect(m_newToolBar, &QPushButton::clicked, this, [this] {
        bool ok;
        QString title = QInputDialog::getText(
            this, {}, tr("Toolbar Name:"), QLineEdit::Normal, {}, &ok);
        if (ok && !title.isEmpty()) {
            auto toolbar = new ToolBar(ToolBarOption::IsCustom);
            toolbar->setWindowTitle(title);
            m_mainWindow->addToolBar(toolbar);
            toolbar->show();
        }
    });
    connect(m_renameToolBar, &QPushButton::clicked, this, [this, sortModel] {
        const QModelIndex index = sortModel->mapToSource(m_toolbarList->currentIndex());
        if (!index.isValid())
            return;
        const auto name = index.data(Qt::DisplayRole).toString();
        bool ok;
        QString title = QInputDialog::getText(
            this, {}, tr("Toolbar Name:"), QLineEdit::Normal, name, &ok);
        if (ok && !title.isEmpty()) {
            auto toolbar = index.data(ToolBarListModel::ToolBarRole).value<ToolBar *>();
            toolbar->setWindowTitle(title);
        }
    });
    connect(m_deleteToolBar, &QPushButton::clicked, this, [this, sortModel] {
        const QModelIndex index = sortModel->mapToSource(m_toolbarList->currentIndex());
        if (!index.isValid())
            return;
        const auto name = index.data(Qt::DisplayRole).toString();
        const auto button = QMessageBox::question(
            this, {},
            tr("Do you really want to delete the toolbar '%1'?").arg(name),
            QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::Yes) {
            auto toolbar = index.data(ToolBarListModel::ToolBarRole).value<ToolBar *>();
            m_mainWindow->removeToolBar(toolbar);
            delete toolbar;
        }
    });
    connect(m_renameToolBar, &QPushButton::clicked, this, []() {});

    const auto canResetAny = [this] {
        int count = m_mainWindow->toolBarCount();
        for (int i = 0; i < count; ++i) {
            auto toolbar = m_mainWindow->toolBarAt(i);
            if (toolbar->canBeReset())
                return true;
        }
        return false;
    }();
    m_resetAll->setEnabled(canResetAny);

    auto updateButtons = [this, sortModel] {
        const QModelIndex index = sortModel->mapToSource(m_toolbarList->currentIndex());
        auto toolbar = index.data(ToolBarListModel::ToolBarRole).value<ToolBar *>();
        const bool canBeReset = toolbar != nullptr && toolbar->canBeReset();
        const bool isCustomToolBar = toolbar != nullptr && (toolbar->options() & ToolBarOption::IsCustom);
        m_reset->setEnabled(canBeReset);
        m_deleteToolBar->setEnabled(isCustomToolBar);
        m_renameToolBar->setEnabled(isCustomToolBar);
    };
    connect(
        m_toolbarList->selectionModel(), &QItemSelectionModel::currentChanged, this,
        updateButtons);
    updateButtons();

    m_mainWindow->d->setCustomizingToolBars(true);
}

void ToolBarCustomizationDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    {
        auto *toolbarsPage = new QWidget;
        m_tabWidget->addTab(toolbarsPage, tr("Toolbars"));

        auto *layout = new QHBoxLayout(toolbarsPage);

        m_toolbarList = new QListView(toolbarsPage);
        layout->addWidget(m_toolbarList);

        auto *buttonLayout = new QVBoxLayout();
        layout->addLayout(buttonLayout);

        m_reset = new QPushButton(tr("Reset"), toolbarsPage);
        buttonLayout->addWidget(m_reset);

        m_resetAll = new QPushButton(tr("Reset All"), toolbarsPage);
        buttonLayout->addWidget(m_resetAll);

        m_newToolBar = new QPushButton(tr("New..."), toolbarsPage);
        buttonLayout->addWidget(m_newToolBar);

        m_renameToolBar = new QPushButton(tr("Rename..."), toolbarsPage);
        buttonLayout->addWidget(m_renameToolBar);

        m_deleteToolBar = new QPushButton(tr("Delete"), toolbarsPage);
        buttonLayout->addWidget(m_deleteToolBar);

        buttonLayout->addStretch();
    }

    {
        auto *optionsPage = new QWidget();
        m_tabWidget->addTab(optionsPage, tr("Options"));

        auto *layout = new QVBoxLayout(optionsPage);

        m_showToolTips = new QCheckBox(tr("Show Screen Tips on toolbars"), optionsPage);
        layout->addWidget(m_showToolTips);

        layout->addStretch();
    }

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    m_tabWidget->setCurrentIndex(0);

    setWindowTitle(tr("Toolbars"));
}

ToolBarCustomizationDialog::~ToolBarCustomizationDialog()
{
    if (!m_mainWindow.isNull())
        m_mainWindow->d->setCustomizingToolBars(false);
}

}

#include "toolbarcustomizationdialog.moc"

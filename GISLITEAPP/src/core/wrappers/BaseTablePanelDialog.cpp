/**
 * @file BaseTablePanelDialog.cpp
 * @brief Implementation of BaseTablePanelDialog reusable tactical table dialog.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "BaseTablePanelDialog.h"

#include <QHeaderView>
#include <QDebug>

namespace GISApp::Core::Wrappers {

GenericTableFilterProxyModel::GenericTableFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void GenericTableFilterProxyModel::setSearchText(const QString &text)
{
    m_searchText = text.trimmed();
    invalidate();
}

bool GenericTableFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto *baseDialog = qobject_cast<BaseTablePanelDialog*>(parent());
    if (baseDialog) {
        // Delegate to derived class custom filter first (e.g. affiliation, domain)
        if (!baseDialog->customFilterAcceptsRow(sourceRow, sourceParent)) {
            return false;
        }
    }

    if (m_searchText.isEmpty()) {
        return true;
    }

    QAbstractItemModel *model = sourceModel();
    if (!model) {
        return true;
    }

    // Default multi-column text search
    const int cols = model->columnCount(sourceParent);
    for (int col = 0; col < cols; ++col) {
        const QModelIndex idx = model->index(sourceRow, col, sourceParent);
        const QString cellText = model->data(idx, Qt::DisplayRole).toString();
        if (cellText.contains(m_searchText, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

BaseTablePanelDialog::BaseTablePanelDialog(QAbstractItemModel *model, QWidget *parent)
    : QDialog(parent)
    , m_sourceModel(nullptr)
    , m_proxyModel(nullptr)
    , m_tableView(nullptr)
    , m_searchEdit(nullptr)
    , m_statusBadge(nullptr)
    , m_refreshBtn(nullptr)
    , m_customFilterLayout(nullptr)
{
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    resize(1020, 580);
    setMinimumSize(750, 400);

    setupBaseUi();
    setupBaseConnections();

    if (model) {
        setModel(model);
    }
}

void BaseTablePanelDialog::setupBaseUi()
{
    setObjectName(QStringLiteral("BaseTablePanelDialog"));
    setAttribute(Qt::WA_StyledBackground, true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // 1. Top Control Bar (Search, Custom Filters Hook, Status Badge, Refresh)
    auto *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(8);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("🔍 Search..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(220);
    toolbarLayout->addWidget(m_searchEdit);

    // Sub-layout for derived classes to insert custom controls
    m_customFilterLayout = new QHBoxLayout();
    m_customFilterLayout->setSpacing(8);
    setupCustomFilters(m_customFilterLayout);
    toolbarLayout->addLayout(m_customFilterLayout);

    toolbarLayout->addStretch(1);

    m_statusBadge = new QLabel(this);
    m_statusBadge->setObjectName(QStringLiteral("StatusBadge"));
    toolbarLayout->addWidget(m_statusBadge);

    m_refreshBtn = new QPushButton(tr("⟳ Refresh"), this);
    toolbarLayout->addWidget(m_refreshBtn);

    mainLayout->addLayout(toolbarLayout);

    // 2. QTableView and Proxy Model
    m_proxyModel = new GenericTableFilterProxyModel(this);

    m_tableView = new QTableView(this);
    m_tableView->setModel(m_proxyModel);
    m_tableView->setSortingEnabled(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setShowGrid(true);
    m_tableView->verticalHeader()->setVisible(true);
    m_tableView->verticalHeader()->setDefaultSectionSize(28);

    auto *hHeader = m_tableView->horizontalHeader();
    hHeader->setStretchLastSection(true);
    hHeader->setSectionsMovable(true);
    hHeader->setHighlightSections(false);
    hHeader->setDefaultAlignment(Qt::AlignCenter);

    mainLayout->addWidget(m_tableView, 1);
}

void BaseTablePanelDialog::setupBaseConnections()
{
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &BaseTablePanelDialog::onSearchTextChanged);

    connect(m_refreshBtn, &QPushButton::clicked,
            this, &BaseTablePanelDialog::refreshData);

    connect(m_tableView, &QTableView::clicked,
            this, &BaseTablePanelDialog::onTableCellClicked);

    connect(m_tableView, &QTableView::doubleClicked,
            this, &BaseTablePanelDialog::onTableCellDoubleClicked);
}

void BaseTablePanelDialog::showDialog()
{
    show();
    raise();
    activateWindow();
}

void BaseTablePanelDialog::closeDialog()
{
    close();
}

bool BaseTablePanelDialog::isDialogVisible() const
{
    return isVisible();
}

void BaseTablePanelDialog::setModel(QAbstractItemModel *model)
{
    if (m_sourceModel) {
        disconnect(m_sourceModel, &QAbstractItemModel::modelReset,
                   this, &BaseTablePanelDialog::updateStatistics);
    }

    m_sourceModel = model;
    if (m_proxyModel) {
        m_proxyModel->setSourceModel(m_sourceModel);
    }

    if (m_sourceModel) {
        connect(m_sourceModel, &QAbstractItemModel::modelReset,
                this, &BaseTablePanelDialog::updateStatistics);
    }

    updateStatistics();
}

QAbstractItemModel* BaseTablePanelDialog::model() const
{
    return m_sourceModel;
}

void BaseTablePanelDialog::setSearchFilter(const QString &filterText)
{
    if (m_searchEdit) {
        m_searchEdit->setText(filterText);
    }
}

void BaseTablePanelDialog::updateStatistics()
{
    if (!m_statusBadge) {
        return;
    }

    int total = m_sourceModel ? m_sourceModel->rowCount() : 0;
    int visible = m_proxyModel ? m_proxyModel->rowCount() : 0;

    m_statusBadge->setText(QStringLiteral("Showing: <b>%1</b> / %2").arg(visible).arg(total));
}

void BaseTablePanelDialog::refreshData()
{
    if (m_sourceModel) {
        // Trigger model reset / reload if supported
        updateStatistics();
    }
}

void BaseTablePanelDialog::setupCustomFilters(QHBoxLayout *customFilterLayout)
{
    Q_UNUSED(customFilterLayout);
}

bool BaseTablePanelDialog::customFilterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceRow);
    Q_UNUSED(sourceParent);
    return true;
}

void BaseTablePanelDialog::handleRowSelected(int sourceRow)
{
    emit rowSelected(sourceRow);
}

void BaseTablePanelDialog::handleRowDoubleClicked(int sourceRow)
{
    emit rowDoubleClicked(sourceRow);
}

void BaseTablePanelDialog::onSearchTextChanged(const QString &text)
{
    if (m_proxyModel) {
        m_proxyModel->setSearchText(text);
        updateStatistics();
    }
}

void BaseTablePanelDialog::onTableCellClicked(const QModelIndex &index)
{
    if (!index.isValid() || !m_proxyModel) {
        return;
    }

    const QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    if (sourceIndex.isValid()) {
        handleRowSelected(sourceIndex.row());
    }
}

void BaseTablePanelDialog::onTableCellDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid() || !m_proxyModel) {
        return;
    }

    const QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    if (sourceIndex.isValid()) {
        handleRowDoubleClicked(sourceIndex.row());
    }
}

} // namespace GISApp::Core::Wrappers

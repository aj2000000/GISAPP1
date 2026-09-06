/**
 * @file SampleEntityTablePanelDialog.cpp
 * @brief Implementation of SampleEntityTablePanelDialog.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "SampleEntityTablePanelDialog.h"
#include "SampleEntityTableModel.h"

#include <QLabel>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

namespace GISApp::UI::SampleEntities {

/**
 * @brief Constructs SampleEntityTablePanelDialog with model and configurations.
 * @param[in] model Pointer to SampleEntityTableModel.
 * @param[in] parent Optional parent QWidget.
 */
SampleEntityTablePanelDialog::SampleEntityTablePanelDialog(
    GISApp::UIModels::SampleEntities::SampleEntityTableModel *model,
    QWidget *parent)
    : BaseTablePanelDialog(model, parent)
    , m_entityModel(model)
{
    setObjectName(QStringLiteral("SampleEntityTablePanelDialog"));
    setWindowTitle(tr("Sample Entities Table - GISLITE"));
    searchEdit()->setPlaceholderText(tr("🔍 Search name, ID, remarks..."));

    if (tableView()) {
        tableView()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tableView(), &QTableView::customContextMenuRequested,
                this, &SampleEntityTablePanelDialog::showTableContextMenu);
    }

    configureColumnWidths();
    updateStatistics();
}

/**
 * @brief Rebinds model and refreshes view.
 * @param[in] model Pointer to SampleEntityTableModel.
 */
void SampleEntityTablePanelDialog::setEntityModel(GISApp::UIModels::SampleEntities::SampleEntityTableModel *model)
{
    m_entityModel = model;
    setModel(model);
    configureColumnWidths();
    updateStatistics();
}

/**
 * @brief Formats column widths for clean readability.
 */
void SampleEntityTablePanelDialog::configureColumnWidths()
{
    if (!tableView()) return;

    QHeaderView *header = tableView()->horizontalHeader();
    if (!header) return;

    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(true);

    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnId, 80);
    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnName, 140);
    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnType, 80);
    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnLatitude, 110);
    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnLongitude, 110);
    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnHeight, 90);
    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnDirection, 90);
    tableView()->setColumnWidth(GISApp::UIModels::SampleEntities::SampleEntityTableModel::ColumnReportTime, 150);
}

/**
 * @brief Updates statistics badge in the toolbar.
 */
void SampleEntityTablePanelDialog::updateStatistics()
{
    if (!statusBadge()) return;

    const int total = m_entityModel ? m_entityModel->rowCount() : 0;
    const int filtered = m_proxyModel ? m_proxyModel->rowCount() : total;

    QString badgeText = tr("Total: %1").arg(total);
    if (filtered != total) {
        badgeText += tr(" | Filtered: %1").arg(filtered);
    }

    statusBadge()->setText(badgeText);
}

/**
 * @brief Handles single-click selection.
 * @param[in] sourceRow Source model row index.
 */
void SampleEntityTablePanelDialog::handleRowSelected(int sourceRow)
{
    if (!m_entityModel) return;

    const auto entity = m_entityModel->getEntityAt(sourceRow);
    emit entitySelected(static_cast<int>(entity.Id()));
}

/**
 * @brief Handles double-click navigation and shows details.
 * @param[in] sourceRow Source model row index.
 */
void SampleEntityTablePanelDialog::handleRowDoubleClicked(int sourceRow)
{
    if (!m_entityModel) return;

    const auto entity = m_entityModel->getEntityAt(sourceRow);
    const int entityId = static_cast<int>(entity.Id());
    emit entityDoubleClicked(entityId);
    emit requestEntityDetails(entityId);
    emit requestCenterOnEntity(entity.location().latatitude, entity.location().longitude);
}

/**
 * @brief Displays custom right-click context menu on table items.
 * @param[in] pos Mouse coordinate relative to table view viewport.
 */
void SampleEntityTablePanelDialog::showTableContextMenu(const QPoint &pos)
{
    if (!tableView() || !m_entityModel || !m_proxyModel) return;

    QModelIndex proxyIndex = tableView()->indexAt(pos);
    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) return;

    const auto entity = m_entityModel->getEntityAt(sourceIndex.row());
    const int entityId = static_cast<int>(entity.Id());
    const double lat = entity.location().latatitude;
    const double lon = entity.location().longitude;

    QMenu menu(this);
    menu.setObjectName(QStringLiteral("SampleEntityContextMenu"));

    QAction *detailsAct = menu.addAction(tr("📄 Show Details"));
    connect(detailsAct, &QAction::triggered, this, [this, entityId]() {
        emit requestEntityDetails(entityId);
    });

    QAction *centerAct = menu.addAction(tr("🎯 Center on Map"));
    connect(centerAct, &QAction::triggered, this, [this, lat, lon]() {
        emit requestCenterOnEntity(lat, lon);
    });

    QAction *copyCoordAct = menu.addAction(tr("📋 Copy Coordinates"));
    connect(copyCoordAct, &QAction::triggered, this, [lat, lon]() {
        QString coordText = QString::asprintf("%.6f, %.6f", lat, lon);
        QGuiApplication::clipboard()->setText(coordText);
    });

    menu.addSeparator();

    QAction *saveDbAct = menu.addAction(tr("💾 Persisted in SQLite DB"));
    saveDbAct->setEnabled(false);

    menu.exec(tableView()->viewport()->mapToGlobal(pos));
}

} // namespace GISApp::UI::SampleEntities

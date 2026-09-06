/**
 * @file ComplexEntityTablePanelDialog.cpp
 * @brief Implementation of ComplexEntityTablePanelDialog.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntityTablePanelDialog.h"
#include "ComplexEntityTableModel.h"

#include <QLabel>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

namespace GISApp::UI::ComplexEntities {

/**
 * @brief Constructs ComplexEntityTablePanelDialog with model and configurations.
 * @param[in] model Pointer to ComplexEntityTableModel.
 * @param[in] parent Optional parent QWidget.
 */
ComplexEntityTablePanelDialog::ComplexEntityTablePanelDialog(
    GISApp::UIModels::ComplexEntities::ComplexEntityTableModel *model,
    QWidget *parent)
    : BaseTablePanelDialog(model, parent)
    , m_entityModel(model)
{
    setObjectName(QStringLiteral("ComplexEntityTablePanelDialog"));
    setWindowTitle(tr("Complex Entities Table - GISLITE"));
    searchEdit()->setPlaceholderText(tr("🔍 Search name, ID, annotations, details..."));

    if (tableView()) {
        tableView()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tableView(), &QTableView::customContextMenuRequested,
                this, &ComplexEntityTablePanelDialog::showTableContextMenu);
    }

    configureColumnWidths();
    updateStatistics();
}

/**
 * @brief Rebinds model and refreshes view.
 * @param[in] model Pointer to ComplexEntityTableModel.
 */
void ComplexEntityTablePanelDialog::setEntityModel(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel *model)
{
    m_entityModel = model;
    setModel(model);
    configureColumnWidths();
    updateStatistics();
}

/**
 * @brief Formats column widths for clean readability across tactical workstations.
 */
void ComplexEntityTablePanelDialog::configureColumnWidths()
{
    if (!tableView()) return;

    QHeaderView *header = tableView()->horizontalHeader();
    if (!header) return;

    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(true);

    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnId, 75);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnName, 130);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnType, 90);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnPointsCount, 60);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnPrimaryCoords, 160);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnAnnotations, 150);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnSpecialParams, 120);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnDetailsCount, 60);
    tableView()->setColumnWidth(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel::ColumnReportTime, 145);
}

/**
 * @brief Updates statistics badge in the toolbar.
 */
void ComplexEntityTablePanelDialog::updateStatistics()
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
void ComplexEntityTablePanelDialog::handleRowSelected(int sourceRow)
{
    if (!m_entityModel) return;

    const auto entity = m_entityModel->getEntityAt(sourceRow);
    emit entitySelected(entity.Id());
}

/**
 * @brief Handles double-click navigation and shows details.
 * @param[in] sourceRow Source model row index.
 */
void ComplexEntityTablePanelDialog::handleRowDoubleClicked(int sourceRow)
{
    if (!m_entityModel) return;

    const auto entity = m_entityModel->getEntityAt(sourceRow);
    emit entityDoubleClicked(entity.Id());
    emit requestEntityDetails(entity.Id());

    const auto &pts = entity.locationPoints();
    if (!pts.isEmpty()) {
        emit requestCenterOnEntity(pts.first().latatitude, pts.first().longitude);
    }
}

/**
 * @brief Displays custom right-click context menu on table items.
 * @param[in] pos Mouse coordinate relative to table view viewport.
 */
void ComplexEntityTablePanelDialog::showTableContextMenu(const QPoint &pos)
{
    if (!tableView() || !m_entityModel || !m_proxyModel) return;

    QModelIndex proxyIndex = tableView()->indexAt(pos);
    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) return;

    const auto entity = m_entityModel->getEntityAt(sourceIndex.row());
    const quint32 entityId = entity.Id();

    double lat = 0.0;
    double lon = 0.0;
    const auto &pts = entity.locationPoints();
    if (!pts.isEmpty()) {
        lat = pts.first().latatitude;
        lon = pts.first().longitude;
    }

    QMenu menu(this);
    menu.setObjectName(QStringLiteral("ComplexEntityContextMenu"));

    QAction *detailsAct = menu.addAction(tr("📄 Show Details"));
    connect(detailsAct, &QAction::triggered, this, [this, entityId]() {
        emit requestEntityDetails(entityId);
    });

    if (!pts.isEmpty()) {
        QAction *centerAct = menu.addAction(tr("🎯 Center on Map"));
        connect(centerAct, &QAction::triggered, this, [this, lat, lon]() {
            emit requestCenterOnEntity(lat, lon);
        });

        QAction *copyCoordAct = menu.addAction(tr("📋 Copy Coordinates"));
        connect(copyCoordAct, &QAction::triggered, this, [lat, lon]() {
            QString coordText = QString::asprintf("%.6f, %.6f", lat, lon);
            QGuiApplication::clipboard()->setText(coordText);
        });
    }

    menu.addSeparator();

    QAction *saveDbAct = menu.addAction(tr("💾 Persisted in SQLite DB"));
    saveDbAct->setEnabled(false);

    menu.exec(tableView()->viewport()->mapToGlobal(pos));
}

} // namespace GISApp::UI::ComplexEntities

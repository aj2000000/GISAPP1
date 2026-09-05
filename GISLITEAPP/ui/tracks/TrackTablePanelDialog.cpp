/**
 * @file TrackTablePanelDialog.cpp
 * @brief Implementation of TrackTablePanelDialog specializing BaseTablePanelDialog.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "TrackTablePanelDialog.h"
#include "TrackTableModel.h"

#include <QComboBox>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>

namespace GISApp::UI::Tracks {

TrackTablePanelDialog::TrackTablePanelDialog(GISApp::UIModels::Tracks::TrackTableModel *model, QWidget *parent)
    : BaseTablePanelDialog(model, parent)
    , m_trackModel(model)
{
    setObjectName(QStringLiteral("TrackTablePanelDialog"));
    setWindowTitle(tr("Tactical Track Table - GISLITE"));
    searchEdit()->setPlaceholderText(tr("🔍 Search callsign, ID, remarks..."));

    configureColumnWidths();
    updateStatistics();
}

void TrackTablePanelDialog::setTrackModel(GISApp::UIModels::Tracks::TrackTableModel *model)
{
    m_trackModel = model;
    setModel(model);
    configureColumnWidths();
    updateStatistics();
}

void TrackTablePanelDialog::setupCustomFilters(QHBoxLayout *customFilterLayout)
{
    auto *affLabel = new QLabel(tr("Affiliation:"), this);
    affLabel->setObjectName(QStringLiteral("TableFilterLabel"));
    customFilterLayout->addWidget(affLabel);

    m_affiliationCombo = new QComboBox(this);
    m_affiliationCombo->addItems({tr("All"), tr("Hostile"), tr("Friendly"), tr("Neutral"), tr("Unknown")});
    connect(m_affiliationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TrackTablePanelDialog::onFilterDropdownChanged);
    customFilterLayout->addWidget(m_affiliationCombo);

    auto *domainLabel = new QLabel(tr("Domain:"), this);
    domainLabel->setObjectName(QStringLiteral("TableFilterLabel"));
    customFilterLayout->addWidget(domainLabel);

    m_domainCombo = new QComboBox(this);
    m_domainCombo->addItems({tr("All"), tr("Air"), tr("Surface"), tr("Land"), tr("Subsurface")});
    connect(m_domainCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TrackTablePanelDialog::onFilterDropdownChanged);
    customFilterLayout->addWidget(m_domainCombo);
}

void TrackTablePanelDialog::onFilterDropdownChanged()
{
    if (m_proxyModel) {
        m_proxyModel->invalidate();
    }
    updateStatistics();
}

bool TrackTablePanelDialog::customFilterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    if (!m_trackModel) {
        return true;
    }

    const auto track = m_trackModel->getTrackAt(sourceRow);

    // 1. Affiliation filter
    if (m_affiliationCombo) {
        const QString aff = m_affiliationCombo->currentText();
        if (!aff.isEmpty() && aff != QStringLiteral("All")) {
            if (track.identityString().compare(aff, Qt::CaseInsensitive) != 0) {
                return false;
            }
        }
    }

    // 2. Domain filter
    if (m_domainCombo) {
        const QString dom = m_domainCombo->currentText();
        if (!dom.isEmpty() && dom != QStringLiteral("All")) {
            if (track.domainString().compare(dom, Qt::CaseInsensitive) != 0) {
                return false;
            }
        }
    }

    return true;
}

void TrackTablePanelDialog::handleRowSelected(int sourceRow)
{
    if (!m_trackModel) {
        return;
    }

    const auto track = m_trackModel->getTrackAt(sourceRow);
    if (track.trackId() > 0) {
        qDebug() << "[TrackTablePanelDialog] Row clicked for track" << track.trackId()
                 << "at coordinates:" << track.latitude() << track.longitude();
        emit trackSelected(track.latitude(), track.longitude());
    }
}

void TrackTablePanelDialog::handleRowDoubleClicked(int sourceRow)
{
    if (!m_trackModel) {
        return;
    }

    const auto track = m_trackModel->getTrackAt(sourceRow);
    if (track.trackId() > 0) {
        qDebug() << "[TrackTablePanelDialog] Row double-clicked for track" << track.trackId()
                 << "at coordinates:" << track.latitude() << track.longitude();
        emit trackSelected(track.latitude(), track.longitude());
    }
}

void TrackTablePanelDialog::updateStatistics()
{
    if (!m_trackModel || !statusBadge()) {
        return;
    }

    const auto &allTracks = m_trackModel->tracks();
    int total = allTracks.size();
    int hostiles = 0;
    int friendlies = 0;
    int neutrals = 0;

    for (const auto &t : allTracks) {
        switch (t.identity()) {
        case GISApp::Domain::Tracks::TrackIdentity::Hostile:
            hostiles++;
            break;
        case GISApp::Domain::Tracks::TrackIdentity::Friendly:
            friendlies++;
            break;
        case GISApp::Domain::Tracks::TrackIdentity::Neutral:
            neutrals++;
            break;
        default:
            break;
        }
    }

    int visibleCount = m_proxyModel ? m_proxyModel->rowCount() : total;

    statusBadge()->setText(QStringLiteral(
        "Showing: <b>%1</b>/%2 | <span style='color:#ff4d4f;'>Hostile: %3</span> | "
        "<span style='color:#40a9ff;'>Friendly: %4</span> | <span style='color:#73d13d;'>Neutral: %5</span>"
    ).arg(visibleCount).arg(total).arg(hostiles).arg(friendlies).arg(neutrals));
}

void TrackTablePanelDialog::refreshData()
{
    if (m_trackModel) {
        m_trackModel->reloadTracks();
    }
    updateStatistics();
}

void TrackTablePanelDialog::configureColumnWidths()
{
    if (!m_tableView) {
        return;
    }

    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnId, 60);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnCallsign, 110);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnAffiliation, 95);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnDomain, 85);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnLatitude, 105);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnLongitude, 105);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnAltitude, 85);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnSpeed, 90);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnHeading, 75);
    m_tableView->setColumnWidth(GISApp::UIModels::Tracks::TrackTableModel::ColumnReportTime, 95);

    m_tableView->sortByColumn(GISApp::UIModels::Tracks::TrackTableModel::ColumnId, Qt::AscendingOrder);
}

} // namespace GISApp::UI::Tracks

/**
 * @file TrackController.cpp
 * @brief Implementation of TrackController orchestrating tactical track workflows and context menus.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "TrackController.h"
#include "TacticalTrackService.h"
#include "TrackMapRenderer.h"
#include "TrackMapFeatureAdapter.h"
#include "IMapFeature.h"
#include "MapController.h"
#include "MapWidget.h"
#include "fieldkeyvaluemapper.h"
#include "TrackDetailDialog.h"

#include <QMapLibre/Map>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <cmath>
#include <algorithm>

namespace GISApp::Controllers::Tracks {

TrackController::TrackController(GISApp::Services::Tracks::TacticalTrackService *trackService,
                                 GISApp::UI::Renderers::TrackMapRenderer *renderer,
                                 GISApp::Controllers::MapController *mapController,
                                 QObject *parent)
    : QObject(parent)
    , m_trackService(trackService)
    , m_renderer(renderer)
    , m_mapController(mapController)
{
    setupConnections();
}

TrackController::~TrackController()
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->unregisterContextMenuContributor(this);
    }
}

void TrackController::setupConnections()
{
    if (m_trackService) {
        connect(m_trackService, &GISApp::Services::Tracks::TacticalTrackService::tracksUpdated,
                this, &TrackController::onTracksUpdated);
        connect(m_trackService, &GISApp::Services::Tracks::TacticalTrackService::trackUpdated,
                this, &TrackController::onTrackUpdated);
        connect(m_trackService, &GISApp::Services::Tracks::TacticalTrackService::trackRemoved,
                this, &TrackController::onTrackRemoved);
    }

    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
    }
}

void TrackController::setTrackMapRenderer(GISApp::UI::Renderers::TrackMapRenderer *renderer)
{
    m_renderer = renderer;
    if (m_renderer && m_trackService) {
        onTracksUpdated(m_trackService->getAllTracks());
    }
}

void TrackController::setMapController(GISApp::Controllers::MapController *mapController)
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->unregisterContextMenuContributor(this);
    }

    m_mapController = mapController;

    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
    }
}

void TrackController::initialize()
{
    if (m_mapController && m_mapController->attachedMap()) {
        m_mapController->attachedMap()->registerContextMenuContributor(this);
    }

    if (m_renderer && m_trackService) {
        onTracksUpdated(m_trackService->getAllTracks());
    }
}

void TrackController::onTracksUpdated(const QVector<GISApp::Domain::Tracks::TacticalTrack> &tracks)
{
    if (!m_renderer) {
        return;
    }

    QVector<GISApp::UI::Renderers::TrackMapFeatureAdapter> adapters;
    adapters.reserve(tracks.size());
    for (const auto &track : tracks) {
        adapters.emplace_back(track);
    }

    QVector<const GISApp::Core::Interfaces::IMapFeature*> features;
    features.reserve(adapters.size());
    for (const auto &adapter : adapters) {
        features.append(&adapter);
    }

    m_renderer->renderFeatures(features);
}

bool TrackController::calculateTracksCenter(double &outLat, double &outLon, double &outZoom) const
{
    if (!m_trackService) {
        return false;
    }
    const auto tracks = m_trackService->getAllTracks();
    if (tracks.isEmpty()) {
        return false;
    }

    double sumLat = 0.0;
    double sumLon = 0.0;
    for (const auto &t : tracks) {
        sumLat += t.latatitude();
        sumLon += t.longitude();
    }
    outLat = sumLat / tracks.size();
    outLon = sumLon / tracks.size();
    outZoom = 9.5;
    return true;
}

void TrackController::setTracksVisible(bool visible)
{
    if (m_renderer) {
        m_renderer->setTracksVisible(visible);
    }
    emit trackVisibilityChanged(visible);
    qDebug() << "[TrackController] Delegated tactical tracks visibility:" << visible;
}

bool TrackController::isTracksVisible() const
{
    return m_renderer ? m_renderer->isTracksVisible() : true;
}

void TrackController::onTrackSelected(double latitude, double longitude)
{
    if (m_mapController) {
        qDebug() << "[TrackController] Navigating camera to selected track at:" << latitude << longitude;
        m_mapController->setCenter(latitude, longitude);
    }
}

bool TrackController::contributeActions(QMenu *parentMenu, const QPoint &screenPos, const QPointF &geoCoord)
{
    if (!parentMenu || !m_trackService || !isTracksVisible()) {
        return false;
    }

    const auto tracks = m_trackService->getAllTracks();
    if (tracks.isEmpty()) {
        return false;
    }

    // Access native MapLibre Map core if attached for precise viewport pixel projection
    QMapLibre::Map *mapCore = nullptr;
    if (m_mapController && m_mapController->attachedMap()) {
        mapCore = m_mapController->attachedMap()->rawMap();
    }

    struct TrackHit {
        Domain::Tracks::TacticalTrack track;
        double distancePx;
    };
    QVector<TrackHit> hits;

    constexpr double hitRadiusPx = 22.0; // 22 pixels tolerance around cursor

    for (const auto &trk : tracks) {
        double dist = -1.0;
        if (mapCore) {
            QPointF p = mapCore->pixelForCoordinate(QMapLibre::Coordinate(trk.latatitude(), trk.longitude()));
            double dx = p.x() - screenPos.x();
            double dy = p.y() - screenPos.y();
            dist = std::sqrt(dx * dx + dy * dy);
        } else {
            // Fallback to geographic angular delta converted to approximate screen scale
            double dLat = trk.latatitude() - geoCoord.x();
            double dLon = trk.longitude() - geoCoord.y();
            dist = std::sqrt(dLat * dLat + dLon * dLon) * 500.0;
        }

        if (dist >= 0.0 && dist <= hitRadiusPx) {
            hits.append({trk, dist});
        }
    }

    if (hits.isEmpty()) {
        return false;
    }

    // Sort hits closest to cursor first
    std::sort(hits.begin(), hits.end(), [](const TrackHit &a, const TrackHit &b) {
        return a.distancePx < b.distancePx;
    });

    auto &mapper = FieldKeyValueMapper::instance();

    // Render every hit entity as its own dedicated sub-menu
    for (const auto &hit : hits) {
        const auto &track = hit.track;
        int trackId = track.trackId();
        QString identStr = mapper.trackIdentityMapping(track.identity());
        QString icon = (track.identity() == HOSTILE) ? QStringLiteral("🔴") :
                       (track.identity() == FRIENDLY) ? QStringLiteral("🔵") :
                       (track.identity() == 3) ? QStringLiteral("🟢") : QStringLiteral("🟡");

        // Level 2: Entity Sub-Menu on the Right-Click Menu
        QMenu *entityMenu = parentMenu->addMenu(QStringLiteral("%1 %2 (ID: %3 • %4)")
                            .arg(icon)
                            .arg(track.trackName().isEmpty() ? QStringLiteral("TRK-%1").arg(trackId) : track.trackName())
                            .arg(trackId)
                            .arg(identStr));

        // Level 3 Items (Sub-sub menu of the right-click menu, inside entity sub-menu):
        // 1. Show Details
        QAction *detailsAct = entityMenu->addAction(QStringLiteral("📄 Show Details"));
        connect(detailsAct, &QAction::triggered, this, [this, trackId]() {
            showTrackDetails(trackId);
        });

        // 2. Edit Track (Placeholder only)
        QAction *editAct = entityMenu->addAction(QStringLiteral("✏️ Edit Track (Placeholder)"));
        editAct->setEnabled(false);
        editAct->setToolTip(QStringLiteral("Track editing is currently disabled"));

        // 3. Delete Track (Placeholder only)
        QAction *deleteAct = entityMenu->addAction(QStringLiteral("🗑️ Delete Track (Placeholder)"));
        deleteAct->setEnabled(false);
        deleteAct->setToolTip(QStringLiteral("Track deletion is currently disabled"));

        entityMenu->addSeparator();

        // 4. Center on Map
        QAction *centerAct = entityMenu->addAction(QStringLiteral("🎯 Center on Map"));
        double tLat = track.latatitude();
        double tLon = track.longitude();
        connect(centerAct, &QAction::triggered, this, [this, tLat, tLon]() {
            onTrackSelected(tLat, tLon);
        });
    }

    return true;
}

void TrackController::showTrackDetails(int trackId)
{
    if (!m_trackService) {
        return;
    }

    // Single-instance enforcement: If inspector dialog is already active, raise and focus it
    if (m_detailDialogs.contains(trackId) && m_detailDialogs[trackId]) {
        m_detailDialogs[trackId]->raise();
        m_detailDialogs[trackId]->activateWindow();
        return;
    }

    auto opt = m_trackService->getTrackById(trackId);
    if (!opt.has_value()) {
        return;
    }

    // Instantiate dialog as a pure presentation view without repository coupling
    auto *dialog = new GISApp::UI::Tracks::TrackDetailDialog(opt.value());
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // Clean up cached dialog reference upon user close
    connect(dialog, &QObject::destroyed, this, [this, trackId]() {
        m_detailDialogs.remove(trackId);
    });

    m_detailDialogs.insert(trackId, dialog);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void TrackController::onTrackUpdated(const GISApp::Domain::Tracks::TacticalTrack &track)
{
    int trackId = track.trackId();
    if (m_detailDialogs.contains(trackId) && m_detailDialogs[trackId]) {
        m_detailDialogs[trackId]->updateTrackData(track);
    }
}

void TrackController::onTrackRemoved(int trackId)
{
    if (m_detailDialogs.contains(trackId) && m_detailDialogs[trackId]) {
        m_detailDialogs[trackId]->close();
        m_detailDialogs.remove(trackId);
    }
}

void TrackController::centerOnTrack(int trackId)
{
    if (!m_trackService || !m_mapController) {
        return;
    }
    auto opt = m_trackService->getTrackById(trackId);
    if (opt.has_value()) {
        m_mapController->setCenter(opt->latatitude(), opt->longitude());
    }
}

void TrackController::deleteTrack(int trackId)
{
    if (m_trackService) {
        m_trackService->deleteTrack(trackId);
    }
}

} // namespace GISApp::Controllers::Tracks

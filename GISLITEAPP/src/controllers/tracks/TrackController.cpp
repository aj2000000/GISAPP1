/**
 * @file TrackController.cpp
 * @brief Implementation of TrackController orchestrating tactical track workflows.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "TrackController.h"
#include "TacticalTrackService.h"
#include "TrackMapRenderer.h"
#include "MapController.h"

#include <QDebug>

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

void TrackController::setupConnections()
{
    if (m_trackService) {
        connect(m_trackService, &GISApp::Services::Tracks::TacticalTrackService::geoJsonUpdated,
                this, &TrackController::onGeoJsonUpdated);
    }
}

void TrackController::setTrackMapRenderer(GISApp::UI::Renderers::TrackMapRenderer *renderer)
{
    m_renderer = renderer;
    if (m_renderer && m_trackService) {
        m_renderer->renderTracks(m_trackService->getTracksAsGeoJson());
    }
}

void TrackController::setMapController(GISApp::Controllers::MapController *mapController)
{
    m_mapController = mapController;
}

void TrackController::initialize()
{
    if (m_renderer && m_trackService) {
        m_renderer->renderTracks(m_trackService->getTracksAsGeoJson());
    }
}

void TrackController::onGeoJsonUpdated(const QByteArray &geoJsonData)
{
    if (m_renderer) {
        m_renderer->renderTracks(geoJsonData);
    }
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

} // namespace GISApp::Controllers::Tracks

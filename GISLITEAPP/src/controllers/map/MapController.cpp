/**
 * @file MapController.cpp
 * @brief Implementation of MapController camera orchestration and telemetry dispatch.
 */

#include "MapController.h"
#include "MapWidget.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GISApp::Controllers {

MapController::MapController(QObject *parent)
    : QObject(parent)
    , m_mapWidget(nullptr)
    , m_currentLatitude(28.6139)
{
}

MapController::MapController(GISApp::UI::MapWidget *mapWidget, QObject *parent)
    : QObject(parent)
    , m_mapWidget(nullptr)
    , m_currentLatitude(28.6139)
{
    attachMap(mapWidget);
}

void MapController::attachMap(GISApp::UI::MapWidget *mapWidget)
{
    if (m_mapWidget == mapWidget) return;

    m_mapWidget = mapWidget;
    if (m_mapWidget) {
        connectMapSignals();
        syncTelemetry();
    }
}

void MapController::syncTelemetry()
{
    if (!m_mapWidget) return;

    GISApp::Core::Models::GeoCoordinate centerCoord = m_mapWidget->center();
    if (centerCoord.isValid()) {
        m_currentLatitude = centerCoord.latitude();
        emit coordinateUpdated(centerCoord);
        emit coordinateUpdatedRaw(centerCoord.latitude(), centerCoord.longitude(), centerCoord.altitude());
    } else {
        emit coordinateUpdated(GISApp::Core::Models::GeoCoordinate(28.6139, 77.2090, 0.0, true));
        emit coordinateUpdatedRaw(28.6139, 77.2090, 0.0);
    }

    double currentZoom = m_mapWidget->zoom();
    double currentScale = calculateScaleDenominator(currentZoom, m_currentLatitude);
    emit zoomAndScaleUpdated(currentZoom, currentScale);
}

void MapController::connectMapSignals()
{
    if (!m_mapWidget) return;

    connect(m_mapWidget, &GISApp::UI::MapWidget::coordinateHovered,
            this, &MapController::onCoordinateHovered);

    connect(m_mapWidget, &GISApp::UI::MapWidget::zoomChanged,
            this, &MapController::onZoomChanged);

    connect(m_mapWidget, &GISApp::UI::MapWidget::bearingChanged,
            this, &MapController::onBearingChanged);
}

void MapController::onCoordinateHovered(const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (!coord.isValid()) return;

    m_currentLatitude = coord.latitude();
    emit coordinateUpdated(coord);
    emit coordinateUpdatedRaw(coord.latitude(), coord.longitude(), coord.altitude());

    // Update zoom and geodetic representative scale denominator on every mouse hover
    double currentZoom = m_mapWidget ? m_mapWidget->zoom() : 4.0;
    double currentScale = calculateScaleDenominator(currentZoom, m_currentLatitude);
    emit zoomAndScaleUpdated(currentZoom, currentScale);
}

void MapController::onZoomChanged(double zoom)
{
    double scale = calculateScaleDenominator(zoom, m_currentLatitude);
    emit zoomAndScaleUpdated(zoom, scale);
    emit statusNotification(QString("Map Zoom: %1").arg(zoom, 0, 'f', 2), 1000);
}

void MapController::onBearingChanged(double bearing)
{
    emit statusNotification(QString("Bearing: %1°").arg(bearing, 0, 'f', 1), 1000);
}

void MapController::zoomIn(double delta)
{
    if (m_mapWidget) {
        m_mapWidget->zoomIn(delta);
    }
}

void MapController::zoomOut(double delta)
{
    if (m_mapWidget) {
        m_mapWidget->zoomOut(delta);
    }
}

void MapController::setZoom(double zoom)
{
    if (m_mapWidget) {
        m_mapWidget->setZoom(zoom);
    }
}

void MapController::resetBearing()
{
    if (m_mapWidget) {
        m_mapWidget->resetBearing();
        emit statusNotification(tr("Map Bearing Reset to North"), 1500);
    }
}

void MapController::setCenter(double latitude, double longitude)
{
    if (m_mapWidget) {
        m_mapWidget->setCenter(latitude, longitude);
    }
}

void MapController::setCenter(const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (m_mapWidget) {
        m_mapWidget->setCenter(coord);
    }
}

double MapController::calculateScaleDenominator(double zoomLevel, double latitude) const
{
    // Standard OGC Web Mercator scale at latitude:
    // Earth circumference at equator ≈ 40,075,016.686 meters.
    // At standard screen DPI (96 dpi, 0.28 mm/pixel):
    // Equatorial scale at zoom 0 ≈ 559,082,264.
    double latRad = latitude * M_PI / 180.0;
    double cosLat = std::cos(latRad);
    if (cosLat < 0.001) cosLat = 0.001; // Guard against polar singularities

    return (559082264.028 / std::pow(2.0, zoomLevel)) * cosLat;
}

} // namespace GISApp::Controllers

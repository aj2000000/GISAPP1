/**
 * @file MapController.h
 * @brief Header definition for MapController camera orchestration and telemetry service.
 */

#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include <QString>
#include "GeoCoordinate.h"

namespace GISApp::UI {
class MapWidget;
}

namespace GISApp::Controllers {

/**
 * @class MapController
 * @brief Orchestrates GIS camera navigation, coordinate conversions, and telemetry dispatch.
 *
 * MapController acts as an architectural Controller in the Model-View-Controller (MVC)
 * pattern. It decouples the UI widgets (such as ZoomControlsWidget, MainBaseUI, and TacticalStatusBar)
 * from the low-level rendering mechanics of MapWidget.
 *
 * Responsibilities:
 * - Executes camera movements (zooming, panning, rotating/bearing resets).
 * - Translates raw viewport zoom levels into standard OGC Web Mercator scale denominators.
 * - Dispatches normalized coordinate and telemetry notifications to listening components.
 */
class MapController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an unattached MapController.
     * @param[in] parent Optional parent QObject for Qt memory management.
     */
    explicit MapController(QObject *parent = nullptr);

    /**
     * @brief Constructs a MapController and immediately binds to the given MapWidget.
     * @param[in] mapWidget Pointer to MapWidget instance to control.
     * @param[in] parent Optional parent QObject.
     */
    explicit MapController(GISApp::UI::MapWidget *mapWidget, QObject *parent = nullptr);

    /**
     * @brief Virtual destructor ensuring clean disconnection of map signals.
     */
    virtual ~MapController() override = default;

    /**
     * @brief Attaches a target MapWidget and connects internal event listeners.
     * @param[in] mapWidget Pointer to target MapWidget. Can be nullptr to detach.
     */
    void attachMap(GISApp::UI::MapWidget *mapWidget);

    /**
     * @brief Returns the currently attached MapWidget instance.
     * @return Pointer to attached MapWidget, or nullptr if unattached.
     */
    [[nodiscard]] GISApp::UI::MapWidget* attachedMap() const { return m_mapWidget; }

    /**
     * @brief Instructs the attached map to perform a zoom-in step.
     * @param[in] delta Zoom delta step (default: 1.0).
     */
    void zoomIn(double delta = 1.0);

    /**
     * @brief Instructs the attached map to perform a zoom-out step.
     * @param[in] delta Zoom delta step (default: 1.0).
     */
    void zoomOut(double delta = 1.0);

    /**
     * @brief Sets absolute camera zoom level.
     * @param[in] zoom Target zoom level (clamped to non-negative).
     */
    void setZoom(double zoom);

    /**
     * @brief Resets map rotation bearing back to true North (0.0°).
     */
    void resetBearing();

    /**
     * @brief Re-centers camera onto specific geodetic coordinates.
     * @param[in] latitude Target latitude in decimal degrees [-90.0, 90.0].
     * @param[in] longitude Target longitude in decimal degrees [-180.0, 180.0].
     */
    void setCenter(double latitude, double longitude);

    /**
     * @brief Re-centers camera using domain GeoCoordinate value object.
     * @param[in] coord Domain GeoCoordinate value object.
     */
    void setCenter(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Calculates the standard OGC Web Mercator scale denominator at given zoom and latitude.
     * @param[in] zoomLevel Current camera zoom level.
     * @param[in] latitude Latitude in decimal degrees (scale varies with latitude on Mercator).
     * @return Scale denominator S (e.g. 500,000 for a 1:500,000 map scale).
     * @note Uses Earth circumference 40,075,016.686m and standard 96 DPI screen assumption (0.28 mm/pixel).
     */
    [[nodiscard]] double calculateScaleDenominator(double zoomLevel, double latitude = 0.0) const;

    /**
     * @brief Forces an immediate broadcast of current map coordinates, zoom, and calculated scale.
     * Synchronizes UI status widgets (such as TacticalStatusBar) with the active map state.
     */
    void syncTelemetry();

signals:
    /**
     * @brief Emitted when cursor coordinates update, packed in a GeoCoordinate.
     * @param[in] coord Valid GeoCoordinate object.
     */
    void coordinateUpdated(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Emitted when cursor coordinates update with raw geodetic numbers.
     * @param[in] latitude Latitude in decimal degrees [-90.0, 90.0].
     * @param[in] longitude Longitude in decimal degrees [-180.0, 180.0].
     * @param[in] altitude Altitude in meters above mean sea level (MSL).
     */
    void coordinateUpdatedRaw(double latitude, double longitude, double altitude);

    /**
     * @brief Emitted when camera zoom changes, providing computed scale denominator.
     * @param[in] zoom New zoom level.
     * @param[in] scaleDenominator Computed Mercator representative fraction denominator (1:scale).
     */
    void zoomAndScaleUpdated(double zoom, double scaleDenominator);

    /**
     * @brief Emitted when a user status notification should be displayed on the UI.
     * @param[in] message User-friendly notification string.
     * @param[in] timeout Display timeout in milliseconds.
     */
    void statusNotification(const QString &message, int timeout = 2000);

private slots:
    /**
     * @brief Internal slot receiving coordinateHovered signals from MapWidget.
     * @param[in] coord Domain GeoCoordinate from map canvas.
     */
    void onCoordinateHovered(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Internal slot receiving zoomChanged signals from MapWidget.
     * @param[in] zoom Updated zoom level.
     */
    void onZoomChanged(double zoom);

    /**
     * @brief Internal slot receiving bearingChanged signals from MapWidget.
     * @param[in] bearing Updated bearing angle in degrees.
     */
    void onBearingChanged(double bearing);

private:
    /**
     * @brief Helper to connect Qt signals from attached MapWidget.
     */
    void connectMapSignals();

    /// Pointer to attached MapWidget instance (not owned by MapController)
    GISApp::UI::MapWidget *m_mapWidget;

    /// Last known latitude used to refine Mercator scale calculations
    double m_currentLatitude;
};

} // namespace GISApp::Controllers

#endif // MAPCONTROLLER_H

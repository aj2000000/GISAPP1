/**
 * @file MapWidget.h
 * @brief Header definition for MapWidget wrapping MapLibre Native Qt QRhiWidget.
 */

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QString>
#include "GeoCoordinate.h"

// Forward declare MapLibre classes to reduce compilation coupling
namespace QMapLibre {
class MapWidget;
class Map;
class Settings;
}

namespace GISApp::UI {

/**
 * @class MapWidget
 * @brief Hardware-accelerated GIS map canvas wrapping MapLibre Native Qt (QRhiWidget).
 *
 * MapWidget encapsulates the low-level QMapLibre::MapWidget and its associated QMapLibre::Map
 * rendering pipeline. It bridges domain-level GIS representations (such as GeoCoordinate)
 * with MapLibre's internal types.
 *
 * Key Responsibilities:
 * - Initializes MapLibre settings with offline and online style fallback logic.
 * - Exposes high-level camera navigation APIs (zoom, pan, tilt/pitch, bearing).
 * - Intercepts native input events and emits domain-friendly telemetry signals (coordinateHovered, zoomChanged).
 * - Interacts with MapController for presentation logic and TacticalStatusBar for telemetry display.
 */
class MapWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the MapWidget, configures initial settings, and loads base map style.
     * @param[in] parent Optional parent QWidget; ownership is managed by Qt parent-child hierarchy.
     */
    explicit MapWidget(QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor releasing MapLibre resources.
     */
    virtual ~MapWidget() override;

    /**
     * @brief Increments current map camera zoom level.
     * @param[in] delta Zoom delta step (default: 1.0). Must be positive.
     */
    void zoomIn(double delta = 1.0);

    /**
     * @brief Decrements current map camera zoom level.
     * @param[in] delta Zoom delta step (default: 1.0). Must be positive.
     */
    void zoomOut(double delta = 1.0);

    /**
     * @brief Sets absolute camera zoom level.
     * @param[in] zoom Target zoom level (typically between 0.0 and 22.0). Clamped to non-negative.
     */
    void setZoom(double zoom);

    /**
     * @brief Retrieves the current camera zoom level.
     * @return Current zoom level as double.
     */
    [[nodiscard]] double zoom() const;

    /**
     * @brief Sets the camera rotation bearing in clockwise degrees from true North.
     * @param[in] degrees Rotation angle in degrees [0.0, 360.0).
     */
    void setBearing(double degrees);

    /**
     * @brief Retrieves current camera rotation bearing.
     * @return Clockwise bearing degrees relative to North.
     */
    [[nodiscard]] double bearing() const;

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
     * @param[in] coord Valid GeoCoordinate instance containing target latitude/longitude.
     */
    void setCenter(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Retrieves current geodetic center coordinate of the map viewport.
     * @return Valid GISApp::Core::Models::GeoCoordinate representing camera focus center.
     */
    [[nodiscard]] GISApp::Core::Models::GeoCoordinate center() const;

    /**
     * @brief Sets style via remote or local file URL (e.g., https://... or file://...).
     * @param[in] url Style URL pointing to MapLibre style JSON specification.
     */
    void setStyleUrl(const QString &url);

    /**
     * @brief Sets style directly via raw JSON string.
     * @param[in] json Full MapLibre Style Specification JSON string.
     */
    void setStyleJson(const QString &json);

    /**
     * @brief Retrieves active style URL.
     * @return Current style URL string, or empty if set via JSON.
     */
    [[nodiscard]] QString styleUrl() const;

    /**
     * @brief Retrieves active style JSON content.
     * @return Current style JSON string, or empty if set via URL.
     */
    [[nodiscard]] QString styleJson() const;

    /**
     * @brief Provides direct access to the underlying MapLibre Map core object.
     * @return Pointer to QMapLibre::Map instance, or nullptr if not initialized.
     * @warning Direct manipulation bypasses MapWidget change tracking.
     */
    [[nodiscard]] QMapLibre::Map* rawMap() const;

    /**
     * @brief Provides access to the wrapped QMapLibre::MapWidget QRhiWidget.
     * @return Pointer to QMapLibre::MapWidget instance, or nullptr.
     */
    [[nodiscard]] QMapLibre::MapWidget* nativeWidget() const;

signals:
    /**
     * @brief Emitted when mouse cursor moves across map viewport, packed as domain GeoCoordinate.
     * @param[in] coord Domain GeoCoordinate value object with valid lat/lon.
     */
    void coordinateHovered(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Emitted when mouse cursor moves across map viewport with raw floating point coordinates.
     * @param[in] latitude Geodetic latitude [-90.0, 90.0].
     * @param[in] longitude Geodetic longitude [-180.0, 180.0].
     */
    void coordinateHoveredRaw(double latitude, double longitude);

    /**
     * @brief Emitted when user clicks on map viewport.
     * @param[in] coord Domain GeoCoordinate of the clicked location.
     */
    void coordinateClicked(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Emitted when camera zoom level changes.
     * @param[in] zoom New zoom level value.
     */
    void zoomChanged(double zoom);

    /**
     * @brief Emitted when camera bearing angle changes.
     * @param[in] bearing New bearing degrees [0.0, 360.0).
     */
    void bearingChanged(double bearing);

    /**
     * @brief Emitted on internal MapLibre map state changes (loading, rendering, tiles).
     * @param[in] changeType Integer representation of QMapLibre::Map::MapChange enum.
     */
    void mapChanged(int changeType);

    /**
     * @brief Emitted once the underlying QMapLibre::Map core instance is initialized and ready.
     */
    void mapReady();

protected:
    /**
     * @brief Intercepts events from child widgets to capture hover coordinates.
     * @param[in] watched Pointer to watched QObject (e.g. m_nativeMapWidget).
     * @param[in] event Event to inspect.
     * @return True if event was consumed, false to allow default handling.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief Handles mouse movement over the container widget when mouse tracking is active.
     * @param[in] event Mouse event containing canvas coordinates.
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief Handles widget display events to finalize map initialization.
     * @param[in] event Show event.
     */
    void showEvent(QShowEvent *event) override;

private slots:
    /**
     * @brief Internal slot handling notifications from QMapLibre::Map::mapChanged.
     * @param[in] change Map change enum code.
     */
    void onMapLibreChange(int change);

private:
    /**
     * @brief Checks if native QMapLibre::Map core is created, binds change signals, and loads initial style.
     */
    void checkMapReady();

    /**
     * @brief Resolves geodetic coordinates for a viewport pixel point and emits telemetry signals.
     * @param[in] pos Screen pixel position relative to the map canvas.
     */
    void handleHoverPosition(const QPointF &pos);

    /**
     * @brief Instantiates QMapLibre::Settings and QMapLibre::MapWidget and binds event signals.
     */
    void setupMapLibre();

    /**
     * @brief Evaluates offline fallback JSON vs online URL and initializes base style.
     */
    void loadInitialStyle();

    /// Wrapped native MapLibre QRhiWidget instance
    QMapLibre::MapWidget *m_nativeMapWidget{nullptr};

    /// Cached zoom level used to throttle redundant zoomChanged signal emissions
    double m_lastReportedZoom{4.0};

    /// Cached bearing degrees used to throttle redundant bearingChanged signal emissions
    double m_lastReportedBearing{0.0};

    /// Flag indicating whether MapLibre core instance has been connected
    bool m_mapInitialized{false};

    /// Polling timer to detect when Map core is instantiated by QRhiWidget
    class QTimer *m_initTimer{nullptr};
};

} // namespace GISApp::UI

#endif // MAPWIDGET_H

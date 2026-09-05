/**
 * @file TacticalStatusBar.h
 * @brief Header definition for TacticalStatusBar 3D geodetic telemetry bar.
 */

#ifndef TACTICALSTATUSBAR_H
#define TACTICALSTATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include <QString>
#include "GeoCoordinate.h"

namespace GISApp::UI {

/**
 * @class TacticalStatusBar
 * @brief Dedicated telemetry status bar positioned directly above the standard QStatusBar.
 *
 * TacticalStatusBar renders real-time mission telemetry including:
 * - Spatial Reference System (SRS) identifier (e.g. WGS 84 / EPSG:4326).
 * - Current camera zoom level and calculated Mercator representative fraction scale (1:N).
 * - Geodetic latitude, longitude with cardinal directions (N/S, E/W), and elevation/altitude.
 *
 * It collaborates with MapController to reflect live cursor hover and navigation telemetry.
 */
class TacticalStatusBar : public QStatusBar
{
    Q_OBJECT

public:
    /**
     * @brief Constructs TacticalStatusBar and sets up telemetry label layouts.
     * @param[in] parent Optional parent QWidget.
     */
    explicit TacticalStatusBar(QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor for clean teardown.
     */
    virtual ~TacticalStatusBar() override = default;

    /**
     * @brief Updates the right-aligned geodetic coordinate readout from a GeoCoordinate object.
     * @param[in] coord Domain GeoCoordinate value object containing latitude, longitude, and altitude.
     */
    void updateCoordinates(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Updates the geodetic coordinate readout using discrete scalar parameters.
     * @param[in] latitude Latitude in decimal degrees [-90.0, 90.0].
     * @param[in] longitude Longitude in decimal degrees [-180.0, 180.0].
     * @param[in] altitude Altitude in meters above mean sea level (default: 0.0).
     */
    void updateCoordinates(double latitude, double longitude, double altitude = 0.0);

    /**
     * @brief Updates the scale denominator readout on the telemetry information label.
     * @param[in] scaleDenominator Scale denominator value (e.g. 500,000 for 1:500,000).
     */
    void updateScale(double scaleDenominator);

    /**
     * @brief Atomically updates both zoom level and scale denominator readouts.
     * @param[in] zoomLevel Current map zoom level.
     * @param[in] scaleDenominator Calculated Mercator scale denominator.
     */
    void updateZoomAndScale(double zoomLevel, double scaleDenominator);

private:
    /// Left-aligned label displaying spatial reference system, zoom level, and map scale
    QLabel *m_infoLabel;

    /// Right-aligned fixed-width label displaying latitude, longitude, and altitude
    QLabel *m_coordLabel;
};

} // namespace GISApp::UI

#endif // TACTICALSTATUSBAR_H

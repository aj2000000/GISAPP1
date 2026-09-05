/**
 * @file TrackMapRenderer.h
 * @brief MapLibre Native GPU layer renderer for tactical track visualization.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef TRACKMAPRENDERER_H
#define TRACKMAPRENDERER_H

#include "BaseMapFeatureRenderer.h"

namespace GISApp::UI::Renderers {

/**
 * @class TrackMapRenderer
 * @brief Specializes BaseMapFeatureRenderer for high-performance tactical track visualization.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the Presentation / View layer (ui/map/renderers/).
 * - Specializes GISApp::Core::Wrappers::BaseMapFeatureRenderer for tactical track entities.
 * - Leverages GPU Data-Driven Styling: renders tactical track entities with dynamic colors
 *   (Hostile = #ff3344, Friendly = #00d2ff, Neutral = #00e676, Unknown = #ffd600),
 *   glow halos, and callsign labels evaluated directly on MapLibre shaders.
 */
class TrackMapRenderer : public GISApp::Core::Wrappers::BaseMapFeatureRenderer
{
    Q_OBJECT

public:
    /**
     * @brief Constructs TrackMapRenderer associated with an optional MapWidget canvas.
     * @param[in] mapWidget Pointer to target MapWidget canvas hosting the native MapLibre engine.
     * @param[in] parent Optional parent QObject for Qt lifecycle management.
     */
    explicit TrackMapRenderer(GISApp::UI::MapWidget *mapWidget = nullptr, QObject *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~TrackMapRenderer() override = default;

    /**
     * @brief Checks if tactical track layers are currently set to visible.
     * @return True if visible, false if hidden.
     */
    [[nodiscard]] bool isTracksVisible() const { return isFeaturesVisible(); }

public slots:
    /**
     * @brief Renders or updates tactical tracks on the map canvas using GeoJSON data.
     * @param[in] geoJsonData GeoJSON FeatureCollection serialized as a UTF-8 QByteArray.
     */
    void renderTracks(const QByteArray &geoJsonData) { renderGeoJson(geoJsonData); }

    /**
     * @brief Sets the visibility of tactical track layers on the map canvas.
     * @param[in] visible True to display track layers, false to hide.
     */
    void setTracksVisible(bool visible) { setFeaturesVisible(visible); }

protected:
    [[nodiscard]] QString defaultColorHex() const override { return QStringLiteral("#ff3344"); }
};

} // namespace GISApp::UI::Renderers

#endif // TRACKMAPRENDERER_H

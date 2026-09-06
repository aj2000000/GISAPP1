/**
 * @file TrackMapRenderer.h
 * @brief MapLibre Native GPU layer renderer for tactical track visualization.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TRACKMAPRENDERER_H
#define TRACKMAPRENDERER_H

#include "BaseMapFeatureRenderer.h"

namespace QMapLibre {
class Map;
}

namespace GISApp::UI::Renderers {

/**
 * @class TrackMapRenderer
 * @brief Specializes BaseMapFeatureRenderer for high-performance tactical track visualization.
 *
 * Architectural Role & Responsibilities:
 * - Resides strictly in the Presentation / View layer (ui/map/renderers/).
 * - Specializes GISApp::Core::Wrappers::BaseMapFeatureRenderer for the "source_tactical_tracks" source
 *   and "tactical_tracks" GPU sub-layers (glow halo, core dot, callsign label).
 * - Implements setupGpuLayers() to construct tactical track shaders and paint properties.
 * - Remains 100% agnostic of domain models, consuming abstract IMapFeature pointers via renderFeatures().
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
     * @brief Sets the visibility of tactical track layers on the map canvas.
     * @param[in] visible True to display track layers, false to hide.
     */
    void setTracksVisible(bool visible) { setFeaturesVisible(visible); }

protected:
    /**
     * @brief Configures tactical track MapLibre GPU vector layers (glow halo, core marker, callsign label).
     *
     * Constructs and attaches:
     * 1. tactical_tracks_glow: 140px semi-transparent halo evaluated from feature "color"
     * 2. tactical_tracks_circle: 8px solid core dot with white border evaluated from feature "color"
     * 3. tactical_tracks_label: Callsign text label positioned below the marker
     *
     * @param[in] map Raw QMapLibre::Map native engine instance.
     * @param[in] visibility Initial visibility string ("visible" or "none").
     */
    void setupGpuLayers(QMapLibre::Map *map, const QString &visibility) override;

    [[nodiscard]] QString defaultColorHex() const override { return QStringLiteral("#ff3344"); }
};

} // namespace GISApp::UI::Renderers

#endif // TRACKMAPRENDERER_H

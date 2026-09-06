/**
 * @file SampleEntityMapRenderer.h
 * @brief MapLibre Native GPU layer renderer for canonical SampleEntity visualization.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYMAPRENDERER_H
#define SAMPLEENTITYMAPRENDERER_H

#include "BaseMapFeatureRenderer.h"

namespace QMapLibre {
class Map;
}

namespace GISApp::UI::Renderers {

/**
 * @class SampleEntityMapRenderer
 * @brief Specializes BaseMapFeatureRenderer for canonical SampleEntity visualization by entity_type.
 *
 * Architectural Role & Responsibilities:
 * - Resides strictly in the Presentation / View layer (`ui/map/renderers/`).
 * - Manages MapLibre runtime source "source_sample_entities" and GPU vector sub-layers:
 *   1. sample_entities_circle: Circle marker layer for entity_type == 1 (Point).
 *   2. sample_entities_curve_glow: Soft outer halo line layer for entity_type == 2 (Bezier curve).
 *   3. sample_entities_curve: Core antialiased line layer for entity_type == 2 (Bezier curve).
 *   4. sample_entities_icon: Tactical symbol layer for entity_type == 3 (sample.svg / sample.png).
 *   5. sample_entities_label: Callsign and identifier text label layer.
 * - Injects "sample_icon" sprite texture into the native MapLibre engine.
 * - Remains 100% decoupled from database persistence and network UDP protocols.
 */
class SampleEntityMapRenderer : public GISApp::Core::Wrappers::BaseMapFeatureRenderer
{
    Q_OBJECT

public:
    /**
     * @brief Constructs SampleEntityMapRenderer associated with an optional MapWidget canvas.
     * @param[in] mapWidget Pointer to target MapWidget canvas hosting the native MapLibre engine.
     * @param[in] parent Optional parent QObject for Qt lifecycle management.
     */
    explicit SampleEntityMapRenderer(GISApp::UI::MapWidget *mapWidget = nullptr, QObject *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~SampleEntityMapRenderer() override = default;

    /**
     * @brief Checks if sample entity layers are currently set to visible.
     * @return True if visible, false if hidden.
     */
    [[nodiscard]] bool isEntitiesVisible() const { return isFeaturesVisible(); }

public slots:
    /**
     * @brief Sets the visibility of sample entity layers on the map canvas.
     * @param[in] visible True to display sample entity layers, false to hide.
     */
    void setEntitiesVisible(bool visible) { setFeaturesVisible(visible); }

protected:
    /**
     * @brief Configures MapLibre GPU vector layers and sprite textures for sample entities.
     *
     * Injects the tactical "sample_icon" sprite and attaches:
     * 1. sample_entities_curve_glow (Line halo for type 2)
     * 2. sample_entities_curve (Line core for type 2)
     * 3. sample_entities_circle (Circle marker for type 1)
     * 4. sample_entities_icon (Symbol icon for type 3)
     * 5. sample_entities_label (Text label for all entities)
     *
     * @param[in] map Raw QMapLibre::Map native engine instance.
     * @param[in] visibility Initial visibility string ("visible" or "none").
     */
    void setupGpuLayers(QMapLibre::Map *map, const QString &visibility) override;

    /**
     * @brief Fallback hex color string.
     * @return "#00e676"
     */
    [[nodiscard]] QString defaultColorHex() const override { return QStringLiteral("#00e676"); }

private:
    /**
     * @brief Loads and registers the "sample_icon" sprite into MapLibre if not already loaded.
     * @param[in] map Raw QMapLibre::Map native engine instance.
     */
    void ensureSpriteRegistered(QMapLibre::Map *map);
};

} // namespace GISApp::UI::Renderers

#endif // SAMPLEENTITYMAPRENDERER_H

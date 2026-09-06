/**
 * @file ComplexEntityMapRenderer.h
 * @brief MapLibre Native GPU layer renderer for ComplexEntity visualization by entity type.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYMAPRENDERER_H
#define COMPLEXENTITYMAPRENDERER_H

#include "BaseMapFeatureRenderer.h"

namespace QMapLibre {
class Map;
}

namespace GISApp::UI::Renderers {

/**
 * @class ComplexEntityMapRenderer
 * @brief Specializes BaseMapFeatureRenderer for ComplexEntity multi-geometry GPU visualization.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the **Presentation / View Layer** (`ui/map/renderers/`).
 * - Controls MapLibre runtime source `"source_complex_entities"` and GPU vector layers:
 *   1. Polygon fill and outline for entity_type == 3.
 *   2. Line glow and line core for entity_type == 2.
 *   3. Point glow and circle marker for entity_type == 1.
 *   4. Custom image symbol layer for entity_type == 5.
 *   5. Directional custom painter symbol layer with heading rotation for entity_type == 6.
 *   6. 4-way spatial annotation layers (Top, Bottom, Left, Right).
 *   7. Entity callsign label layer.
 * - Injects procedural tactical textures (`complex_icon`, `complex_painter_icon`) into MapLibre engine.
 * - Entirely decoupled from UDP networking and SQLite persistence.
 */
class ComplexEntityMapRenderer : public GISApp::Core::Wrappers::BaseMapFeatureRenderer
{
    Q_OBJECT

public:
    /**
     * @brief Constructs ComplexEntityMapRenderer with optional MapWidget canvas.
     * @param[in] mapWidget Pointer to target MapWidget.
     * @param[in] parent Optional parent QObject.
     */
    explicit ComplexEntityMapRenderer(GISApp::UI::MapWidget *mapWidget = nullptr, QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~ComplexEntityMapRenderer() override = default;

    /**
     * @brief Checks whether complex entity layers are visible.
     * @return True if visible, false if hidden.
     */
    [[nodiscard]] bool isEntitiesVisible() const { return isFeaturesVisible(); }

public slots:
    /**
     * @brief Sets visibility of complex entity layers on the map canvas.
     * @param[in] visible True to show, false to hide.
     */
    void setEntitiesVisible(bool visible) { setFeaturesVisible(visible); }

protected:
    /**
     * @brief Configures MapLibre GPU vector layers, shaders, and expressions.
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility string ("visible" or "none").
     */
    void setupGpuLayers(QMapLibre::Map *map, const QString &visibility) override;

    /**
     * @brief Fallback hex color string for complex entity feature highlights.
     * @return "#d500f9"
     */
    [[nodiscard]] QString defaultColorHex() const override { return QStringLiteral("#d500f9"); }

private:
    /**
     * @brief Injects procedural sprites (complex_icon, complex_painter_icon) into the MapLibre engine.
     * @param[in] map Pointer to active QMapLibre::Map instance.
     */
    void ensureSpritesRegistered(QMapLibre::Map *map);

    /**
     * @brief Configures GPU vector layers for Point entities (Entity Type 1).
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Creates tactical point glow halo and core circular marker layers.
     */
    void setupPointEntityLayers(QMapLibre::Map *map, const QString &visibility);

    /**
     * @brief Configures GPU vector layers for Polyline entities (Entity Type 2).
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Creates tactical line glow halo and anti-aliased line core layers.
     */
    void setupLineEntityLayers(QMapLibre::Map *map, const QString &visibility);

    /**
     * @brief Configures GPU vector layers for Polygon entities (Entity Type 3).
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Creates semi-transparent polygon fill and perimeter stroke line layers.
     */
    void setupPolygonEntityLayers(QMapLibre::Map *map, const QString &visibility);

    /**
     * @brief Configures GPU symbol layers for Custom Image Icon entities (Entity Type 5).
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Configures tactical holographic diamond sprite symbol layer.
     */
    void setupCustomIconEntityLayers(QMapLibre::Map *map, const QString &visibility);

    /**
     * @brief Configures GPU symbol layers for Custom Painter entities (Entity Type 6).
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Configures directional tactical chevron sprite with dynamic heading rotation.
     */
    void setupCustomPainterEntityLayers(QMapLibre::Map *map, const QString &visibility);

    /**
     * @brief Configures GPU vector layers for Formation Boundary entities (Entity Type 7).
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Configures boundary glow, solid/dashed/dotted line styles, midpoint echelon symbol,
     *       and dual top/bottom flank unit designation labels.
     */
    void setupFormationBoundaryEntityLayers(QMapLibre::Map *map, const QString &visibility);

    /**
     * @brief Configures GPU vector layers for Tactical Deployment Area entities (Entity Type 8).
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Configures deployment glow, solid/dashed/dotted bezier perimeter styles,
     *       and perimeter echelon symbol badge.
     */
    void setupDeploymentAreaEntityLayers(QMapLibre::Map *map, const QString &visibility);

    /**
     * @brief Configures GPU vector layers for interactive control point vertex editing.
     * @param[in] map Pointer to active QMapLibre::Map engine instance.
     * @param[in] visibility Initial visibility state ("visible" or "none").
     * @note Renders high-contrast tactical yellow glow halo, solid circular core with selection highlight,
     *       and 1-indexed numeric vertex position labels.
     */
    void setupControlPointEditLayers(QMapLibre::Map *map, const QString &visibility);
};

} // namespace GISApp::UI::Renderers

#endif // COMPLEXENTITYMAPRENDERER_H

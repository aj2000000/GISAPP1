/**
 * @file IMapFeatureRenderer.h
 * @brief Pure virtual interface defining lifecycle and layer management for MapLibre map renderers.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef IMAPFEATURERENDERER_H
#define IMAPFEATURERENDERER_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QVector>

namespace GISApp::Core::Interfaces {

class IMapFeature;

/**
 * @class IMapFeatureRenderer
 * @brief Polymorphic interface governing MapLibre Native GPU sources and vector layers for map features.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the Core Interfaces layer (`src/core/interfaces/`).
 * - Standardizes the lifecycle of GPU vector layers and GeoJSON runtime sources across all feature types.
 * - Establishes contracts for visibility toggles, dynamic data ingestion, and layer re-injection upon
 *   map engine recovery or style reloads.
 * - Enables polymorphic management of diverse entity renderers (TrackMapRenderer, SensorMapRenderer, etc.)
 *   from higher-level map and layer controllers.
 */
class IMapFeatureRenderer
{
public:
    /**
     * @brief Virtual destructor ensuring clean polymorphic destruction.
     */
    virtual ~IMapFeatureRenderer() = default;

    /**
     * @brief Retrieves the MapLibre runtime source identifier (e.g. "source_tactical_tracks").
     * @return Source name identifier string.
     */
    [[nodiscard]] virtual QString sourceId() const = 0;

    /**
     * @brief Retrieves the list of GPU layer identifiers registered with MapLibre Native.
     * @return List of layer IDs managed by this renderer.
     */
    [[nodiscard]] virtual QStringList layerIds() const = 0;

    /**
     * @brief Checks whether the MapLibre source and vector layers are currently configured in the map style.
     * @return True if layers are active in the map engine, false otherwise.
     */
    [[nodiscard]] virtual bool isLayersConfigured() const = 0;

    /**
     * @brief Checks if the feature layers are currently set to visible.
     * @return True if visible, false if hidden.
     */
    [[nodiscard]] virtual bool isFeaturesVisible() const = 0;

    /**
     * @brief Toggles the visibility of all managed GPU layers on the map canvas.
     * @param[in] visible True to display feature layers, false to hide.
     */
    virtual void setFeaturesVisible(bool visible) = 0;

    /**
     * @brief Ingests and renders pre-serialized GeoJSON data directly to the MapLibre source.
     * @param[in] geoJsonData Serialized UTF-8 GeoJSON FeatureCollection payload.
     */
    virtual void renderGeoJson(const QByteArray &geoJsonData) = 0;

    /**
     * @brief Ingests and renders a collection of polymorphic IMapFeature pointers.
     * @param[in] features Vector of feature pointers to serialize and render.
     */
    virtual void renderFeatures(const QVector<const IMapFeature*> &features) = 0;

    /**
     * @brief Clears all rendered features from the map canvas.
     */
    virtual void clearFeatures() = 0;
};

} // namespace GISApp::Core::Interfaces

#endif // IMAPFEATURERENDERER_H

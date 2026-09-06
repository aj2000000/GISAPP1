/**
 * @file BaseMapFeatureRenderer.h
 * @brief Reusable base renderer implementing GPU data-driven MapLibre layer management.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef BASEMAPFEATURERENDERER_H
#define BASEMAPFEATURERENDERER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QVector>

#include "IMapFeatureRenderer.h"
#include "IMapFeature.h"

namespace QMapLibre {
class Map;
}

namespace GISApp::UI {
class MapWidget;
}

namespace GISApp::Core::Wrappers {

/**
 * @class BaseMapFeatureRenderer
 * @brief Concrete base renderer managing MapLibre Native GPU sources, shaders, and data-driven vector layers.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the Core Wrappers layer (`src/core/wrappers/`).
 * - Implements GISApp::Core::Interfaces::IMapFeatureRenderer and inherits QObject.
 * - Standardizes connection to the MapWidget canvas and its underlying QMapLibre::Map instance.
 * - Manages automatic layer recovery during style reloads or map engine restarts.
 * - Employs GPU Data-Driven Styling: layer attributes (colors, labels, headings) are resolved directly
 *   on the GPU using MapLibre expressions, enabling high-performance rendering of thousands of features.
 * - Implements the Template Method pattern: provides ready-to-use glow, core shape, and label layers,
 *   while offering virtual hooks for derived classes to inject domain-specific symbols or overlays.
 */
class BaseMapFeatureRenderer : public QObject, public GISApp::Core::Interfaces::IMapFeatureRenderer
{
    Q_OBJECT

public:
    /**
     * @brief Constructs BaseMapFeatureRenderer with target source ID, optional layer prefix, and MapWidget.
     * @param[in] sourceId Unique MapLibre runtime source identifier (e.g. "source_tactical_tracks").
     * @param[in] layerPrefix Optional prefix for GPU layer IDs (defaults to sourceId stripped of "source_").
     * @param[in] mapWidget Optional target MapWidget canvas.
     * @param[in] parent Optional parent QObject for Qt lifecycle management.
     */
    explicit BaseMapFeatureRenderer(const QString &sourceId,
                                    const QString &layerPrefix = QString(),
                                    GISApp::UI::MapWidget *mapWidget = nullptr,
                                    QObject *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~BaseMapFeatureRenderer() override = default;

    // --- IMapFeatureRenderer Interface Implementation ---

    [[nodiscard]] QString sourceId() const override { return m_sourceId; }
    [[nodiscard]] QStringList layerIds() const override { return m_layerIds; }
    [[nodiscard]] bool isLayersConfigured() const override { return m_layersConfigured; }
    [[nodiscard]] bool isFeaturesVisible() const override { return m_featuresVisible; }

    void setFeaturesVisible(bool visible) override;
    void renderGeoJson(const QByteArray &geoJsonData) override;
    void renderFeatures(const QVector<const GISApp::Core::Interfaces::IMapFeature*> &features) override;
    void clearFeatures() override;

    /**
     * @brief Reconfigures GPU layers on the MapLibre engine, preserving source data and visibility.
     *
     * Removes existing GPU layers associated with this renderer from the MapLibre map,
     * resets layer configuration state, calls ensureLayersConfigured() to append them
     * at the top of the current layer stack, and re-pushes any cached GeoJSON features.
     * Essential when restacking layer ordering across layer groups or dynamically changing z-orders.
     *
     * @note Thread-safety: Must be called from the Qt main UI thread.
     */
    virtual void reconfigureLayers();

    // --- Map Canvas Attachment ---

    /**
     * @brief Associates or changes the target MapWidget canvas.
     * @param[in] mapWidget Pointer to target MapWidget instance.
     */
    void setMapWidget(GISApp::UI::MapWidget *mapWidget);

    /**
     * @brief Retrieves the associated MapWidget canvas.
     * @return Pointer to active MapWidget, or nullptr if none attached.
     */
    [[nodiscard]] GISApp::UI::MapWidget* mapWidget() const { return m_mapWidget; }

public slots:
    /**
     * @brief Slot invoked when MapWidget native engine finishes initialization.
     */
    virtual void onMapReady();

    /**
     * @brief Slot invoked on MapWidget lifecycle change notifications.
     * @param[in] change QMapLibre::Map::MapChange integer code.
     */
    virtual void onMapChanged(int change);

protected:
    /**
     * @brief Configures MapLibre runtime sources and standard GPU vector layers.
     */
    virtual void ensureLayersConfigured();

    /**
     * @brief Pure virtual hook configuring entity-specific MapLibre GPU vector layers and shaders.
     *
     * Subclasses (e.g. TrackMapRenderer, RadarMapRenderer, ZoneMapRenderer) MUST implement this
     * method to construct their specific visual layers (glow, markers, lines, polygons, labels)
     * and register their layer IDs in m_layerIds.
     *
     * @param[in] map Pointer to active QMapLibre::Map native engine instance.
     * @param[in] visibility Initial visibility string ("visible" or "none").
     */
    virtual void setupGpuLayers(QMapLibre::Map *map, const QString &visibility) = 0;

    /**
     * @brief Returns the fallback color hex used when a feature doesn't provide one.
     * @return Fallback hex color string.
     */
    [[nodiscard]] virtual QString defaultColorHex() const { return QStringLiteral("#ff3344"); }

    /**
     * @brief Pushes GeoJSON payload into the MapLibre engine source.
     * @param[in] geoJsonData Serialized UTF-8 GeoJSON string.
     */
    virtual void pushGeoJsonToMap(const QByteArray &geoJsonData);

    /// Target MapWidget canvas
    GISApp::UI::MapWidget *m_mapWidget{nullptr};

    /// Unique MapLibre source ID
    QString m_sourceId;

    /// Prefix used for GPU vector layer names
    QString m_layerPrefix;

    /// List of registered MapLibre layer IDs
    QStringList m_layerIds;

    /// Cached GeoJSON payload for style reload recovery
    QByteArray m_cachedGeoJson;

    /// Layer visibility flag
    bool m_featuresVisible{true};

    /// Configuration state flag
    bool m_layersConfigured{false};
};

} // namespace GISApp::Core::Wrappers

#endif // BASEMAPFEATURERENDERER_H

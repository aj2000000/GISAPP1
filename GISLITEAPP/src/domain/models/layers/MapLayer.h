/**
 * @file MapLayer.h
 * @brief Header definition for MapLayer leaf entity in GIS layer hierarchy.
 */

#ifndef MAPLAYER_H
#define MAPLAYER_H

#include "LayerNode.h"

namespace GISApp::Domain::Layers {

/**
 * @enum LayerType
 * @brief Enumerates the supported GIS layer formats and source representations.
 */
enum class LayerType {
    Raster,         ///< GDAL GeoTIFF, ECW, MrSID raster imagery
    Vector,         ///< Shapefile, GeoPackage, OGR vector features
    GeoJSON,        ///< Vector GeoJSON data source
    Elevation,      ///< DTED / SRTM elevation DEM
    Tile,           ///< MapLibre vector/raster tile service (MBTiles/PMTiles/XYZ)
    TacticalTrack,  ///< Live tactical symbols and telemetry overlay
    SampleEntity    ///< Sample entity overlay
};

/**
 * @class MapLayer
 * @brief Domain entity modeling an individual renderable GIS map layer.
 *
 * MapLayer encapsulates layer metadata, styling properties (opacity), data source URI,
 * and z-order rendering rank. It serves as the leaf node in the composite layer tree.
 */
class MapLayer : public LayerNode
{
public:
    /**
     * @brief Constructs a MapLayer leaf entity.
     * @param[in] id Unique layer identifier (e.g. "layer_satellite").
     * @param[in] name Display title (e.g. "Satellite Imagery").
     * @param[in] type Format type (Raster, Vector, Tile, etc.).
     * @param[in] sourceUri File path or endpoint URI.
     * @param[in] parent Optional parent LayerGroup or root node.
     */
    explicit MapLayer(const QString &id,
                      const QString &name,
                      LayerType type = LayerType::Vector,
                      const QString &sourceUri = QString(),
                      LayerNode *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~MapLayer() override = default;

    /**
     * @brief Discriminator confirming this node is a leaf layer.
     * @return Always false.
     */
    [[nodiscard]] bool isGroup() const override { return false; }

    /**
     * @brief Retrieves layer type enum.
     * @return Active LayerType.
     */
    [[nodiscard]] LayerType layerType() const { return m_layerType; }

    /**
     * @brief Sets layer type.
     * @param[in] type New LayerType.
     */
    void setLayerType(LayerType type) { m_layerType = type; }

    /**
     * @brief Retrieves data source URI or local file path.
     * @return Source URI string.
     */
    [[nodiscard]] QString sourceUri() const { return m_sourceUri; }

    /**
     * @brief Sets data source URI.
     * @param[in] uri File path or remote URI.
     */
    void setSourceUri(const QString &uri) { m_sourceUri = uri; }

    /**
     * @brief Retrieves parent group name tag.
     * @return Group name string.
     */
    [[nodiscard]] QString groupName() const { return m_groupName; }

    /**
     * @brief Sets group name tag.
     * @param[in] group Group name string.
     */
    void setGroupName(const QString &group) { m_groupName = group; }

    /**
     * @brief Retrieves layer opacity factor.
     * @return Opacity value in range [0.0, 1.0].
     */
    [[nodiscard]] double opacity() const { return m_opacity; }

    /**
     * @brief Sets layer opacity factor.
     * @param[in] opacity Value clamped to [0.0, 1.0].
     */
    void setOpacity(double opacity);

    /**
     * @brief Retrieves serialized JSON configuration parameters.
     * @return JSON string.
     */
    [[nodiscard]] QString configJson() const { return m_configJson; }

    /**
     * @brief Sets serialized JSON configuration parameters.
     * @param[in] json JSON string.
     */
    void setConfigJson(const QString &json) { m_configJson = json; }

    /**
     * @brief Returns a representative unicode symbol icon based on LayerType.
     * @return Unicode icon string (e.g. "🛰️", "📐", "🗺️").
     */
    [[nodiscard]] QString typeIcon() const;

    /**
     * @brief Serializes LayerType enum into database storage string.
     * @param[in] type LayerType enum.
     * @return Lowercase string identifier (e.g. "raster", "vector").
     */
    [[nodiscard]] static QString layerTypeToString(LayerType type);

    /**
     * @brief Deserializes database storage string into LayerType enum.
     * @param[in] str Type string.
     * @return Corresponding LayerType enum.
     */
    [[nodiscard]] static LayerType stringToLayerType(const QString &str);

private:
    /// Format classification
    LayerType m_layerType;

    /// File path or remote dataset URI
    QString m_sourceUri;

    /// Parent category folder name for database persistence
    QString m_groupName;

    /// Transparency factor [0.0, 1.0]
    double m_opacity;

    /// Optional extended configuration in JSON format
    QString m_configJson;
};

} // namespace GISApp::Domain::Layers

#endif // MAPLAYER_H

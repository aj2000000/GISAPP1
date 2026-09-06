/**
 * @file MapLayer.cpp
 * @brief Implementation of MapLayer leaf entity.
 */

#include "MapLayer.h"
#include <algorithm>

namespace GISApp::Domain::Layers {

MapLayer::MapLayer(const QString &id,
                   const QString &name,
                   LayerType type,
                   const QString &sourceUri,
                   LayerNode *parent)
    : LayerNode(id, name, parent)
    , m_layerType(type)
    , m_sourceUri(sourceUri)
    , m_groupName(QString())
    , m_opacity(1.0)
    , m_configJson("{}")
{
}

void MapLayer::setOpacity(double opacity)
{
    m_opacity = std::clamp(opacity, 0.0, 1.0);
}

QString MapLayer::typeIcon() const
{
    switch (m_layerType) {
    case LayerType::Raster:        return "🗺️";
    case LayerType::Vector:        return "📐";
    case LayerType::GeoJSON:       return "📍";
    case LayerType::Elevation:     return "⛰️";
    case LayerType::Tile:          return "🌐";
    case LayerType::TacticalTrack: return "🎯";
    case LayerType::SampleEntity:  return "🔷";
    case LayerType::ComplexEntity: return "💠";
    }
    return "📄";
}

QString MapLayer::layerTypeToString(LayerType type)
{
    switch (type) {
    case LayerType::Raster:        return "raster";
    case LayerType::Vector:        return "vector";
    case LayerType::GeoJSON:       return "geojson";
    case LayerType::Elevation:     return "elevation";
    case LayerType::Tile:          return "tile";
    case LayerType::TacticalTrack: return "tactical_track";
    case LayerType::SampleEntity:  return "sample_entity";
    case LayerType::ComplexEntity: return "complex_entity";
    }
    return "vector";
}

LayerType MapLayer::stringToLayerType(const QString &str)
{
    QString s = str.toLower().trimmed();
    if (s == "raster")         return LayerType::Raster;
    if (s == "vector")         return LayerType::Vector;
    if (s == "geojson")        return LayerType::GeoJSON;
    if (s == "elevation")      return LayerType::Elevation;
    if (s == "tile")           return LayerType::Tile;
    if (s == "tactical_track") return LayerType::TacticalTrack;
    if (s == "sample_entity")  return LayerType::SampleEntity;
    if (s == "complex_entity") return LayerType::ComplexEntity;
    return LayerType::Vector;
}

} // namespace GISApp::Domain::Layers

/**
 * @file BaseMapFeatureRenderer.cpp
 * @brief Implementation of BaseMapFeatureRenderer managing MapLibre GPU layers and data-driven styling.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "BaseMapFeatureRenderer.h"
#include "MapWidget.h"

#include <QMapLibre/Map>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

namespace GISApp::Core::Wrappers {

BaseMapFeatureRenderer::BaseMapFeatureRenderer(const QString &sourceId,
                                               const QString &layerPrefix,
                                               GISApp::UI::MapWidget *mapWidget,
                                               QObject *parent)
    : QObject(parent)
    , m_sourceId(sourceId)
    , m_layerPrefix(layerPrefix.isEmpty() ? (sourceId.startsWith(QStringLiteral("source_")) ? sourceId.mid(7) : sourceId) : layerPrefix)
{
    setMapWidget(mapWidget);
}

void BaseMapFeatureRenderer::setMapWidget(GISApp::UI::MapWidget *mapWidget)
{
    if (m_mapWidget == mapWidget) {
        return;
    }

    if (m_mapWidget) {
        disconnect(m_mapWidget, nullptr, this, nullptr);
    }

    m_mapWidget = mapWidget;
    m_layersConfigured = false;

    if (m_mapWidget) {
        connect(m_mapWidget, &GISApp::UI::MapWidget::mapReady,
                this, &BaseMapFeatureRenderer::onMapReady);
        connect(m_mapWidget, &GISApp::UI::MapWidget::mapChanged,
                this, &BaseMapFeatureRenderer::onMapChanged);

        if (m_mapWidget->rawMap()) {
            onMapReady();
        }
    }
}

void BaseMapFeatureRenderer::setFeaturesVisible(bool visible)
{
    m_featuresVisible = visible;

    if (!m_mapWidget) {
        return;
    }
    QMapLibre::Map *map = m_mapWidget->rawMap();
    if (!map) {
        return;
    }

    const QString visStr = visible ? QStringLiteral("visible") : QStringLiteral("none");
    for (const QString &layerId : m_layerIds) {
        if (map->layerExists(layerId)) {
            map->setLayoutProperty(layerId, QStringLiteral("visibility"), visStr);
        }
    }

    qDebug() << "[BaseMapFeatureRenderer] Visibility for" << m_sourceId << "set to:" << visible;
}

void BaseMapFeatureRenderer::renderGeoJson(const QByteArray &geoJsonData)
{
    m_cachedGeoJson = geoJsonData;
    pushGeoJsonToMap(geoJsonData);
}

void BaseMapFeatureRenderer::renderFeatures(const QVector<const GISApp::Core::Interfaces::IMapFeature*> &features)
{
    QJsonArray featuresArray;

    for (const auto *feature : features) {
        if (feature) {
            featuresArray.append(feature->toGeoJsonFeature());
        }
    }

    QJsonObject collection;
    collection[QStringLiteral("type")] = QStringLiteral("FeatureCollection");
    collection[QStringLiteral("features")] = featuresArray;

    QJsonDocument doc(collection);
    renderGeoJson(doc.toJson(QJsonDocument::Compact));
}

void BaseMapFeatureRenderer::clearFeatures()
{
    renderGeoJson(QByteArrayLiteral("{\"type\":\"FeatureCollection\",\"features\":[]}"));
}

void BaseMapFeatureRenderer::reconfigureLayers()
{
    if (!m_mapWidget) {
        return;
    }
    QMapLibre::Map *map = m_mapWidget->rawMap();
    if (!map) {
        return;
    }

    // 1. Remove registered GPU layers from map engine
    for (const QString &layerId : m_layerIds) {
        if (map->layerExists(layerId)) {
            map->removeLayer(layerId);
        }
    }

    // 2. Clear layer tracking state and rebuild layers on top of stack
    m_layerIds.clear();
    m_layersConfigured = false;
    ensureLayersConfigured();

    // 3. Re-inject cached GeoJSON data if present
    if (!m_cachedGeoJson.isEmpty()) {
        pushGeoJsonToMap(m_cachedGeoJson);
    }

    qDebug() << "[BaseMapFeatureRenderer] Reconfigured GPU layers for source:" << m_sourceId
             << "Active layers:" << m_layerIds;
}

void BaseMapFeatureRenderer::onMapReady()
{
    qInfo() << "[BaseMapFeatureRenderer] Map engine ready for" << m_sourceId << ". Configuring GPU layers...";
    ensureLayersConfigured();

    if (!m_cachedGeoJson.isEmpty()) {
        pushGeoJsonToMap(m_cachedGeoJson);
    }
}

void BaseMapFeatureRenderer::onMapChanged(int change)
{
    if (!m_mapWidget) {
        return;
    }
    QMapLibre::Map *map = m_mapWidget->rawMap();
    if (!map) {
        return;
    }

    // Recover GPU layers if wiped during style reloads
    if (!map->sourceExists(m_sourceId) || (!m_layerIds.isEmpty() && !map->layerExists(m_layerIds.first()))) {
        qDebug() << "[BaseMapFeatureRenderer] Style changed or layers missing (code:" << change
                 << "). Reconfiguring for source:" << m_sourceId;
        m_layersConfigured = false;
        ensureLayersConfigured();
        if (!m_cachedGeoJson.isEmpty()) {
            pushGeoJsonToMap(m_cachedGeoJson);
        }
    }
}

void BaseMapFeatureRenderer::ensureLayersConfigured()
{
    if (!m_mapWidget) {
        return;
    }
    QMapLibre::Map *map = m_mapWidget->rawMap();
    if (!map) {
        return;
    }

    // 1. Create or register GeoJSON runtime source
    if (!map->sourceExists(m_sourceId)) {
        QByteArray initialGeoJson = m_cachedGeoJson;
        if (initialGeoJson.isEmpty()) {
            initialGeoJson = QByteArrayLiteral("{\"type\":\"FeatureCollection\",\"features\":[]}");
        }

        QVariantMap sourceParams;
        sourceParams[QStringLiteral("type")] = QStringLiteral("geojson");
        sourceParams[QStringLiteral("data")] = initialGeoJson;
        map->addSource(m_sourceId, sourceParams);
        qDebug() << "[BaseMapFeatureRenderer] Added MapLibre source:" << m_sourceId;
    }

    const QString visStr = m_featuresVisible ? QStringLiteral("visible") : QStringLiteral("none");

    // 2. Setup entity-specific GPU layers (delegated to specialized subclass)
    setupGpuLayers(map, visStr);

    m_layersConfigured = true;
}

void BaseMapFeatureRenderer::pushGeoJsonToMap(const QByteArray &geoJsonData)
{
    if (!m_mapWidget) {
        return;
    }
    QMapLibre::Map *map = m_mapWidget->rawMap();
    if (!map) {
        return;
    }

    if (!m_layersConfigured || !map->sourceExists(m_sourceId)) {
        ensureLayersConfigured();
    }

    QVariantMap updateParams;
    updateParams[QStringLiteral("type")] = QStringLiteral("geojson");
    updateParams[QStringLiteral("data")] = geoJsonData;

    map->updateSource(m_sourceId, updateParams);
    qDebug() << "[BaseMapFeatureRenderer] Updated MapLibre source" << m_sourceId
             << "(" << geoJsonData.size() << "bytes).";
}

} // namespace GISApp::Core::Wrappers

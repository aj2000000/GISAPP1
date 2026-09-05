/**
 * @file BaseMapFeatureRenderer.cpp
 * @brief Implementation of BaseMapFeatureRenderer managing MapLibre GPU layers and data-driven styling.
 * @author BrahmaxisGIS Development Team
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

    // 2. Setup standard data-driven GPU layers
    setupGpuLayers(map, visStr);

    // 3. Virtual hook for custom layers
    setupCustomGpuLayers(map, visStr);

    m_layersConfigured = true;
}

void BaseMapFeatureRenderer::setupGpuLayers(QMapLibre::Map *map, const QString &visibility)
{
    const QString glowLayerId = m_layerPrefix + QStringLiteral("_glow");
    const QString circleLayerId = m_layerPrefix + QStringLiteral("_circle");
    const QString labelLayerId = m_layerPrefix + QStringLiteral("_label");

    // Expression for dynamic data-driven color: ["coalesce", ["get", "color"], defaultColorHex]
    const QVariant colorExpression = QVariantList{
        QStringLiteral("coalesce"),
        QVariantList{QStringLiteral("get"), QStringLiteral("color")},
        defaultColorHex()
    };

    // 1. Outer Glow Halo Layer
    if (!map->layerExists(glowLayerId)) {
        QVariantMap glowLayer;
        glowLayer[QStringLiteral("id")] = glowLayerId;
        glowLayer[QStringLiteral("type")] = QStringLiteral("circle");
        glowLayer[QStringLiteral("source")] = m_sourceId;

        QVariantMap glowPaint;
        glowPaint[QStringLiteral("circle-radius")] = 140.0;
        glowPaint[QStringLiteral("circle-color")] = colorExpression;
        glowPaint[QStringLiteral("circle-opacity")] = 0.35;
        glowPaint[QStringLiteral("circle-stroke-width")] = 0.0;
        glowLayer[QStringLiteral("paint")] = glowPaint;

        QVariantMap glowLayout;
        glowLayout[QStringLiteral("visibility")] = visibility;
        glowLayer[QStringLiteral("layout")] = glowLayout;

        map->addLayer(glowLayerId, glowLayer);
        if (!m_layerIds.contains(glowLayerId)) {
            m_layerIds.append(glowLayerId);
        }
        qDebug() << "[BaseMapFeatureRenderer] Added GPU glow layer:" << glowLayerId;
    }

    // 2. Core Marker Circle Layer
    if (!map->layerExists(circleLayerId)) {
        QVariantMap circleLayer;
        circleLayer[QStringLiteral("id")] = circleLayerId;
        circleLayer[QStringLiteral("type")] = QStringLiteral("circle");
        circleLayer[QStringLiteral("source")] = m_sourceId;

        QVariantMap circlePaint;
        circlePaint[QStringLiteral("circle-radius")] = 8.0;
        circlePaint[QStringLiteral("circle-color")] = colorExpression;
        circlePaint[QStringLiteral("circle-stroke-width")] = 2.0;
        circlePaint[QStringLiteral("circle-stroke-color")] = QStringLiteral("#ffffff");
        circlePaint[QStringLiteral("circle-opacity")] = 1.0;
        circleLayer[QStringLiteral("paint")] = circlePaint;

        QVariantMap circleLayout;
        circleLayout[QStringLiteral("visibility")] = visibility;
        circleLayer[QStringLiteral("layout")] = circleLayout;

        map->addLayer(circleLayerId, circleLayer);
        if (!m_layerIds.contains(circleLayerId)) {
            m_layerIds.append(circleLayerId);
        }
        qDebug() << "[BaseMapFeatureRenderer] Added GPU core layer:" << circleLayerId;
    }

    // 3. Callsign / Label Symbol Layer
    if (!map->layerExists(labelLayerId)) {
        QVariantMap labelLayer;
        labelLayer[QStringLiteral("id")] = labelLayerId;
        labelLayer[QStringLiteral("type")] = QStringLiteral("symbol");
        labelLayer[QStringLiteral("source")] = m_sourceId;

        QVariantMap labelLayout;
        labelLayout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("callsign")},
            QStringLiteral("")
        };
        labelLayout[QStringLiteral("text-size")] = 11.0;
        labelLayout[QStringLiteral("text-offset")] = QVariantList{0.0, 1.3};
        labelLayout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        labelLayout[QStringLiteral("text-font")] = QVariantList{
            QStringLiteral("Open Sans Regular"),
            QStringLiteral("Arial Unicode MS Regular")
        };
        labelLayout[QStringLiteral("visibility")] = visibility;
        labelLayer[QStringLiteral("layout")] = labelLayout;

        QVariantMap labelPaint;
        labelPaint[QStringLiteral("text-color")] = QStringLiteral("#ffffff");
        labelPaint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        labelPaint[QStringLiteral("text-halo-width")] = 1.5;
        labelLayer[QStringLiteral("paint")] = labelPaint;

        map->addLayer(labelLayerId, labelLayer);
        if (!m_layerIds.contains(labelLayerId)) {
            m_layerIds.append(labelLayerId);
        }
        qDebug() << "[BaseMapFeatureRenderer] Added GPU label layer:" << labelLayerId;
    }
}

void BaseMapFeatureRenderer::setupCustomGpuLayers(QMapLibre::Map *map, const QString &visibility)
{
    Q_UNUSED(map);
    Q_UNUSED(visibility);
    // Base implementation is a no-op; specialized derived renderers override this.
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

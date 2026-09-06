#include "TrackMapRenderer.h"

#include <QMapLibre/Map>
#include <QVariantMap>
#include <QVariantList>
#include <QDebug>

namespace GISApp::UI::Renderers {

TrackMapRenderer::TrackMapRenderer(GISApp::UI::MapWidget *mapWidget, QObject *parent)
    : BaseMapFeatureRenderer(QStringLiteral("source_tactical_tracks"),
                             QStringLiteral("tactical_tracks"),
                             mapWidget,
                             parent)
{
}

void TrackMapRenderer::setupGpuLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) {
        return;
    }

    const QString glowLayerId = m_layerPrefix + QStringLiteral("_glow");
    const QString circleLayerId = m_layerPrefix + QStringLiteral("_circle");
    const QString labelLayerId = m_layerPrefix + QStringLiteral("_label");

    // Expression for dynamic data-driven color: matches canonical identities with MIL-STD colors
    const QVariant colorExpression = QVariantList{
        QStringLiteral("match"),
        QVariantList{QStringLiteral("get"), QStringLiteral("identity")},
        QStringLiteral("HOSTILE"),  QStringLiteral("#ff3344"), // MIL-STD Red
        QStringLiteral("Hostile"),  QStringLiteral("#ff3344"),
        QStringLiteral("FRIENDLY"), QStringLiteral("#00d2ff"), // MIL-STD Cyan/Blue
        QStringLiteral("Friendly"), QStringLiteral("#00d2ff"),
        QStringLiteral("NEUTRAL"),  QStringLiteral("#00e676"), // MIL-STD Green
        QStringLiteral("Neutral"),  QStringLiteral("#00e676"),
        QStringLiteral("#ffd600")                              // Fallback / Unknown Amber
    };

    // 1. Outer Glow Halo Layer
    if (!map->layerExists(glowLayerId)) {
        QVariantMap glowLayer;
        glowLayer[QStringLiteral("id")] = glowLayerId;
        glowLayer[QStringLiteral("type")] = QStringLiteral("circle");
        glowLayer[QStringLiteral("source")] = m_sourceId;

        QVariantMap glowPaint;
        glowPaint[QStringLiteral("circle-radius")] = 40.0;
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
        qDebug() << "[TrackMapRenderer] Added GPU glow layer:" << glowLayerId;
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
        qDebug() << "[TrackMapRenderer] Added GPU core layer:" << circleLayerId;
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
        qDebug() << "[TrackMapRenderer] Added GPU label layer:" << labelLayerId;
    }
}

} // namespace GISApp::UI::Renderers

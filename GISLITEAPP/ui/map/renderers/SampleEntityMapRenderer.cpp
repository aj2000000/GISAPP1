/**
 * @file SampleEntityMapRenderer.cpp
 * @brief Implementation of SampleEntityMapRenderer GPU layers.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "SampleEntityMapRenderer.h"

#include <QMapLibre/Map>
#include <QVariantMap>
#include <QVariantList>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QFile>
#include <QDebug>

namespace GISApp::UI::Renderers {

SampleEntityMapRenderer::SampleEntityMapRenderer(GISApp::UI::MapWidget *mapWidget, QObject *parent)
    : BaseMapFeatureRenderer(QStringLiteral("source_sample_entities"),
                             QStringLiteral("sample_entities"),
                             mapWidget,
                             parent)
{
}

void SampleEntityMapRenderer::ensureSpriteRegistered(QMapLibre::Map *map)
{
    if (!map) {
        return;
    }

    QImage iconImg;
    if (QFile::exists(QStringLiteral(":/icons/sample.png"))) {
        iconImg.load(QStringLiteral(":/icons/sample.png"));
    }

    if (iconImg.isNull() && QFile::exists(QStringLiteral(":/icons/sample.svg"))) {
        QSvgRenderer svgRenderer(QStringLiteral(":/icons/sample.svg"));
        iconImg = QImage(64, 64, QImage::Format_RGBA8888);
        iconImg.fill(Qt::transparent);
        QPainter painter(&iconImg);
        svgRenderer.render(&painter);
    }

    if (iconImg.isNull()) {
        // Procedural fallback high-tech tactical cyan icon
        iconImg = QImage(48, 48, QImage::Format_RGBA8888);
        iconImg.fill(Qt::transparent);
        QPainter p(&iconImg);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(0, 229, 255), 2));
        p.setBrush(QColor(10, 25, 47, 220));
        QPolygon pts;
        pts << QPoint(24, 4) << QPoint(44, 24) << QPoint(24, 44) << QPoint(4, 24);
        p.drawPolygon(pts);
        p.setPen(QPen(QColor(0, 229, 255), 1.5));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPoint(24, 24), 9, 9);
        p.setBrush(QColor(0, 229, 255));
        p.drawEllipse(QPoint(24, 24), 4, 4);
    }

    QImage formattedImg = iconImg.convertToFormat(QImage::Format_RGBA8888);
    map->addImage(QStringLiteral("sample_icon"), formattedImg);
    map->addImage(QStringLiteral("sample.png"), formattedImg);
    map->addImage(QStringLiteral("sample.svg"), formattedImg);
    qDebug() << "[SampleEntityMapRenderer] Injected sample sprite textures into MapLibre engine.";
}

void SampleEntityMapRenderer::setupGpuLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) {
        return;
    }

    ensureSpriteRegistered(map);

    const QString curveGlowLayerId = m_layerPrefix + QStringLiteral("_curve_glow");
    const QString curveLayerId     = m_layerPrefix + QStringLiteral("_curve");
    const QString circleLayerId    = m_layerPrefix + QStringLiteral("_circle");
    const QString iconLayerId      = m_layerPrefix + QStringLiteral("_icon");
    const QString labelLayerId     = m_layerPrefix + QStringLiteral("_label");

    // Dynamic color expression matching entity type
    const QVariant colorExpression = QVariantList{
        QStringLiteral("match"),
        QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
        1, QStringLiteral("#00e676"), // Type 1: Point -> Vibrant Emerald Green
        2, QStringLiteral("#ffab00"), // Type 2: Bezier Curve -> Tactical Amber
        3, QStringLiteral("#00e5ff"), // Type 3: Tactical Icon -> Cyan
        QStringLiteral("#00e676")     // Fallback
    };

    // 1. Layer: Bezier Curve Glow Halo (entity_type == 2)
    if (!map->layerExists(curveGlowLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = curveGlowLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;

        // Filter: line features with entity_type == 2
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            2
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = colorExpression;
        paint[QStringLiteral("line-width")] = 8.0;
        paint[QStringLiteral("line-opacity")] = 0.35;
        paint[QStringLiteral("line-blur")] = 3.0;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(curveGlowLayerId, layer);
        if (!m_layerIds.contains(curveGlowLayerId)) {
            m_layerIds.append(curveGlowLayerId);
        }
        qDebug() << "[SampleEntityMapRenderer] Attached GPU layer:" << curveGlowLayerId;
    }

    // 2. Layer: Bezier Curve Core Line (entity_type == 2)
    if (!map->layerExists(curveLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = curveLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;

        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            2
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = colorExpression;
        paint[QStringLiteral("line-width")] = 3.5;
        paint[QStringLiteral("line-opacity")] = 0.95;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(curveLayerId, layer);
        if (!m_layerIds.contains(curveLayerId)) {
            m_layerIds.append(curveLayerId);
        }
        qDebug() << "[SampleEntityMapRenderer] Attached GPU layer:" << curveLayerId;
    }

    // 3. Layer: Point Core Circle (entity_type == 1)
    if (!map->layerExists(circleLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = circleLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("circle");
        layer[QStringLiteral("source")] = m_sourceId;

        // Filter: Point features with entity_type == 1
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("geometry-type")}, QStringLiteral("Point")},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 1}
        };

        QVariantMap paint;
        paint[QStringLiteral("circle-radius")] = 8.0;
        paint[QStringLiteral("circle-color")] = colorExpression;
        paint[QStringLiteral("circle-stroke-width")] = 2.0;
        paint[QStringLiteral("circle-stroke-color")] = QStringLiteral("#ffffff");
        paint[QStringLiteral("circle-opacity")] = 1.0;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(circleLayerId, layer);
        if (!m_layerIds.contains(circleLayerId)) {
            m_layerIds.append(circleLayerId);
        }
        qDebug() << "[SampleEntityMapRenderer] Attached GPU layer:" << circleLayerId;
    }

    // 4. Layer: Icon Symbol (entity_type == 3)
    if (!map->layerExists(iconLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = iconLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;

        // Filter: Point features with entity_type == 3
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("geometry-type")}, QStringLiteral("Point")},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 3}
        };

        QVariantMap layout;
        layout[QStringLiteral("icon-image")] = QStringLiteral("sample_icon");
        layout[QStringLiteral("icon-size")] = 0.65;
        layout[QStringLiteral("icon-allow-overlap")] = true;
        layout[QStringLiteral("icon-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(iconLayerId, layer);
        if (!m_layerIds.contains(iconLayerId)) {
            m_layerIds.append(iconLayerId);
        }
        qDebug() << "[SampleEntityMapRenderer] Attached GPU layer:" << iconLayerId;
    }

    // 5. Layer: Label Symbol (For all sample entities)
    if (!map->layerExists(labelLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = labelLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("name")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-size")] = 11.0;
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 1.4};
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-font")] = QVariantList{
            QStringLiteral("Open Sans Regular"),
            QStringLiteral("Arial Unicode MS Regular")
        };
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffffff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(labelLayerId, layer);
        if (!m_layerIds.contains(labelLayerId)) {
            m_layerIds.append(labelLayerId);
        }
        qDebug() << "[SampleEntityMapRenderer] Attached GPU layer:" << labelLayerId;
    }
}

} // namespace GISApp::UI::Renderers

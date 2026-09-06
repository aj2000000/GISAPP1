/**
 * @file ComplexEntityMapRenderer.cpp
 * @brief Implementation of ComplexEntityMapRenderer GPU layers, textures, and data-driven shaders.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntityMapRenderer.h"

#include <QMapLibre/Map>
#include <QVariantMap>
#include <QVariantList>
#include <QImage>
#include <QPainter>
#include <QPolygon>
#include <QDebug>

namespace GISApp::UI::Renderers {

ComplexEntityMapRenderer::ComplexEntityMapRenderer(GISApp::UI::MapWidget *mapWidget, QObject *parent)
    : BaseMapFeatureRenderer(QStringLiteral("source_complex_entities"),
                             QStringLiteral("complex_entities"),
                             mapWidget,
                             parent)
{
}

void ComplexEntityMapRenderer::ensureSpritesRegistered(QMapLibre::Map *map)
{
    if (!map) {
        return;
    }

    // 1. Procedural Custom Image Sprite: Tactical Holographic Diamond (complex_icon)
    {
        QImage iconImg(64, 64, QImage::Format_RGBA8888);
        iconImg.fill(Qt::transparent);
        QPainter p(&iconImg);
        p.setRenderHint(QPainter::Antialiasing);

        // Outer diamond glow
        p.setPen(QPen(QColor(213, 0, 249, 180), 2.5));
        p.setBrush(QColor(30, 10, 50, 210));
        QPolygon diamond;
        diamond << QPoint(32, 4) << QPoint(60, 32) << QPoint(32, 60) << QPoint(4, 32);
        p.drawPolygon(diamond);

        // Inner reticle circle
        p.setPen(QPen(QColor(0, 229, 255), 1.5));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPoint(32, 32), 12, 12);

        // Center dot
        p.setBrush(QColor(0, 229, 255));
        p.drawEllipse(QPoint(32, 32), 4, 4);

        // Crosshairs
        p.drawLine(32, 10, 32, 16);
        p.drawLine(32, 48, 32, 54);
        p.drawLine(10, 32, 16, 32);
        p.drawLine(48, 32, 54, 32);

        map->addImage(QStringLiteral("complex_icon"), iconImg.convertToFormat(QImage::Format_RGBA8888));
    }

    // 2. Procedural Custom Painter Sprite: Directional Tactical Chevron (complex_painter_icon)
    {
        QImage painterImg(64, 64, QImage::Format_RGBA8888);
        painterImg.fill(Qt::transparent);
        QPainter p(&painterImg);
        p.setRenderHint(QPainter::Antialiasing);

        // Tactical forward chevron
        QPolygon chevron;
        chevron << QPoint(32, 6) << QPoint(54, 52) << QPoint(32, 40) << QPoint(10, 52);

        p.setPen(QPen(QColor(0, 230, 118), 2.0));
        p.setBrush(QColor(10, 40, 25, 230));
        p.drawPolygon(chevron);

        // Heading direction core line
        p.setPen(QPen(QColor(255, 255, 255), 2.0));
        p.drawLine(32, 12, 32, 34);

        map->addImage(QStringLiteral("complex_painter_icon"), painterImg.convertToFormat(QImage::Format_RGBA8888));
    }

    qDebug() << "[ComplexEntityMapRenderer] Injected complex entity procedural textures into MapLibre engine.";
}

void ComplexEntityMapRenderer::setupGpuLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) {
        return;
    }

    ensureSpritesRegistered(map);

    // 1. Entity Type 3: Polygon (Fill, Outline, 4-Way Annotations, Callsign Label)
    setupPolygonEntityLayers(map, visibility);

    // 2. Entity Type 2: Polyline (Tactical Glow, Core Line, 4-Way Annotations, Callsign Label)
    setupLineEntityLayers(map, visibility);

    // 3. Entity Type 7: Formation Boundary (Glow, Solid/Dashed/Dotted, Echelon, Flank Annotations)
    setupFormationBoundaryEntityLayers(map, visibility);

    // 4. Entity Type 8: Tactical Deployment Area / Bezier Curve (Glow, Solid/Dashed/Dotted, Echelon - No Annotations)
    setupDeploymentAreaEntityLayers(map, visibility);

    // 5. Entity Type 1: Point (Tactical Glow Halo, Circular Marker, 4-Way Annotations, Callsign Label)
    setupPointEntityLayers(map, visibility);

    // 6. Entity Type 5: Custom Holographic Diamond Sprite (Icon, 4-Way Annotations, Callsign Label)
    setupCustomIconEntityLayers(map, visibility);

    // 7. Entity Type 6: Custom Directional Tactical Chevron with Heading (Painter, 4-Way Annotations, Callsign Label)
    setupCustomPainterEntityLayers(map, visibility);

    // 8. Interactive Control Point Handles (Yellow Glow, Core Dot, Numeric Index Labels)
    setupControlPointEditLayers(map, visibility);

    qDebug() << "[ComplexEntityMapRenderer] Successfully configured" << m_layerIds.size()
             << "GPU vector layers across all entity types for source:" << m_sourceId;
}

/**
 * @brief Configures GPU vector layers for Point entities (Entity Type 1).
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Creates tactical point glow halo, circular marker, 4-way spatial annotations, and callsign label.
 */
void ComplexEntityMapRenderer::setupPointEntityLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString pointGlowLayerId   = m_layerPrefix + QStringLiteral("_point_glow");
    const QString pointCircleLayerId = m_layerPrefix + QStringLiteral("_point_circle");
    const QString annTopLayerId      = m_layerPrefix + QStringLiteral("_point_annotation_top");
    const QString annBottomLayerId   = m_layerPrefix + QStringLiteral("_point_annotation_bottom");
    const QString annLeftLayerId     = m_layerPrefix + QStringLiteral("_point_annotation_left");
    const QString annRightLayerId    = m_layerPrefix + QStringLiteral("_point_annotation_right");
    const QString labelLayerId       = m_layerPrefix + QStringLiteral("_point_label");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Layer: Point Glow Halo (entity_type == 1 and not anchor)
    if (!map->layerExists(pointGlowLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = pointGlowLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("circle");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 1},
            QVariantList{QStringLiteral("!="), QVariantList{QStringLiteral("get"), QStringLiteral("is_anchor")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("circle-radius")] = 28.0;
        paint[QStringLiteral("circle-color")] = QStringLiteral("#d500f9");
        paint[QStringLiteral("circle-opacity")] = 0.35;
        paint[QStringLiteral("circle-stroke-width")] = 0.0;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(pointGlowLayerId, layer);
        if (!m_layerIds.contains(pointGlowLayerId)) m_layerIds.append(pointGlowLayerId);
    }

    // 2. Layer: Point Core Marker (entity_type == 1 and not anchor)
    if (!map->layerExists(pointCircleLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = pointCircleLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("circle");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 1},
            QVariantList{QStringLiteral("!="), QVariantList{QStringLiteral("get"), QStringLiteral("is_anchor")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("circle-radius")] = 7.5;
        paint[QStringLiteral("circle-color")] = QStringLiteral("#d500f9");
        paint[QStringLiteral("circle-stroke-width")] = 2.0;
        paint[QStringLiteral("circle-stroke-color")] = QStringLiteral("#ffffff");
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(pointCircleLayerId, layer);
        if (!m_layerIds.contains(pointCircleLayerId)) m_layerIds.append(pointCircleLayerId);
    }

    // 3. Layer: Top Annotation
    if (!map->layerExists(annTopLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annTopLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            1
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("top_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("bottom");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, -1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffab00");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annTopLayerId, layer);
        if (!m_layerIds.contains(annTopLayerId)) m_layerIds.append(annTopLayerId);
    }

    // 4. Layer: Bottom Annotation
    if (!map->layerExists(annBottomLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annBottomLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            1
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("bottom_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e5ff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annBottomLayerId, layer);
        if (!m_layerIds.contains(annBottomLayerId)) m_layerIds.append(annBottomLayerId);
    }

    // 5. Layer: Left Annotation
    if (!map->layerExists(annLeftLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annLeftLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            1
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("left_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("right");
        layout[QStringLiteral("text-offset")] = QVariantList{-1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e676");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annLeftLayerId, layer);
        if (!m_layerIds.contains(annLeftLayerId)) m_layerIds.append(annLeftLayerId);
    }

    // 6. Layer: Right Annotation
    if (!map->layerExists(annRightLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annRightLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            1
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("right_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("left");
        layout[QStringLiteral("text-offset")] = QVariantList{1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ff5252");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annRightLayerId, layer);
        if (!m_layerIds.contains(annRightLayerId)) m_layerIds.append(annRightLayerId);
    }

    // 7. Layer: Callsign / Name Label
    if (!map->layerExists(labelLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = labelLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            1
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_name")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 2.8};
        layout[QStringLiteral("text-size")] = 11.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffffff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#000000");
        paint[QStringLiteral("text-halo-width")] = 2.0;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(labelLayerId, layer);
        if (!m_layerIds.contains(labelLayerId)) m_layerIds.append(labelLayerId);
    }
}

/**
 * @brief Configures GPU vector layers for Polyline entities (Entity Type 2).
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Creates tactical line glow halo, core line, 4-way spatial annotations, and callsign label.
 */
void ComplexEntityMapRenderer::setupLineEntityLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString lineGlowLayerId = m_layerPrefix + QStringLiteral("_line_glow");
    const QString lineCoreLayerId = m_layerPrefix + QStringLiteral("_line_core");
    const QString annTopLayerId   = m_layerPrefix + QStringLiteral("_line_annotation_top");
    const QString annBottomLayerId= m_layerPrefix + QStringLiteral("_line_annotation_bottom");
    const QString annLeftLayerId  = m_layerPrefix + QStringLiteral("_line_annotation_left");
    const QString annRightLayerId = m_layerPrefix + QStringLiteral("_line_annotation_right");
    const QString labelLayerId    = m_layerPrefix + QStringLiteral("_line_label");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Layer: Line Glow Halo (entity_type == 2)
    if (!map->layerExists(lineGlowLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = lineGlowLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            2
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QStringLiteral("#00e5ff");
        paint[QStringLiteral("line-width")] = 8.0;
        paint[QStringLiteral("line-blur")] = 3.0;
        paint[QStringLiteral("line-opacity")] = 0.4;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(lineGlowLayerId, layer);
        if (!m_layerIds.contains(lineGlowLayerId)) m_layerIds.append(lineGlowLayerId);
    }

    // 2. Layer: Line Core (entity_type == 2)
    if (!map->layerExists(lineCoreLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = lineCoreLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            2
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QStringLiteral("#00e5ff");
        paint[QStringLiteral("line-width")] = 3.0;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(lineCoreLayerId, layer);
        if (!m_layerIds.contains(lineCoreLayerId)) m_layerIds.append(lineCoreLayerId);
    }

    // // 7. Layer: Callsign / Name Label
    // if (!map->layerExists(labelLayerId)) {
    //     QVariantMap layer;
    //     layer[QStringLiteral("id")] = labelLayerId;
    //     layer[QStringLiteral("type")] = QStringLiteral("symbol");
    //     layer[QStringLiteral("source")] = m_sourceId;
    //     layer[QStringLiteral("filter")] = QVariantList{
    //         QStringLiteral("=="),
    //         QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
    //         2
    //     };

    //     QVariantMap layout;
    //     layout[QStringLiteral("text-field")] = QVariantList{
    //         QStringLiteral("coalesce"),
    //         QVariantList{QStringLiteral("get"), QStringLiteral("entity_name")},
    //         QStringLiteral("")
    //     };
    //     layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
    //     layout[QStringLiteral("text-offset")] = QVariantList{0.0, 2.8};
    //     layout[QStringLiteral("text-size")] = 11.0;
    //     layout[QStringLiteral("text-font")] = fontStack;
    //     layout[QStringLiteral("visibility")] = visibility;
    //     layer[QStringLiteral("layout")] = layout;

    //     QVariantMap paint;
    //     paint[QStringLiteral("text-color")] = QStringLiteral("#ffffff");
    //     paint[QStringLiteral("text-halo-color")] = QStringLiteral("#000000");
    //     paint[QStringLiteral("text-halo-width")] = 2.0;
    //     layer[QStringLiteral("paint")] = paint;

    //     map->addLayer(labelLayerId, layer);
    //     if (!m_layerIds.contains(labelLayerId)) m_layerIds.append(labelLayerId);
    // }
}

/**
 * @brief Configures GPU vector layers for Polygon entities (Entity Type 3).
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Creates polygon fill, perimeter outline, 4-way spatial annotations, and callsign label.
 */
void ComplexEntityMapRenderer::setupPolygonEntityLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString polyFillLayerId = m_layerPrefix + QStringLiteral("_polygon_fill");
    const QString polyLineLayerId = m_layerPrefix + QStringLiteral("_polygon_outline");
    const QString annTopLayerId   = m_layerPrefix + QStringLiteral("_polygon_annotation_top");
    const QString annBottomLayerId= m_layerPrefix + QStringLiteral("_polygon_annotation_bottom");
    const QString annLeftLayerId  = m_layerPrefix + QStringLiteral("_polygon_annotation_left");
    const QString annRightLayerId = m_layerPrefix + QStringLiteral("_polygon_annotation_right");
    const QString labelLayerId    = m_layerPrefix + QStringLiteral("_polygon_label");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Layer: Polygon Fill (entity_type == 3)
    if (!map->layerExists(polyFillLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = polyFillLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("fill");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            3
        };

        QVariantMap paint;
        paint[QStringLiteral("fill-color")] = QStringLiteral("#7c4dff");
        paint[QStringLiteral("fill-opacity")] = 0.25;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(polyFillLayerId, layer);
        if (!m_layerIds.contains(polyFillLayerId)) m_layerIds.append(polyFillLayerId);
    }

    // 2. Layer: Polygon Outline (entity_type == 3)
    if (!map->layerExists(polyLineLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = polyLineLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            3
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QStringLiteral("#b388ff");
        paint[QStringLiteral("line-width")] = 2.0;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(polyLineLayerId, layer);
        if (!m_layerIds.contains(polyLineLayerId)) m_layerIds.append(polyLineLayerId);
    }

    // 3. Layer: Top Annotation
    if (!map->layerExists(annTopLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annTopLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            3
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("top_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("bottom");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, -1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffab00");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annTopLayerId, layer);
        if (!m_layerIds.contains(annTopLayerId)) m_layerIds.append(annTopLayerId);
    }

    // 4. Layer: Bottom Annotation
    if (!map->layerExists(annBottomLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annBottomLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            3
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("bottom_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e5ff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annBottomLayerId, layer);
        if (!m_layerIds.contains(annBottomLayerId)) m_layerIds.append(annBottomLayerId);
    }

    // 5. Layer: Left Annotation
    if (!map->layerExists(annLeftLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annLeftLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            3
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("left_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("right");
        layout[QStringLiteral("text-offset")] = QVariantList{-1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e676");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annLeftLayerId, layer);
        if (!m_layerIds.contains(annLeftLayerId)) m_layerIds.append(annLeftLayerId);
    }

    // 6. Layer: Right Annotation
    if (!map->layerExists(annRightLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annRightLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            3
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("right_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("left");
        layout[QStringLiteral("text-offset")] = QVariantList{1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ff5252");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annRightLayerId, layer);
        if (!m_layerIds.contains(annRightLayerId)) m_layerIds.append(annRightLayerId);
    }

    // 7. Layer: Callsign / Name Label
    if (!map->layerExists(labelLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = labelLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            3
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_name")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 2.8};
        layout[QStringLiteral("text-size")] = 11.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffffff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#000000");
        paint[QStringLiteral("text-halo-width")] = 2.0;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(labelLayerId, layer);
        if (!m_layerIds.contains(labelLayerId)) m_layerIds.append(labelLayerId);
    }
}

/**
 * @brief Configures GPU symbol layers for Custom Image Icon entities (Entity Type 5).
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Configures tactical diamond sprite, 4-way spatial annotations, and callsign label.
 */
void ComplexEntityMapRenderer::setupCustomIconEntityLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString iconLayerId     = m_layerPrefix + QStringLiteral("_icon");
    const QString annTopLayerId   = m_layerPrefix + QStringLiteral("_icon_annotation_top");
    const QString annBottomLayerId= m_layerPrefix + QStringLiteral("_icon_annotation_bottom");
    const QString annLeftLayerId  = m_layerPrefix + QStringLiteral("_icon_annotation_left");
    const QString annRightLayerId = m_layerPrefix + QStringLiteral("_icon_annotation_right");
    const QString labelLayerId    = m_layerPrefix + QStringLiteral("_icon_label");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Layer: Custom Image Sprite (entity_type == 5)
    if (!map->layerExists(iconLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = iconLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            5
        };

        QVariantMap layout;
        layout[QStringLiteral("icon-image")] = QStringLiteral("complex_icon");
        layout[QStringLiteral("icon-size")] = 0.75;
        layout[QStringLiteral("icon-allow-overlap")] = true;
        layout[QStringLiteral("icon-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(iconLayerId, layer);
        if (!m_layerIds.contains(iconLayerId)) m_layerIds.append(iconLayerId);
    }

    // 2. Layer: Top Annotation
    if (!map->layerExists(annTopLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annTopLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            5
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("top_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("bottom");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, -1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffab00");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annTopLayerId, layer);
        if (!m_layerIds.contains(annTopLayerId)) m_layerIds.append(annTopLayerId);
    }

    // 3. Layer: Bottom Annotation
    if (!map->layerExists(annBottomLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annBottomLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            5
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("bottom_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e5ff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annBottomLayerId, layer);
        if (!m_layerIds.contains(annBottomLayerId)) m_layerIds.append(annBottomLayerId);
    }

    // 4. Layer: Left Annotation
    if (!map->layerExists(annLeftLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annLeftLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            5
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("left_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("right");
        layout[QStringLiteral("text-offset")] = QVariantList{-1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e676");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annLeftLayerId, layer);
        if (!m_layerIds.contains(annLeftLayerId)) m_layerIds.append(annLeftLayerId);
    }

    // 5. Layer: Right Annotation
    if (!map->layerExists(annRightLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annRightLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            5
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("right_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("left");
        layout[QStringLiteral("text-offset")] = QVariantList{1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ff5252");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annRightLayerId, layer);
        if (!m_layerIds.contains(annRightLayerId)) m_layerIds.append(annRightLayerId);
    }

    // 6. Layer: Callsign / Name Label
    if (!map->layerExists(labelLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = labelLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            5
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_name")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 2.8};
        layout[QStringLiteral("text-size")] = 11.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffffff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#000000");
        paint[QStringLiteral("text-halo-width")] = 2.0;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(labelLayerId, layer);
        if (!m_layerIds.contains(labelLayerId)) m_layerIds.append(labelLayerId);
    }
}

/**
 * @brief Configures GPU symbol layers for Custom Painter entities (Entity Type 6).
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Configures directional chevron sprite, 4-way spatial annotations, and callsign label.
 */
void ComplexEntityMapRenderer::setupCustomPainterEntityLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString painterLayerId  = m_layerPrefix + QStringLiteral("_custom_painter");
    const QString annTopLayerId   = m_layerPrefix + QStringLiteral("_painter_annotation_top");
    const QString annBottomLayerId= m_layerPrefix + QStringLiteral("_painter_annotation_bottom");
    const QString annLeftLayerId  = m_layerPrefix + QStringLiteral("_painter_annotation_left");
    const QString annRightLayerId = m_layerPrefix + QStringLiteral("_painter_annotation_right");
    const QString labelLayerId    = m_layerPrefix + QStringLiteral("_painter_label");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Layer: Custom Painter with Directional Heading (entity_type == 6)
    if (!map->layerExists(painterLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = painterLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            6
        };

        QVariantMap layout;
        layout[QStringLiteral("icon-image")] = QStringLiteral("complex_painter_icon");
        layout[QStringLiteral("icon-rotate")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("heading")},
            0.0
        };
        layout[QStringLiteral("icon-rotation-alignment")] = QStringLiteral("map");
        layout[QStringLiteral("icon-size")] = 0.75;
        layout[QStringLiteral("icon-allow-overlap")] = true;
        layout[QStringLiteral("icon-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(painterLayerId, layer);
        if (!m_layerIds.contains(painterLayerId)) m_layerIds.append(painterLayerId);
    }

    // 2. Layer: Top Annotation
    if (!map->layerExists(annTopLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annTopLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            6
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("top_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("bottom");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, -1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffab00");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annTopLayerId, layer);
        if (!m_layerIds.contains(annTopLayerId)) m_layerIds.append(annTopLayerId);
    }

    // 3. Layer: Bottom Annotation
    if (!map->layerExists(annBottomLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annBottomLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            6
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("bottom_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 1.6};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e5ff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annBottomLayerId, layer);
        if (!m_layerIds.contains(annBottomLayerId)) m_layerIds.append(annBottomLayerId);
    }

    // 4. Layer: Left Annotation
    if (!map->layerExists(annLeftLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annLeftLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            6
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("left_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("right");
        layout[QStringLiteral("text-offset")] = QVariantList{-1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#00e676");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annLeftLayerId, layer);
        if (!m_layerIds.contains(annLeftLayerId)) m_layerIds.append(annLeftLayerId);
    }

    // 5. Layer: Right Annotation
    if (!map->layerExists(annRightLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = annRightLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            6
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("right_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("left");
        layout[QStringLiteral("text-offset")] = QVariantList{1.6, 0.0};
        layout[QStringLiteral("text-size")] = 10.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ff5252");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#10141a");
        paint[QStringLiteral("text-halo-width")] = 1.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(annRightLayerId, layer);
        if (!m_layerIds.contains(annRightLayerId)) m_layerIds.append(annRightLayerId);
    }

    // 6. Layer: Callsign / Name Label
    if (!map->layerExists(labelLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = labelLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            6
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_name")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 2.8};
        layout[QStringLiteral("text-size")] = 11.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#ffffff");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#000000");
        paint[QStringLiteral("text-halo-width")] = 2.0;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(labelLayerId, layer);
        if (!m_layerIds.contains(labelLayerId)) m_layerIds.append(labelLayerId);
    }
}

/**
 * @brief Configures GPU vector layers for Formation Boundary entities (Entity Type 7).
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Configures boundary glow, solid/dashed/dotted line styles, midpoint echelon symbol,
 *       and dual top/bottom flank unit designation labels. No top/bottom/name annotations.
 */
void ComplexEntityMapRenderer::setupFormationBoundaryEntityLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString boundaryGlowLayerId       = m_layerPrefix + QStringLiteral("_boundary_glow");
    const QString boundaryLineSolidLayerId  = m_layerPrefix + QStringLiteral("_boundary_line_solid");
    const QString boundaryLineDashedLayerId = m_layerPrefix + QStringLiteral("_boundary_line_dashed");
    const QString boundaryLineDottedLayerId = m_layerPrefix + QStringLiteral("_boundary_line_dotted");
    const QString boundaryEchelonLayerId    = m_layerPrefix + QStringLiteral("_boundary_echelon");
    const QString boundaryLeftLayerId       = m_layerPrefix + QStringLiteral("_boundary_left");
    const QString boundaryRightLayerId      = m_layerPrefix + QStringLiteral("_boundary_right");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Formation Boundary Glow
    if (!map->layerExists(boundaryGlowLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = boundaryGlowLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            7
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#ffd600")
        };
        paint[QStringLiteral("line-width")] = 8.0;
        paint[QStringLiteral("line-blur")] = 3.0;
        paint[QStringLiteral("line-opacity")] = 0.35;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(boundaryGlowLayerId, layer);
        if (!m_layerIds.contains(boundaryGlowLayerId)) m_layerIds.append(boundaryGlowLayerId);
    }

    // 2. Formation Boundary Line - Solid (special_param2 != 2 and != 3)
    if (!map->layerExists(boundaryLineSolidLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = boundaryLineSolidLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 7},
            QVariantList{QStringLiteral("!="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dashed")}, true},
            QVariantList{QStringLiteral("!="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dotted")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#ffd600")
        };
        paint[QStringLiteral("line-width")] = 3.0;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(boundaryLineSolidLayerId, layer);
        if (!m_layerIds.contains(boundaryLineSolidLayerId)) m_layerIds.append(boundaryLineSolidLayerId);
    }

    // 3. Formation Boundary Line - Dashed (special_param2 == 2)
    if (!map->layerExists(boundaryLineDashedLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = boundaryLineDashedLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 7},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dashed")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#ffd600")
        };
        paint[QStringLiteral("line-width")] = 3.0;
        paint[QStringLiteral("line-dasharray")] = QVariantList{4.0, 2.5};
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(boundaryLineDashedLayerId, layer);
        if (!m_layerIds.contains(boundaryLineDashedLayerId)) m_layerIds.append(boundaryLineDashedLayerId);
    }

    // 4. Formation Boundary Line - Dotted (special_param2 == 3)
    if (!map->layerExists(boundaryLineDottedLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = boundaryLineDottedLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 7},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dotted")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#ffd600")
        };
        paint[QStringLiteral("line-width")] = 3.0;
        paint[QStringLiteral("line-dasharray")] = QVariantList{1.5, 2.0};
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(boundaryLineDottedLayerId, layer);
        if (!m_layerIds.contains(boundaryLineDottedLayerId)) m_layerIds.append(boundaryLineDottedLayerId);
    }

    // 5. Formation Boundary Echelon Indicator (entity_type == 7, midpoint anchor symbol)
    if (!map->layerExists(boundaryEchelonLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = boundaryEchelonLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 7},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_anchor")}, true}
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("echelon_symbol")},
            QStringLiteral("XX")
        };
        layout[QStringLiteral("text-size")] = 14.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-anchor")] = QStringLiteral("center");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 0.0};
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("text-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#ffd600")
        };
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#0a0f18");
        paint[QStringLiteral("text-halo-width")] = 3.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(boundaryEchelonLayerId, layer);
        if (!m_layerIds.contains(boundaryEchelonLayerId)) m_layerIds.append(boundaryEchelonLayerId);
    }

    // 6. Formation Boundary Top Flank Unit Annotation (left_annotation: e.g. "11 Inf Div")
    if (!map->layerExists(boundaryLeftLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = boundaryLeftLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 7},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_anchor")}, true}
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("left_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-size")] = 11.5;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-anchor")] = QStringLiteral("bottom");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, -1.2};
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("text-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#ffd600")
        };
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#0a0f18");
        paint[QStringLiteral("text-halo-width")] = 2.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(boundaryLeftLayerId, layer);
        if (!m_layerIds.contains(boundaryLeftLayerId)) m_layerIds.append(boundaryLeftLayerId);
    }

    // 7. Formation Boundary Bottom Flank Unit Annotation (right_annotation: e.g. "14 Inf Div")
    if (!map->layerExists(boundaryRightLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = boundaryRightLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 7},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_anchor")}, true}
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("right_annotation")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-size")] = 11.5;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-anchor")] = QStringLiteral("top");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 1.2};
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("text-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#ffd600")
        };
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#0a0f18");
        paint[QStringLiteral("text-halo-width")] = 2.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(boundaryRightLayerId, layer);
        if (!m_layerIds.contains(boundaryRightLayerId)) m_layerIds.append(boundaryRightLayerId);
    }
}

/**
 * @brief Configures GPU vector layers for Tactical Deployment Area entities (Entity Type 8).
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Configures deployment glow, solid/dashed/dotted bezier perimeter styles,
 *       and perimeter echelon symbol badge. No annotations or callsign labels are rendered.
 */
void ComplexEntityMapRenderer::setupDeploymentAreaEntityLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString deployGlowLayerId       = m_layerPrefix + QStringLiteral("_deployment_glow");
    const QString deployLineSolidLayerId  = m_layerPrefix + QStringLiteral("_deployment_solid");
    const QString deployLineDashedLayerId = m_layerPrefix + QStringLiteral("_deployment_dashed");
    const QString deployLineDottedLayerId = m_layerPrefix + QStringLiteral("_deployment_dotted");
    const QString deployEchelonLayerId    = m_layerPrefix + QStringLiteral("_deployment_echelon");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Tactical Deployment Area Glow (entity_type == 8)
    if (!map->layerExists(deployGlowLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = deployGlowLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")},
            8
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#2979ff")
        };
        paint[QStringLiteral("line-width")] = 6.0;
        paint[QStringLiteral("line-blur")] = 3.0;
        paint[QStringLiteral("line-opacity")] = 0.35;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(deployGlowLayerId, layer);
        if (!m_layerIds.contains(deployGlowLayerId)) m_layerIds.append(deployGlowLayerId);
    }

    // 2. Tactical Deployment Area Line - Solid (special_param2 != 2 and != 3)
    if (!map->layerExists(deployLineSolidLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = deployLineSolidLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 8},
            QVariantList{QStringLiteral("!="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dashed")}, true},
            QVariantList{QStringLiteral("!="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dotted")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#2979ff")
        };
        paint[QStringLiteral("line-width")] = 2.5;
        paint[QStringLiteral("line-opacity")] = 0.95;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(deployLineSolidLayerId, layer);
        if (!m_layerIds.contains(deployLineSolidLayerId)) m_layerIds.append(deployLineSolidLayerId);
    }

    // 3. Tactical Deployment Area Line - Dashed (special_param2 == 2)
    if (!map->layerExists(deployLineDashedLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = deployLineDashedLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 8},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dashed")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#2979ff")
        };
        paint[QStringLiteral("line-width")] = 2.5;
        paint[QStringLiteral("line-opacity")] = 0.95;
        paint[QStringLiteral("line-dasharray")] = QVariantList{4.0, 2.5};
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(deployLineDashedLayerId, layer);
        if (!m_layerIds.contains(deployLineDashedLayerId)) m_layerIds.append(deployLineDashedLayerId);
    }

    // 4. Tactical Deployment Area Line - Dotted (special_param2 == 3)
    if (!map->layerExists(deployLineDottedLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = deployLineDottedLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("line");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 8},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_dotted")}, true}
        };

        QVariantMap paint;
        paint[QStringLiteral("line-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#2979ff")
        };
        paint[QStringLiteral("line-width")] = 2.5;
        paint[QStringLiteral("line-opacity")] = 0.95;
        paint[QStringLiteral("line-dasharray")] = QVariantList{1.5, 2.0};
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layout[QStringLiteral("line-cap")] = QStringLiteral("round");
        layout[QStringLiteral("line-join")] = QStringLiteral("round");
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(deployLineDottedLayerId, layer);
        if (!m_layerIds.contains(deployLineDottedLayerId)) m_layerIds.append(deployLineDottedLayerId);
    }

    // 5. Tactical Deployment Area Echelon Symbol (entity_type == 8, perimeter anchor)
    if (!map->layerExists(deployEchelonLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = deployEchelonLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("all"),
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("entity_type")}, 8},
            QVariantList{QStringLiteral("=="), QVariantList{QStringLiteral("get"), QStringLiteral("is_anchor")}, true}
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("echelon_symbol")},
            QStringLiteral("X")
        };
        layout[QStringLiteral("text-size")] = 14.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-anchor")] = QStringLiteral("center");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, 0.0};
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("text-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("boundary_color")},
            QStringLiteral("#2979ff")
        };
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#0a0f18");
        paint[QStringLiteral("text-halo-width")] = 3.5;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(deployEchelonLayerId, layer);
        if (!m_layerIds.contains(deployEchelonLayerId)) m_layerIds.append(deployEchelonLayerId);
    }
}

/**
 * @brief Configures GPU vector layers for interactive control point vertex editing.
 * @param[in] map Pointer to active QMapLibre::Map engine instance.
 * @param[in] visibility Initial visibility state ("visible" or "none").
 * @note Renders high-contrast tactical yellow glow halo, solid circular core with selection highlight,
 *       and 1-indexed numeric vertex position labels.
 */
void ComplexEntityMapRenderer::setupControlPointEditLayers(QMapLibre::Map *map, const QString &visibility)
{
    if (!map) return;

    const QString controlPointGlowLayerId  = m_layerPrefix + QStringLiteral("_control_point_glow");
    const QString controlPointCoreLayerId  = m_layerPrefix + QStringLiteral("_control_point_core");
    const QString controlPointLabelLayerId = m_layerPrefix + QStringLiteral("_control_point_label");

    const QVariant fontStack = QVariantList{
        QStringLiteral("Open Sans Regular"),
        QStringLiteral("Arial Unicode MS Regular")
    };

    // 1. Layer: Control Point Glow Halo
    if (!map->layerExists(controlPointGlowLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = controlPointGlowLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("circle");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("is_control_point")},
            true
        };

        QVariantMap paint;
        paint[QStringLiteral("circle-color")] = QStringLiteral("#facc15");
        paint[QStringLiteral("circle-radius")] = 14.0;
        paint[QStringLiteral("circle-blur")] = 0.8;
        paint[QStringLiteral("circle-opacity")] = 0.55;
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(controlPointGlowLayerId, layer);
        if (!m_layerIds.contains(controlPointGlowLayerId)) m_layerIds.append(controlPointGlowLayerId);
    }

    // 2. Layer: Control Point Core (Tactical Yellow Dot with Selection Highlight)
    if (!map->layerExists(controlPointCoreLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = controlPointCoreLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("circle");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("is_control_point")},
            true
        };

        QVariantMap paint;
        paint[QStringLiteral("circle-color")] = QVariantList{
            QStringLiteral("case"),
            QVariantList{QStringLiteral("get"), QStringLiteral("is_selected")},
            QStringLiteral("#ffffff"),
            QStringLiteral("#facc15")
        };
        paint[QStringLiteral("circle-radius")] = QVariantList{
            QStringLiteral("case"),
            QVariantList{QStringLiteral("get"), QStringLiteral("is_selected")},
            9.0,
            7.0
        };
        paint[QStringLiteral("circle-stroke-width")] = 2.0;
        paint[QStringLiteral("circle-stroke-color")] = QStringLiteral("#0f172a");
        layer[QStringLiteral("paint")] = paint;

        QVariantMap layout;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        map->addLayer(controlPointCoreLayerId, layer);
        if (!m_layerIds.contains(controlPointCoreLayerId)) m_layerIds.append(controlPointCoreLayerId);
    }

    // 3. Layer: Control Point Index Label (1, 2, 3...)
    if (!map->layerExists(controlPointLabelLayerId)) {
        QVariantMap layer;
        layer[QStringLiteral("id")] = controlPointLabelLayerId;
        layer[QStringLiteral("type")] = QStringLiteral("symbol");
        layer[QStringLiteral("source")] = m_sourceId;
        layer[QStringLiteral("filter")] = QVariantList{
            QStringLiteral("=="),
            QVariantList{QStringLiteral("get"), QStringLiteral("is_control_point")},
            true
        };

        QVariantMap layout;
        layout[QStringLiteral("text-field")] = QVariantList{
            QStringLiteral("coalesce"),
            QVariantList{QStringLiteral("get"), QStringLiteral("point_index_str")},
            QStringLiteral("")
        };
        layout[QStringLiteral("text-anchor")] = QStringLiteral("bottom");
        layout[QStringLiteral("text-offset")] = QVariantList{0.0, -1.2};
        layout[QStringLiteral("text-size")] = 11.0;
        layout[QStringLiteral("text-font")] = fontStack;
        layout[QStringLiteral("text-allow-overlap")] = true;
        layout[QStringLiteral("text-ignore-placement")] = true;
        layout[QStringLiteral("visibility")] = visibility;
        layer[QStringLiteral("layout")] = layout;

        QVariantMap paint;
        paint[QStringLiteral("text-color")] = QStringLiteral("#facc15");
        paint[QStringLiteral("text-halo-color")] = QStringLiteral("#0f172a");
        paint[QStringLiteral("text-halo-width")] = 2.0;
        layer[QStringLiteral("paint")] = paint;

        map->addLayer(controlPointLabelLayerId, layer);
        if (!m_layerIds.contains(controlPointLabelLayerId)) m_layerIds.append(controlPointLabelLayerId);
    }
}

} // namespace GISApp::UI::Renderers

/**
 * @file SampleEntityMapFeatureAdapter.h
 * @brief Adapter pattern bridging SampleEntity domain model into IMapFeature for MapLibre GPU rendering.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYMAPFEATUREADAPTER_H
#define SAMPLEENTITYMAPFEATUREADAPTER_H

#include "IMapFeature.h"
#include "sampleentity.h"

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QPointF>
#include <cmath>

namespace GISApp::UI::Renderers {

/**
 * @class SampleEntityMapFeatureAdapter
 * @brief Projects domain SampleEntity instances into IMapFeature for MapLibre Native GPU rendering.
 *
 * Architectural Role & Design Patterns:
 * - Implements the **Adapter Pattern** (GoF) within Presentation / View layer (`ui/map/renderers/`).
 * - Decouples domain model (`SampleEntity`) from MapLibre GPU rendering primitives and GeoJSON syntax.
 * - Supports the three canonical wire structure entity types (`WireStructures.h` line 174):
 *   - Type 1: Point marker.
 *   - Type 2: Parametric cubic Bezier curve evaluated around the geodetic anchor point.
 *   - Type 3: SVG/PNG tactical symbol icon (`sample.svg` / `sample.png`).
 */
class SampleEntityMapFeatureAdapter : public GISApp::Core::Interfaces::IMapFeature
{
public:
    /**
     * @brief Geometry role when adapting an entity.
     */
    enum class SubFeatureType {
        Primary,     ///< Primary geometry (Point for type 1/3, LineString for type 2)
        CenterAnchor ///< Center geodetic point anchor (used for type 2 curve labeling/hit-testing)
    };

    /**
     * @brief Default constructor.
     */
    SampleEntityMapFeatureAdapter() = default;

    /**
     * @brief Constructs adapter wrapping a pointer to a SampleEntity domain entity.
     * @param[in] entity Pointer to the domain entity.
     * @param[in] subType Primary or center anchor sub-feature.
     */
    explicit SampleEntityMapFeatureAdapter(const GISApp::Domain::SampleEntities::SampleEntity *entity,
                                           SubFeatureType subType = SubFeatureType::Primary)
        : m_entity(entity)
        , m_subType(subType)
    {
    }

    /**
     * @brief Constructs adapter wrapping a reference to a SampleEntity domain entity.
     * @param[in] entity Const reference to the domain entity.
     * @param[in] subType Primary or center anchor sub-feature.
     */
    explicit SampleEntityMapFeatureAdapter(const GISApp::Domain::SampleEntities::SampleEntity &entity,
                                           SubFeatureType subType = SubFeatureType::Primary)
        : m_entity(&entity)
        , m_subType(subType)
    {
    }

    /**
     * @brief Virtual destructor.
     */
    virtual ~SampleEntityMapFeatureAdapter() override = default;

    // --- IMapFeature Implementation ---

    /**
     * @brief Returns unique feature identifier for MapLibre source matching.
     * @return Stringified entity identifier.
     */
    [[nodiscard]] QString featureId() const override
    {
        if (!m_entity) return QStringLiteral("0");
        if (m_subType == SubFeatureType::CenterAnchor) {
            return QStringLiteral("sample_%1_anchor").arg(m_entity->Id());
        }
        return QString::number(m_entity->Id());
    }

    /**
     * @brief Category classification for map feature filtering.
     * @return Category string literal "sample_entity".
     */
    [[nodiscard]] QString featureCategory() const override
    {
        return QStringLiteral("sample_entity");
    }

    /**
     * @brief WGS-84 Latitude in decimal degrees.
     * @return Latitude [-90.0, 90.0].
     */
    [[nodiscard]] double latitude() const override
    {
        return m_entity ? m_entity->location().latatitude : 0.0;
    }

    /**
     * @brief WGS-84 Longitude in decimal degrees.
     * @return Longitude [-180.0, 180.0].
     */
    [[nodiscard]] double longitude() const override
    {
        return m_entity ? m_entity->location().longitude : 0.0;
    }

    /**
     * @brief Geometric altitude in meters above mean sea level.
     * @return Altitude value.
     */
    [[nodiscard]] double altitude() const override
    {
        return m_entity ? m_entity->location().height : 0.0;
    }

    /**
     * @brief Heading / direction in decimal degrees.
     * @return Heading in [0.0, 360.0).
     */
    [[nodiscard]] double heading() const override
    {
        return m_entity ? m_entity->location().dir : 0.0;
    }

    /**
     * @brief Speed scalar value.
     * @return 0.0
     */
    [[nodiscard]] double speed() const override
    {
        return 0.0;
    }

    /**
     * @brief Display callsign or formatted entity label.
     * @return Label string, defaulting to "SMPL-<id>" if empty.
     */
    [[nodiscard]] QString callsign() const override
    {
        if (!m_entity) return QStringLiteral("SMPL-0");
        return m_entity->Name().isEmpty()
            ? QStringLiteral("SMPL-%1").arg(m_entity->Id())
            : m_entity->Name();
    }

    /**
     * @brief Affiliation or tactical identity string.
     * @return Identity string "SAMPLE".
     */
    [[nodiscard]] QString identityString() const override
    {
        return QStringLiteral("SAMPLE");
    }

    /**
     * @brief Operational domain classification string.
     * @return Domain string "LAND".
     */
    [[nodiscard]] QString domainString() const override
    {
        return QStringLiteral("LAND");
    }

    /**
     * @brief Military/standard symbology identifier or icon resource name.
     * @return Resource name "sample_icon".
     */
    [[nodiscard]] QString symbolCode() const override
    {
        return QStringLiteral("sample_icon");
    }

    /**
     * @brief Resolves visual hex color based on entity type.
     * @return Formatted hex color string.
     */
    [[nodiscard]] QString colorHex() const override
    {
        if (!m_entity) return QStringLiteral("#00e676");
        switch (m_entity->type()) {
        case 1:
            return QStringLiteral("#00e676"); // Vibrant Emerald Green for Point
        case 2:
            return QStringLiteral("#ffab00"); // Amber/Orange for Bezier Curve
        case 3:
            return QStringLiteral("#00e5ff"); // Cyan for Tactical Icon
        default:
            return QStringLiteral("#00e676");
        }
    }

    /**
     * @brief Generates interpolated cubic Bezier curve points centered around a geographic coordinate.
     *
     * Constructs control points $P_0, P_1, P_2, P_3$ such that the curve smoothly flows through
     * the center point at $t=0.5$, oriented along heading angle $\alpha$.
     *
     * @param[in] centerLon Longitude of center coordinate.
     * @param[in] centerLat Latitude of center coordinate.
     * @param[in] headingDeg Heading angle in degrees (clockwise from North).
     * @param[in] spanDeg Characteristic geographic radius/span in degrees (default ~0.04 deg).
     * @param[in] steps Number of interpolation segments (default 32).
     * @return QVector of interpolated geodetic coordinates (x=lon, y=lat).
     */
    [[nodiscard]] static QVector<QPointF> generateBezierCurve(double centerLon,
                                                               double centerLat,
                                                               double headingDeg,
                                                               double spanDeg = 0.04,
                                                               int steps = 32)
    {
        QVector<QPointF> points;
        points.reserve(steps + 1);

        // Compute orientation angle in radians (default 45 degrees if heading is 0)
        double angleRad = (std::abs(headingDeg) > 0.001)
                          ? (headingDeg * M_PI / 180.0)
                          : (45.0 * M_PI / 180.0);

        // Unit tangent vector along curve orientation and perpendicular normal vector
        double ux = std::sin(angleRad);
        double uy = std::cos(angleRad);
        double vx = -uy;
        double vy = ux;

        // Control points: P0, P1, P2, P3 configured so that B(0.5) = (centerLon, centerLat)
        QPointF p0(centerLon - 0.5 * spanDeg * ux, centerLat - 0.5 * spanDeg * uy);
        QPointF p1(centerLon - 0.2 * spanDeg * ux + 0.4 * spanDeg * vx,
                   centerLat - 0.2 * spanDeg * uy + 0.4 * spanDeg * vy);
        QPointF p2(centerLon + 0.2 * spanDeg * ux - 0.4 * spanDeg * vx,
                   centerLat + 0.2 * spanDeg * uy - 0.4 * spanDeg * vy);
        QPointF p3(centerLon + 0.5 * spanDeg * ux, centerLat + 0.5 * spanDeg * uy);

        for (int i = 0; i <= steps; ++i) {
            double t = static_cast<double>(i) / static_cast<double>(steps);
            double oneMinusT = 1.0 - t;
            double b0 = oneMinusT * oneMinusT * oneMinusT;
            double b1 = 3.0 * oneMinusT * oneMinusT * t;
            double b2 = 3.0 * oneMinusT * t * t;
            double b3 = t * t * t;

            double lon = b0 * p0.x() + b1 * p1.x() + b2 * p2.x() + b3 * p3.x();
            double lat = b0 * p0.y() + b1 * p1.y() + b2 * p2.y() + b3 * p3.y();
            points.append(QPointF(lon, lat));
        }

        return points;
    }

    /**
     * @brief Serializes the adapted sample entity into a standard GeoJSON Feature
     *        tailored for MapLibre GPU shaders and data-driven filtering.
     * @return QJsonObject representing the GeoJSON feature.
     */
    [[nodiscard]] QJsonObject toGeoJsonFeature() const override
    {
        QJsonObject feature;
        feature[QStringLiteral("type")] = QStringLiteral("Feature");
        feature[QStringLiteral("id")] = featureId();

        QJsonObject properties;
        properties[QStringLiteral("id")] = featureId();
        properties[QStringLiteral("entity_id")] = m_entity ? static_cast<int>(m_entity->Id()) : 0;
        properties[QStringLiteral("entity_type")] = m_entity ? static_cast<int>(m_entity->type()) : 1;
        properties[QStringLiteral("name")] = callsign();
        properties[QStringLiteral("color")] = colorHex();
        properties[QStringLiteral("height")] = altitude();
        properties[QStringLiteral("dir")] = heading();
        properties[QStringLiteral("remarks")] = m_entity ? m_entity->remarks() : QString();
        properties[QStringLiteral("time")] = m_entity ? m_entity->reportTime().toString(Qt::ISODate) : QString();
        properties[QStringLiteral("icon")] = symbolCode();

        if (m_subType == SubFeatureType::CenterAnchor) {
            properties[QStringLiteral("is_anchor")] = true;
            // Center Point geometry
            QJsonObject geometry;
            geometry[QStringLiteral("type")] = QStringLiteral("Point");
            QJsonArray coordinates;
            coordinates.append(longitude());
            coordinates.append(latitude());
            geometry[QStringLiteral("coordinates")] = coordinates;
            feature[QStringLiteral("geometry")] = geometry;
        } else if (m_entity && m_entity->type() == 2) {
            // Type 2: Parametric cubic Bezier curve LineString
            properties[QStringLiteral("is_anchor")] = false;
            QJsonObject geometry;
            geometry[QStringLiteral("type")] = QStringLiteral("LineString");
            QJsonArray coordinates;

            const auto curvePoints = generateBezierCurve(longitude(), latitude(), heading());
            for (const auto &pt : curvePoints) {
                QJsonArray coordPair;
                coordPair.append(pt.x()); // Lon
                coordPair.append(pt.y()); // Lat
                coordinates.append(coordPair);
            }
            geometry[QStringLiteral("coordinates")] = coordinates;
            feature[QStringLiteral("geometry")] = geometry;
        } else {
            // Type 1 (Point) & Type 3 (Icon): Point geometry
            properties[QStringLiteral("is_anchor")] = false;
            QJsonObject geometry;
            geometry[QStringLiteral("type")] = QStringLiteral("Point");
            QJsonArray coordinates;
            coordinates.append(longitude());
            coordinates.append(latitude());
            geometry[QStringLiteral("coordinates")] = coordinates;
            feature[QStringLiteral("geometry")] = geometry;
        }

        feature[QStringLiteral("properties")] = properties;
        return feature;
    }

private:
    const GISApp::Domain::SampleEntities::SampleEntity *m_entity{nullptr}; ///< Wrapped domain model.
    SubFeatureType m_subType{SubFeatureType::Primary};                     ///< Sub-feature classification.
};

} // namespace GISApp::UI::Renderers

#endif // SAMPLEENTITYMAPFEATUREADAPTER_H

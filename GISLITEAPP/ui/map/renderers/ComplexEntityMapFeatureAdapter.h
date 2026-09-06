/**
 * @file ComplexEntityMapFeatureAdapter.h
 * @brief Adapter pattern projecting ComplexEntity domain models into GeoJSON features for MapLibre rendering.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYMAPFEATUREADAPTER_H
#define COMPLEXENTITYMAPFEATUREADAPTER_H

#include "IMapFeature.h"
#include "ComplexEntity.h"

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QPointF>
#include <cmath>

namespace GISApp::UI::Renderers {

/**
 * @class ComplexEntityMapFeatureAdapter
 * @brief Projects ComplexEntity domain instances into MapLibre IMapFeature GeoJSON geometries.
 *
 * Architectural Role & Design Patterns:
 * - Implements the **Adapter Pattern** (GoF) in the Presentation / View layer (`ui/map/renderers/`).
 * - Bridges domain model (`ComplexEntity`) to MapLibre Native GPU rendering pipeline.
 * - Supports all 6 canonical entity types:
 *   - Type 1: Point marker with halo.
 *   - Type 2: Multi-point LineString with midpoint label anchor.
 *   - Type 3: Closed-ring Polygon fill & outline with centroid label anchor.
 *   - Type 4: Text-only annotation (point geometry without marker symbols).
 *   - Type 5: Custom Image sprite icon.
 *   - Type 6: Custom Painter directional tactical glyph rotated by heading.
 * - Encodes 4-way spatial annotations (Top, Bottom, Left, Right) into GeoJSON feature properties.
 */
class ComplexEntityMapFeatureAdapter : public GISApp::Core::Interfaces::IMapFeature
{
public:
    enum class GeometryRole {
        Primary,     ///< Primary geometry (Point, LineString, or Polygon)
        LabelAnchor, ///< Center/midpoint point anchor for line/polygon annotations
        ControlPoint ///< Individual control point handle for interactive editing (yellow dot)
    };

    /**
     * @brief Default constructor.
     */
    ComplexEntityMapFeatureAdapter() = default;

    /**
     * @brief Constructs adapter from pointer.
     * @param[in] entity Pointer to domain ComplexEntity.
     * @param[in] role Primary geometry or label anchor.
     */
    explicit ComplexEntityMapFeatureAdapter(const GISApp::Domain::ComplexEntities::ComplexEntity *entity,
                                            GeometryRole role = GeometryRole::Primary)
        : m_entity(entity)
        , m_role(role)
    {
    }

    /**
     * @brief Constructs adapter from reference.
     * @param[in] entity Reference to domain ComplexEntity.
     * @param[in] role Primary geometry or label anchor.
     */
    explicit ComplexEntityMapFeatureAdapter(const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
                                            GeometryRole role = GeometryRole::Primary)
        : m_entity(&entity)
        , m_role(role)
    {
    }

    /**
     * @brief Constructs adapter for an individual geometry control point vertex (yellow dot).
     * @param[in] entity Reference to domain ComplexEntity.
     * @param[in] controlPointIndex 0-based vertex index in locationPoints().
     * @param[in] isSelected True if this vertex is the currently active/selected point.
     */
    ComplexEntityMapFeatureAdapter(const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
                                   int controlPointIndex,
                                   bool isSelected = false)
        : m_entity(&entity)
        , m_role(GeometryRole::ControlPoint)
        , m_controlPointIndex(controlPointIndex)
        , m_isSelectedControlPoint(isSelected)
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~ComplexEntityMapFeatureAdapter() override = default;

    // --- IMapFeature Implementation ---

    [[nodiscard]] QString featureId() const override
    {
        if (!m_entity) return QStringLiteral("0");
        if (m_role == GeometryRole::ControlPoint) {
            return QStringLiteral("complex_%1_cp_%2").arg(m_entity->id()).arg(m_controlPointIndex);
        }
        if (m_role == GeometryRole::LabelAnchor) {
            return QStringLiteral("complex_%1_anchor").arg(m_entity->id());
        }
        return QStringLiteral("complex_%1").arg(m_entity->id());
    }

    [[nodiscard]] QString featureCategory() const override
    {
        return QStringLiteral("complex_entity");
    }

    [[nodiscard]] double latitude() const override
    {
        if (!m_entity) return 0.0;
        if (m_role == GeometryRole::ControlPoint) {
            const auto &pts = m_entity->locationPoints();
            if (m_controlPointIndex >= 0 && m_controlPointIndex < pts.size()) {
                return pts[m_controlPointIndex].latatitude;
            }
            return 0.0;
        }
        if (m_role == GeometryRole::LabelAnchor) {
            double cLat = 0.0, cLon = 0.0;
            if (m_entity->entityType() == 8) {
                if (m_entity->calculatePerimeterAnchor(cLat, cLon)) return cLat;
            } else if (m_entity->entityType() == 7 || m_entity->entityType() == 2) {
                if (m_entity->calculateMidpoint(cLat, cLon)) return cLat;
            } else {
                if (m_entity->calculateCentroid(cLat, cLon)) return cLat;
            }
        }
        return m_entity->primaryLocation().latatitude;
    }

    [[nodiscard]] double longitude() const override
    {
        if (!m_entity) return 0.0;
        if (m_role == GeometryRole::ControlPoint) {
            const auto &pts = m_entity->locationPoints();
            if (m_controlPointIndex >= 0 && m_controlPointIndex < pts.size()) {
                return pts[m_controlPointIndex].longitude;
            }
            return 0.0;
        }
        if (m_role == GeometryRole::LabelAnchor) {
            double cLat = 0.0, cLon = 0.0;
            if (m_entity->entityType() == 8) {
                if (m_entity->calculatePerimeterAnchor(cLat, cLon)) return cLon;
            } else if (m_entity->entityType() == 7 || m_entity->entityType() == 2) {
                if (m_entity->calculateMidpoint(cLat, cLon)) return cLon;
            } else {
                if (m_entity->calculateCentroid(cLat, cLon)) return cLon;
            }
        }
        return m_entity->primaryLocation().longitude;
    }

    [[nodiscard]] double altitude() const override
    {
        return m_entity ? m_entity->primaryLocation().height : 0.0;
    }

    [[nodiscard]] double heading() const override
    {
        return m_entity ? m_entity->primaryLocation().dir : 0.0;
    }

    [[nodiscard]] double speed() const override
    {
        return 0.0;
    }

    [[nodiscard]] QString callsign() const override
    {
        if (!m_entity) return QStringLiteral("CPLX-0");
        return m_entity->name().isEmpty()
            ? QStringLiteral("CPLX-%1").arg(m_entity->id())
            : m_entity->name();
    }

    [[nodiscard]] QString identityString() const override
    {
        return QStringLiteral("COMPLEX");
    }

    [[nodiscard]] QString domainString() const override
    {
        return QStringLiteral("LAND");
    }

    [[nodiscard]] QString symbolCode() const override
    {
        if (!m_entity) return QStringLiteral("complex_icon");
        return (m_entity->entityType() == 6)
            ? QStringLiteral("complex_painter_icon")
            : QStringLiteral("complex_icon");
    }

    [[nodiscard]] QString colorHex() const override
    {
        if (!m_entity) return QStringLiteral("#d500f9");
        switch (m_entity->entityType()) {
        case 1: return QStringLiteral("#d500f9");
        case 2: return QStringLiteral("#00e5ff");
        case 3: return QStringLiteral("#7c4dff");
        case 4: return QStringLiteral("#ffab00");
        case 5: return QStringLiteral("#2979ff");
        case 6: return QStringLiteral("#ff1744");
        case 7:
            return (m_entity->specialParam3() == 2)
                ? QStringLiteral("#ff1744")  // Hostile Formation Boundary
                : QStringLiteral("#ffd600"); // Friendly Formation Boundary
        case 8:
            return QStringLiteral("#2979ff"); // Tactical Deployment Blue
        default: return QStringLiteral("#d500f9");
        }
    }

    /**
     * @brief Generates a smooth, closed Catmull-Rom / cubic Bezier spline loop.
     *
     * Given a set of geodetic control points (N >= 3), generates a smooth C1-continuous
     * closed boundary passing through every control point. If N < 3, generates a
     * rounded convex envelope around available points.
     *
     * @param[in] points Geodetic control points.
     * @param[in] stepsPerSegment Number of interpolation steps between consecutive points.
     * @return QVector of interpolated geodetic coordinates (x=lon, y=lat).
     */
    [[nodiscard]] static QVector<QPointF> generateClosedBezierSpline(
        const QVector<STRUCT_LOCATION> &points,
        int stepsPerSegment = 12)
    {
        QVector<QPointF> result;
        const int n = points.size();
        if (n == 0) return result;

        if (n == 1) {
            const double cLon = points.first().longitude;
            const double cLat = points.first().latatitude;
            constexpr double rx = 0.03;
            constexpr double ry = 0.02;
            constexpr int totalSteps = 36;
            for (int i = 0; i <= totalSteps; ++i) {
                double theta = 2.0 * M_PI * i / totalSteps;
                result.append(QPointF(cLon + rx * std::cos(theta), cLat + ry * std::sin(theta)));
            }
            return result;
        }

        if (n == 2) {
            const double lon1 = points[0].longitude, lat1 = points[0].latatitude;
            const double lon2 = points[1].longitude, lat2 = points[1].latatitude;
            const double midLon = (lon1 + lon2) * 0.5;
            const double midLat = (lat1 + lat2) * 0.5;
            const double dx = (lon2 - lon1) * 0.5;
            const double dy = (lat2 - lat1) * 0.5;
            const double normalX = -dy * 0.4;
            const double normalY = dx * 0.4;

            constexpr int totalSteps = 36;
            for (int i = 0; i <= totalSteps; ++i) {
                double theta = 2.0 * M_PI * i / totalSteps;
                double lon = midLon + dx * std::cos(theta) + normalX * std::sin(theta);
                double lat = midLat + dy * std::cos(theta) + normalY * std::sin(theta);
                result.append(QPointF(lon, lat));
            }
            return result;
        }

        // N >= 3: Smooth closed Catmull-Rom spline converted to cubic Bezier segments
        result.reserve(n * stepsPerSegment + 1);

        for (int i = 0; i < n; ++i) {
            const auto &pPrev = points[(i - 1 + n) % n];
            const auto &p0    = points[i];
            const auto &p1    = points[(i + 1) % n];
            const auto &pNext = points[(i + 2) % n];

            // Cubic Bezier control points from Catmull-Rom formulation
            const double b0_x = p0.longitude;
            const double b0_y = p0.latatitude;

            const double b1_x = p0.longitude + (p1.longitude - pPrev.longitude) / 6.0;
            const double b1_y = p0.latatitude + (p1.latatitude - pPrev.latatitude) / 6.0;

            const double b2_x = p1.longitude - (pNext.longitude - p0.longitude) / 6.0;
            const double b2_y = p1.latatitude - (pNext.latatitude - p0.latatitude) / 6.0;

            const double b3_x = p1.longitude;
            const double b3_y = p1.latatitude;

            for (int s = 0; s < stepsPerSegment; ++s) {
                const double t = static_cast<double>(s) / static_cast<double>(stepsPerSegment);
                const double oneMinusT = 1.0 - t;
                const double c0 = oneMinusT * oneMinusT * oneMinusT;
                const double c1 = 3.0 * oneMinusT * oneMinusT * t;
                const double c2 = 3.0 * oneMinusT * t * t;
                const double c3 = t * t * t;

                const double lon = c0 * b0_x + c1 * b1_x + c2 * b2_x + c3 * b3_x;
                const double lat = c0 * b0_y + c1 * b1_y + c2 * b2_y + c3 * b3_y;
                result.append(QPointF(lon, lat));
            }
        }

        if (!result.isEmpty()) {
            result.append(result.first());
        }

        return result;
    }

    [[nodiscard]] QJsonObject toGeoJsonFeature() const override
    {
        QJsonObject feature;
        feature[QStringLiteral("type")] = QStringLiteral("Feature");

        if (!m_entity) {
            return feature;
        }

        const UINT_8 type = m_entity->entityType();
        const auto &points = m_entity->locationPoints();

        // 1. Build Geometry
        QJsonObject geometry;

        if (m_role == GeometryRole::ControlPoint) {
            geometry[QStringLiteral("type")] = QStringLiteral("Point");
            QJsonArray coords;
            if (m_controlPointIndex >= 0 && m_controlPointIndex < points.size()) {
                coords.append(points[m_controlPointIndex].longitude);
                coords.append(points[m_controlPointIndex].latatitude);
            } else {
                coords.append(0.0);
                coords.append(0.0);
            }
            geometry[QStringLiteral("coordinates")] = coords;
            feature[QStringLiteral("geometry")] = geometry;

            QJsonObject props;
            props[QStringLiteral("entity_id")] = static_cast<int>(m_entity->id());
            props[QStringLiteral("entity_type")] = static_cast<int>(type);
            props[QStringLiteral("is_control_point")] = true;
            props[QStringLiteral("point_index")] = m_controlPointIndex;
            props[QStringLiteral("point_number")] = m_controlPointIndex + 1;
            props[QStringLiteral("point_index_str")] = QString::number(m_controlPointIndex + 1);
            props[QStringLiteral("is_selected")] = m_isSelectedControlPoint;
            feature[QStringLiteral("properties")] = props;
            return feature;
        }

        if (m_role == GeometryRole::LabelAnchor) {
            // Anchor point for lines, polygons, formation boundaries, or deployment areas
            double cLat = 0.0, cLon = 0.0;
            bool ok = false;
            if (type == 8) {
                ok = m_entity->calculatePerimeterAnchor(cLat, cLon);
            } else if (type == 7 || type == 2) {
                ok = m_entity->calculateMidpoint(cLat, cLon);
            } else {
                ok = m_entity->calculateCentroid(cLat, cLon);
            }
            if (!ok) {
                STRUCT_LOCATION prim = m_entity->primaryLocation();
                cLat = prim.latatitude;
                cLon = prim.longitude;
            }
            geometry[QStringLiteral("type")] = QStringLiteral("Point");
            QJsonArray coords;
            coords.append(cLon);
            coords.append(cLat);
            geometry[QStringLiteral("coordinates")] = coords;
        } else {
            switch (type) {
            case 8: { // Deployment Area (smooth closed Bezier spline loop)
                geometry[QStringLiteral("type")] = QStringLiteral("LineString");
                const auto splinePts = generateClosedBezierSpline(points);
                QJsonArray coords;
                for (const auto &pt : splinePts) {
                    QJsonArray ptCoord;
                    ptCoord.append(pt.x()); // lon
                    ptCoord.append(pt.y()); // lat
                    coords.append(ptCoord);
                }
                if (coords.isEmpty()) {
                    STRUCT_LOCATION prim = m_entity->primaryLocation();
                    coords.append(QJsonArray{prim.longitude, prim.latatitude});
                    coords.append(QJsonArray{prim.longitude + 0.001, prim.latatitude + 0.001});
                }
                geometry[QStringLiteral("coordinates")] = coords;
                break;
            }
            case 2:   // Line
            case 7: { // Formation Boundary (group of points)
                geometry[QStringLiteral("type")] = QStringLiteral("LineString");
                QJsonArray coords;
                for (const auto &pt : points) {
                    QJsonArray ptCoord;
                    ptCoord.append(pt.longitude);
                    ptCoord.append(pt.latatitude);
                    coords.append(ptCoord);
                }
                // Ensure LineString contains at least 2 points per GeoJSON specification
                if (coords.isEmpty()) {
                    STRUCT_LOCATION prim = m_entity->primaryLocation();
                    QJsonArray ptCoord;
                    ptCoord.append(prim.longitude);
                    ptCoord.append(prim.latatitude);
                    coords.append(ptCoord);
                }
                if (coords.size() == 1) {
                    const QJsonArray &firstPt = coords.first().toArray();
                    QJsonArray offsetPt;
                    offsetPt.append(firstPt.at(0).toDouble() + 0.0001);
                    offsetPt.append(firstPt.at(1).toDouble() + 0.0001);
                    coords.append(offsetPt);
                }
                geometry[QStringLiteral("coordinates")] = coords;
                break;
            }
            case 3: { // Polygon
                geometry[QStringLiteral("type")] = QStringLiteral("Polygon");
                QJsonArray ring;
                for (const auto &pt : points) {
                    QJsonArray ptCoord;
                    ptCoord.append(pt.longitude);
                    ptCoord.append(pt.latatitude);
                    ring.append(ptCoord);
                }
                // Ensure closed ring (at least 3 vertices + closing vertex = 4 points)
                if (ring.size() >= 3) {
                    QJsonArray firstPt = ring.first().toArray();
                    QJsonArray lastPt = ring.last().toArray();
                    if (firstPt != lastPt) {
                        ring.append(firstPt);
                    }
                } else {
                    // Fallback to valid minimal triangular ring around primary location
                    STRUCT_LOCATION prim = m_entity->primaryLocation();
                    double cLon = prim.longitude;
                    double cLat = prim.latatitude;
                    double d = 0.001;
                    ring = QJsonArray{
                        QJsonArray{cLon - d, cLat - d},
                        QJsonArray{cLon + d, cLat - d},
                        QJsonArray{cLon, cLat + d},
                        QJsonArray{cLon - d, cLat - d}
                    };
                }
                QJsonArray polygonCoords;
                polygonCoords.append(ring);
                geometry[QStringLiteral("coordinates")] = polygonCoords;
                break;
            }
            case 1: // Point
            case 4: // Text Only
            case 5: // Custom Image
            case 6: // Custom Painter
            default: {
                geometry[QStringLiteral("type")] = QStringLiteral("Point");
                STRUCT_LOCATION prim = m_entity->primaryLocation();
                QJsonArray coords;
                coords.append(prim.longitude);
                coords.append(prim.latatitude);
                geometry[QStringLiteral("coordinates")] = coords;
                break;
            }
            }
        }

        feature[QStringLiteral("geometry")] = geometry;

        // 2. Build Properties
        QJsonObject props;
        props[QStringLiteral("entity_id")] = static_cast<int>(m_entity->id());
        props[QStringLiteral("entity_name")] = m_entity->name();
        props[QStringLiteral("entity_type")] = static_cast<int>(m_entity->entityType());
        props[QStringLiteral("type_desc")] = GISApp::Domain::ComplexEntities::ComplexEntity::entityTypeToString(m_entity->entityType());

        // 4-way spatial annotations
        props[QStringLiteral("left_annotation")] = m_entity->leftAnnotation();
        props[QStringLiteral("right_annotation")] = m_entity->rightAnnotation();
        props[QStringLiteral("top_annotation")] = m_entity->topAnnotation();
        props[QStringLiteral("bottom_annotation")] = m_entity->bottomAnnotation();

        // Special parameters
        props[QStringLiteral("special_param1")] = static_cast<int>(m_entity->specialParam1());
        props[QStringLiteral("special_param2")] = static_cast<int>(m_entity->specialParam2());
        props[QStringLiteral("special_param3")] = static_cast<int>(m_entity->specialParam3());
        props[QStringLiteral("special_param4")] = static_cast<int>(m_entity->specialParam4());

        // Kinematics / Heading
        props[QStringLiteral("heading")] = m_entity->primaryLocation().dir;
        props[QStringLiteral("altitude")] = m_entity->primaryLocation().height;

        props[QStringLiteral("remarks")] = m_entity->remarks();
        props[QStringLiteral("is_anchor")] = (m_role == GeometryRole::LabelAnchor);

        if (type == 5) {
            props[QStringLiteral("icon_name")] = QStringLiteral("complex_icon");
        } else if (type == 6) {
            props[QStringLiteral("icon_name")] = QStringLiteral("complex_painter_icon");
        } else if (type == 7) {
            props[QStringLiteral("is_boundary")] = true;
            QString echelon;
            switch (m_entity->specialParam1()) {
            case 1: echelon = QStringLiteral("•••"); break; // Platoon
            case 2: echelon = QStringLiteral("I"); break;   // Company
            case 3: echelon = QStringLiteral("II"); break;  // Battalion
            case 4: echelon = QStringLiteral("X"); break;   // Brigade
            case 5: echelon = QStringLiteral("XX"); break;  // Division
            case 6: echelon = QStringLiteral("XXX"); break; // Corps
            case 7: echelon = QStringLiteral("XXXX"); break;// Army
            default: echelon = QStringLiteral("XX"); break;
            }
            props[QStringLiteral("echelon_symbol")] = echelon;

            // Line Style (SP2): 1=Solid, 2=Dashed, 3=Dotted
            props[QStringLiteral("line_style")] = static_cast<int>(m_entity->specialParam2());
            props[QStringLiteral("is_dashed")] = (m_entity->specialParam2() == 2);
            props[QStringLiteral("is_dotted")] = (m_entity->specialParam2() == 3);

            // Allegiance (SP3): 1=Friendly, 2=Hostile
            const bool isHostile = (m_entity->specialParam3() == 2);
            props[QStringLiteral("is_hostile")] = isHostile;
            props[QStringLiteral("boundary_color")] = isHostile ? QStringLiteral("#ff1744") : QStringLiteral("#ffd600");
        } else if (type == 8) {
            props[QStringLiteral("is_deployment")] = true;
            QString echelon;
            switch (m_entity->specialParam1()) {
            case 1: echelon = QStringLiteral("•••"); break; // Platoon
            case 2: echelon = QStringLiteral("I"); break;   // Company
            case 3: echelon = QStringLiteral("II"); break;  // Battalion
            case 4: echelon = QStringLiteral("X"); break;   // Brigade
            case 5: echelon = QStringLiteral("XX"); break;  // Division
            case 6: echelon = QStringLiteral("XXX"); break; // Corps
            case 7: echelon = QStringLiteral("XXXX"); break;// Army
            default: echelon = QStringLiteral("X"); break;
            }
            props[QStringLiteral("echelon_symbol")] = echelon;

            // Line Style (SP2): 1=Solid, 2=Dashed, 3=Dotted
            props[QStringLiteral("line_style")] = static_cast<int>(m_entity->specialParam2());
            props[QStringLiteral("is_dashed")] = (m_entity->specialParam2() == 2);
            props[QStringLiteral("is_dotted")] = (m_entity->specialParam2() == 3);
            props[QStringLiteral("boundary_color")] = QStringLiteral("#2979ff");
        }

        feature[QStringLiteral("properties")] = props;
        return feature;
    }

private:
    const GISApp::Domain::ComplexEntities::ComplexEntity *m_entity{nullptr};
    GeometryRole m_role{GeometryRole::Primary};
    int m_controlPointIndex{0};
    bool m_isSelectedControlPoint{false};
};

} // namespace GISApp::UI::Renderers

#endif // COMPLEXENTITYMAPFEATUREADAPTER_H

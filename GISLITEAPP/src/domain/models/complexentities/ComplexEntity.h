/**
 * @file ComplexEntity.h
 * @brief Domain entity modeling a tactical Complex Entity with multi-geometry and dynamic attributes.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITY_H
#define COMPLEXENTITY_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

#include "IrsTypes.h"
#include "WireStructures.h"

namespace GISApp::Domain::ComplexEntities {

/**
 * @class ComplexEntity
 * @brief Domain representation of a Complex Entity matching wire structure STRUCT_COMPLEX_ENTITY.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the **Domain Layer** (`src/domain/models/complexentities/`).
 * - Encapsulates all attributes defined by `STRUCT_COMPLEX_ENTITY`:
 *   - Entity ID, Name, Entity Type (1=Point, 2=Line, 3=Polygon, 4=TextOnly, 5=CustomImage, 6=CustomPainter).
 *   - Geographic coordinate points (vector of `STRUCT_LOCATION`).
 *   - 4-way spatial annotations (Left, Right, Top, Bottom).
 *   - Special operational parameters 1 through 4.
 *   - Dynamic extensible key-value details (vector of `STRUCT_DETAILS`).
 *   - Detection / report timestamp and operational remarks.
 * - Completely free of hallucinated properties; preserves exact wire structure semantic integrity.
 * - Provides serialization helpers for SQLite database storage and centroid/bounding-box computation.
 */
class ComplexEntity
{
public:
    /**
     * @brief Default constructor.
     */
    ComplexEntity() = default;

    /**
     * @brief Constructs ComplexEntity with identification and type.
     * @param[in] id Unique integer entity identifier.
     * @param[in] name User-facing entity callsign/name.
     * @param[in] type Geometry/rendering discriminator [1..6].
     */
    ComplexEntity(UINT_32 id, const QString &name, UINT_8 type);

    /**
     * @brief Destructor.
     */
    ~ComplexEntity() = default;

    // --- Identification & Classification ---

    [[nodiscard]] UINT_32 id() const { return m_entityId; }
    [[nodiscard]] UINT_32 Id() const { return m_entityId; }
    void setId(UINT_32 id) { m_entityId = id; }

    [[nodiscard]] QString name() const { return m_entityName; }
    [[nodiscard]] QString Name() const { return m_entityName; }
    void setName(const QString &name) { m_entityName = name; }

    [[nodiscard]] UINT_8 entityType() const { return m_entityType; }
    [[nodiscard]] UINT_8 type() const { return m_entityType; }
    void setEntityType(UINT_8 type) { m_entityType = type; }

    // --- Spatial Geometry ---

    [[nodiscard]] UINT_16 noOfLocationPoints() const { return static_cast<UINT_16>(m_locationPoints.size()); }

    [[nodiscard]] const QVector<STRUCT_LOCATION>& locationPoints() const { return m_locationPoints; }
    void setLocationPoints(const QVector<STRUCT_LOCATION> &points) { m_locationPoints = points; }
    void addLocationPoint(const STRUCT_LOCATION &point) { m_locationPoints.append(point); }

    /**
     * @brief Returns the primary coordinate (first point or 0,0 fallback).
     * @return STRUCT_LOCATION representing primary reference point.
     */
    [[nodiscard]] STRUCT_LOCATION primaryLocation() const;

    /**
     * @brief Calculates arithmetic centroid of all location points.
     * @param[out] outLat Calculated latitude in degrees.
     * @param[out] outLon Calculated longitude in degrees.
     * @return True if centroid successfully computed, false if points vector empty.
     */
    bool calculateCentroid(double &outLat, double &outLon) const;

    /**
     * @brief Calculates exact geodesic distance midpoint along the polyline path.
     *
     * Computes cumulative distance along all segments in m_locationPoints and finds
     * the exact point at 50% total length. Guaranteed to lie directly on the polyline path.
     *
     * @param[out] outLat Calculated latitude in degrees.
     * @param[out] outLon Calculated longitude in degrees.
     * @return True if midpoint successfully computed, false if points vector empty.
     * @note Ideal for placing boundary echelon symbols and midpoint labels.
     */
    bool calculateMidpoint(double &outLat, double &outLon) const;

    /**
     * @brief Calculates perimeter anchor point for tactical deployment boundaries.
     *
     * Finds the southernmost (base) coordinate among location points to anchor
     * the echelon symbol along the boundary line per tactical graphics doctrine.
     *
     * @param[out] outLat Calculated latitude in degrees.
     * @param[out] outLon Calculated longitude in degrees.
     * @return True if anchor successfully computed, false if points vector empty.
     */
    bool calculatePerimeterAnchor(double &outLat, double &outLon) const;

    // --- 4-Way Spatial Annotations ---

    [[nodiscard]] QString leftAnnotation() const { return m_leftAnnotation; }
    void setLeftAnnotation(const QString &val) { m_leftAnnotation = val; }

    [[nodiscard]] QString rightAnnotation() const { return m_rightAnnotation; }
    void setRightAnnotation(const QString &val) { m_rightAnnotation = val; }

    [[nodiscard]] QString topAnnotation() const { return m_topAnnotation; }
    void setTopAnnotation(const QString &val) { m_topAnnotation = val; }

    [[nodiscard]] QString bottomAnnotation() const { return m_bottomAnnotation; }
    void setBottomAnnotation(const QString &val) { m_bottomAnnotation = val; }

    // --- Special Operational Parameters ---

    [[nodiscard]] UINT_16 specialParam1() const { return m_specialParam1; }
    void setSpecialParam1(UINT_16 val) { m_specialParam1 = val; }

    [[nodiscard]] UINT_16 specialParam2() const { return m_specialParam2; }
    void setSpecialParam2(UINT_16 val) { m_specialParam2 = val; }

    [[nodiscard]] UINT_16 specialParam3() const { return m_specialParam3; }
    void setSpecialParam3(UINT_16 val) { m_specialParam3 = val; }

    [[nodiscard]] UINT_16 specialParam4() const { return m_specialParam4; }
    void setSpecialParam4(UINT_16 val) { m_specialParam4 = val; }

    // --- Dynamic Extensible Details ---

    [[nodiscard]] int noOfDetails() const { return m_entityDetails.size(); }

    [[nodiscard]] const QVector<STRUCT_DETAILS>& entityDetails() const { return m_entityDetails; }
    [[nodiscard]] const QVector<STRUCT_DETAILS>& details() const { return m_entityDetails; }
    void setEntityDetails(const QVector<STRUCT_DETAILS> &details) { m_entityDetails = details; }
    void addDetail(const STRUCT_DETAILS &detail) { m_entityDetails.append(detail); }

    // --- Timestamp & Operational Remarks ---

    [[nodiscard]] QDateTime reportTime() const { return m_reportTime; }
    [[nodiscard]] QDateTime lastUpdated() const { return m_reportTime; }
    void setReportTime(const QDateTime &dt) { m_reportTime = dt; }

    [[nodiscard]] QString remarks() const { return m_remarks; }
    void setRemarks(const QString &rem) { m_remarks = rem; }

    // --- JSON Serialization Helpers for SQLite Persistence ---

    [[nodiscard]] QString locationPointsToJson() const;
    void loadLocationPointsFromJson(const QString &jsonStr);

    [[nodiscard]] QString detailsToJson() const;
    void loadDetailsFromJson(const QString &jsonStr);

    /**
     * @brief Human-readable string representation of entity type enum code.
     * @param[in] type Integer type code [1..6].
     * @return Display name (e.g. "Point", "Line", "Polygon", etc.).
     */
    static QString entityTypeToString(UINT_8 type);

private:
    UINT_32                  m_entityId{0};
    QString                  m_entityName;
    UINT_8                   m_entityType{1};
    QVector<STRUCT_LOCATION> m_locationPoints;
    QString                  m_leftAnnotation;
    QString                  m_rightAnnotation;
    QString                  m_topAnnotation;
    QString                  m_bottomAnnotation;
    UINT_16                  m_specialParam1{0};
    UINT_16                  m_specialParam2{0};
    UINT_16                  m_specialParam3{0};
    UINT_16                  m_specialParam4{0};
    QVector<STRUCT_DETAILS>  m_entityDetails;
    QDateTime                m_reportTime;
    QString                  m_remarks;
};

} // namespace GISApp::Domain::ComplexEntities

#endif // COMPLEXENTITY_H

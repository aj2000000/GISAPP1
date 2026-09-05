/**
 * @file IMapFeature.h
 * @brief Pure virtual interface modeling any geographic entity renderable on the MapLibre canvas.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef IMAPFEATURE_H
#define IMAPFEATURE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

namespace GISApp::Core::Interfaces {

/**
 * @class IMapFeature
 * @brief Polymorphic interface representing an individual spatial entity renderable on the GPU map canvas.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the Core Interfaces layer (`src/core/interfaces/`).
 * - Adheres to the Interface Segregation Principle (ISP) and Dependency Inversion Principle (DIP).
 * - Decouples domain entities (such as TacticalTrack, RadarSensor, ThreatZone, Waypoint) from the
 *   underlying map rendering engine (MapLibre Native).
 * - Provides standardized access to spatial coordinates, kinematic telemetry, classification metadata,
 *   and property-driven symbology styling (e.g. colors, icons, labels, heading rotation).
 * - Delivers a default GeoJSON serialization method (`toGeoJsonFeature()`) ensuring consistent MapLibre
 *   GPU source integration without forcing domain entities to write ad-hoc JSON code.
 */
class IMapFeature
{
public:
    /**
     * @brief Virtual destructor ensuring clean polymorphic destruction.
     */
    virtual ~IMapFeature() = default;

    /**
     * @brief Unique identifier of the feature (e.g. "TRK-101", "RADAR-01").
     * @return Unique feature identifier string.
     */
    [[nodiscard]] virtual QString featureId() const = 0;

    /**
     * @brief Category classification of the feature (e.g. "track", "sensor", "zone").
     * @return Category name string.
     */
    [[nodiscard]] virtual QString featureCategory() const = 0;

    /**
     * @brief WGS-84 Latitude coordinate in decimal degrees [-90.0, 90.0].
     * @return Latitude in decimal degrees.
     */
    [[nodiscard]] virtual double latitude() const = 0;

    /**
     * @brief WGS-84 Longitude coordinate in decimal degrees [-180.0, 180.0].
     * @return Longitude in decimal degrees.
     */
    [[nodiscard]] virtual double longitude() const = 0;

    /**
     * @brief Altitude or elevation above mean sea level in meters.
     * @return Altitude in meters.
     */
    [[nodiscard]] virtual double altitude() const = 0;

    /**
     * @brief Heading / course over ground in decimal degrees [0.0, 360.0).
     * @return Heading in degrees clockwise from true north.
     */
    [[nodiscard]] virtual double heading() const = 0;

    /**
     * @brief Velocity or speed in meters per second (or knots).
     * @return Speed scalar value.
     */
    [[nodiscard]] virtual double speed() const = 0;

    /**
     * @brief Callsign or user-facing display label for the feature.
     * @return Display label string.
     */
    [[nodiscard]] virtual QString callsign() const = 0;

    /**
     * @brief Affiliation or tactical identity string (e.g. "Hostile", "Friendly", "Neutral", "Unknown").
     * @return Identity classification string.
     */
    [[nodiscard]] virtual QString identityString() const = 0;

    /**
     * @brief Operational domain classification string (e.g. "Air", "Surface", "Subsurface", "Land").
     * @return Domain classification string.
     */
    [[nodiscard]] virtual QString domainString() const = 0;

    /**
     * @brief Military/standard symbology identifier or icon resource name.
     * @return Symbology identifier string.
     */
    [[nodiscard]] virtual QString symbolCode() const = 0;

    /**
     * @brief Hex color string representing the entity's visual accent (e.g. "#ff3344" for Hostile).
     * @return Formatted hex color string.
     */
    [[nodiscard]] virtual QString colorHex() const = 0;

    /**
     * @brief Converts the feature to a standard GeoJSON Feature object with Point geometry and property dict.
     * @return Formatted QJsonObject representing the GeoJSON Feature.
     * @note Derived classes representing non-point geometries (e.g. Polygons for zones) may override this.
     */
    [[nodiscard]] virtual QJsonObject toGeoJsonFeature() const
    {
        QJsonObject feature;
        feature[QStringLiteral("type")] = QStringLiteral("Feature");
        feature[QStringLiteral("id")] = featureId();

        QJsonObject geometry;
        geometry[QStringLiteral("type")] = QStringLiteral("Point");
        QJsonArray coordinates;
        coordinates.append(longitude());
        coordinates.append(latitude());
        geometry[QStringLiteral("coordinates")] = coordinates;
        feature[QStringLiteral("geometry")] = geometry;

        QJsonObject properties;
        properties[QStringLiteral("id")] = featureId();
        properties[QStringLiteral("callsign")] = callsign();
        properties[QStringLiteral("identity")] = identityString();
        properties[QStringLiteral("domain")] = domainString();
        properties[QStringLiteral("color")] = colorHex();
        properties[QStringLiteral("heading")] = heading();
        properties[QStringLiteral("speed")] = speed();
        properties[QStringLiteral("altitude")] = altitude();
        properties[QStringLiteral("symbol")] = symbolCode();
        feature[QStringLiteral("properties")] = properties;

        return feature;
    }
};

} // namespace GISApp::Core::Interfaces

#endif // IMAPFEATURE_H

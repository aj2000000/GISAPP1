/**
 * @file TrackMapFeatureAdapter.h
 * @brief Adapter pattern implementation bridging pure domain TacticalTrack entities into IMapFeature for MapLibre GPU rendering.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TRACKMAPFEATUREADAPTER_H
#define TRACKMAPFEATUREADAPTER_H

#include "IMapFeature.h"
#include "TacticalTrack.h"

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

namespace GISApp::UI::Renderers {

/**
 * @class TrackMapFeatureAdapter
 * @brief Adapter that projects a pure domain TacticalTrack entity into the IMapFeature polymorphic interface.
 *
 * Architectural Role & Design Patterns:
 * - Implements the **Adapter Pattern** (GoF) within the Presentation / View layer (`ui/map/renderers/`).
 * - Decouples the domain model (`TacticalTrack`) completely from GIS presentation interfaces, GeoJSON formatting,
 *   and MapLibre GPU shader attributes.
 * - Encapsulates tactical symbology rules (MIL-STD-2525 style identity colors) and visual property mapping
 *   specifically for the MapLibre engine layers (glow, core circle, callsign label).
 */
class TrackMapFeatureAdapter : public GISApp::Core::Interfaces::IMapFeature
{
public:
    /**
     * @brief Default constructor.
     */
    TrackMapFeatureAdapter() = default;

    /**
     * @brief Constructs an adapter wrapping a pointer to a TacticalTrack domain entity.
     * @param[in] track Pointer to the domain entity being adapted for map rendering.
     */
    explicit TrackMapFeatureAdapter(const GISApp::Domain::Tracks::TacticalTrack *track)
        : m_track(track)
    {
    }

    /**
     * @brief Constructs an adapter wrapping a reference to a TacticalTrack domain entity.
     * @param[in] track Const reference to the domain entity being adapted for map rendering.
     */
    explicit TrackMapFeatureAdapter(const GISApp::Domain::Tracks::TacticalTrack &track)
        : m_track(&track)
    {
    }

    virtual ~TrackMapFeatureAdapter() override = default;

    // --- IMapFeature Implementation ---

    /**
     * @brief Returns unique feature identifier for MapLibre source matching.
     * @return Stringified track identifier.
     */
    [[nodiscard]] QString featureId() const override
    {
        return m_track ? QString::number(m_track->trackId()) : QStringLiteral("0");
    }

    /**
     * @brief Category classification for map feature filtering.
     * @return Category string literal "track".
     */
    [[nodiscard]] QString featureCategory() const override
    {
        return QStringLiteral("track");
    }

    /**
     * @brief WGS-84 Latitude in decimal degrees.
     * @return Latitude [-90.0, 90.0].
     */
    [[nodiscard]] double latitude() const override
    {
        return m_track ? m_track->latitude() : 0.0;
    }

    /**
     * @brief WGS-84 Longitude in decimal degrees.
     * @return Longitude [-180.0, 180.0].
     */
    [[nodiscard]] double longitude() const override
    {
        return m_track ? m_track->longitude() : 0.0;
    }

    /**
     * @brief Geometric altitude in meters above mean sea level.
     * @return Altitude value.
     */
    [[nodiscard]] double altitude() const override
    {
        return m_track ? m_track->altitude() : 0.0;
    }

    /**
     * @brief Heading / course over ground in decimal degrees.
     * @return Heading in [0.0, 360.0).
     */
    [[nodiscard]] double heading() const override
    {
        return m_track ? m_track->heading() : 0.0;
    }

    /**
     * @brief Speed or velocity scalar (not in wire structure, defaults to 0.0).
     * @return 0.0
     */
    [[nodiscard]] double speed() const override
    {
        return 0.0;
    }

    /**
     * @brief Display callsign or formatted track label.
     * @return Callsign string, defaulting to "TRK-<id>" if empty.
     */
    [[nodiscard]] QString callsign() const override
    {
        if (!m_track) return QStringLiteral("TRK-0");
        return m_track->trackName().isEmpty()
            ? QStringLiteral("TRK-%1").arg(m_track->trackId())
            : m_track->trackName();
    }

    /**
     * @brief Affiliation or tactical identity string.
     * @return Identification string ("HOSTILE", "FRIENDLY", "NEUTRAL", "UNKNOWN").
     */
    [[nodiscard]] QString identityString() const override
    {
        return m_track ? m_track->identityString() : QStringLiteral("UNKNOWN");
    }

    /**
     * @brief Operational domain classification string.
     * @return Domain string ("AIR", "SURFACE", "SUBSURFACE", "LAND", "UNKNOWN").
     */
    [[nodiscard]] QString domainString() const override
    {
        return m_track ? m_track->domainString() : QStringLiteral("UNKNOWN");
    }

    /**
     * @brief Military/standard symbology identifier or icon resource name.
     * @return Symbology identifier string.
     */
    [[nodiscard]] QString symbolCode() const override
    {
        return m_track ? m_track->symbolCode() : QString();
    }

    /**
     * @brief Resolves visual hex color based on tactical affiliation (MIL-STD presentation rule).
     * @return Formatted hex color string.
     */
    [[nodiscard]] QString colorHex() const override
    {
        if (!m_track) return QStringLiteral("#ffd600");
        switch (m_track->identity()) {
        case HOSTILE:
            return QStringLiteral("#ff3344"); // Tactical Hostile Red
        case FRIENDLY:
            return QStringLiteral("#00d2ff"); // Tactical Friendly Blue/Cyan
        case 3:
            return QStringLiteral("#00e676"); // Tactical Neutral Green
        default:
            return QStringLiteral("#ffd600"); // Tactical Unknown Amber/Yellow
        }
    }

    /**
     * @brief Serializes the adapted tactical track into a standard GeoJSON Point Feature
     *        tailored for MapLibre GPU shaders and data-driven filters.
     * @return QJsonObject representing the GeoJSON feature.
     */
    [[nodiscard]] QJsonObject toGeoJsonFeature() const override
    {
        QJsonObject feature;
        feature[QStringLiteral("type")] = QStringLiteral("Feature");
        feature[QStringLiteral("id")] = m_track ? m_track->trackId() : 0;

        // Geometry: Point [longitude, latitude]
        QJsonObject geometry;
        geometry[QStringLiteral("type")] = QStringLiteral("Point");
        QJsonArray coordinates;
        coordinates.append(longitude());
        coordinates.append(latitude());
        geometry[QStringLiteral("coordinates")] = coordinates;
        feature[QStringLiteral("geometry")] = geometry;

        // Properties: Telemetry, Kinematics, & Data-Driven Styling
        QJsonObject properties;
        properties[QStringLiteral("id")] = featureId();
        properties[QStringLiteral("track_id")] = m_track ? m_track->trackId() : 0;
        properties[QStringLiteral("callsign")] = callsign();
        properties[QStringLiteral("track_name")] = m_track ? m_track->trackName() : QString();
        properties[QStringLiteral("identity")] = identityString();
        properties[QStringLiteral("color")] = colorHex();
        properties[QStringLiteral("domain")] = domainString();
        properties[QStringLiteral("height")] = altitude();
        properties[QStringLiteral("dir")] = heading();
        properties[QStringLiteral("type")] = m_track ? static_cast<int>(m_track->type()) : 0;
        properties[QStringLiteral("sub_type")] = m_track ? static_cast<int>(m_track->subType()) : 0;
        properties[QStringLiteral("classification")] = m_track ? static_cast<int>(m_track->classification()) : 0;
        properties[QStringLiteral("strength")] = m_track ? static_cast<int>(m_track->strength()) : 1;
        properties[QStringLiteral("act_type")] = m_track ? static_cast<int>(m_track->actType()) : 0;
        properties[QStringLiteral("symbol")] = symbolCode();
        properties[QStringLiteral("remarks")] = m_track ? m_track->remarks() : QString();
        properties[QStringLiteral("time")] = m_track ? m_track->reportTime().toString(Qt::ISODate) : QString();

        feature[QStringLiteral("properties")] = properties;
        return feature;
    }

private:
    const GISApp::Domain::Tracks::TacticalTrack *m_track{nullptr}; ///< Pointer to wrapped domain entity.
};

} // namespace GISApp::UI::Renderers

#endif // TRACKMAPFEATUREADAPTER_H

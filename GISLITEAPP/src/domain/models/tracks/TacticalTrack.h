/**
 * @file TacticalTrack.h
 * @brief Header definition for TacticalTrack domain entity modeling dynamic military/civilian tracks.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef TACTICALTRACK_H
#define TACTICALTRACK_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

#include "IMapFeature.h"

namespace GISApp::Domain::Tracks {

/**
 * @enum TrackIdentity
 * @brief Standard tactical affiliation/identification enumeration.
 */
enum class TrackIdentity {
    Unknown = 0,  ///< Unidentified or pending classification
    Hostile = 1,  ///< Hostile target
    Friendly = 2, ///< Friendly force/asset
    Neutral = 3   ///< Neutral entity (civilian aircraft, commercial vessel)
};

/**
 * @enum TrackDomain
 * @brief Operational domain enumeration.
 */
enum class TrackDomain {
    Unknown = 0,
    Air = 1,
    Surface = 2,
    Subsurface = 3,
    Land = 4
};

/**
 * @class TacticalTrack
 * @brief Domain entity encapsulating an individual tactical track with telemetry, classification, and kinematics.
 *
 * TacticalTrack represents a tracked entity reported by radar, sonar, telemetry, or external tactical
 * data links. It implements GISApp::Core::Interfaces::IMapFeature to allow direct, zero-allocation
 * rendering on the MapLibre canvas with GPU data-driven styling (identity color, heading, domain symbol).
 */
class TacticalTrack : public GISApp::Core::Interfaces::IMapFeature
{
public:
    /**
     * @brief Default constructor.
     */
    TacticalTrack();

    /**
     * @brief Parameterized constructor for TacticalTrack.
     * @param[in] id Numerical unique track identifier.
     * @param[in] callsign Callsign or display title of track.
     * @param[in] lat WGS-84 Latitude in decimal degrees [-90.0, 90.0].
     * @param[in] lon WGS-84 Longitude in decimal degrees [-180.0, 180.0].
     * @param[in] alt Altitude above sea level in meters.
     * @param[in] heading Heading / course over ground in degrees [0.0, 360.0).
     */
    TacticalTrack(int id,
                  const QString &callsign,
                  double lat,
                  double lon,
                  double alt = 0.0,
                  double heading = 0.0);

    virtual ~TacticalTrack() override = default;

    // --- IMapFeature Implementation ---

    [[nodiscard]] QString featureId() const override { return QString::number(m_trackId); }
    [[nodiscard]] QString featureCategory() const override { return QStringLiteral("track"); }
    [[nodiscard]] double latitude() const override { return m_latitude; }
    [[nodiscard]] double longitude() const override { return m_longitude; }
    [[nodiscard]] double altitude() const override { return m_altitude; }
    [[nodiscard]] double heading() const override { return m_heading; }
    [[nodiscard]] double speed() const override { return m_speed; }
    [[nodiscard]] QString callsign() const override { return m_callsign.isEmpty() ? QStringLiteral("TRK-%1").arg(m_trackId) : m_callsign; }
    [[nodiscard]] QString identityString() const override;
    [[nodiscard]] QString domainString() const override;
    [[nodiscard]] QString symbolCode() const override { return m_symbolCode; }
    [[nodiscard]] QString colorHex() const override { return identityColorHex(); }
    [[nodiscard]] QJsonObject toGeoJsonFeature() const override;

    // --- Accessors & Mutators ---

    [[nodiscard]] int trackId() const { return m_trackId; }
    void setTrackId(int id) { m_trackId = id; }

    void setCallsign(const QString &callsign) { m_callsign = callsign; }
    void setLatitude(double lat) { m_latitude = lat; }
    void setLongitude(double lon) { m_longitude = lon; }
    void setAltitude(double alt) { m_altitude = alt; }
    void setHeading(double heading) { m_heading = heading; }
    void setSpeed(double speed) { m_speed = speed; }

    [[nodiscard]] TrackIdentity identity() const { return m_identity; }
    void setIdentity(TrackIdentity identity) { m_identity = identity; }

    [[nodiscard]] TrackDomain domain() const { return m_domain; }
    void setDomain(TrackDomain domain) { m_domain = domain; }

    void setSymbolCode(const QString &code) { m_symbolCode = code; }

    [[nodiscard]] QDateTime reportTime() const { return m_reportTime; }
    void setReportTime(const QDateTime &time) { m_reportTime = time; }

    [[nodiscard]] QString remarks() const { return m_remarks; }
    void setRemarks(const QString &remarks) { m_remarks = remarks; }

    /**
     * @brief Returns hex color code associated with tactical identity (MIL-STD-2525 styling).
     * @return Hex color string (e.g. "#ff3344" for Hostile, "#00d2ff" for Friendly).
     */
    [[nodiscard]] QString identityColorHex() const;

private:
    int m_trackId{0};
    QString m_callsign;
    double m_latitude{0.0};
    double m_longitude{0.0};
    double m_altitude{0.0};
    double m_heading{0.0};
    double m_speed{0.0};
    TrackIdentity m_identity{TrackIdentity::Unknown};
    TrackDomain m_domain{TrackDomain::Unknown};
    QString m_symbolCode;
    QDateTime m_reportTime;
    QString m_remarks;
};

} // namespace GISApp::Domain::Tracks

#endif // TACTICALTRACK_H

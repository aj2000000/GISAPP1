/**
 * @file TacticalTrack.h
 * @brief Header definition for TacticalTrack domain entity modeled directly from canonical STRUCT_TRACK.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TACTICALTRACK_H
#define TACTICALTRACK_H

#include <QString>
#include <QDateTime>
#include <QVector>

#include "IrsTypes.h"
#include "WireStructures.h"

namespace GISApp::Domain::Tracks {

/**
 * @class TacticalTrack
 * @brief Pure domain entity directly encapsulating the canonical STRUCT_TRACK wire specification.
 *
 * Architectural Role & Design Patterns:
 * - Resides strictly in the Domain Model layer (`src/domain/models/tracks/`).
 * - Directly mirrors the true source of truth: `STRUCT_TRACK`, `STRUCT_LOCATION`, and `STRUCT_TRACK_ATTRIBUTES`.
 * - Pure POCO (Plain Old C++ Object) with value semantics; free of any map presentation dependencies
 *   or serialization formats (GeoJSON, MapLibre GPU shaders).
 */
class TacticalTrack
{
public:
    /**
     * @brief Default constructor initializing default coordinates and current UTC timestamp.
     */
    TacticalTrack();

    /**
     * @brief Parameterized constructor for TacticalTrack.
     * @param[in] id Numerical unique track identifier (track_id).
     * @param[in] name Designated track callsign/name (track_name).
     * @param[in] lat WGS-84 Latitude in decimal degrees [-90.0, 90.0].
     * @param[in] lon WGS-84 Longitude in decimal degrees [-180.0, 180.0].
     * @param[in] height Altitude / elevation above MSL in meters.
     * @param[in] dir Heading / bearing direction in degrees [0.0, 360.0).
     */
    TacticalTrack(int id,
                  const QString &name,
                  double lat,
                  double lon,
                  double height = 0.0,
                  double dir = 0.0);

    /**
     * @brief Non-virtual destructor for efficient value-type lifecycle.
     */
    ~TacticalTrack() = default;

    // --- Identification ---

    /**
     * @brief Retrieves numerical track identifier (track_id).
     * @return Integer track ID.
     */
    [[nodiscard]] int trackId() const { return static_cast<int>(m_trackId); }

    /**
     * @brief Sets numerical track identifier.
     * @param[in] id Unique track ID.
     */
    void setTrackId(int id) { m_trackId = static_cast<UINT_32>(id); }

    /**
     * @brief Retrieves designated track callsign/name (track_name).
     * @return Track name string, defaulting to "TRK-<id>" if empty.
     */
    [[nodiscard]] QString trackName() const { return m_trackName.isEmpty() ? QStringLiteral("TRK-%1").arg(m_trackId) : m_trackName; }

    /**
     * @brief Sets track callsign/name.
     * @param[in] name Display callsign/name.
     */
    void setTrackName(const QString &name) { m_trackName = name; }

    /**
     * @brief Convenience alias for trackName().
     * @return Callsign string.
     */
    [[nodiscard]] QString callsign() const { return trackName(); }

    /**
     * @brief Convenience alias for setTrackName().
     * @param[in] callsign Callsign string.
     */
    void setCallsign(const QString &callsign) { setTrackName(callsign); }

    // --- Tactical Affiliation / Identity ---

    /**
     * @brief Retrieves tactical identity (track_identity: HOSTILE, FRIENDLY, NEUTRAL, UNKNOWN).
     * @return IDENTITY code.
     */
    [[nodiscard]] IDENTITY identity() const { return m_identity; }

    /**
     * @brief Sets tactical identity.
     * @param[in] identity IDENTITY code.
     */
    void setIdentity(IDENTITY identity) { m_identity = identity; }

    /**
     * @brief Returns human-readable representation of identity code.
     * @return String description ("HOSTILE", "FRIENDLY", "NEUTRAL", "UNKNOWN").
     */
    [[nodiscard]] QString identityString() const;

    // --- Kinematics & Spatial Telemetry (STRUCT_LOCATION) ---

    /**
     * @brief Retrieves WGS-84 Latitude coordinate.
     * @return Latitude in decimal degrees [-90.0, 90.0].
     */
    [[nodiscard]] double latatitude() const { return m_location.latatitude; }
    [[nodiscard]] double latitude() const { return m_location.latatitude; }

    /**
     * @brief Sets WGS-84 Latitude coordinate.
     * @param[in] lat Latitude in decimal degrees.
     */
    void setLatatitude(double lat) { m_location.latatitude = lat; }
    void setLatitude(double lat) { m_location.latatitude = lat; }

    /**
     * @brief Retrieves WGS-84 Longitude coordinate.
     * @return Longitude in decimal degrees [-180.0, 180.0].
     */
    [[nodiscard]] double longitude() const { return m_location.longitude; }

    /**
     * @brief Sets WGS-84 Longitude coordinate.
     * @param[in] lon Longitude in decimal degrees.
     */
    void setLongitude(double lon) { m_location.longitude = lon; }

    /**
     * @brief Retrieves elevation / height above MSL in meters.
     * @return Height in meters.
     */
    [[nodiscard]] double height() const { return m_location.height; }
    [[nodiscard]] double altitude() const { return m_location.height; }

    /**
     * @brief Sets elevation / height above MSL.
     * @param[in] h Height in meters.
     */
    void setHeight(double h) { m_location.height = h; }
    void setAltitude(double h) { m_location.height = h; }

    /**
     * @brief Retrieves heading / bearing direction angle in degrees.
     * @return Direction in degrees [0.0, 360.0).
     */
    [[nodiscard]] double dir() const { return m_location.dir; }
    [[nodiscard]] double heading() const { return m_location.dir; }

    /**
     * @brief Sets heading / bearing direction angle in degrees.
     * @param[in] d Direction in degrees.
     */
    void setDir(double d) { m_location.dir = d; }
    void setHeading(double d) { m_location.dir = d; }

    /**
     * @brief Returns entire packed STRUCT_LOCATION structure.
     * @return STRUCT_LOCATION.
     */
    [[nodiscard]] STRUCT_LOCATION location() const { return m_location; }

    /**
     * @brief Sets entire packed STRUCT_LOCATION structure.
     * @param[in] loc STRUCT_LOCATION.
     */
    void setLocation(const STRUCT_LOCATION &loc) { m_location = loc; }

    // --- Track Attributes (STRUCT_TRACK_ATTRIBUTES) ---

    /**
     * @brief Retrieves the packed STRUCT_TRACK_ATTRIBUTES structure.
     * @return STRUCT_TRACK_ATTRIBUTES.
     */
    [[nodiscard]] STRUCT_TRACK_ATTRIBUTES attributes() const { return m_attributes; }

    /**
     * @brief Sets the packed STRUCT_TRACK_ATTRIBUTES structure.
     * @param[in] attr STRUCT_TRACK_ATTRIBUTES.
     */
    void setAttributes(const STRUCT_TRACK_ATTRIBUTES &attr) { m_attributes = attr; }

    /**
     * @brief Domain type (Air=1, Surface=2, Subsurface=3, Land=4).
     */
    [[nodiscard]] UINT_8 type() const { return m_attributes.type; }
    void setType(UINT_8 type) { m_attributes.type = type; }

    /**
     * @brief Human-readable domain string.
     */
    [[nodiscard]] QString domainString() const;

    /**
     * @brief Specific subtype.
     */
    [[nodiscard]] UINT_8 subType() const { return m_attributes.sub_type; }
    void setSubType(UINT_8 subType) { m_attributes.sub_type = subType; }

    /**
     * @brief IFF / security classification.
     */
    [[nodiscard]] UINT_8 classification() const { return m_attributes.classification; }
    void setClassification(UINT_8 c) { m_attributes.classification = c; }

    /**
     * @brief Formation strength.
     */
    [[nodiscard]] UINT_8 strength() const { return m_attributes.strength; }
    void setStrength(UINT_8 s) { m_attributes.strength = s; }

    /**
     * @brief Operational activity type.
     */
    [[nodiscard]] UINT_8 actType() const { return m_attributes.act_type; }
    void setActType(UINT_8 act) { m_attributes.act_type = act; }

    /**
     * @brief Operational activity subtype.
     */
    [[nodiscard]] UINT_8 actSubType() const { return m_attributes.act_sub_type; }
    void setActSubType(UINT_8 actSub) { m_attributes.act_sub_type = actSub; }

    /**
     * @brief Operational activity classification.
     */
    [[nodiscard]] UINT_8 actClassification() const { return m_attributes.act_classification; }
    void setActClassification(UINT_8 actClass) { m_attributes.act_classification = actClass; }

    // --- System Track Designation & Sensor Sources ---

    /**
     * @brief System track designation (SYSTEM1=1, SYSTEM2/FUSED=2).
     */
    [[nodiscard]] SYSTEM_TRACK_TYPE systemTrackType() const { return m_sysTrackType; }
    void setSystemTrackType(SYSTEM_TRACK_TYPE type) { m_sysTrackType = type; }

    /**
     * @brief Number of contributing sensor sources.
     */
    [[nodiscard]] UINT_8 noOfSources() const { return m_noOfSources; }
    void setNoOfSources(UINT_8 count) { m_noOfSources = count; }

    /**
     * @brief Contributing sensor source records.
     */
    [[nodiscard]] const QVector<STRUCT_TRACK_SOURCE>& trackSources() const { return m_trackSources; }
    void setTrackSources(const QVector<STRUCT_TRACK_SOURCE> &sources) {
        m_trackSources = sources;
        m_noOfSources = static_cast<UINT_8>(sources.size());
    }

    // --- Symbology, Timestamp & Remarks ---

    /**
     * @brief Retrieves military/tactical symbol code string.
     * @return Symbol code string.
     */
    [[nodiscard]] QString symbolCode() const { return m_symbolCode; }
    void setSymbolCode(const QString &code) { m_symbolCode = code; }

    /**
     * @brief Retrieves UTC timestamp of the track report.
     * @return QDateTime in UTC.
     */
    [[nodiscard]] QDateTime reportTime() const { return m_reportTime; }
    void setReportTime(const QDateTime &time) { m_reportTime = time; }

    /**
     * @brief Retrieves freeform operational remarks.
     * @return Remarks string.
     */
    [[nodiscard]] QString remarks() const { return m_remarks; }
    void setRemarks(const QString &remarks) { m_remarks = remarks; }

private:
    UINT_32                 m_trackId{0};
    QString                 m_trackName;
    STRUCT_LOCATION         m_location{0.0, 0.0, 0.0, 0.0};
    IDENTITY                m_identity{0}; // Unknown
    STRUCT_TRACK_ATTRIBUTES m_attributes{0, 0, 0, 1, 0, 0, 0};
    SYSTEM_TRACK_TYPE       m_sysTrackType{1};
    UINT_8                  m_noOfSources{0};
    QVector<STRUCT_TRACK_SOURCE> m_trackSources;
    QString                 m_symbolCode;
    QDateTime               m_reportTime;
    QString                 m_remarks;
};

} // namespace GISApp::Domain::Tracks

#endif // TACTICALTRACK_H

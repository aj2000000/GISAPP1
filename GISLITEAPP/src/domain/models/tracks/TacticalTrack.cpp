/**
 * @file TacticalTrack.cpp
 * @brief Implementation of TacticalTrack domain entity and GeoJSON feature generation.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "TacticalTrack.h"

namespace GISApp::Domain::Tracks {

TacticalTrack::TacticalTrack()
    : m_trackId(0)
    , m_latitude(0.0)
    , m_longitude(0.0)
    , m_altitude(0.0)
    , m_heading(0.0)
    , m_speed(0.0)
    , m_identity(TrackIdentity::Unknown)
    , m_domain(TrackDomain::Unknown)
    , m_reportTime(QDateTime::currentDateTimeUtc())
{
}

TacticalTrack::TacticalTrack(int id,
                             const QString &callsign,
                             double lat,
                             double lon,
                             double alt,
                             double heading)
    : m_trackId(id)
    , m_callsign(callsign)
    , m_latitude(lat)
    , m_longitude(lon)
    , m_altitude(alt)
    , m_heading(heading)
    , m_speed(0.0)
    , m_identity(TrackIdentity::Unknown)
    , m_domain(TrackDomain::Unknown)
    , m_reportTime(QDateTime::currentDateTimeUtc())
{
}

QString TacticalTrack::identityString() const
{
    switch (m_identity) {
    case TrackIdentity::Hostile:  return QStringLiteral("Hostile");
    case TrackIdentity::Friendly: return QStringLiteral("Friendly");
    case TrackIdentity::Neutral:  return QStringLiteral("Neutral");
    case TrackIdentity::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

QString TacticalTrack::identityColorHex() const
{
    switch (m_identity) {
    case TrackIdentity::Hostile:  return QStringLiteral("#ff3344"); // Tactical Hostile Red
    case TrackIdentity::Friendly: return QStringLiteral("#00d2ff"); // Tactical Friendly Blue/Cyan
    case TrackIdentity::Neutral:  return QStringLiteral("#00e676"); // Tactical Neutral Green
    case TrackIdentity::Unknown:
    default:
        return QStringLiteral("#ffd600"); // Tactical Unknown Amber/Yellow
    }
}

QString TacticalTrack::domainString() const
{
    switch (m_domain) {
    case TrackDomain::Air:        return QStringLiteral("Air");
    case TrackDomain::Surface:    return QStringLiteral("Surface");
    case TrackDomain::Subsurface: return QStringLiteral("Subsurface");
    case TrackDomain::Land:       return QStringLiteral("Land");
    case TrackDomain::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

QJsonObject TacticalTrack::toGeoJsonFeature() const
{
    QJsonObject feature;
    feature[QStringLiteral("type")] = QStringLiteral("Feature");
    feature[QStringLiteral("id")] = m_trackId;

    // Geometry: Point [longitude, latitude]
    QJsonObject geometry;
    geometry[QStringLiteral("type")] = QStringLiteral("Point");
    QJsonArray coordinates;
    coordinates.append(m_longitude);
    coordinates.append(m_latitude);
    geometry[QStringLiteral("coordinates")] = coordinates;
    feature[QStringLiteral("geometry")] = geometry;

    // Properties: Telemetry & Styling
    QJsonObject properties;
    properties[QStringLiteral("track_id")] = m_trackId;
    properties[QStringLiteral("callsign")] = m_callsign.isEmpty() ? QStringLiteral("TRK-%1").arg(m_trackId) : m_callsign;
    properties[QStringLiteral("identity")] = identityString();
    properties[QStringLiteral("color")] = identityColorHex();
    properties[QStringLiteral("domain")] = domainString();
    properties[QStringLiteral("altitude")] = m_altitude;
    properties[QStringLiteral("heading")] = m_heading;
    properties[QStringLiteral("speed")] = m_speed;
    properties[QStringLiteral("symbol")] = m_symbolCode;
    properties[QStringLiteral("remarks")] = m_remarks;
    properties[QStringLiteral("time")] = m_reportTime.toString(Qt::ISODate);

    feature[QStringLiteral("properties")] = properties;

    return feature;
}

} // namespace GISApp::Domain::Tracks

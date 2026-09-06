/**
 * @file TacticalTrack.cpp
 * @brief Implementation of TacticalTrack domain entity.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "TacticalTrack.h"
#include "fieldkeyvaluemapper.h"

namespace GISApp::Domain::Tracks {

TacticalTrack::TacticalTrack()
    : m_trackId(0)
    , m_identity(0)
    , m_sysTrackType(1)
    , m_noOfSources(0)
    , m_reportTime(QDateTime::currentDateTimeUtc())
{
    m_location.latatitude = 0.0;
    m_location.longitude = 0.0;
    m_location.height = 0.0;
    m_location.dir = 0.0;

    m_attributes.type = 0;
    m_attributes.sub_type = 0;
    m_attributes.classification = 0;
    m_attributes.strength = 1;
    m_attributes.act_type = 0;
    m_attributes.act_sub_type = 0;
    m_attributes.act_classification = 0;
}

TacticalTrack::TacticalTrack(int id,
                             const QString &name,
                             double lat,
                             double lon,
                             double height,
                             double dir)
    : m_trackId(static_cast<UINT_32>(id))
    , m_trackName(name)
    , m_identity(0)
    , m_sysTrackType(1)
    , m_noOfSources(0)
    , m_reportTime(QDateTime::currentDateTimeUtc())
{
    m_location.latatitude = lat;
    m_location.longitude = lon;
    m_location.height = height;
    m_location.dir = dir;

    m_attributes.type = 0;
    m_attributes.sub_type = 0;
    m_attributes.classification = 0;
    m_attributes.strength = 1;
    m_attributes.act_type = 0;
    m_attributes.act_sub_type = 0;
    m_attributes.act_classification = 0;
}

QString TacticalTrack::identityString() const
{
    return FieldKeyValueMapper::instance().trackIdentityMapping(m_identity);
}

QString TacticalTrack::domainString() const
{
    return FieldKeyValueMapper::instance().trackTypeMapping(m_attributes.type);
}

} // namespace GISApp::Domain::Tracks

#include "sampleentity.h"

namespace GISApp {
namespace Domain {
namespace SampleEntities {



SampleEntity::SampleEntity(int id,int etype, double lat, double lon, double height, double dir)
{
    m_Id=id;
    m_type=etype;
    m_location.latatitude = lat;
    m_location.longitude = lon;
    m_location.height = height;
    m_location.dir = dir;
}

UINT_32 SampleEntity::Id() const
{
    return m_Id;
}

void SampleEntity::setId(UINT_32 newId)
{
    m_Id = newId;
}

QString SampleEntity::Name() const
{
    return m_Name;
}

void SampleEntity::setName(const QString &newName)
{
    m_Name = newName;
}

UINT_8 SampleEntity::type() const
{
    return m_type;
}

void SampleEntity::setType(UINT_8 newType)
{
    m_type = newType;
}

STRUCT_LOCATION SampleEntity::location() const
{
    return m_location;
}

void SampleEntity::setLocation(const STRUCT_LOCATION &newLocation)
{
    m_location = newLocation;
}

QDateTime SampleEntity::reportTime() const
{
    return m_reportTime;
}

void SampleEntity::setReportTime(const QDateTime &newReportTime)
{
    m_reportTime = newReportTime;
}

QString SampleEntity::remarks() const
{
    return m_remarks;
}

void SampleEntity::setRemarks(const QString &newRemarks)
{
    m_remarks = newRemarks;
}

}
}
}

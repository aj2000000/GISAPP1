#ifndef SAMPLEENTITY_H
#define SAMPLEENTITY_H




#include <QString>
#include <QDateTime>
#include <QVector>

#include "IrsTypes.h"
#include "WireStructures.h"

namespace GISApp::Domain::SampleEntities {

class SampleEntity
{
public:

    SampleEntity(int id,
                 int etype,
                 double lat,
                 double lon,
                 double height = 0.0,
                 double dir = 0.0);



    /**
     * @brief Non-virtual destructor for efficient value-type lifecycle.
     */
    ~SampleEntity() = default;


    UINT_32 Id() const;
    void setId(UINT_32 newId);

    QString Name() const;
    void setName(const QString &newName);

    UINT_8 type() const;
    void setType(UINT_8 newType);

    STRUCT_LOCATION location() const;
    void setLocation(const STRUCT_LOCATION &newLocation);

    QDateTime reportTime() const;
    void setReportTime(const QDateTime &newReportTime);

    QString remarks() const;
    void setRemarks(const QString &newRemarks);

private:
    UINT_32                  m_Id{0};
    QString                 m_Name;
    UINT_8                  m_type{0};
    STRUCT_LOCATION         m_location{0.0, 0.0, 0.0, 0.0};
    QDateTime               m_reportTime;
    QString                 m_remarks;
};

} // namespace


#endif // SAMPLEENTITY_H

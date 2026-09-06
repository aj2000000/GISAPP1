#include "udpsampleentitymessagehandler.h"
#include "protocol/MessageIds.h"
#include "protocol/WireStructures.h"
#include "sampleentity.h"
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QTimeZone>



/**
 * @file UdpSampleEntityMessageHandler.cpp
 * @brief Strategy handler for deserializing MAIN_LITE_TRACK_MSG (Message ID: 613) UDP packets.
 * @author GISLITE Development Team
 * @date 2026
 */



namespace GISApp::Core::Udp::Handlers {

UdpSampleEntityMessageHandler::UdpSampleEntityMessageHandler(GISApp::Repositories::ISampleEntityRepository *sampleEntityRepo)
    : m_sampleEntityRepo(sampleEntityRepo)
{
}

MESSAGE_ID UdpSampleEntityMessageHandler::messageId() const
{
    return MAIN_LITE_SAMPLE_ENTITY_MSG_ID; // 613
}

bool UdpSampleEntityMessageHandler::processPayload(const QByteArray &payload)
{
    const char *dataPtr = payload.constData();
    qsizetype totalBytes = payload.size();
    qsizetype offset = 0;

    if (totalBytes < static_cast<qsizetype>(sizeof(STRUCT_MESSAGE_HEADER) + sizeof(UINT_16))) {
        qWarning() << "[UdpSampleEntityMessageHandler] Payload too short for track header:" << totalBytes;
        return false;
    }

    STRUCT_MESSAGE_HEADER header;
    SMEMCPY(&header, dataPtr + offset, sizeof(STRUCT_MESSAGE_HEADER));
    offset += sizeof(STRUCT_MESSAGE_HEADER);


    UINT_16 noOfEntity = 0;
    SMEMCPY(&noOfEntity, dataPtr + offset, sizeof(UINT_16));
    offset += sizeof(UINT_16);

    qDebug() << "[UdpSampleEntityMessageHandler] 🎯 Ingesting batch of" << noOfEntity << "Entity from UDP payload.";

    if (!m_sampleEntityRepo) {
        qWarning() << "[UdpSampleEntityMessageHandler] No EntityRepo attached! Dropping entity.";
        return false;
    }

    QVector<GISApp::Domain::SampleEntities::SampleEntity> entitiesList;
    entitiesList.reserve(noOfEntity);

    constexpr qsizetype minEntitiesBytes = sizeof(UINT_32)
                                      + sizeof(STRING_100)
                                      + sizeof(UINT_8)
                                      + sizeof(STRUCT_LOCATION)
                                      + sizeof(STRUCT_DATE_TIME)
                                      + sizeof(STRING_100);


    for (UINT_16 i = 0; i < noOfEntity; ++i) {
        if (offset + minEntitiesBytes > totalBytes) {
            qWarning() << "[UdpSampleEntityMessageHandler] Payload truncated at entity index" << i << "of" << noOfEntity;
            break;
        }

        STRUCT_SAMPLE_ENTITY rawEntity;
        SMEMCPY(&rawEntity, dataPtr + offset, minEntitiesBytes);
        offset += minEntitiesBytes;

        // Safely extract fixed-width C-strings
        QByteArray nameBytes(rawEntity.entity_name, sizeof(rawEntity.entity_name));
        QString callsign = QString::fromUtf8(nameBytes.constData()).trimmed();



        QByteArray remBytes(rawEntity.entity_remark, sizeof(rawEntity.entity_remark));
        QString remarks = QString::fromUtf8(remBytes.constData()).trimmed();

        // Construct Domain TacticalTrack directly from STRUCT_TRACK
        GISApp::Domain::SampleEntities::SampleEntity sampleEntity(
            static_cast<int>(rawEntity.entity_id),
            static_cast<int>(rawEntity.entity_type),
            rawEntity.entity_loc.latatitude,
            rawEntity.entity_loc.longitude,
            rawEntity.entity_loc.height,
            rawEntity.entity_loc.dir
        );

        sampleEntity.setName(callsign);
        sampleEntity.setRemarks(remarks);

        // Reconstruct timestamp
        QDate date(rawEntity.entity_report_time.date.year,
                   rawEntity.entity_report_time.date.month,
                   rawEntity.entity_report_time.date.day);
        QTime time(rawEntity.entity_report_time.time.hour,
                   rawEntity.entity_report_time.time.minute,
                   rawEntity.entity_report_time.time.second);
        if (date.isValid() && time.isValid()) {
            sampleEntity.setReportTime(QDateTime(date, time, QTimeZone::UTC));
        } else {
            sampleEntity.setReportTime(QDateTime::currentDateTimeUtc());
        }

        entitiesList.append(sampleEntity);
    }

    if (!entitiesList.isEmpty()) {
        m_sampleEntityRepo->upsertSampleEntities(entitiesList);
        qInfo() << "[UdpSampleEntityMessageHandler] Successfully upserted" << entitiesList.size()
                << "Sample Entity into repository.";
    }

    return true;
}

} // namespace GISApp::Core::Udp::Handlers

/**
 * @file UdpComplexEntityMessageHandler.cpp
 * @brief Strategy handler for deserializing MAIN_LITE_COMPLEX_ENTITY_MSG (Message ID: 905) UDP packets.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "UdpComplexEntityMessageHandler.h"
#include "protocol/MessageIds.h"
#include "protocol/WireStructures.h"
#include "ComplexEntity.h"

#include <QDebug>
#include <QDate>
#include <QTime>
#include <QTimeZone>

namespace GISApp::Core::Udp::Handlers {

UdpComplexEntityMessageHandler::UdpComplexEntityMessageHandler(GISApp::Repositories::IComplexEntityRepository *complexEntityRepo)
    : m_complexEntityRepo(complexEntityRepo)
{
}

MESSAGE_ID UdpComplexEntityMessageHandler::messageId() const
{
    return MAIN_LITE_COMPLEX_ENTITY_MSG_ID; // 905
}

bool UdpComplexEntityMessageHandler::processPayload(const QByteArray &payload)
{
    const char *dataPtr = payload.constData();
    const qsizetype totalBytes = payload.size();
    qsizetype offset = 0;

    if (totalBytes < static_cast<qsizetype>(sizeof(STRUCT_MESSAGE_HEADER) + sizeof(UINT_16))) {
        qWarning() << "[UdpComplexEntityMessageHandler] Payload too short for complex entity header:" << totalBytes;
        return false;
    }

    STRUCT_MESSAGE_HEADER header;
    SMEMCPY(&header, dataPtr + offset, sizeof(STRUCT_MESSAGE_HEADER));
    offset += sizeof(STRUCT_MESSAGE_HEADER);

    UINT_16 noOfEntity = 0;
    SMEMCPY(&noOfEntity, dataPtr + offset, sizeof(UINT_16));
    offset += sizeof(UINT_16);

    qDebug() << "[UdpComplexEntityMessageHandler] 🎯 Ingesting batch of" << noOfEntity << "Complex Entities from UDP payload.";

    if (!m_complexEntityRepo) {
        qWarning() << "[UdpComplexEntityMessageHandler] No ComplexEntityRepo attached! Dropping entities.";
        return false;
    }

    QVector<GISApp::Domain::ComplexEntities::ComplexEntity> entitiesList;
    entitiesList.reserve(noOfEntity);

    // Minimum fixed bytes per entity without dynamic points and details:
    // Wire format adhering strictly to STRUCT_COMPLEX_ENTITY:
    // entity_id (4) + entity_name (100) + entity_type (1) + no_of_location_points (2)
    // + 4 annotations (4 * 100) + 4 special params (4 * 2) + no_of_details (4)
    // Total min bytes = 4 + 100 + 1 + 2 + 400 + 8 + 4 = 519 bytes
    constexpr qsizetype minEntityFixedBytes = sizeof(UINT_32)
                                           + sizeof(STRING_100)
                                           + sizeof(UINT_8)
                                           + sizeof(UINT_16)
                                           + (4 * sizeof(STRING_100))
                                           + (4 * sizeof(UINT_16))
                                           + sizeof(int);

    for (UINT_16 i = 0; i < noOfEntity; ++i) {
        if (offset + minEntityFixedBytes > totalBytes) {
            qWarning() << "[UdpComplexEntityMessageHandler] Payload truncated at entity index" << i << "of" << noOfEntity;
            break;
        }

        // 1. entity_id (UINT_32)
        UINT_32 entityId = 0;
        SMEMCPY(&entityId, dataPtr + offset, sizeof(UINT_32));
        offset += sizeof(UINT_32);

        // 2. entity_name (STRING_100)
        STRING_100 rawName;
        SMEMCPY(rawName, dataPtr + offset, sizeof(STRING_100));
        offset += sizeof(STRING_100);
        QByteArray nameBytes(rawName, sizeof(rawName));
        QString entityName = QString::fromUtf8(nameBytes.constData()).trimmed();

        // 3. entity_type (UINT_8)
        UINT_8 entityType = 1;
        SMEMCPY(&entityType, dataPtr + offset, sizeof(UINT_8));
        offset += sizeof(UINT_8);

        // 4. no_of_location_points (UINT_16)
        UINT_16 noOfPoints = 0;
        SMEMCPY(&noOfPoints, dataPtr + offset, sizeof(UINT_16));
        offset += sizeof(UINT_16);

        // 5. entity_location_points (vector of STRUCT_LOCATION)
        QVector<STRUCT_LOCATION> locationPoints;
        locationPoints.reserve(noOfPoints);
        for (UINT_16 p = 0; p < noOfPoints; ++p) {
            if (offset + static_cast<qsizetype>(sizeof(STRUCT_LOCATION)) > totalBytes) {
                qWarning() << "[UdpComplexEntityMessageHandler] Truncated inside location points for entity" << entityId;
                break;
            }
            STRUCT_LOCATION loc{};
            SMEMCPY(&loc, dataPtr + offset, sizeof(STRUCT_LOCATION));
            offset += sizeof(STRUCT_LOCATION);
            locationPoints.append(loc);
        }

        // 6. 4-way Annotations: left, right, top, bottom
        STRING_100 rawLeftAnn, rawRightAnn, rawTopAnn, rawBottomAnn;
        SMEMCPY(rawLeftAnn, dataPtr + offset, sizeof(STRING_100));
        offset += sizeof(STRING_100);
        SMEMCPY(rawRightAnn, dataPtr + offset, sizeof(STRING_100));
        offset += sizeof(STRING_100);
        SMEMCPY(rawTopAnn, dataPtr + offset, sizeof(STRING_100));
        offset += sizeof(STRING_100);
        SMEMCPY(rawBottomAnn, dataPtr + offset, sizeof(STRING_100));
        offset += sizeof(STRING_100);

        QByteArray leftBytes(rawLeftAnn, sizeof(rawLeftAnn));
        QString leftAnn = QString::fromUtf8(leftBytes.constData()).trimmed();

        QByteArray rightBytes(rawRightAnn, sizeof(rawRightAnn));
        QString rightAnn = QString::fromUtf8(rightBytes.constData()).trimmed();

        QByteArray topBytes(rawTopAnn, sizeof(rawTopAnn));
        QString topAnn = QString::fromUtf8(topBytes.constData()).trimmed();

        QByteArray bottomBytes(rawBottomAnn, sizeof(rawBottomAnn));
        QString bottomAnn = QString::fromUtf8(bottomBytes.constData()).trimmed();

        // For formation boundary (Type 7), only left and right annotations exist; top and bottom must remain blank
        if (entityType == 7) {
            topAnn.clear();
            bottomAnn.clear();
        } else if (entityType == 8) {
            // For tactical deployment area (Type 8), all 4 annotations must remain blank per doctrine
            leftAnn.clear();
            rightAnn.clear();
            topAnn.clear();
            bottomAnn.clear();
        }

        // 7. Special Parameters 1..4
        UINT_16 sp1 = 0, sp2 = 0, sp3 = 0, sp4 = 0;
        SMEMCPY(&sp1, dataPtr + offset, sizeof(UINT_16));
        offset += sizeof(UINT_16);
        SMEMCPY(&sp2, dataPtr + offset, sizeof(UINT_16));
        offset += sizeof(UINT_16);
        SMEMCPY(&sp3, dataPtr + offset, sizeof(UINT_16));
        offset += sizeof(UINT_16);
        SMEMCPY(&sp4, dataPtr + offset, sizeof(UINT_16));
        offset += sizeof(UINT_16);

        // 8. no_of_details (int)
        int noOfDetails = 0;
        SMEMCPY(&noOfDetails, dataPtr + offset, sizeof(int));
        offset += sizeof(int);

        // 9. entity_details (vector of STRUCT_DETAILS)
        QVector<STRUCT_DETAILS> entityDetails;
        if (noOfDetails > 0) {
            entityDetails.reserve(noOfDetails);
            for (int d = 0; d < noOfDetails; ++d) {
                if (offset + static_cast<qsizetype>(sizeof(STRUCT_DETAILS)) > totalBytes) {
                    qWarning() << "[UdpComplexEntityMessageHandler] Truncated inside details for entity" << entityId;
                    break;
                }
                STRUCT_DETAILS det{};
                SMEMCPY(&det, dataPtr + offset, sizeof(STRUCT_DETAILS));
                offset += sizeof(STRUCT_DETAILS);
                entityDetails.append(det);
            }
        }

        // Construct Domain ComplexEntity
        GISApp::Domain::ComplexEntities::ComplexEntity complexEntity(entityId, entityName, entityType);
        complexEntity.setLocationPoints(locationPoints);
        complexEntity.setLeftAnnotation(leftAnn);
        complexEntity.setRightAnnotation(rightAnn);
        complexEntity.setTopAnnotation(topAnn);
        complexEntity.setBottomAnnotation(bottomAnn);
        complexEntity.setSpecialParam1(sp1);
        complexEntity.setSpecialParam2(sp2);
        complexEntity.setSpecialParam3((entityType == 8) ? 0 : sp3);
        complexEntity.setSpecialParam4((entityType == 7 || entityType == 8) ? 0 : sp4);
        complexEntity.setEntityDetails((entityType == 7 || entityType == 8) ? QVector<STRUCT_DETAILS>() : entityDetails);
        complexEntity.setReportTime(QDateTime::currentDateTimeUtc());
        complexEntity.setRemarks(QString());

        entitiesList.append(complexEntity);
    }

    if (!entitiesList.isEmpty()) {
        m_complexEntityRepo->upsertComplexEntities(entitiesList);
        qDebug() << "[UdpComplexEntityMessageHandler] ✅ Successfully dispatched" << entitiesList.size()
                 << "complex entities to repository.";
    }

    return true;
}

} // namespace GISApp::Core::Udp::Handlers

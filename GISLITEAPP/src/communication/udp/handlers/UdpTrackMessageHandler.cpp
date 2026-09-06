/**
 * @file UdpTrackMessageHandler.cpp
 * @brief Strategy handler for deserializing MAIN_LITE_TRACK_MSG (Message ID: 613) UDP packets.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "UdpTrackMessageHandler.h"
#include "protocol/MessageIds.h"
#include "protocol/WireStructures.h"
#include "TacticalTrack.h"
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QTimeZone>

namespace GISApp::Core::Udp::Handlers {

UdpTrackMessageHandler::UdpTrackMessageHandler(GISApp::Repositories::ITrackRepository *trackRepo)
    : m_trackRepo(trackRepo)
{
}

MESSAGE_ID UdpTrackMessageHandler::messageId() const
{
    return MAIN_LITE_TRACK_MSG_ID; // 613
}

bool UdpTrackMessageHandler::processPayload(const QByteArray &payload)
{
    const char *dataPtr = payload.constData();
    qsizetype totalBytes = payload.size();
    qsizetype offset = 0;

    if (totalBytes < static_cast<qsizetype>(sizeof(STRUCT_MESSAGE_HEADER) + sizeof(UINT_16))) {
        qWarning() << "[UdpTrackMessageHandler] Payload too short for track header:" << totalBytes;
        return false;
    }

    // 1. Read Standard Header
    STRUCT_MESSAGE_HEADER header;
    SMEMCPY(&header, dataPtr + offset, sizeof(STRUCT_MESSAGE_HEADER));
    offset += sizeof(STRUCT_MESSAGE_HEADER);

    // 2. Read Number of Tracks in Message
    UINT_16 noOfTracks = 0;
    SMEMCPY(&noOfTracks, dataPtr + offset, sizeof(UINT_16));
    offset += sizeof(UINT_16);

    qDebug() << "[UdpTrackMessageHandler] 🎯 Ingesting batch of" << noOfTracks << "tracks from UDP payload.";

    if (!m_trackRepo) {
        qWarning() << "[UdpTrackMessageHandler] No TrackRepository attached! Dropping tracks.";
        return false;
    }

    QVector<GISApp::Domain::Tracks::TacticalTrack> tracksList;
    tracksList.reserve(noOfTracks);

    constexpr qsizetype minTrackBytes = sizeof(UINT_32)
                                      + sizeof(STRING_100)
                                      + sizeof(STRUCT_LOCATION)
                                      + sizeof(IDENTITY)
                                      + sizeof(STRUCT_TRACK_ATTRIBUTES)
                                      + sizeof(SYSTEM_TRACK_TYPE)
                                      + sizeof(UINT_8)
                                      + sizeof(STRUCT_TRACK_SYMBOL)
                                      + sizeof(STRUCT_DATE_TIME)
                                      + sizeof(STRING_100);

    for (UINT_16 i = 0; i < noOfTracks; ++i) {
        if (offset + minTrackBytes > totalBytes) {
            qWarning() << "[UdpTrackMessageHandler] Payload truncated at track index" << i << "of" << noOfTracks;
            break;
        }

        STRUCT_TRACK rawTrack;

        // 1. track_id (UINT_32)
        SMEMCPY(&rawTrack.track_id, dataPtr + offset, sizeof(UINT_32));
        offset += sizeof(UINT_32);

        // 2. track_name (STRING_100)
        SMEMCPY(rawTrack.track_name, dataPtr + offset, sizeof(STRING_100));
        offset += sizeof(STRING_100);

        // 3. track_loc (STRUCT_LOCATION)
        SMEMCPY(&rawTrack.track_loc, dataPtr + offset, sizeof(STRUCT_LOCATION));
        offset += sizeof(STRUCT_LOCATION);

        // 4. track_identity (IDENTITY)
        SMEMCPY(&rawTrack.track_identity, dataPtr + offset, sizeof(IDENTITY));
        offset += sizeof(IDENTITY);

        // 5. track_attributes (STRUCT_TRACK_ATTRIBUTES)
        SMEMCPY(&rawTrack.track_attributes, dataPtr + offset, sizeof(STRUCT_TRACK_ATTRIBUTES));
        offset += sizeof(STRUCT_TRACK_ATTRIBUTES);

        // 6. sys_track_type (SYSTEM_TRACK_TYPE)
        SMEMCPY(&rawTrack.sys_track_type, dataPtr + offset, sizeof(SYSTEM_TRACK_TYPE));
        offset += sizeof(SYSTEM_TRACK_TYPE);

        // 7. no_of_sources (UINT_8)
        SMEMCPY(&rawTrack.no_of_sources, dataPtr + offset, sizeof(UINT_8));
        offset += sizeof(UINT_8);

        // 8. track_sources (QVector<STRUCT_TRACK_SOURCE>) - unpack if physically present in packet
        constexpr qsizetype tailSize = sizeof(STRUCT_TRACK_SYMBOL) + sizeof(STRUCT_DATE_TIME) + sizeof(STRING_100);
        const qsizetype remainingTracksMinBytes = static_cast<qsizetype>(noOfTracks - 1 - i) * minTrackBytes;
        if (rawTrack.no_of_sources > 0 &&
            (totalBytes - offset) >= (remainingTracksMinBytes + static_cast<qsizetype>(rawTrack.no_of_sources * sizeof(STRUCT_TRACK_SOURCE)) + tailSize)) {
            for (UINT_8 s = 0; s < rawTrack.no_of_sources; ++s) {
                STRUCT_TRACK_SOURCE src;
                SMEMCPY(&src, dataPtr + offset, sizeof(STRUCT_TRACK_SOURCE));
                offset += sizeof(STRUCT_TRACK_SOURCE);
                rawTrack.track_sources.append(src);
            }
        }

        // 9. track_symbol (STRUCT_TRACK_SYMBOL)
        SMEMCPY(&rawTrack.track_symbol, dataPtr + offset, sizeof(STRUCT_TRACK_SYMBOL));
        offset += sizeof(STRUCT_TRACK_SYMBOL);

        // 10. track_report_time (STRUCT_DATE_TIME)
        SMEMCPY(&rawTrack.track_report_time, dataPtr + offset, sizeof(STRUCT_DATE_TIME));
        offset += sizeof(STRUCT_DATE_TIME);

        // 11. track_remarks (STRING_100)
        SMEMCPY(rawTrack.track_remarks, dataPtr + offset, sizeof(STRING_100));
        offset += sizeof(STRING_100);

        // Safely extract fixed-width C-strings
        QByteArray nameBytes(rawTrack.track_name, sizeof(rawTrack.track_name));
        QString callsign = QString::fromUtf8(nameBytes.constData()).trimmed();

        QByteArray symBytes(rawTrack.track_symbol.symbol_name, sizeof(rawTrack.track_symbol.symbol_name));
        QString symbol = QString::fromUtf8(symBytes.constData()).trimmed();

        QByteArray remBytes(rawTrack.track_remarks, sizeof(rawTrack.track_remarks));
        QString remarks = QString::fromUtf8(remBytes.constData()).trimmed();

        // Construct Domain TacticalTrack directly from STRUCT_TRACK
        GISApp::Domain::Tracks::TacticalTrack track(
            static_cast<int>(rawTrack.track_id),
            callsign,
            rawTrack.track_loc.latatitude,
            rawTrack.track_loc.longitude,
            rawTrack.track_loc.height,
            rawTrack.track_loc.dir
        );

        track.setIdentity(rawTrack.track_identity);
        track.setAttributes(rawTrack.track_attributes);
        track.setSystemTrackType(rawTrack.sys_track_type);
        track.setTrackSources(rawTrack.track_sources);
        track.setSymbolCode(symbol);
        track.setRemarks(remarks);

        // Reconstruct timestamp
        QDate date(rawTrack.track_report_time.date.year,
                   rawTrack.track_report_time.date.month,
                   rawTrack.track_report_time.date.day);
        QTime time(rawTrack.track_report_time.time.hour,
                   rawTrack.track_report_time.time.minute,
                   rawTrack.track_report_time.time.second);
        if (date.isValid() && time.isValid()) {
            track.setReportTime(QDateTime(date, time, QTimeZone::UTC));
        } else {
            track.setReportTime(QDateTime::currentDateTimeUtc());
        }

        tracksList.append(track);
    }

    if (!tracksList.isEmpty()) {
        m_trackRepo->upsertTracks(tracksList);
        qInfo() << "[UdpTrackMessageHandler] Successfully upserted" << tracksList.size()
                << "tactical tracks into repository.";
    }

    return true;
}

} // namespace GISApp::Core::Udp::Handlers

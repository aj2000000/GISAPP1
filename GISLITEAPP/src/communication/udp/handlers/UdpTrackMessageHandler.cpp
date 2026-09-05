/**
 * @file UdpTrackMessageHandler.cpp
 * @brief Strategy handler for deserializing MAIN_LITE_TRACK_MSG (Message ID: 613) UDP packets.
 * @author BrahmaxisGIS Development Team
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

    for (UINT_16 i = 0; i < noOfTracks; ++i) {
        if (offset + static_cast<qsizetype>(sizeof(STRUCT_TRACK_PAYLOAD)) > totalBytes) {
            qWarning() << "[UdpTrackMessageHandler] Payload truncated at track index" << i << "of" << noOfTracks;
            break;
        }

        STRUCT_TRACK_PAYLOAD rawTrack;
        SMEMCPY(&rawTrack, dataPtr + offset, sizeof(STRUCT_TRACK_PAYLOAD));
        offset += sizeof(STRUCT_TRACK_PAYLOAD);

        // Safely extract fixed-width C-strings
        QByteArray nameBytes(rawTrack.track_name, sizeof(rawTrack.track_name));
        QString callsign = QString::fromUtf8(nameBytes.constData()).trimmed();

        QByteArray symBytes(rawTrack.track_symbol.symbol_name, sizeof(rawTrack.track_symbol.symbol_name));
        QString symbol = QString::fromUtf8(symBytes.constData()).trimmed();

        QByteArray remBytes(rawTrack.track_remarks, sizeof(rawTrack.track_remarks));
        QString remarks = QString::fromUtf8(remBytes.constData()).trimmed();

        // Construct Domain TacticalTrack
        GISApp::Domain::Tracks::TacticalTrack track(
            static_cast<int>(rawTrack.track_id),
            callsign,
            rawTrack.track_loc.latatitude,
            rawTrack.track_loc.longitude,
            rawTrack.track_loc.height,
            rawTrack.track_loc.dir
        );

        // Map Identity (Hostile = 1, Friendly = 2, Neutral = 3)
        switch (rawTrack.track_identity) {
        case HOSTILE:
            track.setIdentity(GISApp::Domain::Tracks::TrackIdentity::Hostile);
            break;
        case FRIENDLY:
            track.setIdentity(GISApp::Domain::Tracks::TrackIdentity::Friendly);
            break;
        case 3:
            track.setIdentity(GISApp::Domain::Tracks::TrackIdentity::Neutral);
            break;
        default:
            track.setIdentity(GISApp::Domain::Tracks::TrackIdentity::Unknown);
            break;
        }

        // Map Domain (Air = 1, Surface = 2, Subsurface = 3, Land = 4)
        switch (rawTrack.track_attributes.type) {
        case 1:
            track.setDomain(GISApp::Domain::Tracks::TrackDomain::Air);
            break;
        case 2:
            track.setDomain(GISApp::Domain::Tracks::TrackDomain::Surface);
            break;
        case 3:
            track.setDomain(GISApp::Domain::Tracks::TrackDomain::Subsurface);
            break;
        case 4:
            track.setDomain(GISApp::Domain::Tracks::TrackDomain::Land);
            break;
        default:
            track.setDomain(GISApp::Domain::Tracks::TrackDomain::Unknown);
            break;
        }

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

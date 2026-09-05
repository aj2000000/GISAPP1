/**
 * @file UdpSender.cpp
 * @brief Implementation of UdpSender datagram transmission and multi-packet chunking.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "UdpSender.h"
#include <QtMath>
#include <QThread>
#include <QDebug>

namespace GISApp::Communication::Udp::Transport {

UdpSender::UdpSender(QObject *parent)
    : QObject(parent)
{
}

qint64 UdpSender::sendDatagram(const QString &ip, quint16 port, const QByteArray &datagram)
{
    // If datagram exceeds single packet limit (1500 bytes), automatically fragment into multi-packets
    if (datagram.size() > MAX_UDP_PACKET_SIZE) {
        qDebug() << "[UdpSender] Datagram size (" << datagram.size()
                 << "bytes) exceeds MAX_UDP_PACKET_SIZE (" << MAX_UDP_PACKET_SIZE
                 << "bytes). Automatically fragmenting into multi-packets.";
        return sendMultiPacket(ip, port, datagram) ? datagram.size() : -1;
    }

    qint64 bytesSent = m_socket.writeDatagram(datagram, QHostAddress(ip), port);
    if (bytesSent < 0) {
        qWarning() << "[UdpSender] Failed to transmit datagram to" << ip << ":" << port
                   << "| Error:" << m_socket.errorString();
    } else {
        qDebug() << "[UdpSender] Dispatched" << bytesSent << "bytes to" << ip << ":" << port;
    }
    return bytesSent;
}

bool UdpSender::sendMultiPacket(const QString &ip, quint16 port, const QByteArray &payload)
{
    qsizetype totalLength = payload.size();
    qsizetype headerSize = sizeof(STRUCT_MESSAGE_HEADER);

    if (totalLength < headerSize) {
        qWarning() << "[UdpSender] Payload smaller than STRUCT_MESSAGE_HEADER! Aborting transmission.";
        return false;
    }

    // Enforce MAX_MSG_SIZE (8000 bytes) on total multi-packet payload
    if (totalLength > MAX_MSG_SIZE) {
        qWarning() << "[UdpSender] Payload size (" << totalLength
                   << "bytes) exceeds MAX_MSG_SIZE (" << MAX_MSG_SIZE
                   << "bytes). Aborting transmission.";
        return false;
    }

    qsizetype maxChunkSize = MAX_UDP_PACKET_SIZE - headerSize;
    qsizetype remainingBody = totalLength - headerSize;

    STRUCT_MESSAGE_HEADER baseHeader;
    SMEMCPY(&baseHeader, payload.constData(), headerSize);

    int totalPackets = static_cast<int>(qCeil(static_cast<qreal>(remainingBody) / maxChunkSize));
    if (totalPackets < 1) totalPackets = 1;

    baseHeader.no_of_packets = static_cast<UINT_16>(totalPackets);

    qsizetype offset = headerSize;
    for (int seqNo = 1; seqNo <= totalPackets; ++seqNo) {
        qsizetype chunkSize = qMin(remainingBody, maxChunkSize);

        STRUCT_MESSAGE_HEADER chunkHeader = baseHeader;
        chunkHeader.packet_seq_no = static_cast<UINT_16>(seqNo);
        chunkHeader.message_len   = static_cast<UINT_16>(chunkSize);

        QByteArray packetBuffer;
        packetBuffer.reserve(headerSize + chunkSize);
        packetBuffer.append(reinterpret_cast<const char*>(&chunkHeader), headerSize);
        if (chunkSize > 0) {
            packetBuffer.append(payload.constData() + offset, chunkSize);
        }

        qint64 sent = sendDatagram(ip, port, packetBuffer);
        if (sent < 0) {
            qWarning() << "[UdpSender] MultiPacket send failed at sequence:" << seqNo << "of" << totalPackets;
            return false;
        }

        offset += chunkSize;
        remainingBody -= chunkSize;

        // Micro-sleep to prevent socket buffer congestion
        QThread::usleep(500);
    }

    return true;
}

} // namespace GISApp::Communication::Udp::Transport

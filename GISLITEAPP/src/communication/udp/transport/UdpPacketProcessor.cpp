/**
 * @file UdpPacketProcessor.cpp
 * @brief Implementation of UdpPacketProcessor queue and multi-packet reassembly.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "UdpPacketProcessor.h"
#include <QMutexLocker>
#include <QDebug>

namespace GISApp::Communication::Udp::Transport {

UdpPacketProcessor& UdpPacketProcessor::instance()
{
    static UdpPacketProcessor s_instance;
    return s_instance;
}

UdpPacketProcessor::UdpPacketProcessor(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &UdpPacketProcessor::onProcessTimerTimeout);
    m_timer->start(100); // 100ms interval for responsive packet processing
}

UdpPacketProcessor::~UdpPacketProcessor()
{
    qDeleteAll(m_multiBufferMap);
    m_multiBufferMap.clear();
}

bool UdpPacketProcessor::enqueueDatagram(const STRUCT_MQBUF &buffer)
{
    QMutexLocker locker(&m_mutex);
    if (m_messageCount >= static_cast<quint32>(NO_OF_MSG_IN_QUEUE)) {
        qWarning() << "[UdpPacketProcessor] Ring buffer overflow! Dropping incoming datagram.";
        return false;
    }

    SMEMCPY(&m_receiveQueue[m_messageCount], &buffer, sizeof(STRUCT_MQBUF));
    m_messageCount++;
    return true;
}

quint32 UdpPacketProcessor::pendingCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_messageCount;
}

void UdpPacketProcessor::onProcessTimerTimeout()
{
    processQueuedMessages();
}

void UdpPacketProcessor::processQueuedMessages()
{
    quint32 batchSize = 0;

    {
        QMutexLocker locker(&m_mutex);
        if (m_messageCount == 0) {
            return;
        }

        batchSize = m_messageCount;
        for (quint32 i = 0; i < batchSize; ++i) {
            m_processingQueue[i] = m_receiveQueue[i];
        }
        m_messageCount = 0;
    }

    for (quint32 i = 0; i < batchSize; ++i) {
        const STRUCT_MESSAGE_HEADER &header = m_processingQueue[i].msg_header;

        if (header.no_of_packets <= 1) {
            // Single self-contained datagram packet
            dispatchAssembledPayload(header, m_processingQueue[i].my_buf, header.message_len);
        } else {
            // Fragment of a multi-packet transmission
            addToMultiBufferMap(m_processingQueue[i]);
        }
    }
}

void UdpPacketProcessor::addToMultiBufferMap(const STRUCT_MQBUF &msg)
{
    QString sessionKey = QString("%1_%2").arg(msg.msg_header.source_id).arg(msg.msg_header.message_id);

    if (m_multiBufferMap.contains(sessionKey)) {
        QVector<STRUCT_MQBUF> *list = m_multiBufferMap.value(sessionKey);
        int expectedSeqNo = list->last().msg_header.packet_seq_no + 1;

        if (msg.msg_header.packet_seq_no == expectedSeqNo) {
            list->append(msg);

            // Check if final packet received
            if (msg.msg_header.no_of_packets == msg.msg_header.packet_seq_no) {
                quint64 totalLength = 0;
                for (int i = 0; i < list->size(); ++i) {
                    totalLength += list->at(i).msg_header.message_len;
                }

                // Enforce MAX_MSG_SIZE (8000 bytes) on total reassembled message
                if (sizeof(STRUCT_MESSAGE_HEADER) + totalLength > static_cast<quint64>(MAX_MSG_SIZE)) {
                    qWarning() << "[UdpPacketProcessor] Reassembled message size ("
                               << (sizeof(STRUCT_MESSAGE_HEADER) + totalLength)
                               << "bytes) exceeds MAX_MSG_SIZE (" << MAX_MSG_SIZE
                               << "bytes). Dropping oversized multi-packet message.";
                    delete list;
                    m_multiBufferMap.remove(sessionKey);
                    return;
                }

                QByteArray assembledBody;
                assembledBody.reserve(totalLength);
                for (int i = 0; i < list->size(); ++i) {
                    assembledBody.append(list->at(i).my_buf, list->at(i).msg_header.message_len);
                }

                dispatchAssembledPayload(msg.msg_header, assembledBody.constData(), assembledBody.size());

                delete list;
                m_multiBufferMap.remove(sessionKey);
            }
        } else {
            qWarning() << "[UdpPacketProcessor] MultiPacket out of sequence for key:" << sessionKey
                       << "| Expected:" << expectedSeqNo << "| Got:" << msg.msg_header.packet_seq_no;

            if (msg.msg_header.packet_seq_no == 1) {
                list->clear();
                list->append(msg);
            } else {
                delete list;
                m_multiBufferMap.remove(sessionKey);
            }
        }
    } else {
        if (msg.msg_header.packet_seq_no == 1) {
            auto *newList = new QVector<STRUCT_MQBUF>;
            newList->append(msg);
            m_multiBufferMap.insert(sessionKey, newList);
        } else {
            qWarning() << "[UdpPacketProcessor] Discarding orphaned multi-packet (seq != 1) for key:" << sessionKey;
        }
    }
}

void UdpPacketProcessor::dispatchAssembledPayload(const STRUCT_MESSAGE_HEADER &msgHeader, const char *body, quint64 bodyLen)
{
    // Enforce MAX_MSG_SIZE (8000 bytes) on complete message (header + body)
    quint64 totalSize = sizeof(msgHeader) + bodyLen;
    if (totalSize > static_cast<quint64>(MAX_MSG_SIZE)) {
        qWarning() << "[UdpPacketProcessor] Total message size (" << totalSize
                   << "bytes) exceeds MAX_MSG_SIZE (" << MAX_MSG_SIZE << "bytes). Dropping payload.";
        return;
    }

    QByteArray fullMessage;
    fullMessage.reserve(totalSize);
    fullMessage.append(reinterpret_cast<const char*>(&msgHeader), sizeof(msgHeader));
    if (body && bodyLen > 0) {
        fullMessage.append(body, bodyLen);
    }

    emit packetReassembled(fullMessage);
}

} // namespace GISApp::Communication::Udp::Transport

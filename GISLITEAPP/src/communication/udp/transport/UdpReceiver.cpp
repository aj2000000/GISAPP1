/**
 * @file UdpReceiver.cpp
 * @brief Implementation of UdpReceiver socket listener thread.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "UdpReceiver.h"
#include "UdpPacketProcessor.h"
#include <QDebug>

namespace GISApp::Communication::Udp::Transport {

UdpReceiver::UdpReceiver(quint16 port, QObject *parent)
    : QThread(parent)
    , m_port(port)
{
}

UdpReceiver::~UdpReceiver()
{
    stop();
}

void UdpReceiver::stop()
{
    m_running.store(false);
    requestInterruption();
    quit();
    wait(2000);
}

void UdpReceiver::run()
{
    m_running.store(true);

    m_socket = new QUdpSocket();
    if (!m_socket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qCritical() << "[UdpReceiver] Failed to bind QUdpSocket to port:" << m_port
                    << "| Error:" << m_socket->errorString();
        delete m_socket;
        m_socket = nullptr;
        m_running.store(false);
        return;
    }

    qInfo() << "[UdpReceiver] Successfully bound QUdpSocket to IPv4 port:" << m_port;

    connect(m_socket, &QUdpSocket::readyRead, this, &UdpReceiver::onReadyRead, Qt::DirectConnection);

    // Enter worker thread event loop
    exec();

    if (m_socket) {
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }

    m_running.store(false);
    qInfo() << "[UdpReceiver] Receiver thread stopped on port:" << m_port;
}

void UdpReceiver::onReadyRead()
{
    if (!m_socket) return;

    while (m_socket->hasPendingDatagrams()) {
        qint64 pendingSize = m_socket->pendingDatagramSize();

        // 1. Enforce MAX_MSG_SIZE (8000 bytes) on received single datagram matching STRUCT_MQBUF capacity
        if (pendingSize > MAX_MSG_SIZE) {
            qWarning() << "[UdpReceiver] Datagram size (" << pendingSize
                       << "bytes) exceeds MAX_MSG_SIZE (" << MAX_MSG_SIZE
                       << "bytes). Discarding oversized datagram.";
            m_socket->readDatagram(nullptr, 0);
            continue;
        }

        // 2. Enforce minimum size: must contain at least STRUCT_MESSAGE_HEADER (16 bytes)
        if (pendingSize < static_cast<qint64>(sizeof(STRUCT_MESSAGE_HEADER))) {
            qWarning() << "[UdpReceiver] Datagram size (" << pendingSize
                       << "bytes) smaller than STRUCT_MESSAGE_HEADER ("
                       << sizeof(STRUCT_MESSAGE_HEADER) << "bytes). Discarding runt packet.";
            m_socket->readDatagram(nullptr, 0);
            continue;
        }

        QHostAddress senderAddress;
        quint16 senderPort = 0;

        char packetBuffer[MAX_MSG_SIZE];
        qint64 bytesRead = m_socket->readDatagram(
            packetBuffer,
            MAX_MSG_SIZE,
            &senderAddress,
            &senderPort
        );

        if (bytesRead >= static_cast<qint64>(sizeof(STRUCT_MESSAGE_HEADER))) {
            STRUCT_MQBUF mqBuf{};
            SMEMCPY(&mqBuf.msg_header, packetBuffer, sizeof(STRUCT_MESSAGE_HEADER));

            quint64 bodyLength = bytesRead - sizeof(STRUCT_MESSAGE_HEADER);
            if (bodyLength > 0) {
                SMEMCPY(mqBuf.my_buf, packetBuffer + sizeof(STRUCT_MESSAGE_HEADER), bodyLength);
            }
            mqBuf.msg_header.message_len = static_cast<MESSAGE_LENGTH>(bodyLength);

            // 3. Enqueue into ring buffer (enforces NO_OF_MSG_IN_QUEUE = 3000 limit)
            if (!UdpPacketProcessor::instance().enqueueDatagram(mqBuf)) {
                qWarning() << "[UdpReceiver] Queue overflow (limit:" << NO_OF_MSG_IN_QUEUE
                           << "packets). Dropping datagram from"
                           << senderAddress.toString() << ":" << senderPort;
            }
        }
    }
}

} // namespace GISApp::Communication::Udp::Transport

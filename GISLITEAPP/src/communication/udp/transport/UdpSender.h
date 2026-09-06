/**
 * @file UdpSender.h
 * @brief Outbound UDP datagram transmission worker supporting automatic multi-packet chunking.
 * @author GISLITE Development Team
 * @date 2026
 *
 * @class UdpSender
 * @brief Manages sending unicast and broadcast UDP datagrams to tactical peers.
 *
 * Architectural Role:
 * - Operates in the Transport layer for outbound communications.
 * - Handles fragmentation of large payloads exceeding MTU / MAX_UDP_PACKET_SIZE (1500 bytes).
 * - Populates packet sequence numbers and total packet counts in STRUCT_MESSAGE_HEADER.
 */

#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include "protocol/WireStructures.h"

namespace GISApp::Communication::Udp::Transport {

/**
 * @class UdpSender
 * @brief Outbound UDP socket manager.
 */
class UdpSender : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs UdpSender instance.
     * @param[in] parent Optional QObject parent.
     */
    explicit UdpSender(QObject *parent = nullptr);

    /**
     * @brief Destructor ensuring socket clean up.
     */
    virtual ~UdpSender() override = default;

    /**
     * @brief Transmits a single raw UDP datagram to target host and port.
     * @param[in] ip Target IPv4 address.
     * @param[in] port Target UDP destination port.
     * @param[in] datagram Binary payload.
     * @return Number of bytes sent, or -1 on error.
     */
    qint64 sendDatagram(const QString &ip, quint16 port, const QByteArray &datagram);

    /**
     * @brief Fragments a large payload into sequenced multi-packets and transmits them sequentially.
     * @param[in] ip Target IPv4 address.
     * @param[in] port Target UDP destination port.
     * @param[in] payload Complete payload buffer including wire header.
     * @return True if all fragments were successfully dispatched, false otherwise.
     */
    bool sendMultiPacket(const QString &ip, quint16 port, const QByteArray &payload);

private:
    QUdpSocket m_socket; ///< Reusable UDP transmission socket
};

} // namespace GISApp::Communication::Udp::Transport

#endif // UDPSENDER_H

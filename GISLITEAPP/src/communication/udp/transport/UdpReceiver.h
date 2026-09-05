/**
 * @file UdpReceiver.h
 * @brief High-throughput non-blocking UDP socket receiver running on a dedicated QThread.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 *
 * @class UdpReceiver
 * @brief Background worker listening on a dedicated network port for incoming tactical datagrams.
 *
 * Architectural Role:
 * - Direct contact point with the external network socket (QUdpSocket).
 * - Executes within its own QThread to guarantee zero interference with the Qt GUI event loop.
 * - Reads raw datagrams and transfers them to UdpPacketProcessor for sequence verification and defragmentation.
 */

#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <atomic>
#include "protocol/WireStructures.h"

namespace GISApp::Communication::Udp::Transport {

/**
 * @class UdpReceiver
 * @brief Worker thread managing the incoming QUdpSocket lifecycle.
 */
class UdpReceiver : public QThread
{
    Q_OBJECT

public:
    /**
     * @brief Constructs UdpReceiver bound to a specific network port.
     * @param[in] port Local UDP port to bind for reception (e.g. 8540).
     * @param[in] parent Optional QObject parent.
     */
    explicit UdpReceiver(quint16 port, QObject *parent = nullptr);

    /**
     * @brief Destructor ensuring safe thread termination and socket closure.
     */
    virtual ~UdpReceiver() override;

    /**
     * @brief Requests graceful thread stoppage and interrupts pending socket reads.
     */
    void stop();

    /**
     * @brief Returns the port this receiver is configured to listen on.
     * @return 16-bit unsigned port number.
     */
    quint16 port() const { return m_port; }

protected:
    /**
     * @brief Main thread execution loop initializing QUdpSocket and reading datagrams.
     */
    void run() override;

private slots:
    /**
     * @brief Slot triggered whenever new datagrams are available on the QUdpSocket.
     */
    void onReadyRead();

private:
    quint16 m_port{8540};                  ///< Local port to bind
    QUdpSocket *m_socket{nullptr};         ///< Internal socket owned by this thread
    std::atomic<bool> m_running{false};    ///< Thread run flag
    STRUCT_MQBUF m_buffer;                 ///< Intermediate buffer for incoming datagrams
};

} // namespace GISApp::Communication::Udp::Transport

#endif // UDPRECEIVER_H

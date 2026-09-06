/**
 * @file UdpPacketProcessor.h
 * @brief Thread-safe queue processing and multi-packet reassembly engine.
 * @author GISLITE Development Team
 * @date 2026
 *
 * @class UdpPacketProcessor
 * @brief Singleton worker responsible for defragmenting multi-packet UDP datagrams.
 *
 * Architectural Role:
 * - Operates in the Transport layer.
 * - Receives raw datagrams enqueued by UdpReceiver into a mutex-protected ring buffer.
 * - Manages sequence-ordered reassembly of multi-packet payloads using packet sequence numbers.
 * - Forwards fully reconstituted binary payloads to the Service Layer via packetReassembled signal.
 */

#ifndef UDPPACKETPROCESSOR_H
#define UDPPACKETPROCESSOR_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMap>
#include <QVector>
#include <QByteArray>
#include "protocol/WireStructures.h"

namespace GISApp::Communication::Udp::Transport {

/**
 * @class UdpPacketProcessor
 * @brief Defragmentation engine and packet queue orchestrator.
 */
class UdpPacketProcessor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Returns the global singleton instance.
     * @return Reference to the thread-safe UdpPacketProcessor.
     */
    static UdpPacketProcessor& instance();

    // Prevent copies and moves
    UdpPacketProcessor(const UdpPacketProcessor &) = delete;
    UdpPacketProcessor& operator=(const UdpPacketProcessor &) = delete;

    /**
     * @brief Pushes a newly received datagram into the processing ring buffer.
     * @param[in] buffer Raw datagram buffer containing message header and data.
     * @return True if enqueued successfully, false if queue is full.
     */
    bool enqueueDatagram(const STRUCT_MQBUF &buffer);

    /**
     * @brief Checks the number of pending datagrams in the reception queue.
     * @return Current message count.
     */
    quint32 pendingCount() const;

signals:
    /**
     * @brief Emitted when a single-packet or fully reassembled multi-packet payload is ready.
     * @param[out] message Complete datagram byte array including STRUCT_MESSAGE_HEADER.
     */
    void packetReassembled(const QByteArray &message);

public slots:
    /**
     * @brief Drains the ring buffer and processes queued datagrams.
     */
    void processQueuedMessages();

private slots:
    /**
     * @brief Periodic timer trigger to ensure buffered packets are processed promptly.
     */
    void onProcessTimerTimeout();

private:
    explicit UdpPacketProcessor(QObject *parent = nullptr);
    virtual ~UdpPacketProcessor() override;

    /**
     * @brief Adds a fragmented packet to the reassembly tracker.
     * @param[in] msg Incoming packet fragment.
     */
    void addToMultiBufferMap(const STRUCT_MQBUF &msg);

    /**
     * @brief Packs header and body into a contiguous QByteArray and emits packetReassembled.
     * @param[in] msgHeader Standard wire header.
     * @param[in] body Raw data buffer pointer.
     * @param[in] bodyLen Length of body buffer in bytes.
     */
    void dispatchAssembledPayload(const STRUCT_MESSAGE_HEADER &msgHeader, const char *body, quint64 bodyLen);

    mutable QMutex m_mutex;                                 ///< Protects ring buffer access across threads
    QTimer *m_timer{nullptr};                               ///< Periodic dispatch timer
    STRUCT_MQBUF m_receiveQueue[NO_OF_MSG_IN_QUEUE];        ///< Reception ring buffer written by receiver
    quint32 m_messageCount{0};                              ///< Number of valid datagrams in m_receiveQueue
    STRUCT_MQBUF m_processingQueue[NO_OF_MSG_IN_QUEUE];     ///< Working queue for local draining
    QMap<QString, QVector<STRUCT_MQBUF>*> m_multiBufferMap; ///< Multi-packet fragmentation tracking map
};

} // namespace GISApp::Communication::Udp::Transport

#endif // UDPPACKETPROCESSOR_H

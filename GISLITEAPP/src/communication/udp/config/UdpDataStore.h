/**
 * @file UdpDataStore.h
 * @brief Thread-safe configuration and runtime state store for the UDP communication subsystem.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 *
 * @class UdpDataStore
 * @brief Singleton repository managing UDP ports, target IP addresses, and runtime cryptographic state.
 *
 * Architectural Role:
 * - Provides centralized configuration parameters to UdpReceiver, UdpSender, and UdpServiceMediator.
 * - Protects concurrent access across network threads with internal mutex synchronization.
 * - Holds AES cipher settings and manages incoming/outgoing packet holding queues.
 */

#ifndef UDPDATASTORE_H
#define UDPDATASTORE_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QByteArray>
#include <QString>
#include "security/Aes256Cipher.h"

namespace GISApp::Communication::Udp::Config {

/**
 * @class UdpDataStore
 * @brief Singleton managing network addresses, ports, and thread-shared buffers.
 */
class UdpDataStore : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Returns the global singleton instance.
     * @return Reference to the thread-safe UdpDataStore instance.
     */
    static UdpDataStore& instance();

    // Prevent copies and moves
    UdpDataStore(const UdpDataStore &) = delete;
    UdpDataStore& operator=(const UdpDataStore &) = delete;

    /**
     * @brief Loads network ports and host configurations from environment or settings file.
     * @param[in] configFilePath Optional explicit path to configuration file.
     */
    void loadConfiguration(const QString &configFilePath = QString());

    /**
     * @brief Gets the local UDP listening port.
     * @return 16-bit integer port (default: 8540).
     */
    quint16 listenPort() const;

    /**
     * @brief Sets the local UDP listening port.
     * @param[in] port 16-bit port number.
     */
    void setListenPort(quint16 port);

    /**
     * @brief Gets the target host IP address for outgoing transmissions.
     * @return Host IPv4 address string (default: "127.0.0.1").
     */
    QString targetIp() const;

    /**
     * @brief Sets the target host IP address for outgoing transmissions.
     * @param[in] ip IPv4 address string.
     */
    void setTargetIp(const QString &ip);

    /**
     * @brief Gets the target UDP destination port for outbound transmissions.
     * @return 16-bit port number (default: 8541).
     */
    quint16 targetPort() const;

    /**
     * @brief Sets the target UDP destination port for outbound transmissions.
     * @param[in] port 16-bit port number.
     */
    void setTargetPort(quint16 port);

    /**
     * @brief Appends an encrypted datagram into the processing queue.
     * @param[in] data Raw datagram byte buffer.
     */
    void enqueueReceivedData(const QByteArray &data);

    /**
     * @brief Flushes and retrieves all pending queued datagrams.
     * @return List of pending QByteArray buffers.
     */
    QList<QByteArray> takePendingData();

    /**
     * @brief Accesses the internal AES-256 cipher helper.
     * @return Reference to active Aes256Cipher instance.
     */
    Security::Aes256Cipher& cipher() { return m_cipher; }

private:
    explicit UdpDataStore(QObject *parent = nullptr);
    virtual ~UdpDataStore() override = default;

    mutable QMutex m_mutex;             ///< Synchronization mutex guarding configuration properties
    quint16 m_listenPort{8540};         ///< Local UDP port to bind for reception
    QString m_targetIp{"127.0.0.1"};    ///< Target peer IP for transmission
    quint16 m_targetPort{8541};         ///< Target peer UDP port
    QString m_configPath;               ///< Path to active configuration file
    QList<QByteArray> m_pendingPackets; ///< Holding queue for received payloads
    Security::Aes256Cipher m_cipher;    ///< Symmetric cipher instance
};

} // namespace GISApp::Communication::Udp::Config

#endif // UDPDATASTORE_H

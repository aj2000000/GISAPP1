/**
 * @file UdpServiceMediator.h
 * @brief High-level Service Layer Facade orchestrating the entire UDP communication subsystem.
 * @author GISLITE Development Team
 * @date 2026
 *
 * @class UdpServiceMediator
 * @brief Central service component bridging low-level network threads and the application domain.
 *
 * Architectural Role:
 * - Operates as the Service Layer interface for UDP communication.
 * - Coordinates socket listeners (UdpReceiver), defragmentation (UdpPacketProcessor),
 *   outbound transmissions (UdpSender), cryptographic ciphers (Aes256Cipher),
 *   and message routing (UdpMessageDispatcher).
 * - Exposes clean Qt signals to application controllers and UI models without exposing low-level socket details.
 */

#ifndef UDPSERVICEMEDIATOR_H
#define UDPSERVICEMEDIATOR_H

#include <QObject>
#include <memory>
#include "transport/UdpReceiver.h"
#include "transport/UdpSender.h"
#include "transport/UdpPacketProcessor.h"
#include "handlers/UdpMessageDispatcher.h"
#include "config/UdpDataStore.h"

namespace GISApp::Repositories {
class ITrackRepository;
}

namespace GISApp::Communication::Udp {

/**
 * @class UdpServiceMediator
 * @brief Service facade managing network threads, packet assembly, and message dispatching.
 */
class UdpServiceMediator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs UdpServiceMediator, binds configuration, and creates dispatcher.
     * @param[in] parent Optional parent QObject for memory management.
     */
    explicit UdpServiceMediator(QObject *parent = nullptr);

    /**
     * @brief Destructor terminating background receiver thread cleanly.
     */
    virtual ~UdpServiceMediator() override;

    /**
     * @brief Starts the UDP receiver thread on the configured listen port.
     * @return True if thread was successfully started.
     */
    bool startService();

    /**
     * @brief Gracefully stops the receiver thread and releases socket resources.
     */
    void stopService();

    /**
     * @brief Binds a track repository and registers the UdpTrackMessageHandler for Message ID 613.
     * @param[in] trackRepo Pointer to ITrackRepository.
     */
    void registerTrackRepository(GISApp::Repositories::ITrackRepository *trackRepo);

    /**
     * @brief Provides access to the message dispatcher for registering domain handlers.
     * @return Pointer to internal UdpMessageDispatcher.
     */
    Handlers::UdpMessageDispatcher* dispatcher() const { return m_dispatcher; }

    /**
     * @brief Provides access to the outbound sender.
     * @return Pointer to internal UdpSender.
     */
    Transport::UdpSender* sender() const { return m_sender.get(); }

    /**
     * @brief Transmits a datagram to a remote endpoint.
     * @param[in] ip Target IPv4 address.
     * @param[in] port Target UDP port.
     * @param[in] data Payload bytes.
     * @return True if dispatched.
     */
    bool sendData(const QString &ip, quint16 port, const QByteArray &data);

signals:
    /**
     * @brief Emitted whenever any raw reassembled UDP payload arrives.
     * @param[out] payload Complete binary datagram.
     */
    void rawPayloadReceived(const QByteArray &payload);

    /**
     * @brief Relayed from dispatcher when a tactical/telemetry layer updates its GeoJSON cache.
     * @param[out] layerName Name of the updated layer.
     * @param[out] geoJsonPath Disk path to updated GeoJSON file.
     */
    void telemetryLayerUpdated(const QString &layerName, const QString &geoJsonPath);

private slots:
    /**
     * @brief Internal slot receiving reassembled datagrams from UdpPacketProcessor.
     * @param[in] message Fully assembled binary datagram.
     */
    void onPacketReassembled(const QByteArray &message);

private:
    std::unique_ptr<Transport::UdpReceiver> m_receiver; ///< Dedicated background socket receiver thread
    std::unique_ptr<Transport::UdpSender>   m_sender;   ///< Socket transmitter
    Handlers::UdpMessageDispatcher         *m_dispatcher{nullptr}; ///< Message routing dispatcher
};

} // namespace GISApp::Communication::Udp

#endif // UDPSERVICEMEDIATOR_H

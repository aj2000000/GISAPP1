/**
 * @file UdpComplexEntityMessageHandler.h
 * @brief Strategy handler for deserializing MAIN_LITE_COMPLEX_ENTITY_MSG (Message ID: 905) UDP packets.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef UDPCOMPLEXENTITYMESSAGEHANDLER_H
#define UDPCOMPLEXENTITYMESSAGEHANDLER_H

#include "IUdpMessageHandler.h"
#include "IComplexEntityRepository.h"

namespace GISApp::Core::Udp::Handlers {

/**
 * @class UdpComplexEntityMessageHandler
 * @brief Concrete strategy deserializing dynamic batch complex entity records from binary UDP datagrams.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the **UDP Communication Handlers Layer** (`src/communication/udp/handlers/`).
 * - Implements `IUdpMessageHandler` (Strategy Pattern) registered in `UdpMessageDispatcher`.
 * - Deserializes binary wire payloads for `MAIN_LITE_COMPLEX_ENTITY_MSG_ID` (Message ID 905).
 * - Performs sequential unpacking of dynamic location points and extensible key-value details.
 * - Injects unpacked domain models into `IComplexEntityRepository`.
 */
class UdpComplexEntityMessageHandler : public IUdpMessageHandler
{
public:
    /**
     * @brief Constructs UdpComplexEntityMessageHandler bound to target repository.
     * @param[in] complexEntityRepo Pointer to IComplexEntityRepository.
     */
    explicit UdpComplexEntityMessageHandler(GISApp::Repositories::IComplexEntityRepository *complexEntityRepo);

    /**
     * @brief Destructor.
     */
    virtual ~UdpComplexEntityMessageHandler() override = default;

    /**
     * @brief Returns numerical message identifier handled by this strategy.
     * @return MAIN_LITE_COMPLEX_ENTITY_MSG_ID (905).
     */
    [[nodiscard]] MESSAGE_ID messageId() const override;

    /**
     * @brief Unpacks binary datagram payload and pushes parsed entities to repository.
     * @param[in] payload Reassembled binary datagram payload.
     * @return True if processed successfully, false if malformed or truncated.
     */
    bool processPayload(const QByteArray &payload) override;

private:
    GISApp::Repositories::IComplexEntityRepository *m_complexEntityRepo{nullptr};
};

} // namespace GISApp::Core::Udp::Handlers

#endif // UDPCOMPLEXENTITYMESSAGEHANDLER_H

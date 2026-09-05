/**
 * @file IUdpMessageHandler.h
 * @brief Strategy interface for parsing and processing specific UDP binary message types.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 *
 * @class IUdpMessageHandler
 * @brief Abstract base class for all UDP message deserializers and domain handlers.
 *
 * Architectural Role:
 * - Operates in the Service/Handler layer of the UDP subsystem.
 * - Implements the Strategy design pattern: concrete subclasses register with UdpMessageDispatcher
 *   for specific MESSAGE_ID values.
 * - Encapsulates payload deserialization and invocation of domain repositories or services.
 */

#ifndef IUDPMESSAGEHANDLER_H
#define IUDPMESSAGEHANDLER_H

#include <QByteArray>
#include "protocol/IrsTypes.h"

namespace GISApp::Core::Udp::Handlers {

/**
 * @class IUdpMessageHandler
 * @brief Abstract Strategy interface for message deserialization.
 */
class IUdpMessageHandler {
public:
    virtual ~IUdpMessageHandler() = default;

    /**
     * @brief Returns the unique Message ID that this handler processes.
     * @return 16-bit unsigned MESSAGE_ID numeric identifier.
     */
    virtual MESSAGE_ID messageId() const = 0;

    /**
     * @brief Processes and deserializes the incoming raw UDP binary payload.
     * @param[in] payload Complete datagram QByteArray buffer including wire header.
     * @return True if parsing and processing succeeded, false otherwise.
     */
    virtual bool processPayload(const QByteArray &payload) = 0;
};

} // namespace GISApp::Core::Udp::Handlers

#endif // IUDPMESSAGEHANDLER_H

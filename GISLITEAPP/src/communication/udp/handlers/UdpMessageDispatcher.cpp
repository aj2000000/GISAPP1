/**
 * @file UdpMessageDispatcher.cpp
 * @brief Implementation of central UdpMessageDispatcher routing.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "UdpMessageDispatcher.h"
#include "protocol/WireStructures.h"
#include <QDebug>

namespace GISApp::Core::Udp::Handlers {

UdpMessageDispatcher::UdpMessageDispatcher(QObject *parent)
    : QObject(parent)
{
}

void UdpMessageDispatcher::registerHandler(std::shared_ptr<IUdpMessageHandler> handler)
{
    if (!handler) return;
    MESSAGE_ID id = handler->messageId();
    m_handlers[id] = handler;
    qDebug() << "[UdpMessageDispatcher] Registered handler for Message ID:" << id;
}

void UdpMessageDispatcher::dispatchMessage(const QByteArray &payload)
{
    if (static_cast<size_t>(payload.size()) < sizeof(STRUCT_MESSAGE_HEADER)) {
        qWarning() << "[UdpMessageDispatcher] Payload smaller than STRUCT_MESSAGE_HEADER size!";
        return;
    }

    STRUCT_MESSAGE_HEADER header;
    SMEMCPY(&header, payload.constData(), sizeof(STRUCT_MESSAGE_HEADER));

    MESSAGE_ID msgId = header.message_id;

    if (m_handlers.contains(msgId)) {
        bool success = m_handlers[msgId]->processPayload(payload);
        if (!success) {
            qWarning() << "[UdpMessageDispatcher] Handler failed to process Message ID:" << msgId;
        }
    } else {
        qDebug() << "[UdpMessageDispatcher] No handler registered for Message ID:" << msgId;
    }
}

} // namespace GISApp::Core::Udp::Handlers

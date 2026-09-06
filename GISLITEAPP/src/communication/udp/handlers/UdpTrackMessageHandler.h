/**
 * @file UdpTrackMessageHandler.h
 * @brief Strategy handler for deserializing MAIN_LITE_TRACK_MSG (Message ID: 613).
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef UDPTRACKMESSAGEHANDLER_H
#define UDPTRACKMESSAGEHANDLER_H

#include "IUdpMessageHandler.h"
#include "ITrackRepository.h"

namespace GISApp::Core::Udp::Handlers {

/**
 * @class UdpTrackMessageHandler
 * @brief Strategy deserializing binary track messages (ID: 613) into TacticalTrack domain entities.
 */
class UdpTrackMessageHandler : public IUdpMessageHandler
{
public:
    /**
     * @brief Constructs track message handler.
     * @param[in] trackRepo Repository where deserialized tracks are saved.
     */
    explicit UdpTrackMessageHandler(GISApp::Repositories::ITrackRepository *trackRepo);
    virtual ~UdpTrackMessageHandler() override = default;

    [[nodiscard]] MESSAGE_ID messageId() const override;
    bool processPayload(const QByteArray &payload) override;

private:
    GISApp::Repositories::ITrackRepository *m_trackRepo{nullptr};
};

} // namespace GISApp::Core::Udp::Handlers

#endif // UDPTRACKMESSAGEHANDLER_H

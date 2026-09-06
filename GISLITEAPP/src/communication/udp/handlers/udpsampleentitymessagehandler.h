#ifndef UDPSAMPLEENTITYMESSAGEHANDLER_H
#define UDPSAMPLEENTITYMESSAGEHANDLER_H


#include "IUdpMessageHandler.h"
#include "ISampleEntityRepository.h"

namespace GISApp::Core::Udp::Handlers {
class UdpSampleEntityMessageHandler : public IUdpMessageHandler
{
public:
    explicit UdpSampleEntityMessageHandler(GISApp::Repositories::ISampleEntityRepository *sampleEntityRepo);
    virtual ~UdpSampleEntityMessageHandler() override = default;
    [[nodiscard]] MESSAGE_ID messageId() const override;
    bool processPayload(const QByteArray &payload) override;

private:
    GISApp::Repositories::ISampleEntityRepository *m_sampleEntityRepo{nullptr};
};

} // namespace GISApp::Core::Udp::Handlers
#endif // UDPSAMPLEENTITYMESSAGEHANDLER_H

/**
 * @file UdpServiceMediator.cpp
 * @brief Implementation of UdpServiceMediator network service facade.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "UdpServiceMediator.h"
#include "protocol/WireStructures.h"
#include "handlers/UdpTrackMessageHandler.h"
#include "ITrackRepository.h"

#include "handlers/udpsampleentitymessagehandler.h"
#include "ISampleEntityRepository.h"

#include <QDateTime>
#include <QDebug>

namespace GISApp::Communication::Udp {

UdpServiceMediator::UdpServiceMediator(QObject *parent)
    : QObject(parent)
{
    // Load network configuration
    Config::UdpDataStore::instance().loadConfiguration();

    // Create outbound sender
    m_sender = std::make_unique<Transport::UdpSender>(this);

    // Create central message dispatcher
    m_dispatcher = new Handlers::UdpMessageDispatcher(this);

    // Forward telemetryLayerUpdated from dispatcher
    connect(m_dispatcher, &Handlers::UdpMessageDispatcher::telemetryLayerUpdated,
            this, &UdpServiceMediator::telemetryLayerUpdated);

    // Connect packet reassembly engine output to our dispatch slot
    connect(&Transport::UdpPacketProcessor::instance(),
            &Transport::UdpPacketProcessor::packetReassembled,
            this,
            &UdpServiceMediator::onPacketReassembled);
}

UdpServiceMediator::~UdpServiceMediator()
{
    stopService();
}

bool UdpServiceMediator::startService()
{
    quint16 listenPort = Config::UdpDataStore::instance().listenPort();

    qInfo() << "[UdpServiceMediator] Starting UDP Service on port:" << listenPort;

    if (!m_receiver) {
        m_receiver = std::make_unique<Transport::UdpReceiver>(listenPort, this);
    }

    if (!m_receiver->isRunning()) {
        m_receiver->start();
    }

    return true;
}

void UdpServiceMediator::stopService()
{
    if (m_receiver && m_receiver->isRunning()) {
        qInfo() << "[UdpServiceMediator] Stopping UDP Receiver thread...";
        m_receiver->stop();
    }
}

void UdpServiceMediator::registerTrackRepository(GISApp::Repositories::ITrackRepository *trackRepo)
{
    if (m_dispatcher && trackRepo) {
        auto trackHandler = std::make_shared<GISApp::Core::Udp::Handlers::UdpTrackMessageHandler>(trackRepo);
        m_dispatcher->registerHandler(trackHandler);
        qInfo() << "[UdpServiceMediator] Registered UdpTrackMessageHandler with ITrackRepository.";
    }
}

void UdpServiceMediator::registerSampleEntityRepository(GISApp::Repositories::ISampleEntityRepository *sampleEntityRepo)
{
    if (m_dispatcher && sampleEntityRepo) {
        auto sampleHandler = std::make_shared<GISApp::Core::Udp::Handlers::UdpSampleEntityMessageHandler>(sampleEntityRepo);
        m_dispatcher->registerHandler(sampleHandler);
        qInfo() << "[UdpServiceMediator] Registered UdpSampleEntityMessageHandler with ISampleEntityRepository.";
    }
}


bool UdpServiceMediator::sendData(const QString &ip, quint16 port, const QByteArray &data)
{
    if (!m_sender) return false;
    return m_sender->sendDatagram(ip, port, data) >= 0;
}

void UdpServiceMediator::onPacketReassembled(const QByteArray &message)
{
    if (static_cast<size_t>(message.size()) < sizeof(STRUCT_MESSAGE_HEADER)) {
        qWarning() << "[UdpServiceMediator] Dropping payload smaller than STRUCT_MESSAGE_HEADER:" << message.size();
        return;
    }

    STRUCT_MESSAGE_HEADER header;
    SMEMCPY(&header, message.constData(), sizeof(STRUCT_MESSAGE_HEADER));

    qDebug() << "[UdpServiceMediator] 📥 Datagram Assembled | Time:"
             << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
             << "| Src:" << header.source_id
             << "| Dst:" << header.destination_id
             << "| Msg ID:" << header.message_id
             << "| Len:" << header.message_len;

    // Emit generic notification
    emit rawPayloadReceived(message);

    // Forward to message dispatcher for routing to registered domain handlers
    if (m_dispatcher) {
        m_dispatcher->dispatchMessage(message);
    }
}

} // namespace GISApp::Communication::Udp

/**
 * @file UdpDataStore.cpp
 * @brief Implementation of UdpDataStore configuration and state management.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "UdpDataStore.h"
#include <QMutexLocker>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>

namespace GISApp::Communication::Udp::Config {

UdpDataStore& UdpDataStore::instance()
{
    static UdpDataStore s_instance;
    return s_instance;
}

UdpDataStore::UdpDataStore(QObject *parent)
    : QObject(parent)
{
}

void UdpDataStore::loadConfiguration(const QString &configFilePath)
{
    QMutexLocker locker(&m_mutex);
    m_configPath = configFilePath;

    if (!m_configPath.isEmpty() && QFileInfo::exists(m_configPath)) {
        QSettings settings(m_configPath, QSettings::IniFormat);
        m_listenPort = static_cast<quint16>(settings.value("UDP/ListenPort", 8540).toUInt());
        m_targetIp   = settings.value("UDP/TargetIp", "127.0.0.1").toString();
        m_targetPort = static_cast<quint16>(settings.value("UDP/TargetPort", 8541).toUInt());

        qInfo() << "[UdpDataStore] Loaded network configuration from:" << m_configPath
                << "-> ListenPort:" << m_listenPort
                << "| Target:" << m_targetIp << ":" << m_targetPort;
    } else {
        qInfo() << "[UdpDataStore] Using default network settings -> ListenPort:"
                << m_listenPort << "| Target:" << m_targetIp << ":" << m_targetPort;
    }
}

quint16 UdpDataStore::listenPort() const
{
    QMutexLocker locker(&m_mutex);
    return m_listenPort;
}

void UdpDataStore::setListenPort(quint16 port)
{
    QMutexLocker locker(&m_mutex);
    m_listenPort = port;
}

QString UdpDataStore::targetIp() const
{
    QMutexLocker locker(&m_mutex);
    return m_targetIp;
}

void UdpDataStore::setTargetIp(const QString &ip)
{
    QMutexLocker locker(&m_mutex);
    m_targetIp = ip;
}

quint16 UdpDataStore::targetPort() const
{
    QMutexLocker locker(&m_mutex);
    return m_targetPort;
}

void UdpDataStore::setTargetPort(quint16 port)
{
    QMutexLocker locker(&m_mutex);
    m_targetPort = port;
}

void UdpDataStore::enqueueReceivedData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_pendingPackets.append(data);
}

QList<QByteArray> UdpDataStore::takePendingData()
{
    QMutexLocker locker(&m_mutex);
    QList<QByteArray> copy = m_pendingPackets;
    m_pendingPackets.clear();
    return copy;
}

} // namespace GISApp::Communication::Udp::Config

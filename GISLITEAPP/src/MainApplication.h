#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QObject>
#include <QString>

class MainBaseUI;
class AuthWindow;

namespace GISApp::Communication::Udp {
class UdpServiceMediator;
}

class MainApplication : public QObject
{
    Q_OBJECT

public:
    explicit MainApplication(QObject *parent = nullptr);
    virtual ~MainApplication();

    /**
     * @brief Performs application initialization routines (config, database, services, ui).
     * @return true if initialization succeeded, false otherwise.
     */
    bool initialize();

    /**
     * @brief Performs cleanup before application exits.
     */
    void shutdown();

    MainBaseUI *mainWindow() const;
    GISApp::Communication::Udp::UdpServiceMediator *udpMediator() const { return m_udpMediator; }

    static MainApplication* instance() { return s_instance; }

private:
    inline static MainApplication *s_instance{nullptr};

signals:
    void initializationCompleted();
    void initializationFailed(const QString &reason);

private slots:
    void onUserAuthenticated(const QString &username);
    void onAboutToQuit();

private:
    bool initApplicationMetadata();
    bool initPaths();
    bool initDatabase();
    bool initNetwork();
    bool initServices();
    bool initUi();

    bool m_initialized;
    QString m_appDataDir;
    QString m_configDir;
    AuthWindow *m_authWindow;
    MainBaseUI *m_mainWindow;
    GISApp::Communication::Udp::UdpServiceMediator *m_udpMediator{nullptr};
};

#endif // MAINAPPLICATION_H

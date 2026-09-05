#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QObject>
#include <QString>

class MainBaseUI;
class AuthWindow;

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
};

#endif // MAINAPPLICATION_H

#include "MainApplication.h"
#include "MainBaseUI.h"
#include "AuthWindow.h"
#include "ThemeManager.h"
#include "DatabaseManager.h"
#include "UdpServiceMediator.h"
#include "ITrackRepository.h"
#include "TrackRepository.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

MainApplication::MainApplication(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_authWindow(nullptr)
    , m_mainWindow(nullptr)
{
    s_instance = this;
    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                this, &MainApplication::onAboutToQuit);
    }
}

MainApplication::~MainApplication()
{
    shutdown();
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

bool MainApplication::initialize()
{
    if (m_initialized) {
        qWarning() << "[MainApplication] Application is already initialized.";
        return true;
    }

    qInfo() << "[MainApplication] Starting initialization...";

    if (!initApplicationMetadata()) {
        qCritical() << "[MainApplication] Failed to initialize application metadata.";
        emit initializationFailed("Failed to initialize application metadata.");
        return false;
    }

    if (!initPaths()) {
        qCritical() << "[MainApplication] Failed to initialize application paths.";
        emit initializationFailed("Failed to initialize application paths.");
        return false;
    }

    if (!initDatabase()) {
        qCritical() << "[MainApplication] Failed to initialize database.";
        emit initializationFailed("Failed to initialize database.");
        return false;
    }

    if (!initNetwork()) {
        qCritical() << "[MainApplication] Failed to initialize network/communication.";
        emit initializationFailed("Failed to initialize network.");
        return false;
    }

    if (!initServices()) {
        qCritical() << "[MainApplication] Failed to initialize services.";
        emit initializationFailed("Failed to initialize services.");
        return false;
    }

    if (!initUi()) {
        qCritical() << "[MainApplication] Failed to initialize UI.";
        emit initializationFailed("Failed to initialize UI.");
        return false;
    }

    m_initialized = true;
    qInfo() << "[MainApplication] Initialization completed successfully.";
    emit initializationCompleted();

    return true;
}

MainBaseUI *MainApplication::mainWindow() const
{
    return m_mainWindow;
}

void MainApplication::shutdown()
{
    if (!m_initialized) {
        return;
    }

    qInfo() << "[MainApplication] Shutting down application resources...";

    // Close and release auth window if open
    if (m_authWindow) {
        m_authWindow->close();
        delete m_authWindow;
        m_authWindow = nullptr;
    }

    // Close and release main window
    if (m_mainWindow) {
        m_mainWindow->close();
        delete m_mainWindow;
        m_mainWindow = nullptr;
    }

    // Stop UDP communication service
    if (m_udpMediator) {
        m_udpMediator->stopService();
    }

    // Close database connection if open
    GISApp::Database::DatabaseManager::instance().close();

    m_initialized = false;
    qInfo() << "[MainApplication] Shutdown completed.";
}

void MainApplication::onAboutToQuit()
{
    shutdown();
}

bool MainApplication::initApplicationMetadata()
{
    QCoreApplication::setOrganizationName("GISLITE");
    QCoreApplication::setOrganizationDomain("gislite.org");
    QCoreApplication::setApplicationName("GISLITEAPP");
    QCoreApplication::setApplicationVersion("1.0.0");

    qInfo() << "[MainApplication] Organization:" << QCoreApplication::organizationName();
    qInfo() << "[MainApplication] Application:" << QCoreApplication::applicationName()
            << "v" << QCoreApplication::applicationVersion();
    return true;
}

bool MainApplication::initPaths()
{
    m_appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    QDir dir;
    if (!dir.mkpath(m_appDataDir)) {
        qWarning() << "[MainApplication] Could not create AppData directory:" << m_appDataDir;
    } else {
        qInfo() << "[MainApplication] AppData directory:" << m_appDataDir;
    }

    if (!dir.mkpath(m_configDir)) {
        qWarning() << "[MainApplication] Could not create AppConfig directory:" << m_configDir;
    } else {
        qInfo() << "[MainApplication] AppConfig directory:" << m_configDir;
    }

    return true;
}

bool MainApplication::initDatabase()
{
    qInfo() << "[MainApplication] Initializing database subsystem...";
    QString dbFile = m_appDataDir + "/gislite.db";
    return GISApp::Database::DatabaseManager::instance().initialize(dbFile);
}

bool MainApplication::initNetwork()
{
    qInfo() << "[MainApplication] Initializing network and UDP communication...";
    m_udpMediator = new GISApp::Communication::Udp::UdpServiceMediator(this);
    m_udpMediator->startService();
    return true;
}

bool MainApplication::initServices()
{
    qInfo() << "[MainApplication] Initializing application domain services...";
    // Placeholder for service registration and dependency injection
    return true;
}

bool MainApplication::initUi()
{
    const auto savedTheme = GISApp::UI::ThemeManager::instance().loadSavedThemeOrDefault();
    qInfo() << "[MainApplication] Applying application theme:" << GISApp::UI::ThemeManager::themeName(savedTheme);
    GISApp::UI::ThemeManager::instance().applyTheme(savedTheme, false);

    qInfo() << "[MainApplication] Initializing authentication user interface...";
    m_authWindow = new AuthWindow();
    connect(m_authWindow, &AuthWindow::authenticated,
            this, &MainApplication::onUserAuthenticated);
    m_authWindow->show();
    m_authWindow->byPassLoginforTest();
    return true;
}

void MainApplication::onUserAuthenticated(const QString &username)
{
    qInfo() << "[MainApplication] Authentication successful for user:" << username;

    if (m_authWindow) {
        m_authWindow->close();
        m_authWindow->deleteLater();
        m_authWindow = nullptr;
    }

    // Launch MainBaseUI (with menu bar, status bar, toolbars, and central workspace)
    m_mainWindow = new MainBaseUI(nullptr, MainBaseUI::WindowStartupMode::Maximized);

    // Register live track repository with UDP Mediator
    if (m_udpMediator && m_mainWindow && m_mainWindow->trackRepository()) {
        m_udpMediator->registerTrackRepository(m_mainWindow->trackRepository());
    }

    m_mainWindow->showStatusMessage(tr("Welcome %1 — Workspace Ready").arg(username), 6000);
}


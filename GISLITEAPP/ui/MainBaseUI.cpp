#include "MainBaseUI.h"
#include "ThemeManager.h"
#include "HeaderBar.h"
#include "LeftSidebar.h"
#include "RightToolPanel.h"
#include "ZoomControlsWidget.h"
#include "TacticalStatusBar.h"
#include "MapViewContainer.h"
#include "MapWidget.h"
#include "MapController.h"
#include "LayerTreeModel.h"
#include "SqliteLayerRepository.h"
#include "LayerController.h"
#include "LayerTreePanel.h"
#include "TrackRepository.h"
#include "TacticalTrackService.h"
#include "TrackMapRenderer.h"
#include "TrackController.h"
#include "TrackTableModel.h"
#include "TrackTablePanelDialog.h"
#include "TrackRequestDialog.h"
#include "WireStructures.h"
#include "UdpDataStore.h"
#include "MainApplication.h"

#include "UdpServiceMediator.h"

#include "sampleentityrepository.h"
#include "SampleEntityService.h"

#include "SampleEntityTableModel.h"
#include "SampleEntityTablePanelDialog.h"
#include "SampleEntityDetailDialog.h"
#include "SampleEntityMapRenderer.h"
#include "SampleEntityController.h"

#include "SqliteComplexEntityRepository.h"
#include "ComplexEntityService.h"
#include "ComplexEntityTableModel.h"
#include "ComplexEntityTablePanelDialog.h"
#include "ComplexEntityDetailDialog.h"
#include "ComplexEntityMapRenderer.h"
#include "ComplexEntityController.h"


#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDebug>


MainBaseUI::MainBaseUI(QWidget *parent, WindowStartupMode startupMode)
    : QMainWindow(parent)
    , m_startupMode(startupMode)
    , m_centralContainer(nullptr)
    , m_stackedWidget(nullptr)
    , m_mainToolBar(nullptr)
    , m_statusLabel(nullptr)
    , m_coordinateLabel(nullptr)
    , m_headerBar(nullptr)
    , m_leftSidebar(nullptr)
    , m_tacticalStatusBar(nullptr)
    , m_mapViewContainer(nullptr)
    , m_mapController(nullptr)
    , m_layerController(nullptr)
    , m_trackRepository(nullptr)
    , m_tacticalTrackService(nullptr)
    , m_trackMapRenderer(nullptr)
    , m_trackController(nullptr)
    , m_trackTableModel(nullptr)
    , m_trackTableDialog(nullptr)
    , m_sampleEntityMapRenderer(nullptr)
    , m_sampleEntityController(nullptr)
    , m_complexEntityRepository(nullptr)
    , m_complexEntityService(nullptr)
    , m_complexEntityTableModel(nullptr)
    , m_complexEntityTableDialog(nullptr)
    , m_complexEntityMapRenderer(nullptr)
    , m_complexEntityController(nullptr)
{
    setupUi();
    setupMenuBar();
    setupToolBars();
    setupStatusBar();
    setupDockWidgets();
    setupConnections();
}

MainBaseUI::~MainBaseUI()
{
}

void MainBaseUI::setupUi()
{
    resize(1280, 800);
    setMinimumSize(1024, 600);
    setWindowTitle(tr("GISLITE Application"));

    // Root central widget container
    m_centralContainer = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(m_centralContainer);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 1. Header Bar (Positioned directly after menu bar)
    m_headerBar = new GISApp::UI::HeaderBar(this);
    rootLayout->addWidget(m_headerBar);

    // Work Area: Horizontal layout with LeftSidebar + Center Content
    QHBoxLayout *workAreaLayout = new QHBoxLayout();
    workAreaLayout->setContentsMargins(0, 0, 0, 0);
    workAreaLayout->setSpacing(0);

    // 2. Left Sidebar Navigation
    m_leftSidebar = new GISApp::UI::LeftSidebar(this);
    workAreaLayout->addWidget(m_leftSidebar);

    // 3. Central Area (Holds StackedWidget with MapViewContainer as default view)
    m_stackedWidget = new QStackedWidget(this);
    m_mapViewContainer = new GISApp::UI::MapViewContainer(this);
    m_stackedWidget->addWidget(m_mapViewContainer);
    workAreaLayout->addWidget(m_stackedWidget, 1);
    rootLayout->addLayout(workAreaLayout, 1);

    // Initialize MapController orchestrating MapWidget and telemetry
    m_mapController = new GISApp::Controllers::MapController(m_mapViewContainer->mapWidget(), this);

    // Initialize LayerTreeModel, SqliteLayerRepository, and LayerController
    auto *layerModel = new GISApp::UIModels::Layers::LayerTreeModel(this);
    auto *layerRepo = new GISApp::Repositories::Sqlite::SqliteLayerRepository();
    m_layerController = new GISApp::Controllers::Layers::LayerController(
        layerModel,
        layerRepo,
        m_mapViewContainer->mapWidget(),
        m_mapViewContainer->layerTreePanel(),
        this
    );
    m_layerController->initialize();

    // Initialize Sample Entity Repository, Service, Table UI Model, Renderer & Controller
    m_sampleEntityRepository = new GISApp::Repositories::SampleEntities::SampleEntityRepository(this);
    m_sampleEntityService = new GISApp::Services::SampleEntities::SampleEntityService(m_sampleEntityRepository, this);
    m_sampleEntityTableModel = new GISApp::UIModels::SampleEntities::SampleEntityTableModel(m_sampleEntityRepository, this);
    m_sampleEntityMapRenderer = new GISApp::UI::Renderers::SampleEntityMapRenderer(m_mapViewContainer->mapWidget(), this);
    m_sampleEntityController = new GISApp::Controllers::SampleEntities::SampleEntityController(
        m_sampleEntityService,
        m_sampleEntityMapRenderer,
        m_mapController,
        this
    );
    m_sampleEntityController->initialize();

    // Initialize Complex Entity Repository, Service, Table UI Model, Renderer & Controller
    m_complexEntityRepository = new GISApp::Repositories::ComplexEntities::SqliteComplexEntityRepository(this);
    m_complexEntityService = new GISApp::Services::ComplexEntities::ComplexEntityService(m_complexEntityRepository, this);
    m_complexEntityTableModel = new GISApp::UIModels::ComplexEntities::ComplexEntityTableModel(m_complexEntityRepository, this);
    m_complexEntityMapRenderer = new GISApp::UI::Renderers::ComplexEntityMapRenderer(m_mapViewContainer->mapWidget(), this);
    m_complexEntityController = new GISApp::Controllers::ComplexEntities::ComplexEntityController(
        m_complexEntityService,
        m_complexEntityMapRenderer,
        m_mapController,
        this
    );
    m_complexEntityController->initialize();
    m_complexEntityController->setMapViewContainer(m_mapViewContainer);

    // Initialize Track Repository, Tactical Track Service, Map Layer Renderer & Track Controller
    m_trackRepository = new GISApp::Repositories::Tracks::TrackRepository(this);
    m_tacticalTrackService = new GISApp::Services::Tracks::TacticalTrackService(m_trackRepository, this);
    m_trackMapRenderer = new GISApp::UI::Renderers::TrackMapRenderer(m_mapViewContainer->mapWidget(), this);
    m_trackController = new GISApp::Controllers::Tracks::TrackController(
        m_tacticalTrackService,
        m_trackMapRenderer,
        m_mapController,
        this
    );
    m_trackController->initialize();

    // Initialize Track Table UI Model
    m_trackTableModel = new GISApp::UIModels::Tracks::TrackTableModel(m_trackRepository, this);

    m_layerController->setMapController(m_mapController);
    m_layerController->setTrackController(m_trackController);
    m_layerController->setSampleEntityController(m_sampleEntityController);
    m_layerController->setComplexEntityController(m_complexEntityController);

    // 4. Tactical Status Bar (Positioned directly above standard bottom status bar)
    m_tacticalStatusBar = new GISApp::UI::TacticalStatusBar(this);
    rootLayout->addWidget(m_tacticalStatusBar);

    setCentralWidget(m_centralContainer);

    // Open in maximized window or full screen based on startup mode flag
    switch (m_startupMode) {
    case WindowStartupMode::FullScreen:
        showFullScreen();
        break;
    case WindowStartupMode::Maximized:
        showMaximized();
        break;
    case WindowStartupMode::Normal:
    default:
        showNormal();
        break;
    }
}

void MainBaseUI::setupMenuBar()
{
    QMenuBar *menu = menuBar();

    // File Menu
    QMenu *fileMenu = menu->addMenu(tr("&File"));
    QAction *exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    // View Menu
    QMenu *viewMenu = menu->addMenu(tr("&View"));
    QAction *fullScreenAction = viewMenu->addAction(tr("&Full Screen"), this, [this](bool checked) {
        if (checked) {
            showFullScreen();
        } else {
            showMaximized();
        }
    });
    fullScreenAction->setCheckable(true);
    fullScreenAction->setChecked(m_startupMode == WindowStartupMode::FullScreen);
    fullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));

    // Table Menu
    QMenu *tableMenu = menu->addMenu(tr("&Table"));
    QMenu *trackSubMenu = tableMenu->addMenu(tr("&Track"));
    QAction *trackTableAction = trackSubMenu->addAction(tr("Open &Track Table..."), this, &MainBaseUI::openTrackTableDialog);
    trackTableAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    trackTableAction->setStatusTip(tr("Open modeless tactical track table view dialog"));

    QMenu *sampleEntitySubMenu = tableMenu->addMenu(tr("&Sample Entity"));
    QAction *sampleEntityTableAction = sampleEntitySubMenu->addAction(tr("Open &Sample Entity Table..."), this, &MainBaseUI::openSampleEntityTableDialog);
    sampleEntityTableAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
    sampleEntityTableAction->setStatusTip(tr("Open modeless sample entity table view dialog"));

    QMenu *complexEntitySubMenu = tableMenu->addMenu(tr("&Complex Entity"));
    QAction *complexEntityTableAction = complexEntitySubMenu->addAction(tr("Open &Complex Entity Table..."), this, &MainBaseUI::openComplexEntityTableDialog);
    complexEntityTableAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    complexEntityTableAction->setStatusTip(tr("Open modeless complex entity table view dialog"));

    // Request Menu
    QMenu *requestMenu = menu->addMenu(tr("&Request"));
    QAction *requestTracksAction = requestMenu->addAction(tr("&Tracks..."), this, &MainBaseUI::openTrackRequestDialog);
    requestTracksAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    requestTracksAction->setStatusTip(tr("Request tactical tracks from external system over UDP (Message ID 1501)"));

    QAction *requestSampleAction = requestMenu->addAction(tr("&Sample Entities..."), this, &MainBaseUI::openSampleEntityRequestDialog);
    requestSampleAction->setStatusTip(tr("Request sample entities from external system over UDP (Message ID 1501, entityType 2)"));

    QAction *requestComplexAction = requestMenu->addAction(tr("&Complex Entities..."), this, &MainBaseUI::openComplexEntityRequestDialog);
    requestComplexAction->setStatusTip(tr("Request complex entities from external system over UDP (Message ID 1501, entityType 3)"));

    // Theme Menu
    QMenu *themeMenu = menu->addMenu(tr("&Theme"));
    QActionGroup *themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);

    const QList<GISApp::UI::ThemeType> themes = {
        GISApp::UI::ThemeType::TacticalDark,
        GISApp::UI::ThemeType::CyberEmerald,
        GISApp::UI::ThemeType::MidnightBlue,
        GISApp::UI::ThemeType::HighContrastDark,
        GISApp::UI::ThemeType::LightOps
    };

    for (GISApp::UI::ThemeType theme : themes) {
        QAction *themeAction = themeMenu->addAction(GISApp::UI::ThemeManager::themeName(theme));
        themeAction->setCheckable(true);
        if (theme == GISApp::UI::ThemeManager::instance().currentTheme()) {
            themeAction->setChecked(true);
        }
        themeGroup->addAction(themeAction);

        connect(themeAction, &QAction::triggered, this, [this, theme]() {
            GISApp::UI::ThemeManager::instance().applyTheme(theme);
            showStatusMessage(tr("Theme set to: %1").arg(GISApp::UI::ThemeManager::themeName(theme)), 3000);
        });
    }

    // Help Menu
    QMenu *helpMenu = menu->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, [this]() {
        QMessageBox::about(this, tr("About GISLITE"),
                           tr("<b>GISLITEAPP</b> v1.0.0<br>GIS Application built with Qt, MapLibre, and GDAL."));
    });
}

void MainBaseUI::setupToolBars()
{

    m_mainToolBar = addToolBar(tr("Main Toolbar"));
    m_mainToolBar->setObjectName("MainToolBar");
    m_mainToolBar->setMovable(false);
}

void MainBaseUI::setupStatusBar()
{
    QStatusBar *bar = statusBar();
    bar->setSizeGripEnabled(false);

    m_statusLabel = new QLabel("@ABC", this);
    m_statusLabel->setObjectName("StatusBarLabel");
    bar->addPermanentWidget(m_statusLabel);
}

void MainBaseUI::setupDockWidgets()
{
    // Override in derived classes or specialized pages to attach dock panels
}

void MainBaseUI::setupConnections()
{
    if (m_headerBar) {
        connect(m_headerBar, &GISApp::UI::HeaderBar::actionTriggered, this, [this](const QString &action) {
            showStatusMessage(tr("Header Action: %1").arg(action), 2000);
        });
    }

    if (m_leftSidebar) {
        connect(m_leftSidebar, &GISApp::UI::LeftSidebar::actionTriggered, this, [this](const QString &action) {
            if (action == "Layers" && m_layerController) {
                m_layerController->togglePanel();
            } else if (action == "Tracks") {
                openTrackTableDialog();
            }
            showStatusMessage(tr("Navigation: %1").arg(action), 1500);
        });
    }

    if (rightToolPanel()) {
        connect(rightToolPanel(), &GISApp::UI::RightToolPanel::toolTriggered, this, [this](const QString &tool) {
            if (tool == "Layers" && m_layerController) {
                m_layerController->togglePanel();
            } else if (tool == "Tracks") {
                openTrackTableDialog();
            } else {
                showStatusMessage(tr("Tool: %1").arg(tool), 2000);
            }
        });
    }

    if (m_mapController) {
        connect(m_mapController, &GISApp::Controllers::MapController::coordinateUpdated,
                this, [this](const GISApp::Core::Models::GeoCoordinate &coord) {
                    if (m_tacticalStatusBar) {
                        m_tacticalStatusBar->updateCoordinates(coord);
                    }
                });

        connect(m_mapController, &GISApp::Controllers::MapController::zoomAndScaleUpdated,
                this, [this](double zoom, double scale) {
                    if (m_tacticalStatusBar) {
                        m_tacticalStatusBar->updateZoomAndScale(zoom, scale);
                    }
                });

        connect(m_mapController, &GISApp::Controllers::MapController::statusNotification,
                this, [this](const QString &msg, int timeout) {
                    showStatusMessage(msg, timeout);
                });

        // Initialize TacticalStatusBar with active map camera state
        m_mapController->syncTelemetry();
    }

    if (m_complexEntityController) {
        connect(m_complexEntityController, &GISApp::Controllers::ComplexEntities::ComplexEntityController::requestComplexEntitiesTriggered,
                this, &MainBaseUI::openComplexEntityRequestDialog);
    }
}

GISApp::UI::RightToolPanel* MainBaseUI::rightToolPanel() const
{
    return m_mapViewContainer ? m_mapViewContainer->rightToolPanel() : nullptr;
}

GISApp::UI::ZoomControlsWidget* MainBaseUI::zoomControls() const
{
    return m_mapViewContainer ? m_mapViewContainer->zoomControls() : nullptr;
}

GISApp::UI::MapWidget* MainBaseUI::mapWidget() const
{
    return m_mapViewContainer ? m_mapViewContainer->mapWidget() : nullptr;
}

int MainBaseUI::addPage(QWidget *page, const QString &title)
{
    Q_UNUSED(title);
    if (!m_stackedWidget || !page) {
        return -1;
    }
    return m_stackedWidget->addWidget(page);
}

void MainBaseUI::setCurrentPage(int index)
{
    if (m_stackedWidget && index >= 0 && index < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(index);
    }
}

void MainBaseUI::setCurrentPage(QWidget *page)
{
    if (m_stackedWidget && page) {
        m_stackedWidget->setCurrentWidget(page);
    }
}

QWidget *MainBaseUI::currentPage() const
{
    return m_stackedWidget ? m_stackedWidget->currentWidget() : nullptr;
}

QStackedWidget *MainBaseUI::stackedWidget() const
{
    return m_stackedWidget;
}

void MainBaseUI::showStatusMessage(const QString &message, int timeout)
{
    if (statusBar()) {
        statusBar()->showMessage(message, timeout);
    }
}

void MainBaseUI::updateCoordinates(double latitude, double longitude, double altitude)
{
    if (m_tacticalStatusBar) {
        m_tacticalStatusBar->updateCoordinates(latitude, longitude, altitude);
    }
}

void MainBaseUI::setStartupMode(WindowStartupMode mode)
{
    m_startupMode = mode;
    switch (m_startupMode) {
    case WindowStartupMode::FullScreen:
        showFullScreen();
        break;
    case WindowStartupMode::Maximized:
        showMaximized();
        break;
    case WindowStartupMode::Normal:
        showNormal();
        break;
    }
}

MainBaseUI::WindowStartupMode MainBaseUI::startupMode() const
{
    return m_startupMode;
}

void MainBaseUI::setFullScreenMode(bool fullscreen)
{
    setStartupMode(fullscreen ? WindowStartupMode::FullScreen : WindowStartupMode::Maximized);
}

void MainBaseUI::setMaximizedMode(bool maximized)
{
    setStartupMode(maximized ? WindowStartupMode::Maximized : WindowStartupMode::Normal);
}

void MainBaseUI::openTrackTableDialog()
{
    if (!m_trackTableDialog) {
        m_trackTableDialog = new GISApp::UI::Tracks::TrackTablePanelDialog(m_trackTableModel, this);
        if (m_trackController) {
            connect(m_trackTableDialog, &GISApp::UI::Tracks::TrackTablePanelDialog::trackSelected,
                    m_trackController, &GISApp::Controllers::Tracks::TrackController::onTrackSelected);
        }
    }

    m_trackTableDialog->show();
    m_trackTableDialog->raise();
    m_trackTableDialog->activateWindow();
    showStatusMessage(tr("Opened Tactical Track Table"), 2000);
}

/**
 * @brief Opens or raises the modeless SampleEntityTablePanelDialog.
 */
void MainBaseUI::openSampleEntityTableDialog()
{
    if (!m_sampleEntityTableDialog) {
        m_sampleEntityTableDialog = new GISApp::UI::SampleEntities::SampleEntityTablePanelDialog(m_sampleEntityTableModel, this);

        connect(m_sampleEntityTableDialog, &GISApp::UI::SampleEntities::SampleEntityTablePanelDialog::requestEntityDetails,
                this, &MainBaseUI::showSampleEntityDetails);

        connect(m_sampleEntityTableDialog, &GISApp::UI::SampleEntities::SampleEntityTablePanelDialog::requestCenterOnEntity,
                this, [this](double lat, double lon) {
                    if (m_mapController) {
                        m_mapController->setCenter(lat, lon);
                    }
                });
    }

    m_sampleEntityTableDialog->show();
    m_sampleEntityTableDialog->raise();
    m_sampleEntityTableDialog->activateWindow();
    showStatusMessage(tr("Opened Sample Entity Table"), 2000);
}


/**
 * @brief Opens the TrackRequestDialog to allow operator to query tracks by time range.
 */
void MainBaseUI::openTrackRequestDialog()
{
    GISApp::UI::Tracks::TrackRequestDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        sendTrackEntityRequest(dialog.fromDateTime(), dialog.toDateTime());
    }
}

/**
 * @brief Constructs and dispatches a canonical binary REQ_ENTITY_MESSAGE over UDP.
 * @param[in] fromDt Query start timestamp.
 * @param[in] toDt Query termination timestamp.
 * @return True if datagram sent successfully.
 * @note Adheres strictly to the packed wire specification in WireStructures.h.
 */
bool MainBaseUI::sendTrackEntityRequest(const QDateTime &fromDt, const QDateTime &toDt)
{
    if (!MainApplication::instance() || !MainApplication::instance()->udpMediator()) {
        qWarning() << "[MainBaseUI] Cannot send track entity request: UdpServiceMediator not available.";
        showStatusMessage(tr("Failed to send track request: UDP service unavailable"), 5000);
        return false;
    }

    REQ_ENTITY_MESSAGE reqMsg{};

    // 1. Message Header (16 bytes packed)
    reqMsg.msg_header.source_id      = CSCI_ID_DSS;
    reqMsg.msg_header.destination_id = CSCI_ID_DFE;
    reqMsg.msg_header.message_id     = REQ_ENTITY_MESSAGE_ID; // 1501
    reqMsg.msg_header.message_len    = static_cast<MESSAGE_LENGTH>(sizeof(REQ_ENTITY_MESSAGE) - sizeof(STRUCT_MESSAGE_HEADER));
    reqMsg.msg_header.packet_seq_no  = 1;
    reqMsg.msg_header.no_of_packets  = 1;

    // 2. User Identification
    QString user = qEnvironmentVariable("USER", "OPERATOR_1");
    qstrncpy(reqMsg.user_detail.username, user.toUtf8().constData(), sizeof(reqMsg.user_detail.username));

    // 3. Time Range Parameters
    auto toWireDateTime = [](const QDateTime &dt, STRUCT_DATE_TIME &wireDt) {
        wireDt.date.day    = static_cast<UINT_8>(dt.date().day());
        wireDt.date.month  = static_cast<UINT_8>(dt.date().month());
        wireDt.date.year   = static_cast<UINT_16>(dt.date().year());
        wireDt.time.hour   = static_cast<UINT_8>(dt.time().hour());
        wireDt.time.minute = static_cast<UINT_8>(dt.time().minute());
        wireDt.time.second = static_cast<UINT_16>(dt.time().second());
    };
    toWireDateTime(fromDt, reqMsg.params.fromDateTime);
    toWireDateTime(toDt, reqMsg.params.toDateTime);

    // 4. Requested Entity Type (1 = Tracks)
    reqMsg.req_entity_type = 1;

    QByteArray datagram(reinterpret_cast<const char*>(&reqMsg), sizeof(REQ_ENTITY_MESSAGE));

    QString targetIp = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetIp();
    quint16 targetPort = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetPort();

    bool sent = MainApplication::instance()->udpMediator()->sendData(targetIp, targetPort, datagram);
    if (sent) {
        qInfo() << "[MainBaseUI] 📤 Sent REQ_ENTITY_MESSAGE (1501) to" << targetIp << ":" << targetPort
                << "for range" << fromDt.toString(Qt::ISODate) << "->" << toDt.toString(Qt::ISODate)
                << "size:" << datagram.size() << "bytes";
        showStatusMessage(tr("Track request (Msg 1501) sent to %1:%2 [%3 to %4]")
                              .arg(targetIp)
                              .arg(targetPort)
                              .arg(fromDt.toString("dd/MM HH:mm:ss"))
                              .arg(toDt.toString("dd/MM HH:mm:ss")), 5000);
    } else {
        qWarning() << "[MainBaseUI] Failed to send REQ_ENTITY_MESSAGE to" << targetIp << ":" << targetPort;
        showStatusMessage(tr("Failed to send track request (Msg 1501) to %1:%2").arg(targetIp).arg(targetPort), 5000);
    }

    return sent;
}

/**
 * @brief Opens or raises the SampleEntityDetailDialog for a specific entity ID.
 * @param[in] entityId Integer entity ID.
 */
void MainBaseUI::showSampleEntityDetails(int entityId)
{
    if (m_sampleEntityController) {
        m_sampleEntityController->showEntityDetails(entityId);
    }
}

/**
 * @brief Opens the date/time dialog to query sample entities over UDP with req_entity_type = 2.
 */
void MainBaseUI::openSampleEntityRequestDialog()
{
    GISApp::UI::Tracks::TrackRequestDialog dialog(this);
    dialog.setWindowTitle(tr("Request Sample Entities (Msg 1501) - GISLITE"));
    if (dialog.exec() == QDialog::Accepted) {
        sendSampleEntityRequest(dialog.fromDateTime(), dialog.toDateTime());
    }
}

/**
 * @brief Constructs and dispatches a canonical binary REQ_ENTITY_MESSAGE (1501) with req_entity_type = 2.
 * @param[in] fromDt Query start timestamp.
 * @param[in] toDt Query termination timestamp.
 * @return True if datagram sent successfully.
 */
bool MainBaseUI::sendSampleEntityRequest(const QDateTime &fromDt, const QDateTime &toDt)
{
    if (!MainApplication::instance() || !MainApplication::instance()->udpMediator()) {
        qWarning() << "[MainBaseUI] Cannot send sample entity request: UdpServiceMediator not available.";
        showStatusMessage(tr("Failed to send sample entity request: UDP service unavailable"), 5000);
        return false;
    }

    REQ_ENTITY_MESSAGE reqMsg{};

    // 1. Message Header (16 bytes packed)
    reqMsg.msg_header.source_id      = CSCI_ID_DSS;
    reqMsg.msg_header.destination_id = CSCI_ID_DFE;
    reqMsg.msg_header.message_id     = REQ_ENTITY_MESSAGE_ID; // 1501
    reqMsg.msg_header.message_len    = static_cast<MESSAGE_LENGTH>(sizeof(REQ_ENTITY_MESSAGE) - sizeof(STRUCT_MESSAGE_HEADER));
    reqMsg.msg_header.packet_seq_no  = 1;
    reqMsg.msg_header.no_of_packets  = 1;

    // 2. User Identification
    QString user = qEnvironmentVariable("USER", "OPERATOR_1");
    qstrncpy(reqMsg.user_detail.username, user.toUtf8().constData(), sizeof(reqMsg.user_detail.username));

    // 3. Time Range Parameters
    auto toWireDateTime = [](const QDateTime &dt, STRUCT_DATE_TIME &wireDt) {
        wireDt.date.day    = static_cast<UINT_8>(dt.date().day());
        wireDt.date.month  = static_cast<UINT_8>(dt.date().month());
        wireDt.date.year   = static_cast<UINT_16>(dt.date().year());
        wireDt.time.hour   = static_cast<UINT_8>(dt.time().hour());
        wireDt.time.minute = static_cast<UINT_8>(dt.time().minute());
        wireDt.time.second = static_cast<UINT_16>(dt.time().second());
    };
    toWireDateTime(fromDt, reqMsg.params.fromDateTime);
    toWireDateTime(toDt, reqMsg.params.toDateTime);

    // 4. Requested Entity Type (2 = Sample Entities)
    reqMsg.req_entity_type = 2;

    QByteArray datagram(reinterpret_cast<const char*>(&reqMsg), sizeof(REQ_ENTITY_MESSAGE));

    QString targetIp = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetIp();
    quint16 targetPort = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetPort();

    bool sent = MainApplication::instance()->udpMediator()->sendData(targetIp, targetPort, datagram);
    if (sent) {
        qInfo() << "[MainBaseUI] 📤 Sent REQ_ENTITY_MESSAGE (1501, entityType: 2) to" << targetIp << ":" << targetPort
                << "for range" << fromDt.toString(Qt::ISODate) << "->" << toDt.toString(Qt::ISODate)
                << "size:" << datagram.size() << "bytes";
        showStatusMessage(tr("Sample Entity request (Msg 1501, Type 2) sent to %1:%2 [%3 to %4]")
                              .arg(targetIp)
                              .arg(targetPort)
                              .arg(fromDt.toString("dd/MM HH:mm:ss"))
                              .arg(toDt.toString("dd/MM HH:mm:ss")), 5000);
    } else {
        qWarning() << "[MainBaseUI] Failed to send REQ_ENTITY_MESSAGE to" << targetIp << ":" << targetPort;
        showStatusMessage(tr("Failed to send sample entity request to %1:%2").arg(targetIp).arg(targetPort), 5000);
    }

    return sent;
}

/**
 * @brief Opens or raises the modeless ComplexEntityTablePanelDialog.
 */
void MainBaseUI::openComplexEntityTableDialog()
{
    if (!m_complexEntityTableDialog) {
        m_complexEntityTableDialog = new GISApp::UI::ComplexEntities::ComplexEntityTablePanelDialog(m_complexEntityTableModel, this);

        connect(m_complexEntityTableDialog, &GISApp::UI::ComplexEntities::ComplexEntityTablePanelDialog::requestEntityDetails,
                this, &MainBaseUI::showComplexEntityDetails);

        connect(m_complexEntityTableDialog, &GISApp::UI::ComplexEntities::ComplexEntityTablePanelDialog::requestCenterOnEntity,
                this, [this](double lat, double lon) {
                    if (m_mapController) {
                        m_mapController->setCenter(lat, lon);
                    }
                });
    }

    m_complexEntityTableDialog->show();
    m_complexEntityTableDialog->raise();
    m_complexEntityTableDialog->activateWindow();
    showStatusMessage(tr("Opened Complex Entity Table"), 2000);
}

/**
 * @brief Opens or raises the ComplexEntityDetailDialog for a specific entity ID.
 * @param[in] entityId Unsigned integer entity ID.
 */
void MainBaseUI::showComplexEntityDetails(quint32 entityId)
{
    if (m_complexEntityController) {
        m_complexEntityController->showEntityDetails(entityId);
    }
}

/**
 * @brief Opens the date/time dialog to query complex entities over UDP with req_entity_type = 3.
 */
void MainBaseUI::openComplexEntityRequestDialog()
{
    GISApp::UI::Tracks::TrackRequestDialog dialog(this);
    dialog.setWindowTitle(tr("Request Complex Entities (Msg 1501, Type 3) - GISLITE"));
    if (dialog.exec() == QDialog::Accepted) {
        sendComplexEntityRequest(dialog.fromDateTime(), dialog.toDateTime());
    }
}

/**
 * @brief Constructs and dispatches a canonical binary REQ_ENTITY_MESSAGE (1501) with req_entity_type = 3.
 * @param[in] fromDt Query start timestamp.
 * @param[in] toDt Query termination timestamp.
 * @return True if datagram sent successfully.
 */
bool MainBaseUI::sendComplexEntityRequest(const QDateTime &fromDt, const QDateTime &toDt)
{
    if (!MainApplication::instance() || !MainApplication::instance()->udpMediator()) {
        qWarning() << "[MainBaseUI] Cannot send complex entity request: UdpServiceMediator not available.";
        showStatusMessage(tr("Failed to send complex entity request: UDP service unavailable"), 5000);
        return false;
    }

    REQ_ENTITY_MESSAGE reqMsg{};

    // 1. Message Header (16 bytes packed)
    reqMsg.msg_header.source_id      = CSCI_ID_DSS;
    reqMsg.msg_header.destination_id = CSCI_ID_DFE;
    reqMsg.msg_header.message_id     = REQ_ENTITY_MESSAGE_ID; // 1501
    reqMsg.msg_header.message_len    = static_cast<MESSAGE_LENGTH>(sizeof(REQ_ENTITY_MESSAGE) - sizeof(STRUCT_MESSAGE_HEADER));
    reqMsg.msg_header.packet_seq_no  = 1;
    reqMsg.msg_header.no_of_packets  = 1;

    // 2. User Identification
    QString user = qEnvironmentVariable("USER", "OPERATOR_1");
    qstrncpy(reqMsg.user_detail.username, user.toUtf8().constData(), sizeof(reqMsg.user_detail.username));

    // 3. Time Range Parameters
    auto toWireDateTime = [](const QDateTime &dt, STRUCT_DATE_TIME &wireDt) {
        wireDt.date.day    = static_cast<UINT_8>(dt.date().day());
        wireDt.date.month  = static_cast<UINT_8>(dt.date().month());
        wireDt.date.year   = static_cast<UINT_16>(dt.date().year());
        wireDt.time.hour   = static_cast<UINT_8>(dt.time().hour());
        wireDt.time.minute = static_cast<UINT_8>(dt.time().minute());
        wireDt.time.second = static_cast<UINT_16>(dt.time().second());
    };
    toWireDateTime(fromDt, reqMsg.params.fromDateTime);
    toWireDateTime(toDt, reqMsg.params.toDateTime);

    // 4. Requested Entity Type (3 = Complex Entities)
    reqMsg.req_entity_type = 3;

    QByteArray datagram(reinterpret_cast<const char*>(&reqMsg), sizeof(REQ_ENTITY_MESSAGE));

    QString targetIp = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetIp();
    quint16 targetPort = GISApp::Communication::Udp::Config::UdpDataStore::instance().targetPort();

    bool sent = MainApplication::instance()->udpMediator()->sendData(targetIp, targetPort, datagram);
    if (sent) {
        qInfo() << "[MainBaseUI] 📤 Sent REQ_ENTITY_MESSAGE (1501, entityType: 3) to" << targetIp << ":" << targetPort
                << "for range" << fromDt.toString(Qt::ISODate) << "->" << toDt.toString(Qt::ISODate)
                << "size:" << datagram.size() << "bytes";
        showStatusMessage(tr("Complex Entity request (Msg 1501, Type 3) sent to %1:%2 [%3 to %4]")
                              .arg(targetIp)
                              .arg(targetPort)
                              .arg(fromDt.toString("dd/MM HH:mm:ss"))
                              .arg(toDt.toString("dd/MM HH:mm:ss")), 5000);
    } else {
        qWarning() << "[MainBaseUI] Failed to send REQ_ENTITY_MESSAGE to" << targetIp << ":" << targetPort;
        showStatusMessage(tr("Failed to send complex entity request to %1:%2").arg(targetIp).arg(targetPort), 5000);
    }

    return sent;
}



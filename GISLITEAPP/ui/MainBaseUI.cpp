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
            }
            showStatusMessage(tr("Navigation: %1").arg(action), 1500);
        });
    }

    if (rightToolPanel()) {
        connect(rightToolPanel(), &GISApp::UI::RightToolPanel::toolTriggered, this, [this](const QString &tool) {
            if (tool == "Layers" && m_layerController) {
                m_layerController->togglePanel();
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


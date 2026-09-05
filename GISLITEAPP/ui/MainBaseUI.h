#ifndef MAINBASEUI_H
#define MAINBASEUI_H

#include <QMainWindow>
#include <QLabel>
#include <QStackedWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QString>

namespace GISApp::UI {
class LeftSidebar;
class RightToolPanel;
class ZoomControlsWidget;
class TacticalStatusBar;
class HeaderBar;
class MapViewContainer;
class MapWidget;
}

namespace GISApp::Controllers {
class MapController;
namespace Layers {
class LayerController;
}
}

class MainBaseUI : public QMainWindow
{
    Q_OBJECT

public:
    enum class WindowStartupMode {
        Normal,
        Maximized,
        FullScreen
    };

    explicit MainBaseUI(QWidget *parent = nullptr, WindowStartupMode startupMode = WindowStartupMode::Maximized);
    virtual ~MainBaseUI();

    // Window mode control
    void setStartupMode(WindowStartupMode mode);
    WindowStartupMode startupMode() const;
    void setFullScreenMode(bool fullscreen);
    void setMaximizedMode(bool maximized);

    // Central page management
    int addPage(QWidget *page, const QString &title = QString());
    void setCurrentPage(int index);
    void setCurrentPage(QWidget *page);
    QWidget *currentPage() const;
    QStackedWidget *stackedWidget() const;

    // Component accessors
    GISApp::UI::LeftSidebar *leftSidebar() const { return m_leftSidebar; }
    GISApp::UI::RightToolPanel *rightToolPanel() const;
    GISApp::UI::ZoomControlsWidget *zoomControls() const;
    GISApp::UI::TacticalStatusBar *tacticalStatusBar() const { return m_tacticalStatusBar; }
    GISApp::UI::HeaderBar *headerBar() const { return m_headerBar; }
    GISApp::UI::MapViewContainer *mapViewContainer() const { return m_mapViewContainer; }
    GISApp::UI::MapWidget *mapWidget() const;
    GISApp::Controllers::MapController *mapController() const { return m_mapController; }
    GISApp::Controllers::Layers::LayerController *layerController() const { return m_layerController; }

    // Status bar utilities
    void showStatusMessage(const QString &message, int timeout = 4000);
    void updateCoordinates(double latitude, double longitude, double altitude = 0.0);

protected:
    // Virtual template hooks for customisation
    virtual void setupUi();
    virtual void setupMenuBar();
    virtual void setupToolBars();
    virtual void setupStatusBar();
    virtual void setupDockWidgets();
    virtual void setupConnections();

    // Core UI components
    WindowStartupMode m_startupMode;
    QWidget *m_centralContainer;
    QStackedWidget *m_stackedWidget;
    QToolBar *m_mainToolBar;
    QLabel *m_statusLabel;
    QLabel *m_coordinateLabel;

    GISApp::UI::HeaderBar *m_headerBar;
    GISApp::UI::LeftSidebar *m_leftSidebar;
    GISApp::UI::TacticalStatusBar *m_tacticalStatusBar;
    GISApp::UI::MapViewContainer *m_mapViewContainer;
    GISApp::Controllers::MapController *m_mapController;
    GISApp::Controllers::Layers::LayerController *m_layerController;
};

#endif // MAINBASEUI_H

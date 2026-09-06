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

namespace GISApp::UI::Renderers {
class TrackMapRenderer;
class SampleEntityMapRenderer;
}

namespace GISApp::UI::Tracks {
class TrackTablePanelDialog;
}

namespace GISApp::UIModels::Tracks {
class TrackTableModel;
}

namespace GISApp::Controllers {
class MapController;
namespace Layers {
class LayerController;
}
}

namespace GISApp::Repositories::Tracks {
class TrackRepository;
}

namespace GISApp::Services::Tracks {
class TacticalTrackService;
}

namespace GISApp::Controllers::Tracks {
class TrackController;
}

namespace GISApp::Controllers::SampleEntities {
class SampleEntityController;
}

namespace GISApp::Repositories::SampleEntities {
class SampleEntityRepository;
}
namespace GISApp::Services::SampleEntities {
class SampleEntityService;
}

namespace GISApp::UIModels::SampleEntities {
class SampleEntityTableModel;
}
namespace GISApp::UI::SampleEntities {
class SampleEntityTablePanelDialog;
class SampleEntityDetailDialog;
}

namespace GISApp::UI::Renderers {
class ComplexEntityMapRenderer;
}
namespace GISApp::Controllers::ComplexEntities {
class ComplexEntityController;
}
namespace GISApp::Repositories::ComplexEntities {
class SqliteComplexEntityRepository;
}
namespace GISApp::Services::ComplexEntities {
class ComplexEntityService;
}
namespace GISApp::UIModels::ComplexEntities {
class ComplexEntityTableModel;
}
namespace GISApp::UI::ComplexEntities {
class ComplexEntityTablePanelDialog;
class ComplexEntityDetailDialog;
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
    GISApp::Repositories::Tracks::TrackRepository *trackRepository() const { return m_trackRepository; }
    GISApp::Services::Tracks::TacticalTrackService *tacticalTrackService() const { return m_tacticalTrackService; }
    GISApp::Repositories::SampleEntities::SampleEntityRepository *sampleEntityRepository() const { return m_sampleEntityRepository; }
    GISApp::Services::SampleEntities::SampleEntityService *sampleEntityService() const { return m_sampleEntityService; }

    GISApp::Repositories::ComplexEntities::SqliteComplexEntityRepository *complexEntityRepository() const { return m_complexEntityRepository; }
    GISApp::Services::ComplexEntities::ComplexEntityService *complexEntityService() const { return m_complexEntityService; }
    GISApp::UIModels::ComplexEntities::ComplexEntityTableModel *complexEntityTableModel() const { return m_complexEntityTableModel; }
    GISApp::UI::ComplexEntities::ComplexEntityTablePanelDialog *complexEntityTableDialog() const { return m_complexEntityTableDialog; }
    GISApp::UI::Renderers::ComplexEntityMapRenderer *complexEntityMapRenderer() const { return m_complexEntityMapRenderer; }
    GISApp::Controllers::ComplexEntities::ComplexEntityController *complexEntityController() const { return m_complexEntityController; }

    GISApp::UIModels::SampleEntities::SampleEntityTableModel *sampleEntityTableModel() const { return m_sampleEntityTableModel; }
    GISApp::UI::SampleEntities::SampleEntityTablePanelDialog *sampleEntityTableDialog() const { return m_sampleEntityTableDialog; }
    GISApp::UI::Renderers::SampleEntityMapRenderer *sampleEntityMapRenderer() const { return m_sampleEntityMapRenderer; }
    GISApp::Controllers::SampleEntities::SampleEntityController *sampleEntityController() const { return m_sampleEntityController; }

    
    GISApp::UI::Renderers::TrackMapRenderer *trackMapRenderer() const { return m_trackMapRenderer; }
    GISApp::Controllers::Tracks::TrackController *trackController() const { return m_trackController; }
    GISApp::UIModels::Tracks::TrackTableModel *trackTableModel() const { return m_trackTableModel; }
    GISApp::UI::Tracks::TrackTablePanelDialog *trackTableDialog() const { return m_trackTableDialog; }

    // Status bar utilities
    void showStatusMessage(const QString &message, int timeout = 4000);
    void updateCoordinates(double latitude, double longitude, double altitude = 0.0);

    /**
     * @brief Constructs and dispatches a binary REQ_ENTITY_MESSAGE (ID 1501) over UDP with entityType 1 (Tracks).
     * @param[in] fromDt Starting date/time range.
     * @param[in] toDt Ending date/time range.
     * @return True if datagram sent successfully.
     */
    bool sendTrackEntityRequest(const QDateTime &fromDt, const QDateTime &toDt);

    /**
     * @brief Constructs and dispatches a binary REQ_ENTITY_MESSAGE (ID 1501) over UDP with entityType 2 (Sample Entities).
     * @param[in] fromDt Starting date/time range.
     * @param[in] toDt Ending date/time range.
     * @return True if datagram sent successfully.
     */
    bool sendSampleEntityRequest(const QDateTime &fromDt, const QDateTime &toDt);

    /**
     * @brief Constructs and dispatches a binary REQ_ENTITY_MESSAGE (ID 1501) over UDP with entityType 3 (Complex Entities).
     * @param[in] fromDt Starting date/time range.
     * @param[in] toDt Ending date/time range.
     * @return True if datagram sent successfully.
     */
    bool sendComplexEntityRequest(const QDateTime &fromDt, const QDateTime &toDt);

public slots:
    void openTrackTableDialog();
    void openSampleEntityTableDialog();
    void openComplexEntityTableDialog();

    /**
     * @brief Opens the TrackRequestDialog to configure and send track queries over UDP.
     */
    void openTrackRequestDialog();

    /**
     * @brief Opens the date/time dialog to query sample entities over UDP with req_entity_type = 2.
     */
    void openSampleEntityRequestDialog();

    /**
     * @brief Opens the date/time dialog to query complex entities over UDP with req_entity_type = 3.
     */
    void openComplexEntityRequestDialog();

    /**
     * @brief Opens or raises the SampleEntityDetailDialog for a specific entity ID.
     * @param[in] entityId Integer entity ID.
     */
    void showSampleEntityDetails(int entityId);

    /**
     * @brief Opens or raises the ComplexEntityDetailDialog for a specific entity ID.
     * @param[in] entityId Unsigned integer entity ID.
     */
    void showComplexEntityDetails(quint32 entityId);

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
    GISApp::Repositories::Tracks::TrackRepository *m_trackRepository{nullptr};
    GISApp::Services::Tracks::TacticalTrackService *m_tacticalTrackService{nullptr};
    GISApp::Repositories::SampleEntities::SampleEntityRepository *m_sampleEntityRepository{nullptr};
    GISApp::Services::SampleEntities::SampleEntityService *m_sampleEntityService{nullptr};
    GISApp::UIModels::SampleEntities::SampleEntityTableModel *m_sampleEntityTableModel{nullptr};
    GISApp::UI::SampleEntities::SampleEntityTablePanelDialog *m_sampleEntityTableDialog{nullptr};
    GISApp::UI::Renderers::SampleEntityMapRenderer *m_sampleEntityMapRenderer{nullptr};
    GISApp::Controllers::SampleEntities::SampleEntityController *m_sampleEntityController{nullptr};

    GISApp::Repositories::ComplexEntities::SqliteComplexEntityRepository *m_complexEntityRepository{nullptr};
    GISApp::Services::ComplexEntities::ComplexEntityService *m_complexEntityService{nullptr};
    GISApp::UIModels::ComplexEntities::ComplexEntityTableModel *m_complexEntityTableModel{nullptr};
    GISApp::UI::ComplexEntities::ComplexEntityTablePanelDialog *m_complexEntityTableDialog{nullptr};
    GISApp::UI::Renderers::ComplexEntityMapRenderer *m_complexEntityMapRenderer{nullptr};
    GISApp::Controllers::ComplexEntities::ComplexEntityController *m_complexEntityController{nullptr};

    
    
    GISApp::UI::Renderers::TrackMapRenderer *m_trackMapRenderer{nullptr};
    GISApp::Controllers::Tracks::TrackController *m_trackController{nullptr};
    GISApp::UIModels::Tracks::TrackTableModel *m_trackTableModel{nullptr};
    GISApp::UI::Tracks::TrackTablePanelDialog *m_trackTableDialog{nullptr};


};

#endif // MAINBASEUI_H

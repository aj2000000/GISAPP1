/**
 * @file TrackController.h
 * @brief Controller orchestrating tactical track business workflows and view coordination.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TRACKCONTROLLER_H
#define TRACKCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPointer>
#include "TacticalTrack.h"
#include "IContextMenuContributor.h"

namespace GISApp::UI::Renderers {
class TrackMapRenderer;
}

namespace GISApp::UI::Tracks {
class TrackDetailDialog;
}

namespace GISApp::Controllers {
class MapController;
}

namespace GISApp::Services::Tracks {
class TacticalTrackService;
}

namespace GISApp::Controllers::Tracks {

/**
 * @class TrackController
 * @brief Orchestrates tactical track domain workflows, map layer rendering, and user interactions.
 *
 * Architectural Role:
 * - Resides in the Controller layer (MVC / Clean Architecture).
 * - Coordinates between the pure domain service (TacticalTrackService) and visual presentation views.
 * - Delegates low-level map canvas graphics and GPU shaders exclusively to TrackMapRenderer.
 * - Coordinates track selection events with MapController camera navigation.
 * - Implements IContextMenuContributor to provide rich right-click actions (Show Details, Edit, Delete)
 *   with spatial hit-testing and multi-entity disambiguation.
 */
class TrackController : public QObject, public GISApp::Core::Interfaces::IContextMenuContributor
{
    Q_OBJECT

public:
    /**
     * @brief Constructs TrackController with its core domain and presentation dependencies.
     * @param[in] trackService Pointer to TacticalTrackService domain service.
     * @param[in] renderer Pointer to TrackMapRenderer handling map layer presentation.
     * @param[in] mapController Optional pointer to MapController for camera panning.
     * @param[in] parent Optional parent QObject for Qt hierarchy management.
     */
    explicit TrackController(GISApp::Services::Tracks::TacticalTrackService *trackService,
                             GISApp::UI::Renderers::TrackMapRenderer *renderer,
                             GISApp::Controllers::MapController *mapController = nullptr,
                             QObject *parent = nullptr);

    /**
     * @brief Virtual destructor releasing controller connections and unregistering context menu.
     */
    virtual ~TrackController() override;

    /**
     * @brief Initializes signal connections and pushes the initial track snapshot to the renderer.
     */
    void initialize();

    /**
     * @brief Toggles rendering visibility of tactical tracks on the map canvas.
     * @param[in] visible True to display tracks, false to hide.
     */
    void setTracksVisible(bool visible);

    /**
     * @brief Checks if tactical tracks are currently set to visible.
     * @return True if visible, false otherwise.
     */
    [[nodiscard]] bool isTracksVisible() const;

    /**
     * @brief Sets or updates the active TrackMapRenderer instance.
     * @param[in] renderer Pointer to TrackMapRenderer.
     */
    void setTrackMapRenderer(GISApp::UI::Renderers::TrackMapRenderer *renderer);

    /**
     * @brief Retrieves the active TrackMapRenderer instance.
     * @return Pointer to current TrackMapRenderer.
     */
    [[nodiscard]] GISApp::UI::Renderers::TrackMapRenderer* trackMapRenderer() const { return m_renderer; }

    /**
     * @brief Associates the MapController for camera fly-to and pan operations.
     * @param[in] mapController Pointer to MapController.
     */
    void setMapController(GISApp::Controllers::MapController *mapController);

    /**
     * @brief Computes geographical centroid and recommended zoom level across all active tracks.
     * @param[out] outLat Calculated mean latitude.
     * @param[out] outLon Calculated mean longitude.
     * @param[out] outZoom Recommended zoom level.
     * @return True if at least one track exists, false otherwise.
     */
    [[nodiscard]] bool calculateTracksCenter(double &outLat, double &outLon, double &outZoom) const;

    /**
     * @brief Declares high priority for tactical entity context menu rendering.
     * @return 100
     */
    [[nodiscard]] int priority() const override { return 100; }

    /**
     * @brief Populates tactical track actions or disambiguation sub-menus for tracks near the click position.
     * @param[in,out] parentMenu Target QMenu to append actions/submenus into.
     * @param[in] screenPos Viewport pixel position of the click.
     * @param[in] geoCoord Geographic coordinate (lat, lon).
     * @return True if one or more tracks were hit and actions contributed; false otherwise.
     */
    bool contributeActions(QMenu *parentMenu, const QPoint &screenPos, const QPointF &geoCoord) override;

signals:
    /**
     * @brief Emitted when track visibility state is toggled.
     * @param[in] visible New visibility state.
     */
    void trackVisibilityChanged(bool visible);

public slots:
    /**
     * @brief Receives updated domain tracks from TacticalTrackService and forwards to renderer.
     * @param[in] tracks Vector of updated TacticalTrack domain entities.
     */
    void onTracksUpdated(const QVector<GISApp::Domain::Tracks::TacticalTrack> &tracks);

    /**
     * @brief Handles track selection requests to center the map camera on the target coordinate.
     * @param[in] latitude Geodetic latitude in degrees.
     * @param[in] longitude Geodetic longitude in degrees.
     */
    void onTrackSelected(double latitude, double longitude);

    /**
     * @brief Opens the detailed tactical inspector dialog for the specified track.
     * @param[in] trackId Target track identifier.
     */
    void showTrackDetails(int trackId);

    /**
     * @brief Centers the map camera onto the specified track's current coordinates.
     * @param[in] trackId Target track identifier.
     */
    void centerOnTrack(int trackId);

    /**
     * @brief Removes a tactical track from the domain service and repository.
     * @param[in] trackId Target track identifier.
     */
    void deleteTrack(int trackId);

    /**
     * @brief Slot invoked when an individual tactical track entity is inserted or updated in the domain service.
     * Forwards real-time telemetry updates to any currently open TrackDetailDialog for this track.
     * @param[in] track Updated tactical track domain model.
     */
    void onTrackUpdated(const GISApp::Domain::Tracks::TacticalTrack &track);

    /**
     * @brief Slot invoked when an individual tactical track entity is removed from the domain service.
     * Closes and cleans up any open TrackDetailDialog associated with this track.
     * @param[in] trackId Numerical identifier of the removed track.
     */
    void onTrackRemoved(int trackId);

private:
    /**
     * @brief Establishes internal Qt signal-slot bindings across domain services.
     */
    void setupConnections();

    /// Domain service supplying track entities and serialized GeoJSON.
    GISApp::Services::Tracks::TacticalTrackService *m_trackService{nullptr};

    /// View-tier renderer translating GeoJSON to MapLibre GPU style layers.
    GISApp::UI::Renderers::TrackMapRenderer *m_renderer{nullptr};

    /// Optional camera controller orchestrating zoom and pan navigation.
    GISApp::Controllers::MapController *m_mapController{nullptr};

    /// Active track detail inspector dialogs keyed by track ID.
    QMap<int, QPointer<GISApp::UI::Tracks::TrackDetailDialog>> m_detailDialogs;
};

} // namespace GISApp::Controllers::Tracks

#endif // TRACKCONTROLLER_H

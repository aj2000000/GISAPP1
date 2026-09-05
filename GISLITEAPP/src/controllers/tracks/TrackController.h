/**
 * @file TrackController.h
 * @brief Controller orchestrating tactical track business workflows and view coordination.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef TRACKCONTROLLER_H
#define TRACKCONTROLLER_H

#include <QObject>
#include <QByteArray>

namespace GISApp::UI::Renderers {
class TrackMapRenderer;
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
 * - Handles layer visibility toggles from LayerController and propagates state to the renderer.
 * - Maintains zero direct dependency on MapLibre or underlying graphics rendering engines.
 */
class TrackController : public QObject
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
     * @brief Virtual destructor releasing controller connections.
     */
    virtual ~TrackController() override = default;

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

signals:
    /**
     * @brief Emitted when track visibility state is toggled.
     * @param[in] visible New visibility state.
     */
    void trackVisibilityChanged(bool visible);

public slots:
    /**
     * @brief Receives serialized GeoJSON feature collections from TacticalTrackService and forwards to renderer.
     * @param[in] geoJsonData GeoJSON FeatureCollection byte array.
     */
    void onGeoJsonUpdated(const QByteArray &geoJsonData);

    /**
     * @brief Handles track selection requests to center the map camera on the target coordinate.
     * @param[in] latitude Geodetic latitude in degrees.
     * @param[in] longitude Geodetic longitude in degrees.
     */
    void onTrackSelected(double latitude, double longitude);

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
};

} // namespace GISApp::Controllers::Tracks

#endif // TRACKCONTROLLER_H

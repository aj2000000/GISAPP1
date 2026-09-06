/**
 * @file SampleEntityController.h
 * @brief Controller orchestrating SampleEntity business workflows and map view interactions.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYCONTROLLER_H
#define SAMPLEENTITYCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPointer>
#include <QPoint>
#include <QPointF>

#include "sampleentity.h"
#include "IContextMenuContributor.h"

namespace GISApp::UI::Renderers {
class SampleEntityMapRenderer;
}

namespace GISApp::UI::SampleEntities {
class SampleEntityDetailDialog;
}

namespace GISApp::Controllers {
class MapController;
}

namespace GISApp::Services::SampleEntities {
class SampleEntityService;
}

namespace GISApp::Controllers::SampleEntities {

/**
 * @class SampleEntityController
 * @brief Orchestrates SampleEntity domain workflows, map layer rendering, and user interactions.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the **Controller layer** (MVC / Clean Architecture).
 * - Coordinates between the pure domain application service (`SampleEntityService`) and presentation views.
 * - Bridges entity updates into GPU-accelerated GeoJSON rendering via `SampleEntityMapRenderer`.
 * - Manages interactive map camera operations via `MapController`.
 * - Implements `IContextMenuContributor` (priority 90) to provide rich right-click actions
 *   (Show Details, Center on Map, Copy Coordinates) with spatial hit-testing supporting both
 *   geodetic points and Bezier curve splines.
 * - Tracks modeless `SampleEntityDetailDialog` singletons and pushes real-time telemetry updates.
 */
class SampleEntityController : public QObject, public GISApp::Core::Interfaces::IContextMenuContributor
{
    Q_OBJECT

public:
    /**
     * @brief Constructs SampleEntityController with its core domain and presentation dependencies.
     * @param[in] entityService Pointer to SampleEntityService domain application service.
     * @param[in] renderer Pointer to SampleEntityMapRenderer presentation renderer.
     * @param[in] mapController Optional pointer to MapController for camera navigation.
     * @param[in] parent Optional parent QObject for Qt hierarchy management.
     */
    explicit SampleEntityController(GISApp::Services::SampleEntities::SampleEntityService *entityService,
                                    GISApp::UI::Renderers::SampleEntityMapRenderer *renderer,
                                    GISApp::Controllers::MapController *mapController = nullptr,
                                    QObject *parent = nullptr);

    /**
     * @brief Virtual destructor releasing controller connections and unregistering context menu.
     */
    virtual ~SampleEntityController() override;

    /**
     * @brief Initializes signal connections and pushes the initial entity snapshot to the renderer.
     */
    void initialize();

    /**
     * @brief Toggles rendering visibility of sample entity layers on the map canvas.
     * @param[in] visible True to display entities, false to hide.
     */
    void setEntitiesVisible(bool visible);

    /**
     * @brief Checks if sample entity layers are currently set to visible.
     * @return True if visible, false otherwise.
     */
    [[nodiscard]] bool isEntitiesVisible() const;

    /**
     * @brief Sets or updates the active SampleEntityMapRenderer instance.
     * @param[in] renderer Pointer to SampleEntityMapRenderer.
     */
    void setSampleEntityMapRenderer(GISApp::UI::Renderers::SampleEntityMapRenderer *renderer);

    /**
     * @brief Retrieves the active SampleEntityMapRenderer instance.
     * @return Pointer to current SampleEntityMapRenderer.
     */
    [[nodiscard]] GISApp::UI::Renderers::SampleEntityMapRenderer* sampleEntityMapRenderer() const { return m_renderer; }

    /**
     * @brief Associates the MapController for camera navigation operations.
     * @param[in] mapController Pointer to MapController.
     */
    void setMapController(GISApp::Controllers::MapController *mapController);

    /**
     * @brief Computes geographical centroid and recommended zoom level across all active sample entities.
     * @param[out] outLat Calculated mean latitude.
     * @param[out] outLon Calculated mean longitude.
     * @param[out] outZoom Recommended zoom level.
     * @return True if at least one entity exists, false otherwise.
     */
    [[nodiscard]] bool calculateEntitiesCenter(double &outLat, double &outLon, double &outZoom) const;

    /**
     * @brief Priority score for context menu rendering order.
     * @return 90
     */
    [[nodiscard]] int priority() const override { return 90; }

    /**
     * @brief Populates sample entity actions or disambiguation sub-menus for entities near the click position.
     * Supports hit-testing for points (type 1/3) and Bezier curves (type 2).
     * @param[in,out] parentMenu Target QMenu to append actions/submenus into.
     * @param[in] screenPos Viewport pixel position of the click.
     * @param[in] geoCoord Geographic coordinate (lat, lon).
     * @return True if one or more entities were hit and actions contributed; false otherwise.
     */
    bool contributeActions(QMenu *parentMenu, const QPoint &screenPos, const QPointF &geoCoord) override;

signals:
    /**
     * @brief Emitted when entity visibility state is toggled.
     * @param[in] visible New visibility state.
     */
    void entityVisibilityChanged(bool visible);

public slots:
    /**
     * @brief Receives updated domain entities from SampleEntityService and forwards to renderer.
     * @param[in] entities Vector of updated SampleEntity domain entities.
     */
    void onEntitiesUpdated(const QVector<GISApp::Domain::SampleEntities::SampleEntity> &entities);

    /**
     * @brief Handles entity selection requests to center the map camera on the target coordinate.
     * @param[in] latitude Geodetic latitude in degrees.
     * @param[in] longitude Geodetic longitude in degrees.
     */
    void onEntitySelected(double latitude, double longitude);

    /**
     * @brief Opens the detailed tactical inspector dialog for the specified entity.
     * @param[in] entityId Target entity identifier.
     */
    void showEntityDetails(int entityId);

    /**
     * @brief Centers the map camera onto the specified entity's current coordinates.
     * @param[in] entityId Target entity identifier.
     */
    void centerOnEntity(int entityId);

    /**
     * @brief Removes a sample entity from the domain service.
     * @param[in] entityId Target entity identifier.
     */
    void deleteEntity(int entityId);

    /**
     * @brief Slot invoked when an individual sample entity is inserted or updated in the domain service.
     * Forwards real-time telemetry updates to any currently open SampleEntityDetailDialog for this entity.
     * @param[in] entity Updated sample entity domain model.
     */
    void onEntityUpdated(const GISApp::Domain::SampleEntities::SampleEntity &entity);

    /**
     * @brief Slot invoked when an individual sample entity is removed from the domain service.
     * Closes and cleans up any open SampleEntityDetailDialog associated with this entity.
     * @param[in] entityId Numerical identifier of the removed entity.
     */
    void onEntityRemoved(int entityId);

private:
    /**
     * @brief Establishes internal Qt signal-slot bindings across domain services.
     */
    void setupConnections();

    /// Domain service supplying sample entities.
    GISApp::Services::SampleEntities::SampleEntityService *m_service{nullptr};

    /// View-tier renderer translating GeoJSON to MapLibre GPU style layers.
    GISApp::UI::Renderers::SampleEntityMapRenderer *m_renderer{nullptr};

    /// Optional camera controller orchestrating zoom and pan navigation.
    GISApp::Controllers::MapController *m_mapController{nullptr};

    /// Active sample entity detail inspector dialogs keyed by entity ID.
    QMap<int, QPointer<GISApp::UI::SampleEntities::SampleEntityDetailDialog>> m_detailDialogs;
};

} // namespace GISApp::Controllers::SampleEntities

#endif // SAMPLEENTITYCONTROLLER_H

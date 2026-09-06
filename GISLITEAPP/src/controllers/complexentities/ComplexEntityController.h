/**
 * @file ComplexEntityController.h
 * @brief Controller orchestrating ComplexEntity business workflows, map rendering, and user interactions.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYCONTROLLER_H
#define COMPLEXENTITYCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPointer>
#include <QPoint>
#include <QPointF>

#include "ComplexEntity.h"
#include "IContextMenuContributor.h"
#include "IMapInteractionListener.h"

namespace GISApp::UI::Renderers {
class ComplexEntityMapRenderer;
}

namespace GISApp::UI::ComplexEntities {
class ComplexEntityDetailDialog;
class ComplexEntityEditDialog;
}

namespace GISApp::Controllers {
class MapController;
}

namespace GISApp::Services::ComplexEntities {
class ComplexEntityService;
}

namespace GISApp::UI {
class MapWidget;
class MapViewContainer;
}

namespace GISApp::Controllers::ComplexEntities {

/**
 * @class ComplexEntityController
 * @brief Orchestrates ComplexEntity domain workflows, map layer rendering, and user interactions.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the **Controller layer** (MVC / Clean Architecture).
 * - Coordinates between the pure domain application service (`ComplexEntityService`) and presentation views.
 * - Bridges entity updates into GPU-accelerated GeoJSON rendering via `ComplexEntityMapRenderer`.
 * - Manages interactive map camera operations via `MapController`.
 * - Implements `IContextMenuContributor` (priority 85) to provide rich right-click actions
 *   (Show Details, Center on Map, Copy Coordinates, Request Complex Entities) with spatial hit-testing
 *   supporting points, polylines, polygons, custom images, text, and painter types.
 * - Tracks modeless `ComplexEntityDetailDialog` singletons and pushes real-time telemetry updates.
 */
class ComplexEntityController : public QObject,
                                public GISApp::Core::Interfaces::IContextMenuContributor,
                                public GISApp::Core::Interfaces::IMapInteractionListener
{
    Q_OBJECT

public:
    /**
     * @brief Constructs ComplexEntityController with its core domain and presentation dependencies.
     * @param[in] entityService Pointer to ComplexEntityService domain application service.
     * @param[in] renderer Pointer to ComplexEntityMapRenderer presentation renderer.
     * @param[in] mapController Optional pointer to MapController for camera navigation.
     * @param[in] parent Optional parent QObject for Qt hierarchy management.
     */
    explicit ComplexEntityController(GISApp::Services::ComplexEntities::ComplexEntityService *entityService,
                                     GISApp::UI::Renderers::ComplexEntityMapRenderer *renderer,
                                     GISApp::Controllers::MapController *mapController = nullptr,
                                     QObject *parent = nullptr);

    /**
     * @brief Virtual destructor releasing controller connections and unregistering context menu.
     */
    virtual ~ComplexEntityController() override;

    /**
     * @brief Initializes signal connections and pushes the initial entity snapshot to the renderer.
     */
    void initialize();

    /**
     * @brief Toggles rendering visibility of complex entity layers on the map canvas.
     * @param[in] visible True to display entities, false to hide.
     */
    void setEntitiesVisible(bool visible);

    /**
     * @brief Checks if complex entity layers are currently set to visible.
     * @return True if visible, false otherwise.
     */
    [[nodiscard]] bool isEntitiesVisible() const;

    /**
     * @brief Sets or updates the active ComplexEntityMapRenderer instance.
     * @param[in] renderer Pointer to ComplexEntityMapRenderer.
     */
    void setComplexEntityMapRenderer(GISApp::UI::Renderers::ComplexEntityMapRenderer *renderer);

    /**
     * @brief Retrieves the active ComplexEntityMapRenderer instance.
     * @return Pointer to current ComplexEntityMapRenderer.
     */
    [[nodiscard]] GISApp::UI::Renderers::ComplexEntityMapRenderer* complexEntityMapRenderer() const { return m_renderer; }

    /**
     * @brief Associates the MapController for camera navigation operations.
     * @param[in] mapController Pointer to MapController.
     */
    void setMapController(GISApp::Controllers::MapController *mapController);

    /**
     * @brief Computes geographical centroid and recommended zoom level across all active complex entities.
     * @param[out] outLat Calculated mean latitude.
     * @param[out] outLon Calculated mean longitude.
     * @param[out] outZoom Recommended zoom level.
     * @return True if at least one entity exists, false otherwise.
     */
    [[nodiscard]] bool calculateEntitiesCenter(double &outLat, double &outLon, double &outZoom) const;

    /**
     * @brief Priority score for context menu rendering order.
     * @return 85
     */
    [[nodiscard]] int priority() const override { return 85; }

    /**
     * @brief Populates complex entity actions or disambiguation sub-menus for entities near the click position.
     * Supports hit-testing for points, polylines, polygons, custom images, text, and painter types.
     * @param[in,out] parentMenu Target QMenu to append actions/submenus into.
     * @param[in] screenPos Viewport pixel position of the click.
     * @param[in] geoCoord Geographic coordinate (lat, lon).
     * @return True if one or more entities were hit and actions contributed; false otherwise.
     */
    bool contributeActions(QMenu *parentMenu, const QPoint &screenPos, const QPointF &geoCoord) override;

    /**
     * @brief Handles mouse press events to initiate interactive dragging of control point vertices.
     * @param[in] screenPos Viewport pixel coordinates of click.
     * @param[in] geoCoord Geodetic coordinates (lat, lon).
     * @param[in] button Mouse button pressed.
     * @return True if a yellow control point handle was hit and drag began; false otherwise.
     */
    bool onMapMousePress(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButton button) override;

    /**
     * @brief Handles mouse move events to update control point coordinates or rigid entity position during dragging.
     * @param[in] screenPos Viewport pixel coordinates.
     * @param[in] geoCoord Geodetic coordinates (lat, lon).
     * @param[in] buttons Mouse buttons held down.
     * @return True if dragging a control point; false otherwise.
     */
    bool onMapMouseMove(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButtons buttons) override;

    /**
     * @brief Handles mouse release events to conclude active control point dragging.
     * @param[in] screenPos Viewport pixel coordinates.
     * @param[in] geoCoord Geodetic coordinates (lat, lon).
     * @param[in] button Mouse button released.
     * @return True if drag was active and completed; false otherwise.
     */
    bool onMapMouseRelease(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButton button) override;

signals:
    /**
     * @brief Emitted when entity visibility state is toggled.
     * @param[in] visible New visibility state.
     */
    void entityVisibilityChanged(bool visible);

    /**
     * @brief Emitted when the operator requests querying complex entities from the external system.
     */
    void requestComplexEntitiesTriggered();

public slots:
    /**
     * @brief Receives updated domain entities from ComplexEntityService and forwards to renderer.
     * @param[in] entities Vector of updated ComplexEntity domain entities.
     */
    void onEntitiesUpdated(const QVector<GISApp::Domain::ComplexEntities::ComplexEntity> &entities);

    /**
     * @brief Handles entity selection requests to center the map camera on the target coordinate.
     * @param[in] latitude Geodetic latitude in degrees.
     * @param[in] longitude Geodetic longitude in degrees.
     */
    void onEntitySelected(double latitude, double longitude);

    /**
     * @brief Opens the detailed tactical inspector dialog for the specified entity.
     * @param[in] entityId Target 32-bit entity identifier.
     */
    void showEntityDetails(quint32 entityId);

    /**
     * @brief Opens the interactive tactical overlay editor dialog for the specified entity.
     * @param[in] entityId Target 32-bit entity identifier.
     */
    void showEditEntityDialog(quint32 entityId);

    /**
     * @brief Associates the MapViewContainer hosting map overlays and ribbons.
     * @param[in] mapViewContainer Pointer to MapViewContainer.
     */
    void setMapViewContainer(GISApp::UI::MapViewContainer *mapViewContainer);

    /**
     * @brief Activates in-place location editing for the specified complex entity on the overlay ribbon.
     * Preserves internal geometry by applying rigid translation vector shifts across all points.
     * @param[in] entityId Target 32-bit entity identifier.
     */
    void startLocationEditing(quint32 entityId);

    /**
     * @brief Activates control point editing mode on the overlay ribbon to modify individual geometry vertices.
     * @param[in] entityId Target 32-bit entity identifier.
     */
    void startControlPointEditing(quint32 entityId);

    /**
     * @brief Slot invoked when the overlay ribbon requests shifting the entity's position.
     * @param[in] entityId Target entity ID.
     * @param[in] deltaLat Latitude delta to shift all points.
     * @param[in] deltaLon Longitude delta to shift all points.
     */
    void onLocationPositionShifted(quint32 entityId, double deltaLat, double deltaLon);

    /**
     * @brief Slot invoked when an individual control vertex is moved.
     * @param[in] entityId Target entity ID.
     * @param[in] pointIndex 0-based vertex index.
     * @param[in] newLat New geodetic latitude.
     * @param[in] newLon New geodetic longitude.
     */
    void onControlPointModified(quint32 entityId, int pointIndex, double newLat, double newLon);

    /**
     * @brief Slot invoked when a new control vertex is added.
     * @param[in] entityId Target entity ID.
     * @param[in] afterIndex Insert after this vertex index.
     * @param[in] lat Geodetic latitude.
     * @param[in] lon Geodetic longitude.
     */
    void onControlPointAdded(quint32 entityId, int afterIndex, double lat, double lon);

    /**
     * @brief Slot invoked when a control vertex is removed.
     * @param[in] entityId Target entity ID.
     * @param[in] pointIndex 0-based vertex index to delete.
     */
    void onControlPointRemoved(quint32 entityId, int pointIndex);

    /**
     * @brief Slot invoked when a bulk operation (e.g. rotation) modifies all control points at once.
     * @param[in] entityId Target entity ID.
     * @param[in] newPoints Complete updated point set after the bulk modification.
     */
    void onAllPointsModified(quint32 entityId, const QVector<STRUCT_LOCATION> &newPoints);

    /**
     * @brief Slot invoked when the operator clicks 'Save' on the location editing ribbon.
     * @param[in] entityId Target entity ID.
     */
    void onLocationEditSaved(quint32 entityId);

    /**
     * @brief Slot invoked when the operator clicks 'Reset' on the location editing ribbon.
     * @param[in] entityId Target entity ID.
     */
    void onLocationEditReset(quint32 entityId);

    /**
     * @brief Slot invoked when the operator clicks 'Cancel' on the location editing ribbon.
     * @param[in] entityId Target entity ID.
     */
    void onLocationEditCancelled(quint32 entityId);

    /**
     * @brief Centers the map camera onto the specified entity's primary coordinates.
     * @param[in] entityId Target 32-bit entity identifier.
     */
    void centerOnEntity(quint32 entityId);

    /**
     * @brief Removes a complex entity from the domain service.
     * @param[in] entityId Target 32-bit entity identifier.
     */
    void deleteEntity(quint32 entityId);

    /**
     * @brief Slot invoked when an individual complex entity is inserted or updated in the domain service.
     * Forwards real-time updates to any currently open ComplexEntityDetailDialog for this entity.
     * @param[in] entity Updated complex entity domain model.
     */
    void onEntityUpdated(const GISApp::Domain::ComplexEntities::ComplexEntity &entity);

    /**
     * @brief Slot invoked when an individual complex entity is removed from the domain service.
     * Closes and cleans up any open ComplexEntityDetailDialog associated with this entity.
     * @param[in] entityId Numerical identifier of the removed entity.
     */
    void onEntityRemoved(quint32 entityId);

private:
    /**
     * @brief Establishes internal Qt signal-slot bindings across domain services.
     */
    void setupConnections();

    /// Domain service supplying complex entities.
    GISApp::Services::ComplexEntities::ComplexEntityService *m_service{nullptr};

    /// View-tier renderer translating GeoJSON to MapLibre GPU style layers.
    GISApp::UI::Renderers::ComplexEntityMapRenderer *m_renderer{nullptr};

    /// Optional camera controller orchestrating zoom and pan navigation.
    GISApp::Controllers::MapController *m_mapController{nullptr};

    /// Optional view container hosting map overlays and ribbons.
    GISApp::UI::MapViewContainer *m_mapViewContainer{nullptr};

    /// Backup snapshots of entities before location editing started.
    QMap<quint32, GISApp::Domain::ComplexEntities::ComplexEntity> m_initialEditEntities;

    /// Active complex entity detail inspector dialogs keyed by entity ID.
    QMap<quint32, QPointer<GISApp::UI::ComplexEntities::ComplexEntityDetailDialog>> m_detailDialogs;

    /// Active complex entity editor overlay dialogs keyed by entity ID.
    QMap<quint32, QPointer<GISApp::UI::ComplexEntities::ComplexEntityEditDialog>> m_editDialogs;

    /**
     * @brief Pushes refreshed entity GeoJSON features to MapLibre, including yellow control point markers if editing is active.
     */
    void refreshEntityRendering();

    /// Active entity identifier being edited on overlay (or 0 if none)
    quint32 m_editingEntityId{0};

    /// Active control point index selected in ribbon
    int m_selectedControlPointIndex{0};

    /// True while dragging a control point vertex on the map canvas
    bool m_isDraggingControlPoint{false};

    /// Index of vertex currently being dragged [0..N-1]
    int m_draggingPointIndex{-1};

    /// Previous geographic coordinate during drag
    QPointF m_dragLastGeo;

    /// True while mouse cursor is currently hovering over a yellow control point handle
    bool m_isHoveringControlPoint{false};
};

} // namespace GISApp::Controllers::ComplexEntities

#endif // COMPLEXENTITYCONTROLLER_H

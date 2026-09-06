/**
 * @file LayerController.h
 * @brief Header definition for LayerController orchestration class.
 */

#ifndef LAYERCONTROLLER_H
#define LAYERCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QString>
#include <QModelIndex>

namespace GISApp::Domain::Layers {
class MapLayer;
}

namespace GISApp::Repositories {
class ILayerRepository;
}

namespace GISApp::UIModels::Layers {
class LayerTreeModel;
}

namespace GISApp::UI {
class MapWidget;
}

namespace GISApp::UI::Layers {
class LayerTreePanel;
}

namespace GISApp::Controllers {
class MapController;
}

namespace GISApp::Controllers::Tracks {
class TrackController;
}

namespace GISApp::Controllers::SampleEntities {
class SampleEntityController;
}

namespace GISApp::Controllers::ComplexEntities {
class ComplexEntityController;
}

namespace GISApp::Controllers::Layers {

/**
 * @class LayerController
 * @brief Orchestrates GIS layer lifecycle, database persistence, reordering, and map synchronization.
 *
 * Architectural Role:
 * - Prefetches saved layers from SQLite database (ILayerRepository) on startup.
 * - Manages the LayerTreeModel and connects user UI actions (Move Up, Move Down, Toggle Visibility)
 *   from the floating LayerTreePanel.
 * - Synchronizes visibility and rendering order changes with the underlying MapLibre engine (MapWidget).
 * - Persists updated z-order sequences and visibility toggles back into SQLite.
 */
class LayerController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs LayerController and associates collaborators.
     * @param[in] model Pointer to LayerTreeModel.
     * @param[in] repository Pointer to ILayerRepository (e.g. SqliteLayerRepository). Takes ownership if specified.
     * @param[in] mapWidget Pointer to target MapWidget.
     * @param[in] panel Pointer to floating LayerTreePanel.
     * @param[in] parent Optional parent QObject.
     */
    explicit LayerController(GISApp::UIModels::Layers::LayerTreeModel *model,
                             GISApp::Repositories::ILayerRepository *repository,
                             GISApp::UI::MapWidget *mapWidget,
                             GISApp::UI::Layers::LayerTreePanel *panel,
                             QObject *parent = nullptr);

    /**
     * @brief Destructor releasing repository if owned.
     */
    virtual ~LayerController() override;

    /**
     * @brief Prefetches layers from SQLite, populates the tree model, and initializes map layers.
     * @return True if initialized successfully, false on error.
     */
    bool initialize();

    /**
     * @brief Toggles the visibility of the floating LayerTreePanel.
     */
    void togglePanel();

    /**
     * @brief Sets the MapController instance for camera manipulation (pan/zoom).
     * @param[in] mapController Pointer to MapController.
     */
    void setMapController(GISApp::Controllers::MapController *mapController);


    /**
     * @brief Sets the TrackController for tactical track visibility delegation.
     * @param[in] trackController Pointer to TrackController.
     */
    void setTrackController(GISApp::Controllers::Tracks::TrackController *trackController);

    /**
     * @brief Sets the SampleEntityController for sample entity visibility delegation.
     * @param[in] sampleEntityController Pointer to SampleEntityController.
     */
    void setSampleEntityController(GISApp::Controllers::SampleEntities::SampleEntityController *sampleEntityController);

    /**
     * @brief Sets the ComplexEntityController for complex entity visibility delegation.
     * @param[in] complexEntityController Pointer to ComplexEntityController.
     */
    void setComplexEntityController(GISApp::Controllers::ComplexEntities::ComplexEntityController *complexEntityController);

public slots:
    /**
     * @brief Promotes the currently selected layer in the tree (Move Up).
     */
    void moveSelectedUp();

    /**
     * @brief Demotes the currently selected layer in the tree (Move Down).
     */
    void moveSelectedDown();

    /**
     * @brief Inverts the visibility state of the currently selected layer.
     */
    void toggleSelectedVisibility();

    /**
     * @brief Prompts or registers a new layer dataset into the tree and database.
     */
    void addNewLayer();

    /**
     * @brief Pans the map to the currently selected layer or group in the layer tree.
     */
    void panToSelectedLayer();

    /**
     * @brief Pans the map to the geographic bounds or center of the given layer or group.
     * @param[in] index Model index of target layer or group node.
     */
    void panToLayer(const QModelIndex &index);

private slots:
    /**
     * @brief Slot called when LayerTreeModel emits a layer visibility change.
     * @param[in] layerId Unique layer identifier.
     * @param[in] visible New visibility boolean.
     */
    void onLayerVisibilityChanged(const QString &layerId, bool visible);

    /**
     * @brief Slot called when LayerTreeModel recalculates layer z-orders after reordering.
     * @param[in] orderMap Map associating layer IDs with updated z-order ranks.
     */
    void onLayerOrderChanged(const QMap<QString, int> &orderMap);

private:
    /**
     * @brief Connects signals across model, panel, and map widget.
     */
    void setupConnections();

    /**
     * @brief Ensures mandatory system fixed layers exist in the persistent store.
     *
     * Defines baseline application layers (e.g. BaseMap, Tactical Tracks) and verifies
     * their presence in the repository. If not present, creates them positioned at the
     * topmost z-order, associated with their designated group.
     */
    void ensureFixedLayersExist();

    /**
     * @brief Maps a logical application layer ID to its corresponding MapLibre GPU layer IDs.
     *
     * A single logical layer (e.g. "tactical_tracks") may expand to multiple MapLibre layers
     * (glow, circle, label). "background" expands to the solid fill plus the raster tile layer.
     *
     * @param[in] logicalId The application-level layer identifier (e.g. "background", "tactical_tracks").
     * @return Ordered list of MapLibre layer IDs that belong to this logical layer.
     */
    [[nodiscard]] QStringList resolveMapLibreLayerIds(const QString &logicalId) const;

    /**
     * @brief Re-stacks MapLibre GPU layers to match the z-order from the Layer Tree Model.
     *
     * Removes and re-adds each MapLibre layer using the `beforeLayerId` parameter to enforce
     * the desired draw order. Layers with higher z-order in the tree render on top.
     *
     * @param[in] orderMap Map of logical layer IDs to z-order integers (higher = on top).
     */
    void restackMapLibreLayers(const QMap<QString, int> &orderMap);

    GISApp::UIModels::Layers::LayerTreeModel *m_treeModel;
    GISApp::Repositories::ILayerRepository *m_repository;
    GISApp::UI::MapWidget *m_mapWidget;
    GISApp::UI::Layers::LayerTreePanel *m_panel;
    GISApp::Controllers::MapController *m_mapController{nullptr};
    GISApp::Controllers::Tracks::TrackController *m_trackController{nullptr};
    GISApp::Controllers::SampleEntities::SampleEntityController *m_sampleEntityController{nullptr};
    GISApp::Controllers::ComplexEntities::ComplexEntityController *m_complexEntityController{nullptr};
};

} // namespace GISApp::Controllers::Layers

#endif // LAYERCONTROLLER_H

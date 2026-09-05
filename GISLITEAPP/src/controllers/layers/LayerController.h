/**
 * @file LayerController.h
 * @brief Header definition for LayerController orchestration class.
 */

#ifndef LAYERCONTROLLER_H
#define LAYERCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QString>

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

    GISApp::UIModels::Layers::LayerTreeModel *m_treeModel;
    GISApp::Repositories::ILayerRepository *m_repository;
    GISApp::UI::MapWidget *m_mapWidget;
    GISApp::UI::Layers::LayerTreePanel *m_panel;
};

} // namespace GISApp::Controllers::Layers

#endif // LAYERCONTROLLER_H

/**
 * @file LayerController.cpp
 * @brief Implementation of LayerController orchestration class.
 */

#include "LayerController.h"
#include "LayerTreeModel.h"
#include "ILayerRepository.h"
#include "ITrackRepository.h"
#include "MapWidget.h"
#include "MapController.h"
#include "TrackController.h"
#include "LayerTreePanel.h"
#include "LayerTreeView.h"
#include "MapLayer.h"
#include "LayerGroup.h"

#include <QMapLibre/Map>
#include <QItemSelectionModel>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QDebug>

namespace GISApp::Controllers::Layers {

LayerController::LayerController(GISApp::UIModels::Layers::LayerTreeModel *model,
                                 GISApp::Repositories::ILayerRepository *repository,
                                 GISApp::UI::MapWidget *mapWidget,
                                 GISApp::UI::Layers::LayerTreePanel *panel,
                                 QObject *parent)
    : QObject(parent)
    , m_treeModel(model)
    , m_repository(repository)
    , m_mapWidget(mapWidget)
    , m_panel(panel)
{
    setupConnections();
}

LayerController::~LayerController()
{
    delete m_repository;
}

bool LayerController::initialize()
{
    if (!m_repository || !m_treeModel) {
        qWarning() << "[LayerController] Cannot initialize without valid repository and tree model.";
        return false;
    }

    // Ensure mandatory application fixed layers exist in the persistent store
    ensureFixedLayersExist();

    // Prefetch all layers from SQLite (sorted by z-order ascending)
    QVector<GISApp::Domain::Layers::MapLayer*> layers = m_repository->getAllLayers();
    qInfo() << "[LayerController] Prefetched" << layers.size() << "layers from persistent storage.";

    m_treeModel->loadLayers(layers);

    if (m_panel) {
        m_panel->setModel(m_treeModel);
        m_panel->updateLayerCount(layers.size());
    }

    // Synchronize initial visibility states with MapWidget
    if (m_mapWidget && m_mapWidget->rawMap()) {
        for (const auto *layer : layers) {
            if (layer) {
                m_mapWidget->rawMap()->setLayoutProperty(
                    layer->id(), "visibility", layer->isVisible() ? "visible" : "none");
            }
        }
    }

    return true;
}

void LayerController::togglePanel()
{
    if (!m_panel) return;

    if (m_panel->isVisible()) {
        m_panel->hide();
    } else {
        m_panel->show();
        m_panel->raise();
    }
}

void LayerController::setMapController(GISApp::Controllers::MapController *mapController)
{
    m_mapController = mapController;
}

void LayerController::setTrackRepository(GISApp::Repositories::ITrackRepository *trackRepo)
{
    m_trackRepository = trackRepo;
}

void LayerController::setTrackController(GISApp::Controllers::Tracks::TrackController *trackController)
{
    m_trackController = trackController;
    if (m_trackController && m_repository) {
        auto *layer = m_repository->getLayerById("tactical_tracks");
        if (layer) {
            m_trackController->setTracksVisible(layer->isVisible());
            delete layer;
        }
    }
}

void LayerController::setupConnections()
{
    if (m_panel) {
        connect(m_panel, &GISApp::UI::Layers::LayerTreePanel::panToRequested,
                this, &LayerController::panToLayer);
        connect(m_panel, &GISApp::UI::Layers::LayerTreePanel::moveUpRequested,
                this, &LayerController::moveSelectedUp);
        connect(m_panel, &GISApp::UI::Layers::LayerTreePanel::moveDownRequested,
                this, &LayerController::moveSelectedDown);
        connect(m_panel, &GISApp::UI::Layers::LayerTreePanel::toggleVisibilityRequested,
                this, &LayerController::toggleSelectedVisibility);
        connect(m_panel, &GISApp::UI::Layers::LayerTreePanel::addLayerRequested,
                this, &LayerController::addNewLayer);
    }

    if (m_treeModel) {
        connect(m_treeModel, &GISApp::UIModels::Layers::LayerTreeModel::layerVisibilityChanged,
                this, &LayerController::onLayerVisibilityChanged);
        connect(m_treeModel, &GISApp::UIModels::Layers::LayerTreeModel::layerOrderChanged,
                this, &LayerController::onLayerOrderChanged);
    }

    if (m_mapWidget) {
        connect(m_mapWidget, &GISApp::UI::MapWidget::mapReady, this, [this]() {
            if (!m_mapWidget || !m_mapWidget->rawMap() || !m_repository) return;
            auto layers = m_repository->getAllLayers();
            for (const auto *layer : layers) {
                if (!layer) continue;
                const QString vis = layer->isVisible() ? QStringLiteral("visible") : QStringLiteral("none");
                if (m_mapWidget->rawMap()->layerExists(layer->id())) {
                    m_mapWidget->rawMap()->setLayoutProperty(layer->id(), "visibility", vis);
                }
                if (layer->id() == "background") {
                    if (m_mapWidget->rawMap()->layerExists("tactical_basemap_layer")) {
                        m_mapWidget->rawMap()->setLayoutProperty("tactical_basemap_layer", "visibility", vis);
                    }
                }
                if (layer->id() == "worldmap_layer" && m_mapWidget->rawMap()->layerExists("worldmap_layer")) {
                    m_mapWidget->rawMap()->setLayoutProperty("worldmap_layer", "visibility", vis);
                }
                delete layer;
            }
        });
    }
}

void LayerController::moveSelectedUp()
{
    if (!m_panel || !m_treeModel) return;

    QModelIndex current = m_panel->selectedIndex();
    if (!current.isValid() || current.row() == 0) return;

    int newRow = current.row() - 1;
    QModelIndex parentIndex = current.parent();

    if (m_treeModel->moveLayerUp(current)) {
        // Restore selection to the moved row
        if (m_panel->treeView() && m_panel->treeView()->selectionModel()) {
            QModelIndex nextIndex = m_treeModel->index(newRow, 0, parentIndex);
            m_panel->treeView()->selectionModel()->setCurrentIndex(
                nextIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    }
}

void LayerController::moveSelectedDown()
{
    if (!m_panel || !m_treeModel) return;

    QModelIndex current = m_panel->selectedIndex();
    if (!current.isValid()) return;

    int newRow = current.row() + 1;
    QModelIndex parentIndex = current.parent();

    if (m_treeModel->moveLayerDown(current)) {
        // Restore selection to the moved row
        if (m_panel->treeView() && m_panel->treeView()->selectionModel()) {
            QModelIndex nextIndex = m_treeModel->index(newRow, 0, parentIndex);
            m_panel->treeView()->selectionModel()->setCurrentIndex(
                nextIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    }
}

void LayerController::toggleSelectedVisibility()
{
    if (!m_panel || !m_treeModel) return;

    QModelIndex current = m_panel->selectedIndex();
    if (!current.isValid()) return;

    int currentCheck = m_treeModel->data(current, Qt::CheckStateRole).toInt();
    int newCheck = (currentCheck == Qt::Checked) ? Qt::Unchecked : Qt::Checked;

    m_treeModel->setData(current, newCheck, Qt::CheckStateRole);
}

void LayerController::panToSelectedLayer()
{
    if (!m_panel) return;
    panToLayer(m_panel->selectedIndex());
}

void LayerController::panToLayer(const QModelIndex &index)
{
    if (!index.isValid()) return;
    if (!m_treeModel || !m_mapWidget) return;

    auto *node = m_treeModel->nodeFromIndex(index);
    if (!node) return;

    double targetLat = 12.9716;
    double targetLon = 77.5946;
    double targetZoom = 8.0;

    if (node->isGroup()) {
        auto *group = static_cast<GISApp::Domain::Layers::LayerGroup*>(node);
        qInfo() << "[LayerController] Pan to Group requested:" << group->name();

        QVector<GISApp::Domain::Layers::MapLayer*> groupLayers;
        m_treeModel->collectLayersRecursive(group, groupLayers);

        bool foundCoordinates = false;
        for (const auto *l : groupLayers) {
            if (l && l->id() == "tactical_tracks" && m_trackRepository) {
                auto tracks = m_trackRepository->getAllTracks();
                if (!tracks.isEmpty()) {
                    double sumLat = 0.0;
                    double sumLon = 0.0;
                    for (const auto &t : tracks) {
                        sumLat += t.latitude();
                        sumLon += t.longitude();
                    }
                    targetLat = sumLat / tracks.size();
                    targetLon = sumLon / tracks.size();
                    targetZoom = 9.5;
                    foundCoordinates = true;
                    break;
                }
            }
        }

        if (!foundCoordinates) {
            targetLat = 12.9716;
            targetLon = 77.5946;
            targetZoom = 6.0;
        }
    } else {
        auto *layer = static_cast<GISApp::Domain::Layers::MapLayer*>(node);
        qInfo() << "[LayerController] Pan to Layer requested:" << layer->name() << "(" << layer->id() << ")";

        if (layer->id() == "tactical_tracks") {
            if (m_trackRepository) {
                auto tracks = m_trackRepository->getAllTracks();
                if (!tracks.isEmpty()) {
                    double sumLat = 0.0;
                    double sumLon = 0.0;
                    for (const auto &t : tracks) {
                        sumLat += t.latitude();
                        sumLon += t.longitude();
                    }
                    targetLat = sumLat / tracks.size();
                    targetLon = sumLon / tracks.size();
                    targetZoom = 10.0;
                } else {
                    targetLat = 12.9716;
                    targetLon = 77.5946;
                    targetZoom = 7.0;
                }
            }
        } else if (layer->id() == "background") {
            targetLat = 12.9716;
            targetLon = 77.5946;
            targetZoom = 5.0;
        } else {
            // Check if layer has custom centroid in configJson
            if (!layer->configJson().isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(layer->configJson().toUtf8());
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains("lat") && obj.contains("lon")) {
                        targetLat = obj["lat"].toDouble();
                        targetLon = obj["lon"].toDouble();
                        if (obj.contains("zoom")) targetZoom = obj["zoom"].toDouble();
                    }
                }
            }
        }
    }

    if (m_mapController) {
        m_mapController->setCenter(targetLat, targetLon);
        m_mapController->setZoom(targetZoom);
    } else if (m_mapWidget && m_mapWidget->rawMap()) {
        m_mapWidget->rawMap()->setCoordinate(QMapLibre::Coordinate(targetLat, targetLon));
        m_mapWidget->rawMap()->setZoom(targetZoom);
    }
}

void LayerController::onLayerVisibilityChanged(const QString &layerId, bool visible)
{
    qInfo() << "[LayerController] Layer visibility changed:" << layerId << "->" << visible;

    if (m_repository) {
        m_repository->updateVisibility(layerId, visible);
    }

    if (layerId == "tactical_tracks" && m_trackController) {
        m_trackController->setTracksVisible(visible);
    }

    if (m_mapWidget && m_mapWidget->rawMap()) {
        const QString vis = visible ? QStringLiteral("visible") : QStringLiteral("none");
        if (m_mapWidget->rawMap()->layerExists(layerId)) {
            m_mapWidget->rawMap()->setLayoutProperty(layerId, "visibility", vis);
        }
        if (layerId == "background") {
            if (m_mapWidget->rawMap()->layerExists("tactical_basemap_layer")) {
                m_mapWidget->rawMap()->setLayoutProperty("tactical_basemap_layer", "visibility", vis);
            }
        }
        if (layerId == "worldmap_layer") {
            if (m_mapWidget->rawMap()->layerExists("worldmap_layer")) {
                m_mapWidget->rawMap()->setLayoutProperty("worldmap_layer", "visibility", vis);
            }
        }
        if (layerId == "tactical_tracks" && !m_trackController) {
            if (m_mapWidget->rawMap()->layerExists("tactical_tracks_glow")) {
                m_mapWidget->rawMap()->setLayoutProperty("tactical_tracks_glow", "visibility", vis);
            }
            if (m_mapWidget->rawMap()->layerExists("tactical_tracks_circle")) {
                m_mapWidget->rawMap()->setLayoutProperty("tactical_tracks_circle", "visibility", vis);
            }
            if (m_mapWidget->rawMap()->layerExists("tactical_tracks_label")) {
                m_mapWidget->rawMap()->setLayoutProperty("tactical_tracks_label", "visibility", vis);
            }
        }
    }
}

void LayerController::onLayerOrderChanged(const QMap<QString, int> &orderMap)
{
    qInfo() << "[LayerController] Layer z-orders reordered. Updating database...";

    if (m_repository) {
        m_repository->updateZOrders(orderMap);
    }

    restackMapLibreLayers(orderMap);
}

/**
 * @brief Maps a logical application layer ID to its corresponding MapLibre GPU layer IDs.
 *
 * Each logical layer may correspond to one or more actual MapLibre layers. The returned
 * list is in bottom-to-top draw order within that logical layer's sub-stack.
 *
 * @param[in] logicalId The application-level layer identifier.
 * @return Ordered list of MapLibre layer IDs belonging to this logical layer.
 */
QStringList LayerController::resolveMapLibreLayerIds(const QString &logicalId) const
{
    if (logicalId == QStringLiteral("background")) {
        return {QStringLiteral("background"), QStringLiteral("tactical_basemap_layer")};
    }
    if (logicalId == QStringLiteral("worldmap_layer")) {
        return {QStringLiteral("worldmap_layer")};
    }
    if (logicalId == QStringLiteral("tactical_tracks")) {
        return {QStringLiteral("tactical_tracks_glow"),
                QStringLiteral("tactical_tracks_circle"),
                QStringLiteral("tactical_tracks_label")};
    }
    // For future / custom layers, the MapLibre layer ID matches the logical ID
    return {logicalId};
}

/**
 * @brief Re-stacks MapLibre GPU layers to match the z-order from the Layer Tree Model.
 *
 * Algorithm:
 * 1. Sort logical layers by z-order ascending (lowest z = bottom of render stack).
 * 2. Expand each logical layer into its MapLibre sub-layer IDs (bottom-to-top within each group).
 * 3. Build the desired flat draw order.
 * 4. Compare against the current MapLibre layer stack; bail out if already correct.
 * 5. Remove all managed layers (except "background" which is the immovable root fill).
 * 6. Re-add each layer bottom-to-top using addLayer(id, params, before) so MapLibre
 *    places them in the correct rendering sequence.
 *
 * Runtime layers (tactical_tracks_glow/circle/label) are owned by BaseMapFeatureRenderer.
 * Removing them triggers BaseMapFeatureRenderer::onMapChanged() which re-injects them
 * automatically. We leverage this by removing them last and letting the renderer
 * re-add them at the top of the stack (or before a specified layer).
 *
 * @param[in] orderMap Map of logical layer IDs to z-order integers (higher = rendered on top).
 */
void LayerController::restackMapLibreLayers(const QMap<QString, int> &orderMap)
{
    if (!m_mapWidget || !m_mapWidget->rawMap()) {
        return;
    }

    QMapLibre::Map *map = m_mapWidget->rawMap();

    // 1. Sort logical layers by z-order ascending (bottom first)
    QVector<QPair<int, QString>> sorted;
    sorted.reserve(orderMap.size());
    for (auto it = orderMap.constBegin(); it != orderMap.constEnd(); ++it) {
        sorted.append({it.value(), it.key()});
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });

    // 2. Build desired flat MapLibre layer stack (bottom → top)
    QStringList desiredStack;
    for (const auto &pair : sorted) {
        const QStringList subLayers = resolveMapLibreLayerIds(pair.second);
        for (const QString &mlId : subLayers) {
            desiredStack.append(mlId);
        }
    }

    if (desiredStack.isEmpty()) {
        return;
    }

    // 3. Filter to only layers that currently exist in MapLibre
    QStringList desiredExisting;
    for (const QString &id : desiredStack) {
        if (map->layerExists(id)) {
            desiredExisting.append(id);
        }
    }

    // 4. Get current MapLibre stack and extract the managed subset in current order
    const QVector<QString> currentStack = map->layerIds();
    QStringList currentManaged;
    QSet<QString> desiredSet(desiredExisting.begin(), desiredExisting.end());
    for (const QString &id : currentStack) {
        if (desiredSet.contains(id)) {
            currentManaged.append(id);
        }
    }

    if (currentManaged == desiredExisting) {
        qInfo() << "[LayerController] MapLibre layer stack already matches desired z-order.";
        return;
    }

    qInfo() << "[LayerController] Re-stacking MapLibre layers..."
            << "Current:" << currentManaged
            << "Desired:" << desiredExisting;

    // 5. Identify which layers we can safely remove and re-add.
    //    "background" is a special style-root background layer — MapLibre typically
    //    keeps it at position 0 and it cannot be meaningfully removed/re-added.
    //    We skip it and treat it as an immovable anchor at the bottom.

    QStringList layersToRemove;
    for (const QString &id : desiredExisting) {
        if (id != QStringLiteral("background")) {
            layersToRemove.append(id);
        }
    }

    // 6. Remove all managed layers (reverse order for safety)
    for (int i = layersToRemove.size() - 1; i >= 0; --i) {
        if (map->layerExists(layersToRemove[i])) {
            map->removeLayer(layersToRemove[i]);
        }
    }

    // 7. Re-add layers in desired order (bottom to top).
    //    addLayer(id, params, before="") adds at the TOP of the stack.
    //    So we iterate bottom→top; each successive addLayer goes on top of the previous.
    for (const QString &id : desiredExisting) {
        if (id == QStringLiteral("background")) {
            continue; // Immovable anchor; already at bottom
        }

        if (map->layerExists(id)) {
            continue; // Already present (shouldn't happen after removal, but safety check)
        }

        // Determine layer type and source for reconstruction
        QVariantMap params;
        params[QStringLiteral("id")] = id;

        if (id == QStringLiteral("tactical_basemap_layer")) {
            params[QStringLiteral("type")] = QStringLiteral("raster");
            params[QStringLiteral("source")] = QStringLiteral("tactical_basemap");
        } else if (id == QStringLiteral("worldmap_layer")) {
            params[QStringLiteral("type")] = QStringLiteral("raster");
            params[QStringLiteral("source")] = QStringLiteral("worldmap");
        } else {
            // Runtime layers (tactical_tracks_*) are managed by BaseMapFeatureRenderer.
            // They were removed above; BaseMapFeatureRenderer::onMapChanged() will detect
            // they're missing and automatically re-inject them at the top of the stack.
            // We skip explicit re-addition here.
            continue;
        }

        // Preserve visibility from the database
        QVariantMap layout;
        bool visible = true;
        if (m_repository) {
            // For sub-layers of "background" (tactical_basemap_layer), inherit parent visibility
            QString lookupId = id;
            if (id == QStringLiteral("tactical_basemap_layer")) {
                lookupId = QStringLiteral("background");
            }
            auto *dbLayer = m_repository->getLayerById(lookupId);
            if (dbLayer) {
                visible = dbLayer->isVisible();
                delete dbLayer;
            }
        }
        layout[QStringLiteral("visibility")] = visible ? QStringLiteral("visible") : QStringLiteral("none");
        params[QStringLiteral("layout")] = layout;

        // Add at the top of the current stack (before = "" means top)
        map->addLayer(id, params);
        qDebug() << "[LayerController] Re-added MapLibre layer:" << id;
    }

    qInfo() << "[LayerController] MapLibre layer stack re-ordered. Final:"
            << map->layerIds();
}

void LayerController::addNewLayer()
{
    static int s_layerIndex = 1;
    QString layerId = QString("layer_custom_%1").arg(s_layerIndex++);
    QString layerName = QString("Custom Tactical Overlay %1").arg(s_layerIndex - 1);

    auto *newLayer = new GISApp::Domain::Layers::MapLayer(
        layerId, layerName, GISApp::Domain::Layers::LayerType::Vector, "internal://custom");
    newLayer->setGroupName("Operational");
    newLayer->setZOrder(10 + s_layerIndex);
    newLayer->setVisible(true);

    if (m_repository) {
        m_repository->saveLayer(newLayer);
    }

    // Reload layers into model
    QVector<GISApp::Domain::Layers::MapLayer*> layers = m_repository->getAllLayers();
    m_treeModel->loadLayers(layers);

    if (m_panel) {
        m_panel->updateLayerCount(layers.size());
    }
}

/**
 * @brief Ensures mandatory system fixed layers exist in persistent storage.
 *
 * Defines the application's baseline required layers:
 * 1. BaseMap ("background"): Tactical Dark Base tile layer under "BaseMap" group.
 * 2. WorldMap ("worldmap_layer"): Local tile server raster base map under "BaseMap" group.
 * 3. Tactical Tracks ("tactical_tracks"): Live tactical telemetry overlay under "Tactical" group.
 *
 * Delegates to ILayerRepository::ensureFixedLayers() which checks if each layer ID exists:
 * - If present, leaves it completely untouched (retains user visibility, opacity, and custom z-order).
 * - If missing, dynamically computes the topmost z-order (MAX(z_order) + 1) and creates it.
 */
void LayerController::ensureFixedLayersExist()
{
    if (!m_repository) return;

    // 1. Mandatory BaseMap layer (CartoDB dark tiles)
    GISApp::Domain::Layers::MapLayer baseMapLayer(
        "background",
        "Tactical Dark Base",
        GISApp::Domain::Layers::LayerType::Tile,
        "local://resources/map/styles/tactical_dark_fallback.json"
    );
    baseMapLayer.setGroupName("BaseMap");
    baseMapLayer.setVisible(true);
    baseMapLayer.setOpacity(1.0);

    // 2. WorldMap base layer (local tile server)
    GISApp::Domain::Layers::MapLayer worldMapLayer(
        "worldmap_layer",
        "WorldMap",
        GISApp::Domain::Layers::LayerType::Tile,
        "http://localhost:8000/tiles/{z}/{x}/{y}.png"
    );
    worldMapLayer.setGroupName("BaseMap");
    worldMapLayer.setVisible(false);
    worldMapLayer.setOpacity(1.0);

    // 3. Mandatory Tactical Tracks layer
    GISApp::Domain::Layers::MapLayer tacticalTracksLayer(
        "tactical_tracks",
        "Tactical Track Layer",
        GISApp::Domain::Layers::LayerType::TacticalTrack,
        ""
    );
    tacticalTracksLayer.setGroupName("Tactical");
    tacticalTracksLayer.setVisible(true);
    tacticalTracksLayer.setOpacity(1.0);

    QVector<GISApp::Domain::Layers::MapLayer> fixedLayers = {
        baseMapLayer,
        worldMapLayer,
        tacticalTracksLayer
    };

    m_repository->ensureFixedLayers(fixedLayers);
}

} // namespace GISApp::Controllers::Layers

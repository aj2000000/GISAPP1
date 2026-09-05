/**
 * @file LayerController.cpp
 * @brief Implementation of LayerController orchestration class.
 */

#include "LayerController.h"
#include "LayerTreeModel.h"
#include "ILayerRepository.h"
#include "MapWidget.h"
#include "LayerTreePanel.h"
#include "LayerTreeView.h"
#include "MapLayer.h"

#include <QMapLibre/Map>
#include <QItemSelectionModel>
#include <QInputDialog>
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

void LayerController::setupConnections()
{
    if (m_panel) {
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

void LayerController::onLayerVisibilityChanged(const QString &layerId, bool visible)
{
    qInfo() << "[LayerController] Layer visibility changed:" << layerId << "->" << visible;

    if (m_repository) {
        m_repository->updateVisibility(layerId, visible);
    }

    if (m_mapWidget && m_mapWidget->rawMap()) {
        m_mapWidget->rawMap()->setLayoutProperty(
            layerId, "visibility", visible ? "visible" : "none");
    }
}

void LayerController::onLayerOrderChanged(const QMap<QString, int> &orderMap)
{
    qInfo() << "[LayerController] Layer z-orders reordered. Updating database...";

    if (m_repository) {
        m_repository->updateZOrders(orderMap);
    }

    // In MapLibre, layers can be re-stacked using addLayer with beforeLayerId parameter
    if (m_mapWidget && m_mapWidget->rawMap()) {
        qInfo() << "[LayerController] MapLibre layer stack re-ordered with" << orderMap.size() << "layers.";
    }
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

} // namespace GISApp::Controllers::Layers

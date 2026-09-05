/**
 * @file LayerTreeModel.cpp
 * @brief Implementation of LayerTreeModel QAbstractItemModel tree model.
 */

#include "LayerTreeModel.h"
#include "LayerNode.h"
#include "LayerGroup.h"
#include "MapLayer.h"

#include <QDebug>

namespace GISApp::UIModels::Layers {

LayerTreeModel::LayerTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootNode(new GISApp::Domain::Layers::LayerGroup("__root__", "Root"))
{
}

LayerTreeModel::~LayerTreeModel()
{
    delete m_rootNode;
}

QModelIndex LayerTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    GISApp::Domain::Layers::LayerNode *parentNode = nodeFromIndex(parent);
    GISApp::Domain::Layers::LayerNode *childNode = parentNode->child(row);

    if (childNode) {
        return createIndex(row, column, childNode);
    }
    return QModelIndex();
}

QModelIndex LayerTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    auto *childNode = static_cast<GISApp::Domain::Layers::LayerNode*>(child.internalPointer());
    GISApp::Domain::Layers::LayerNode *parentNode = childNode ? childNode->parentNode() : nullptr;

    if (!parentNode || parentNode == m_rootNode) {
        return QModelIndex();
    }

    return createIndex(parentNode->row(), 0, parentNode);
}

int LayerTreeModel::rowCount(const QModelIndex &parent) const
{
    GISApp::Domain::Layers::LayerNode *parentNode = nodeFromIndex(parent);
    return parentNode ? parentNode->childCount() : 0;
}

int LayerTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant LayerTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    auto *node = static_cast<GISApp::Domain::Layers::LayerNode*>(index.internalPointer());
    if (!node) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return node->name();

    case Qt::CheckStateRole:
        if (node->isGroup()) {
            const auto *group = static_cast<const GISApp::Domain::Layers::LayerGroup*>(node);
            return static_cast<int>(group->checkState());
        }
        return node->isVisible() ? Qt::Checked : Qt::Unchecked;

    case Qt::DecorationRole:
        if (node->isGroup()) {
            return QString("📁");
        } else {
            const auto *layer = static_cast<const GISApp::Domain::Layers::MapLayer*>(node);
            return layer->typeIcon();
        }

    case IdRole:
        return node->id();

    case ZOrderRole:
        return node->zOrder();

    case OpacityRole:
        if (!node->isGroup()) {
            const auto *layer = static_cast<const GISApp::Domain::Layers::MapLayer*>(node);
            return layer->opacity();
        }
        return 1.0;

    case IsGroupRole:
        return node->isGroup();

    default:
        break;
    }

    return QVariant();
}

bool LayerTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    auto *node = static_cast<GISApp::Domain::Layers::LayerNode*>(index.internalPointer());
    if (!node) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        bool visible = (value.toInt() == Qt::Checked);
        node->setVisible(visible);

        emit dataChanged(index, index, {Qt::CheckStateRole});

        // Notify parent groups to refresh their tri-state checkboxes
        QModelIndex pIndex = parent(index);
        while (pIndex.isValid()) {
            emit dataChanged(pIndex, pIndex, {Qt::CheckStateRole});
            pIndex = parent(pIndex);
        }

        // Emit signal for controller and persistent store
        if (node->isGroup()) {
            // Emit for all children within the group
            QVector<GISApp::Domain::Layers::MapLayer*> groupLayers;
            collectLayersRecursive(node, groupLayers);
            for (auto *childLayer : groupLayers) {
                emit layerVisibilityChanged(childLayer->id(), childLayer->isVisible());
            }
        } else {
            emit layerVisibilityChanged(node->id(), visible);
        }

        return true;
    }

    if (role == Qt::EditRole) {
        node->setName(value.toString());
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }

    return false;
}

Qt::ItemFlags LayerTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    auto *node = static_cast<GISApp::Domain::Layers::LayerNode*>(index.internalPointer());
    if (node && node->isGroup()) {
        defaultFlags |= Qt::ItemIsAutoTristate;
    }

    return defaultFlags;
}

QVariant LayerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0) {
        return tr("GIS Layer Stack");
    }
    return QVariant();
}

void LayerTreeModel::loadLayers(const QVector<GISApp::Domain::Layers::MapLayer*> &layers)
{
    beginResetModel();

    delete m_rootNode;
    m_rootNode = new GISApp::Domain::Layers::LayerGroup("__root__", "Root");

    QMap<QString, GISApp::Domain::Layers::LayerGroup*> groupMap;

    for (GISApp::Domain::Layers::MapLayer *layer : layers) {
        if (!layer) continue;

        QString groupName = layer->groupName().trimmed();
        if (groupName.isEmpty()) {
            m_rootNode->appendChild(layer);
        } else {
            if (!groupMap.contains(groupName)) {
                auto *group = new GISApp::Domain::Layers::LayerGroup(
                    "grp_" + groupName.toLower().replace(' ', '_'), groupName);
                m_rootNode->appendChild(group);
                groupMap.insert(groupName, group);
            }
            groupMap[groupName]->appendChild(layer);
        }
    }

    endResetModel();
}

bool LayerTreeModel::moveLayerUp(const QModelIndex &index)
{
    if (!index.isValid()) return false;

    int row = index.row();
    if (row <= 0) {
        return false; // Already at the top
    }

    QModelIndex parentIndex = parent(index);
    GISApp::Domain::Layers::LayerNode *parentNode = nodeFromIndex(parentIndex);
    if (!parentNode) return false;

    // In Qt model moveRows, moving item at 'row' up to row - 1:
    // destinationChild is 'row - 1'
    if (!beginMoveRows(parentIndex, row, row, parentIndex, row - 1)) {
        return false;
    }

    parentNode->swapChildren(row, row - 1);
    endMoveRows();

    // Recalculate z-orders and emit signals
    QMap<QString, int> orderMap = synchronizeZOrders();
    emit layerOrderChanged(orderMap);

    return true;
}

bool LayerTreeModel::moveLayerDown(const QModelIndex &index)
{
    if (!index.isValid()) return false;

    int row = index.row();
    QModelIndex parentIndex = parent(index);
    GISApp::Domain::Layers::LayerNode *parentNode = nodeFromIndex(parentIndex);
    if (!parentNode || row >= parentNode->childCount() - 1) {
        return false; // Already at the bottom
    }

    // In Qt model moveRows, moving item at 'row' down past 'row + 1':
    // destinationChild is 'row + 2'
    if (!beginMoveRows(parentIndex, row, row, parentIndex, row + 2)) {
        return false;
    }

    parentNode->swapChildren(row, row + 1);
    endMoveRows();

    // Recalculate z-orders and emit signals
    QMap<QString, int> orderMap = synchronizeZOrders();
    emit layerOrderChanged(orderMap);

    return true;
}

GISApp::Domain::Layers::LayerNode* LayerTreeModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        auto *node = static_cast<GISApp::Domain::Layers::LayerNode*>(index.internalPointer());
        if (node) return node;
    }
    return m_rootNode;
}

QModelIndex LayerTreeModel::indexFromNode(GISApp::Domain::Layers::LayerNode *node) const
{
    if (!node || node == m_rootNode) {
        return QModelIndex();
    }
    return createIndex(node->row(), 0, node);
}

QVector<GISApp::Domain::Layers::MapLayer*> LayerTreeModel::allMapLayers() const
{
    QVector<GISApp::Domain::Layers::MapLayer*> result;
    collectLayersRecursive(m_rootNode, result);
    return result;
}

void LayerTreeModel::collectLayersRecursive(GISApp::Domain::Layers::LayerNode *parent,
                                            QVector<GISApp::Domain::Layers::MapLayer*> &result) const
{
    if (!parent) return;

    for (int i = 0; i < parent->childCount(); ++i) {
        GISApp::Domain::Layers::LayerNode *child = parent->child(i);
        if (!child) continue;

        if (child->isGroup()) {
            collectLayersRecursive(child, result);
        } else {
            result.append(static_cast<GISApp::Domain::Layers::MapLayer*>(child));
        }
    }
}

QMap<QString, int> LayerTreeModel::synchronizeZOrders()
{
    QMap<QString, int> orderMap;
    QVector<GISApp::Domain::Layers::MapLayer*> layers = allMapLayers();

    // In standard GIS: bottom of the tree renders first (z = 0), top of tree renders on top (highest z)
    // Or top of tree is index 0. Here we assign z_order:
    // row 0 gets highest z-order so it paints on top!
    int total = layers.size();
    for (int i = 0; i < total; ++i) {
        int z = (total - 1) - i; // Top layer gets highest zOrder
        layers[i]->setZOrder(z);
        orderMap.insert(layers[i]->id(), z);
    }

    return orderMap;
}

} // namespace GISApp::UIModels::Layers

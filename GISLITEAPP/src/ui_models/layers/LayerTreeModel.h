/**
 * @file LayerTreeModel.h
 * @brief Header definition for LayerTreeModel QAbstractItemModel tree model.
 */

#ifndef LAYERTREEMODEL_H
#define LAYERTREEMODEL_H

#include <QAbstractItemModel>
#include <QVector>
#include <QMap>

namespace GISApp::Domain::Layers {
class LayerNode;
class LayerGroup;
class MapLayer;
}

namespace GISApp::UIModels::Layers {

/**
 * @class LayerTreeModel
 * @brief Hierarchical Qt item model adapting the LayerNode domain structure for QTreeView.
 *
 * LayerTreeModel acts as the bridge between persistent domain entities and the GUI presentation:
 * - Emits check-state changes when user clicks visibility checkboxes.
 * - Supports Move Up ⬆️ and Move Down ⬇️ operations to reorder the layer rendering stack.
 * - Manages tree hierarchy including parent group categories and child map layers.
 */
class LayerTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum LayerModelRoles {
        IdRole = Qt::UserRole + 1,      ///< Unique layer identifier string
        ZOrderRole,                     ///< Integer z-order rendering priority
        OpacityRole,                    ///< Double opacity factor [0.0, 1.0]
        IsGroupRole,                    ///< Boolean flag indicating if item is a group
        TypeIconRole                    ///< Unicode or decoration icon
    };

    /**
     * @brief Constructs the LayerTreeModel and initializes the root node.
     * @param[in] parent Optional parent QObject.
     */
    explicit LayerTreeModel(QObject *parent = nullptr);

    /**
     * @brief Destructor releasing the root node and all children.
     */
    virtual ~LayerTreeModel() override;

    // QAbstractItemModel interface implementation
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &child) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief Populates the tree model with layers prefetched from the database.
     * @param[in] layers Vector of MapLayer entities (takes ownership of pointers).
     */
    void loadLayers(const QVector<GISApp::Domain::Layers::MapLayer*> &layers);

    /**
     * @brief Moves the selected layer or group up by one position in its sibling order.
     * @param[in] index Model index of the item to promote.
     * @return True if moved successfully, false if already at top or invalid.
     */
    bool moveLayerUp(const QModelIndex &index);

    /**
     * @brief Moves the selected layer or group down by one position in its sibling order.
     * @param[in] index Model index of the item to demote.
     * @return True if moved successfully, false if already at bottom or invalid.
     */
    bool moveLayerDown(const QModelIndex &index);

    /**
     * @brief Retrieves the domain LayerNode pointer associated with a QModelIndex.
     * @param[in] index QModelIndex in the model.
     * @return Pointer to LayerNode, or m_rootNode if invalid index.
     */
    [[nodiscard]] GISApp::Domain::Layers::LayerNode* nodeFromIndex(const QModelIndex &index) const;

    /**
     * @brief Finds the QModelIndex corresponding to a given domain LayerNode.
     * @param[in] node Target LayerNode pointer.
     * @return Valid QModelIndex if found in the tree, invalid QModelIndex otherwise.
     */
    [[nodiscard]] QModelIndex indexFromNode(GISApp::Domain::Layers::LayerNode *node) const;

    /**
     * @brief Collects all leaf MapLayer entities currently in the model.
     * @return Vector of MapLayer pointers (not owned by caller).
     */
    [[nodiscard]] QVector<GISApp::Domain::Layers::MapLayer*> allMapLayers() const;

    /**
     * @brief Recalculates and synchronizes all layer z-orders based on their current tree sequence.
     * @return Map associating layer IDs with their computed z-order integer.
     */
    QMap<QString, int> synchronizeZOrders();

signals:
    /**
     * @brief Emitted when a layer's visibility checkbox state is toggled.
     * @param[in] layerId Unique layer identifier.
     * @param[in] visible New visibility boolean.
     */
    void layerVisibilityChanged(const QString &layerId, bool visible);

    /**
     * @brief Emitted when layer order is modified via Move Up or Move Down.
     * @param[in] orderMap Map associating layer IDs with updated z-order integers.
     */
    void layerOrderChanged(const QMap<QString, int> &orderMap);

private:
    /**
     * @brief Recursively traverses children to collect all MapLayer leaf entities.
     * @param[in] parent Parent node to traverse.
     * @param[out] result Vector to append found layers to.
     */
    void collectLayersRecursive(GISApp::Domain::Layers::LayerNode *parent,
                                QVector<GISApp::Domain::Layers::MapLayer*> &result) const;

    /// Root composite node hosting all top-level groups and layers
    GISApp::Domain::Layers::LayerNode *m_rootNode;
};

} // namespace GISApp::UIModels::Layers

#endif // LAYERTREEMODEL_H

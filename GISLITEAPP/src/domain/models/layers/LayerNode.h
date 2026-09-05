/**
 * @file LayerNode.h
 * @brief Header definition for LayerNode composite base entity in GIS layer hierarchy.
 */

#ifndef LAYERNODE_H
#define LAYERNODE_H

#include <QString>
#include <QVector>

namespace GISApp::Domain::Layers {

/**
 * @class LayerNode
 * @brief Abstract base class in the Composite pattern representing an item in the layer tree.
 *
 * LayerNode provides core tree navigation, ordering, and state properties for both
 * grouping folders (LayerGroup) and actual renderable layers (MapLayer).
 *
 * Architectural Role:
 * - Serves as the domain entity backbone for Qt's QAbstractItemModel (LayerTreeModel).
 * - Tracks z-order index defining rendering stack placement in MapLibre/GDAL.
 * - Maintains bidirectional parent-child links for hierarchical navigation.
 */
class LayerNode
{
public:
    /**
     * @brief Constructs a LayerNode with identity and naming metadata.
     * @param[in] id Unique persistent identifier (e.g. "layer_dark_base").
     * @param[in] name Human-readable display name.
     * @param[in] parent Optional parent node; ownership is managed by the composite tree.
     */
    explicit LayerNode(const QString &id, const QString &name, LayerNode *parent = nullptr);

    /**
     * @brief Virtual destructor releasing all child nodes recursively.
     */
    virtual ~LayerNode();

    /**
     * @brief Discriminator determining if this node is a composite folder group.
     * @return True if node is a LayerGroup, false if it is a leaf MapLayer.
     */
    [[nodiscard]] virtual bool isGroup() const = 0;

    /**
     * @brief Retrieves the unique node identifier.
     * @return Node ID string.
     */
    [[nodiscard]] QString id() const { return m_id; }

    /**
     * @brief Updates the node identifier.
     * @param[in] id New unique ID string.
     */
    void setId(const QString &id) { m_id = id; }

    /**
     * @brief Retrieves the display name.
     * @return Display name string.
     */
    [[nodiscard]] QString name() const { return m_name; }

    /**
     * @brief Updates the display name.
     * @param[in] name New display name string.
     */
    void setName(const QString &name) { m_name = name; }

    /**
     * @brief Checks whether this node is currently visible.
     * @return True if visible, false if hidden.
     */
    [[nodiscard]] bool isVisible() const { return m_isVisible; }

    /**
     * @brief Sets node visibility state.
     * @param[in] visible Boolean visibility flag.
     */
    virtual void setVisible(bool visible);

    /**
     * @brief Checks whether this group node is expanded in the tree view.
     * @return True if expanded, false if collapsed.
     */
    [[nodiscard]] bool isExpanded() const { return m_isExpanded; }

    /**
     * @brief Sets node expansion state.
     * @param[in] expanded Boolean expansion flag.
     */
    void setExpanded(bool expanded) { m_isExpanded = expanded; }

    /**
     * @brief Retrieves the rendering z-order index.
     * @return Integer z-order (higher values render on top of lower values).
     */
    [[nodiscard]] int zOrder() const { return m_zOrder; }

    /**
     * @brief Updates the rendering z-order index.
     * @param[in] zOrder New integer z-order.
     */
    void setZOrder(int zOrder) { m_zOrder = zOrder; }

    /**
     * @brief Retrieves the parent node in the hierarchy.
     * @return Pointer to parent LayerNode, or nullptr if root.
     */
    [[nodiscard]] LayerNode* parentNode() const { return m_parent; }

    /**
     * @brief Sets the parent node pointer.
     * @param[in] parent Pointer to new parent node.
     */
    void setParentNode(LayerNode *parent) { m_parent = parent; }

    /**
     * @brief Returns the number of child nodes under this node.
     * @return Integer count of child items.
     */
    [[nodiscard]] int childCount() const { return m_children.size(); }

    /**
     * @brief Retrieves child node at specified row index.
     * @param[in] row 0-based child index.
     * @return Pointer to child LayerNode, or nullptr if out of bounds.
     */
    [[nodiscard]] LayerNode* child(int row) const;

    /**
     * @brief Determines the 0-based index of this node relative to its parent.
     * @return Row index integer, or 0 if no parent.
     */
    [[nodiscard]] int row() const;

    /**
     * @brief Appends a child node to the end of the children collection.
     * @param[in] child Pointer to child node. Takes ownership.
     */
    void appendChild(LayerNode *child);

    /**
     * @brief Inserts a child node at the given row position.
     * @param[in] row 0-based insertion index.
     * @param[in] child Pointer to child node. Takes ownership.
     */
    void insertChild(int row, LayerNode *child);

    /**
     * @brief Removes and deletes the child node at the given row position.
     * @param[in] row 0-based child index to remove.
     * @return True if removed successfully, false if row out of bounds.
     */
    bool removeChild(int row);

    /**
     * @brief Swaps positions of two child nodes at the specified indices.
     * @param[in] rowA First child row index.
     * @param[in] rowB Second child row index.
     * @return True if swapped successfully, false if either index invalid.
     */
    bool swapChildren(int rowA, int rowB);

    /**
     * @brief Retrieves the read-only list of all children.
     * @return Const reference to QVector of child pointers.
     */
    [[nodiscard]] const QVector<LayerNode*>& children() const { return m_children; }

protected:
    /// Unique persistent identifier
    QString m_id;

    /// User-facing display title
    QString m_name;

    /// Visibility toggle state
    bool m_isVisible;

    /// Tree expansion state in UI view
    bool m_isExpanded;

    /// Rendering stack elevation order
    int m_zOrder;

    /// Pointer to parent node in tree hierarchy
    LayerNode *m_parent;

    /// Ordered list of child node pointers owned by this node
    QVector<LayerNode*> m_children;
};

} // namespace GISApp::Domain::Layers

#endif // LAYERNODE_H

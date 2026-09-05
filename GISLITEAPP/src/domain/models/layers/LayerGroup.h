/**
 * @file LayerGroup.h
 * @brief Header definition for LayerGroup composite folder entity in GIS layer tree.
 */

#ifndef LAYERGROUP_H
#define LAYERGROUP_H

#include "LayerNode.h"
#include <Qt>

namespace GISApp::Domain::Layers {

/**
 * @class LayerGroup
 * @brief Composite node representing a folder or category of layers in the Layer Tree.
 *
 * LayerGroup organizes related layers (e.g. "Base Maps", "Operational Overlays", "Target Tracks").
 * It implements cascading visibility:
 * - When a group is toggled visible/invisible, all children inherit the state.
 * - Supports tri-state logic (Checked, PartiallyChecked, Unchecked) for QTreeView checkboxes.
 */
class LayerGroup : public LayerNode
{
public:
    /**
     * @brief Constructs a LayerGroup with identity and display title.
     * @param[in] id Unique group identifier.
     * @param[in] name User-facing folder title.
     * @param[in] parent Optional parent node.
     */
    explicit LayerGroup(const QString &id, const QString &name, LayerNode *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~LayerGroup() override = default;

    /**
     * @brief Discriminator confirming this node is a composite group.
     * @return Always true.
     */
    [[nodiscard]] bool isGroup() const override { return true; }

    /**
     * @brief Recursively sets visibility on the group and all its descendant layers.
     * @param[in] visible Target visibility state.
     */
    void setVisible(bool visible) override;

    /**
     * @brief Computes tri-state check representation based on child visibility.
     * @return Qt::Checked, Qt::Unchecked, or Qt::PartiallyChecked.
     */
    [[nodiscard]] Qt::CheckState checkState() const;
};

} // namespace GISApp::Domain::Layers

#endif // LAYERGROUP_H

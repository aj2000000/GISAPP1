/**
 * @file LayerGroup.cpp
 * @brief Implementation of LayerGroup composite folder entity.
 */

#include "LayerGroup.h"

namespace GISApp::Domain::Layers {

LayerGroup::LayerGroup(const QString &id, const QString &name, LayerNode *parent)
    : LayerNode(id, name, parent)
{
}

void LayerGroup::setVisible(bool visible)
{
    LayerNode::setVisible(visible);
    for (LayerNode *child : m_children) {
        if (child) {
            child->setVisible(visible);
        }
    }
}

Qt::CheckState LayerGroup::checkState() const
{
    if (m_children.isEmpty()) {
        return m_isVisible ? Qt::Checked : Qt::Unchecked;
    }

    int checkedCount = 0;
    int partialCount = 0;

    for (const LayerNode *child : m_children) {
        if (!child) continue;

        if (child->isGroup()) {
            const auto *group = static_cast<const LayerGroup*>(child);
            Qt::CheckState state = group->checkState();
            if (state == Qt::Checked) {
                checkedCount++;
            } else if (state == Qt::PartiallyChecked) {
                partialCount++;
            }
        } else {
            if (child->isVisible()) {
                checkedCount++;
            }
        }
    }

    if (checkedCount == m_children.size()) {
        return Qt::Checked;
    }
    if (checkedCount == 0 && partialCount == 0) {
        return Qt::Unchecked;
    }
    return Qt::PartiallyChecked;
}

} // namespace GISApp::Domain::Layers

/**
 * @file LayerNode.cpp
 * @brief Implementation of LayerNode composite base entity.
 */

#include "LayerNode.h"

namespace GISApp::Domain::Layers {

LayerNode::LayerNode(const QString &id, const QString &name, LayerNode *parent)
    : m_id(id)
    , m_name(name)
    , m_isVisible(true)
    , m_isExpanded(true)
    , m_zOrder(0)
    , m_parent(parent)
{
}

LayerNode::~LayerNode()
{
    qDeleteAll(m_children);
    m_children.clear();
}

void LayerNode::setVisible(bool visible)
{
    m_isVisible = visible;
}

LayerNode* LayerNode::child(int row) const
{
    if (row < 0 || row >= m_children.size()) {
        return nullptr;
    }
    return m_children.at(row);
}

int LayerNode::row() const
{
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<LayerNode*>(this));
    }
    return 0;
}

void LayerNode::appendChild(LayerNode *child)
{
    if (!child) return;
    child->setParentNode(this);
    m_children.append(child);
}

void LayerNode::insertChild(int row, LayerNode *child)
{
    if (!child) return;
    child->setParentNode(this);
    if (row < 0 || row > m_children.size()) {
        m_children.append(child);
    } else {
        m_children.insert(row, child);
    }
}

bool LayerNode::removeChild(int row)
{
    if (row < 0 || row >= m_children.size()) {
        return false;
    }
    LayerNode *childNode = m_children.takeAt(row);
    delete childNode;
    return true;
}

bool LayerNode::swapChildren(int rowA, int rowB)
{
    if (rowA < 0 || rowA >= m_children.size() ||
        rowB < 0 || rowB >= m_children.size() ||
        rowA == rowB) {
        return false;
    }

    m_children.swapItemsAt(rowA, rowB);

    // Swap z-order values to preserve rendering integrity
    int tempZ = m_children[rowA]->zOrder();
    m_children[rowA]->setZOrder(m_children[rowB]->zOrder());
    m_children[rowB]->setZOrder(tempZ);

    return true;
}

} // namespace GISApp::Domain::Layers

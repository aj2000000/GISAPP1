/**
 * @file LayerTreeView.cpp
 * @brief Implementation of LayerTreeView custom tree view.
 */

#include "LayerTreeView.h"

#include <QHeaderView>
#include <QKeyEvent>

namespace GISApp::UI::Layers {

/**
 * @brief Constructs the LayerTreeView widget with tactical GIS view configurations.
 * @param[in] parent Optional parent widget pointer.
 */
LayerTreeView::LayerTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setObjectName("GISLayerTreeView");
    setHeaderHidden(true);
    setRootIsDecorated(true);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAnimated(true);
    setExpandsOnDoubleClick(true);
}

/**
 * @brief Handles right-click context menu events on layer items.
 * @param[in] event Pointer to context menu event containing cursor coordinates.
 * @note Spawns a themed LayerContextMenu offering Move Up, Move Down, and Toggle Visibility.
 */
void LayerTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);
    menu.setObjectName("LayerContextMenu");

    QAction *moveUpAction = menu.addAction("⬆️  Move Up");
    QAction *moveDownAction = menu.addAction("⬇️  Move Down");
    menu.addSeparator();
    QAction *toggleAction = menu.addAction("👁️  Toggle Visibility");

    connect(moveUpAction, &QAction::triggered, this, &LayerTreeView::moveUpTriggered);
    connect(moveDownAction, &QAction::triggered, this, &LayerTreeView::moveDownTriggered);
    connect(toggleAction, &QAction::triggered, this, &LayerTreeView::toggleVisibilityTriggered);

    menu.exec(event->globalPos());
}

/**
 * @brief Handles tactical keyboard navigation shortcuts.
 * @param[in] event Pointer to keyboard event.
 * @note Supports Ctrl+Up (promote layer), Ctrl+Down (demote layer), and Space (toggle visibility).
 */
void LayerTreeView::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Up) {
            emit moveUpTriggered();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Down) {
            emit moveDownTriggered();
            event->accept();
            return;
        }
    } else if (event->key() == Qt::Key_Space) {
        emit toggleVisibilityTriggered();
        event->accept();
        return;
    }

    QTreeView::keyPressEvent(event);
}

} // namespace GISApp::UI::Layers

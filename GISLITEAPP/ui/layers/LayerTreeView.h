/**
 * @file LayerTreeView.h
 * @brief Header definition for LayerTreeView customized tree view.
 */

#ifndef LAYERTREEVIEW_H
#define LAYERTREEVIEW_H

#include <QTreeView>
#include <QContextMenuEvent>
#include <QMenu>

namespace GISApp::UI::Layers {

/**
 * @class LayerTreeView
 * @brief Custom QTreeView tailored for GIS layer hierarchy navigation and reordering.
 *
 * Provides:
 * - Direct keyboard shortcuts for layer stack reordering (Ctrl+Up / Ctrl+Down).
 * - Right-click context menus offering Move Up, Move Down, and Visibility actions.
 * - Uniform row heights, visual selection styling, and item expansion tracking.
 */
class LayerTreeView : public QTreeView
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the LayerTreeView and sets view options.
     * @param[in] parent Optional parent QWidget.
     */
    explicit LayerTreeView(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~LayerTreeView() override = default;

signals:
    /**
     * @brief Emitted when the user requests panning map to a selected layer or group.
     * @param[in] index Model index of the target layer or group node.
     */
    void panToTriggered(const QModelIndex &index);

    /**
     * @brief Emitted when the user requests promoting the selected layer via context menu or hotkey.
     */
    void moveUpTriggered();

    /**
     * @brief Emitted when the user requests demoting the selected layer via context menu or hotkey.
     */
    void moveDownTriggered();

    /**
     * @brief Emitted when the user toggles visibility for the selected item.
     */
    void toggleVisibilityTriggered();

protected:
    /**
     * @brief Handles right-click context menu events on layer items.
     * @param[in] event Context menu event details.
     */
    void contextMenuEvent(QContextMenuEvent *event) override;

    /**
     * @brief Handles mouse double-click events to trigger pan to layer.
     * @param[in] event Mouse event details.
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /**
     * @brief Handles keyboard navigation (e.g. Ctrl+Up / Ctrl+Down).
     * @param[in] event Key event details.
     */
    void keyPressEvent(QKeyEvent *event) override;
};

} // namespace GISApp::UI::Layers

#endif // LAYERTREEVIEW_H

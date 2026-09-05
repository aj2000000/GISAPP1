/**
 * @file LayerTreePanel.h
 * @brief Header definition for LayerTreePanel floating tactical layer manager panel.
 */

#ifndef LAYERTREEPANEL_H
#define LAYERTREEPANEL_H

#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPoint>
#include <QAbstractItemModel>
#include <QModelIndex>

namespace GISApp::UI::Layers {

class LayerTreeView;

/**
 * @class LayerTreePanel
 * @brief Floating, draggable tactical UI panel managing the layer hierarchy and rendering order.
 *
 * LayerTreePanel floats directly above the MapWidget canvas.
 * Key Features:
 * - Header with draggable handle, title, layer count badge, and close button (✕).
 * - Action toolbar providing:
 *   - ⬆️ Move Up: Moves selected layer up in the tree (promotes z-order).
 *   - ⬇️ Move Down: Moves selected layer down in the tree (demotes z-order).
 *   - 👁️ Toggle Visibility: Inverts visibility of selected layer.
 *   - ➕ Add Layer: Opens dataset import prompt.
 * - Search filter box to quickly filter tree nodes by name.
 * - Encapsulated LayerTreeView displaying checkable layer entries.
 */
class LayerTreePanel : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the LayerTreePanel floating card.
     * @param[in] parent Optional parent QWidget (typically MapViewContainer).
     */
    explicit LayerTreePanel(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~LayerTreePanel() override = default;

    /**
     * @brief Binds a QAbstractItemModel (e.g. LayerTreeModel) to the tree view.
     * @param[in] model Pointer to item model.
     */
    void setModel(QAbstractItemModel *model);

    /**
     * @brief Returns a pointer to the hosted LayerTreeView.
     * @return Pointer to LayerTreeView instance.
     */
    [[nodiscard]] LayerTreeView* treeView() const { return m_treeView; }

    /**
     * @brief Retrieves the currently selected model index in the tree.
     * @return Selected QModelIndex, or invalid index if none selected.
     */
    [[nodiscard]] QModelIndex selectedIndex() const;

    /**
     * @brief Updates the numerical layer count displayed in the header badge.
     * @param[in] count Total active layer count.
     */
    void updateLayerCount(int count);

signals:
    /**
     * @brief Emitted when user clicks Move Up ⬆️.
     */
    void moveUpRequested();

    /**
     * @brief Emitted when user clicks Move Down ⬇️.
     */
    void moveDownRequested();

    /**
     * @brief Emitted when user clicks Toggle Visibility 👁️.
     */
    void toggleVisibilityRequested();

    /**
     * @brief Emitted when user clicks Add Layer ➕.
     */
    void addLayerRequested();

    /**
     * @brief Emitted when the floating panel close button is clicked.
     */
    void closeRequested();

protected:
    /**
     * @brief Captures mouse press on header for panel dragging.
     * @param[in] event Mouse event details.
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief Handles panel translation across parent canvas while dragging header.
     * @param[in] event Mouse move event details.
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief Releases drag mode on mouse release.
     * @param[in] event Mouse release event details.
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    /**
     * @brief Initializes layout, header bar, action toolbar, filter box, and tree view.
     */
    void setupUi();

    /**
     * @brief Connects internal button and tree signals.
     */
    void setupConnections();

    /// Draggable header bar widget
    QWidget *m_headerBar;

    /// Panel title label
    QLabel *m_titleLabel;

    /// Layer count indicator badge
    QLabel *m_countBadge;

    /// Close panel button
    QToolButton *m_closeBtn;

    /// Toolbar buttons
    QToolButton *m_moveUpBtn;
    QToolButton *m_moveDownBtn;
    QToolButton *m_toggleBtn;
    QToolButton *m_addLayerBtn;

    /// Real-time search filter edit
    QLineEdit *m_filterEdit;

    /// Customized tree view
    LayerTreeView *m_treeView;

    /// Dragging state tracking
    bool m_isDragging;
    QPoint m_dragStartPosition;
};

} // namespace GISApp::UI::Layers

#endif // LAYERTREEPANEL_H

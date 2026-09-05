/**
 * @file MapViewContainer.h
 * @brief Header definition for MapViewContainer UI workspace component.
 */

#ifndef MAPVIEWCONTAINER_H
#define MAPVIEWCONTAINER_H

#include <QWidget>

class QResizeEvent;
class QShowEvent;

namespace GISApp::UI {

class MapWidget;
class ZoomControlsWidget;
class RightToolPanel;

namespace Layers {
class LayerTreePanel;
}

/**
 * @class MapViewContainer
 * @brief Central GIS workspace view container integrating MapWidget with floating UI overlays.
 *
 * MapViewContainer acts as the primary canvas inside the MainBaseUI stacked widget.
 * It employs a floating overlay layout pattern where:
 * - The base widget MapWidget (QMapLibre::MapWidget) fills the entire container geometry edge-to-edge.
 * - Floating controls are positioned dynamically on top with configurable margins:
 *   - ZoomControlsWidget pinned to the bottom-left with margin offset from the left/bottom edges.
 *   - RightToolPanel pinned to the top-right with margin offset from the top/right edges.
 *
 * It forwards user interactions from floating tool buttons directly to the map engine and
 * dispatches coordinate hover events upwards to MainBaseUI and the TacticalStatusBar.
 */
class MapViewContainer : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the MapViewContainer, creates child widgets, and configures overlay layout.
     * @param[in] parent Optional parent QWidget; ownership is managed by Qt parent-child hierarchy.
     */
    explicit MapViewContainer(QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor ensuring clean teardown of child widgets.
     */
    virtual ~MapViewContainer() override = default;

    /**
     * @brief Returns a pointer to the hosted MapWidget engine wrapper.
     * @return Pointer to GISApp::UI::MapWidget instance, never nullptr after setupUi().
     */
    [[nodiscard]] MapWidget* mapWidget() const { return m_mapWidget; }

    /**
     * @brief Returns a pointer to the floating zoom and bearing controls widget.
     * @return Pointer to GISApp::UI::ZoomControlsWidget instance.
     */
    [[nodiscard]] ZoomControlsWidget* zoomControls() const { return m_zoomControls; }

    /**
     * @brief Returns a pointer to the floating quick-action tool panel.
     * @return Pointer to GISApp::UI::RightToolPanel instance.
     */
    [[nodiscard]] RightToolPanel* rightToolPanel() const { return m_rightToolPanel; }

    /**
     * @brief Returns a pointer to the floating Layer Tree manager panel.
     * @return Pointer to GISApp::UI::Layers::LayerTreePanel instance.
     */
    [[nodiscard]] GISApp::UI::Layers::LayerTreePanel* layerTreePanel() const { return m_layerTreePanel; }

    /**
     * @brief Configures the pixel margin separating floating panels from the container edges.
     * @param[in] margin Pixel margin (e.g. 16 or 20 pixels). Must be non-negative.
     */
    void setOverlayMargins(int margin);

    /**
     * @brief Retrieves the active overlay pixel margin.
     * @return Offset in pixels from container edges.
     */
    [[nodiscard]] int overlayMargins() const { return m_overlayMargin; }

signals:
    /**
     * @brief Emitted when the mouse cursor hovers over the map canvas.
     * @param[in] latitude Geodetic latitude in decimal degrees [-90.0, 90.0].
     * @param[in] longitude Geodetic longitude in decimal degrees [-180.0, 180.0].
     * @note Consumed by MainBaseUI and forwarded to TacticalStatusBar for telemetry display.
     */
    void coordinateHovered(double latitude, double longitude);

protected:
    /**
     * @brief Handles container resize events to reposition floating overlays over the map canvas.
     * @param[in] event Pointer to QResizeEvent containing old and new widget dimensions.
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief Handles container show events to perform initial overlay positioning.
     * @param[in] event Pointer to QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

private:
    /**
     * @brief Instantiates child widgets and sets default layout flags.
     */
    void setupUi();

    /**
     * @brief Connects signals between ZoomControlsWidget and MapWidget.
     */
    void setupConnections();

    /**
     * @brief Recalculates and applies exact geometries for the base map and floating panels.
     */
    void updateOverlayPositions();

    /// Underlying hardware-accelerated MapLibre map widget instance
    MapWidget *m_mapWidget;

    /// Floating overlay widget providing zoom (+/-) and compass reset buttons
    ZoomControlsWidget *m_zoomControls;

    /// Floating tactical tool panel pinned to top-right corner
    RightToolPanel *m_rightToolPanel;

    /// Floating layer tree panel managing layer ordering and visibility
    GISApp::UI::Layers::LayerTreePanel *m_layerTreePanel;

    /// Offset margin in pixels between floating panels and container edges
    int m_overlayMargin;
};

} // namespace GISApp::UI

#endif // MAPVIEWCONTAINER_H

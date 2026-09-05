/**
 * @file ZoomControlsWidget.h
 * @brief Header definition for ZoomControlsWidget floating map overlay.
 */

#ifndef ZOOMCONTROLSWIDGET_H
#define ZOOMCONTROLSWIDGET_H

#include <QFrame>
#include <QVBoxLayout>
#include <QToolButton>

namespace GISApp::UI {

/**
 * @class ZoomControlsWidget
 * @brief Floating map control panel providing quick-action camera zoom and bearing buttons.
 *
 * ZoomControlsWidget is styled as a semi-transparent floating tactical panel anchored to
 * the bottom-left corner of the MapViewContainer. It emits user intent signals when buttons
 * are pressed, which are wired to MapWidget / MapController camera operations.
 *
 * Provided Controls:
 * - Compass/Reset button: Emits resetCenterRequested() to restore true North bearing.
 * - Zoom-in (+) button: Emits zoomInRequested().
 * - Zoom-out (-) button: Emits zoomOutRequested().
 */
class ZoomControlsWidget : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief Constructs ZoomControlsWidget, configures vertical layout, and sets up buttons.
     * @param[in] parent Optional parent QWidget.
     */
    explicit ZoomControlsWidget(QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~ZoomControlsWidget() override = default;

signals:
    /**
     * @brief Emitted when the compass button is clicked to restore North alignment.
     */
    void resetCenterRequested();

    /**
     * @brief Emitted when the '+' zoom-in button is clicked.
     */
    void zoomInRequested();

    /**
     * @brief Emitted when the '-' zoom-out button is clicked.
     */
    void zoomOutRequested();
};

} // namespace GISApp::UI

#endif // ZOOMCONTROLSWIDGET_H

/**
 * @file IMapInteractionListener.h
 * @brief Strategy interface definition for handling interactive mouse canvas gestures (e.g. control point dragging).
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef IMAPINTERACTIONLISTENER_H
#define IMAPINTERACTIONLISTENER_H

#include <QPointF>
#include <Qt>

namespace GISApp::Core::Interfaces {

/**
 * @class IMapInteractionListener
 * @brief Strategy interface for subsystems intercepting and handling interactive mouse gestures on the map canvas.
 *
 * Architectural Role & Design Patterns:
 * - Implements the **Listener / Chain of Responsibility Pattern** in the Core Interfaces layer (`src/core/interfaces/`).
 * - Decouples the presentation map canvas (`MapWidget`) from domain controllers handling specialized interactions
 *   such as control point vertex dragging, bounding box selection, drawing tools, or measuring instruments.
 * - `MapWidget` intercepts mouse events from the underlying MapLibre renderer (`m_nativeMapWidget`) via `eventFilter`
 *   and forwards them to registered `IMapInteractionListener` instances.
 * - When a listener consumes an event by returning `true`, the default map navigation gesture (e.g. MapLibre canvas panning)
 *   is blocked, allowing smooth and jitter-free manipulation of target geometric entities.
 */
class IMapInteractionListener
{
public:
    /**
     * @brief Virtual destructor ensuring safe polymorphic destruction.
     */
    virtual ~IMapInteractionListener() = default;

    /**
     * @brief Handles mouse press events on the map canvas.
     * @param[in] screenPos Viewport pixel position of the cursor relative to the map canvas widget.
     * @param[in] geoCoord Geodetic coordinate (latitude in .x(), longitude in .y()).
     * @param[in] button The specific Qt mouse button that was pressed (e.g. Qt::LeftButton).
     * @return True if the listener consumed the press event (initiating a drag/action and suppressing map pan); false to allow default handling.
     */
    virtual bool onMapMousePress(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButton button) = 0;

    /**
     * @brief Handles mouse movement events across the map canvas.
     * @param[in] screenPos Current viewport pixel position of the cursor.
     * @param[in] geoCoord Current geodetic coordinate (latitude in .x(), longitude in .y()).
     * @param[in] buttons Bitwise combination of currently held Qt mouse buttons.
     * @return True if the listener consumed the move event (updating active drag); false to allow default handling.
     */
    virtual bool onMapMouseMove(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButtons buttons) = 0;

    /**
     * @brief Handles mouse release events on the map canvas.
     * @param[in] screenPos Viewport pixel position where the button was released.
     * @param[in] geoCoord Geodetic coordinate where the button was released.
     * @param[in] button The specific Qt mouse button that was released.
     * @return True if the listener consumed the release event (completing an active drag/action); false otherwise.
     */
    virtual bool onMapMouseRelease(const QPointF &screenPos, const QPointF &geoCoord, Qt::MouseButton button) = 0;
};

} // namespace GISApp::Core::Interfaces

#endif // IMAPINTERACTIONLISTENER_H

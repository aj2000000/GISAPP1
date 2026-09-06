/**
 * @file IContextMenuContributor.h
 * @brief Strategy interface definition for subsystems contributing right-click context menu actions on the map canvas.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef ICONTEXTMENUCONTRIBUTOR_H
#define ICONTEXTMENUCONTRIBUTOR_H

#include <QPoint>
#include <QPointF>

class QMenu;

namespace GISApp::Core::Interfaces {

/**
 * @class IContextMenuContributor
 * @brief Strategy interface for decoupled subsystems contributing contextual actions to map right-click events.
 *
 * Architectural Role & Design Pattern:
 * - Implements the **Contributor / Strategy Pattern** and **Action Aggregation Pattern**.
 * - Decouples the presentation map canvas (`MapWidget`) from domain-specific entity models
 *   (`TacticalTrack`, `MapLayer`, drawing primitives).
 * - When the user executes a secondary click (right-click) on the map canvas, `MapWidget` queries
 *   all registered contributors ordered by `priority()`.
 * - Each contributor conducts spatial hit-testing against its managed domain entities and dynamically
 *   populates actions or hierarchical sub-menus into the supplied `QMenu`.
 */
class IContextMenuContributor
{
public:
    /**
     * @brief Virtual destructor ensuring safe polymorphic deletion of contributor instances.
     */
    virtual ~IContextMenuContributor() = default;

    /**
     * @brief Declares the evaluation priority order for menu rendering.
     *
     * Higher priority values are evaluated first and rendered at the top of the context menu.
     * Suggested ranges:
     * - Tactical Tracks / Real-time Entities: 100
     * - Vector GIS Features / Layers: 50
     * - Global Map Navigation / Coordinates: 0
     *
     * @return Integer sorting weight.
     */
    [[nodiscard]] virtual int priority() const = 0;

    /**
     * @brief Inspects the clicked coordinate, performs domain hit-testing, and populates contextual actions.
     *
     * @param[in,out] parentMenu Pointer to the active QMenu where actions or sub-menus must be inserted.
     *                           Ownership of created QAction/QMenu objects must be parented to parentMenu.
     * @param[in] screenPos Viewport pixel position of the click relative to the map canvas widget.
     * @param[in] geoCoord Geographic coordinate (latitude in .x(), longitude in .y() or standard QPointF).
     * @return True if this contributor added any actions, separators, or sub-menus; false otherwise.
     *
     * @note Implementations must handle single-target vs. multi-target overlapping disambiguation cleanly.
     */
    virtual bool contributeActions(QMenu *parentMenu, const QPoint &screenPos, const QPointF &geoCoord) = 0;
};

} // namespace GISApp::Core::Interfaces

#endif // ICONTEXTMENUCONTRIBUTOR_H

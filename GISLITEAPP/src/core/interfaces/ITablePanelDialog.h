/**
 * @file ITablePanelDialog.h
 * @brief Core interface definition for modeless tactical table dialogs.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef ITABLEPANELDIALOG_H
#define ITABLEPANELDIALOG_H

#include <QString>

class QAbstractItemModel;

namespace GISApp::Core::Interfaces {

/**
 * @class ITablePanelDialog
 * @brief Abstract interface defining the architectural contract for all tabular panel dialogs.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the Core Interfaces layer (`src/core/interfaces/`).
 * - Establishes a uniform behavioral contract across all tabular dialogs in the application
 *   (e.g., Tactical Tracks, Sensors, System Alerts, Map Layers, Recon Targets).
 * - Standardizes modeless lifecycle management (display, closure, visibility state).
 * - Enforces model-view binding (`setModel`, `model`) and operational controls
 *   (search filtering, telemetry metric calculation, data refreshing).
 * - Enables controllers and navigation menus to manipulate any table dialog polymorphically
 *   without tight coupling to concrete UI implementations.
 */
class ITablePanelDialog
{
public:
    virtual ~ITablePanelDialog() = default;

    /**
     * @brief Displays the table panel dialog non-modally and brings it to the foreground.
     */
    virtual void showDialog() = 0;

    /**
     * @brief Closes or hides the table panel dialog.
     */
    virtual void closeDialog() = 0;

    /**
     * @brief Checks if the table panel dialog is currently visible to the operator.
     * @return True if visible on screen, false otherwise.
     */
    [[nodiscard]] virtual bool isDialogVisible() const = 0;

    /**
     * @brief Binds a Qt item model providing the tabular records to be presented.
     * @param[in] model Pointer to QAbstractItemModel.
     */
    virtual void setModel(QAbstractItemModel *model) = 0;

    /**
     * @brief Retrieves the active Qt item model bound to the dialog.
     * @return Pointer to active QAbstractItemModel, or nullptr if none set.
     */
    [[nodiscard]] virtual QAbstractItemModel* model() const = 0;

    /**
     * @brief Applies or clears a free-text search filter across table records.
     * @param[in] filterText Search query string (case-insensitive).
     */
    virtual void setSearchFilter(const QString &filterText) = 0;

    /**
     * @brief Recalculates and updates the informational metrics badge (e.g. counts, categories).
     */
    virtual void updateStatistics() = 0;

    /**
     * @brief Triggers a reload or synchronization of the underlying tabular data.
     */
    virtual void refreshData() = 0;
};

} // namespace GISApp::Core::Interfaces

#endif // ITABLEPANELDIALOG_H

/**
 * @file ComplexEntityTablePanelDialog.h
 * @brief Modeless tactical table panel dialog for ComplexEntity management and inspection.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYTABLEPANELDIALOG_H
#define COMPLEXENTITYTABLEPANELDIALOG_H

#include "BaseTablePanelDialog.h"

namespace GISApp::UIModels::ComplexEntities {
class ComplexEntityTableModel;
}

namespace GISApp::UI::ComplexEntities {

/**
 * @class ComplexEntityTablePanelDialog
 * @brief Tactical dialog specialization of BaseTablePanelDialog for viewing and filtering complex entities.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the UI Presentation layer (`ui/complexentities/`).
 * - Inherits from `GISApp::Core::Wrappers::BaseTablePanelDialog` using the Template Method pattern.
 * - Displays telemetry and spatial properties from UDP Message ID 905 (`MAIN_LITE_COMPLEX_ENTITY_MSG_ID`).
 * - Features dynamic text searching, automatic status count badges, and column width optimization.
 * - Provides interactive right-click context menu (Show Details, Center on Map, Copy Coordinates).
 */
class ComplexEntityTablePanelDialog : public GISApp::Core::Wrappers::BaseTablePanelDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs ComplexEntityTablePanelDialog with a ComplexEntityTableModel.
     * @param[in] model Pointer to ComplexEntityTableModel.
     * @param[in] parent Optional parent QWidget.
     */
    explicit ComplexEntityTablePanelDialog(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel *model,
                                           QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~ComplexEntityTablePanelDialog() override = default;

    /**
     * @brief Rebinds the active model.
     * @param[in] model Pointer to ComplexEntityTableModel.
     */
    void setEntityModel(GISApp::UIModels::ComplexEntities::ComplexEntityTableModel *model);

    /**
     * @brief Retrieves the active ComplexEntityTableModel.
     * @return Pointer to ComplexEntityTableModel.
     */
    [[nodiscard]] GISApp::UIModels::ComplexEntities::ComplexEntityTableModel* entityModel() const { return m_entityModel; }

    /**
     * @brief Updates status statistics badge based on current model rows.
     */
    void updateStatistics() override;

signals:
    /**
     * @brief Emitted when a complex entity row is clicked by the operator.
     * @param[in] entityId Unique 32-bit entity identifier.
     */
    void entitySelected(quint32 entityId);

    /**
     * @brief Emitted when a complex entity row is double-clicked.
     * @param[in] entityId Unique 32-bit entity identifier.
     */
    void entityDoubleClicked(quint32 entityId);

    /**
     * @brief Emitted when the user requests detailed properties of an entity.
     * @param[in] entityId Unique 32-bit entity identifier.
     */
    void requestEntityDetails(quint32 entityId);

    /**
     * @brief Emitted to center map camera on an entity's primary geodetic location.
     * @param[in] lat Latitude.
     * @param[in] lon Longitude.
     */
    void requestCenterOnEntity(double lat, double lon);

private slots:
    /**
     * @brief Spawns right-click context menu on table rows.
     * @param[in] pos Viewport click position.
     */
    void showTableContextMenu(const QPoint &pos);

protected:
    /**
     * @brief Hook called when an operator single-clicks a table row.
     * @param[in] sourceRow Row index in the source model.
     */
    void handleRowSelected(int sourceRow) override;

    /**
     * @brief Hook called when an operator double-clicks a table row.
     * @param[in] sourceRow Row index in the source model.
     */
    void handleRowDoubleClicked(int sourceRow) override;

private:
    /**
     * @brief Configures optimal column widths and resize modes for the table view.
     */
    void configureColumnWidths();

    /// Typed pointer to underlying source model
    GISApp::UIModels::ComplexEntities::ComplexEntityTableModel *m_entityModel{nullptr};
};

} // namespace GISApp::UI::ComplexEntities

#endif // COMPLEXENTITYTABLEPANELDIALOG_H

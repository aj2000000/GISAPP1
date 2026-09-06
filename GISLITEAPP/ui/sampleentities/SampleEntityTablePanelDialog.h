/**
 * @file SampleEntityTablePanelDialog.h
 * @brief Modeless tactical table panel dialog for SampleEntity management.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYTABLEPANELDIALOG_H
#define SAMPLEENTITYTABLEPANELDIALOG_H

#include "BaseTablePanelDialog.h"

namespace GISApp::UIModels::SampleEntities {
class SampleEntityTableModel;
}

namespace GISApp::UI::SampleEntities {

/**
 * @class SampleEntityTablePanelDialog
 * @brief Tactical dialog specialization of BaseTablePanelDialog for viewing and filtering sample entities.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the UI Presentation layer (`ui/sampleentities/`).
 * - Inherits from `GISApp::Core::Wrappers::BaseTablePanelDialog` using the Template Method pattern.
 * - Displays telemetry data from UDP Message ID 904 (`MAIN_LITE_SAMPLE_ENTITY_MSG_ID`).
 * - Features dynamic text searching, automatic status count badges, and column width optimization.
 */
class SampleEntityTablePanelDialog : public GISApp::Core::Wrappers::BaseTablePanelDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs SampleEntityTablePanelDialog with a SampleEntityTableModel.
     * @param[in] model Pointer to SampleEntityTableModel.
     * @param[in] parent Optional parent QWidget.
     */
    explicit SampleEntityTablePanelDialog(GISApp::UIModels::SampleEntities::SampleEntityTableModel *model,
                                          QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~SampleEntityTablePanelDialog() override = default;

    /**
     * @brief Rebinds the active model.
     * @param[in] model Pointer to SampleEntityTableModel.
     */
    void setEntityModel(GISApp::UIModels::SampleEntities::SampleEntityTableModel *model);

    /**
     * @brief Retrieves the active SampleEntityTableModel.
     * @return Pointer to SampleEntityTableModel.
     */
    [[nodiscard]] GISApp::UIModels::SampleEntities::SampleEntityTableModel* entityModel() const { return m_entityModel; }

    /**
     * @brief Updates status statistics badge based on current model rows.
     */
    void updateStatistics() override;

signals:
    /**
     * @brief Emitted when a sample entity row is clicked by the operator.
     * @param[in] entityId Unique integer identifier.
     */
    void entitySelected(int entityId);

    /**
     * @brief Emitted when a sample entity row is double-clicked.
     * @param[in] entityId Unique integer identifier.
     */
    void entityDoubleClicked(int entityId);

    /**
     * @brief Emitted when the user requests detailed properties of an entity.
     * @param[in] entityId Unique integer identifier.
     */
    void requestEntityDetails(int entityId);

    /**
     * @brief Emitted to center map camera on an entity's geodetic location.
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
    GISApp::UIModels::SampleEntities::SampleEntityTableModel *m_entityModel{nullptr};
};

} // namespace GISApp::UI::SampleEntities

#endif // SAMPLEENTITYTABLEPANELDIALOG_H

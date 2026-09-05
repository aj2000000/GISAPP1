/**
 * @file TrackTablePanelDialog.h
 * @brief Tactical track table panel dialog extending BaseTablePanelDialog.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef TRACKTABLEPANELDIALOG_H
#define TRACKTABLEPANELDIALOG_H

#include "BaseTablePanelDialog.h"

class QComboBox;

namespace GISApp::UIModels::Tracks {
class TrackTableModel;
}

namespace GISApp::UI::Tracks {

/**
 * @class TrackTablePanelDialog
 * @brief Tactical track specialization of BaseTablePanelDialog providing track-specific filters and navigation.
 *
 * Architectural Role & Responsibilities:
 * - Resides in the UI Presentation layer (`ui/tracks/`).
 * - Inherits from GISApp::Core::Wrappers::BaseTablePanelDialog and specializes for tactical tracks.
 * - Implements the Template Method hooks:
 *   - Injects Affiliation and Domain dropdown filters into the top toolbar.
 *   - Enforces tactical multi-criteria row filtering (matching callsign, track ID, remarks, affiliation, domain).
 *   - Formats the metrics status badge with breakdown counts (Hostile, Friendly, Neutral).
 *   - Dispatches geodetic coordinates to command map camera fly-to upon row selection or double-click.
 */
class TrackTablePanelDialog : public GISApp::Core::Wrappers::BaseTablePanelDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs TrackTablePanelDialog with a track table model and optional parent.
     * @param[in] model Pointer to TrackTableModel.
     * @param[in] parent Optional parent QWidget.
     */
    explicit TrackTablePanelDialog(GISApp::UIModels::Tracks::TrackTableModel *model, QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~TrackTablePanelDialog() override = default;

    /**
     * @brief Binds a specialized TrackTableModel.
     * @param[in] model Pointer to TrackTableModel.
     */
    void setTrackModel(GISApp::UIModels::Tracks::TrackTableModel *model);

    /**
     * @brief Retrieves the active TrackTableModel.
     * @return Pointer to TrackTableModel.
     */
    [[nodiscard]] GISApp::UIModels::Tracks::TrackTableModel* trackModel() const { return m_trackModel; }

signals:
    /**
     * @brief Emitted when an operator clicks or double-clicks a track to center the map camera.
     * @param[in] latitude Geodetic latitude in decimal degrees.
     * @param[in] longitude Geodetic longitude in decimal degrees.
     */
    void trackSelected(double latitude, double longitude);

public slots:
    /**
     * @brief Recalculates and formats track breakdown metrics on the status badge.
     */
    void updateStatistics() override;

    /**
     * @brief Triggers a reload of tactical track records from the repository.
     */
    void refreshData() override;

protected:
    /**
     * @brief Injects Affiliation and Domain filter combo boxes into the toolbar.
     * @param[in] customFilterLayout Horizontal layout provided by BaseTablePanelDialog.
     */
    void setupCustomFilters(QHBoxLayout *customFilterLayout) override;

    /**
     * @brief Filters rows based on Affiliation and Domain dropdown selections.
     * @param[in] sourceRow Row index in the source model.
     * @param[in] sourceParent Parent model index.
     * @return True if track matches current filter criteria.
     */
    bool customFilterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    /**
     * @brief Handles single click on a track row to emit trackSelected.
     * @param[in] sourceRow Source model row index.
     */
    void handleRowSelected(int sourceRow) override;

    /**
     * @brief Handles double click on a track row to emit trackSelected.
     * @param[in] sourceRow Source model row index.
     */
    void handleRowDoubleClicked(int sourceRow) override;

private slots:
    void onFilterDropdownChanged();

private:
    void configureColumnWidths();

    /// Specialized track model instance.
    GISApp::UIModels::Tracks::TrackTableModel *m_trackModel{nullptr};

    /// Affiliation classification dropdown filter.
    QComboBox *m_affiliationCombo{nullptr};

    /// Domain classification dropdown filter.
    QComboBox *m_domainCombo{nullptr};
};

} // namespace GISApp::UI::Tracks

#endif // TRACKTABLEPANELDIALOG_H

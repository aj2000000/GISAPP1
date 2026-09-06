/**
 * @file TrackRequestDialog.h
 * @brief Modal dialog for requesting tactical tracks from external systems over UDP.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TRACKREQUESTDIALOG_H
#define TRACKREQUESTDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

namespace GISApp::UI::Tracks {

/**
 * @class TrackRequestDialog
 * @brief User interface dialog capturing date/time parameters to query tactical tracks.
 *
 * Architectural Role & Interactions:
 * - Resides strictly in the UI Presentation layer (MVC).
 * - Invoked by `MainBaseUI::openTrackRequestDialog` when the operator selects
 *   `Request -> Tracks...` from the application menu bar.
 * - Provides interactive calendar popups, precision time spinners, quick-interval presets,
 *   and real-time validation to ensure the interval is temporally ordered (From <= To).
 * - Exposes getter methods (`fromDateTime()`, `toDateTime()`) consumed by `MainBaseUI`
 *   to construct the canonical `REQ_ENTITY_MESSAGE` (Message ID: 1501).
 * - Fully styled through `ThemeManager` without hardcoded stylesheets.
 */
class TrackRequestDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the TrackRequestDialog.
     * @param[in] parent Optional parent widget ownership.
     */
    explicit TrackRequestDialog(QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    ~TrackRequestDialog() override = default;

    /**
     * @brief Retrieves the user-selected starting timestamp.
     * @return QDateTime representing the beginning of the requested track interval.
     */
    [[nodiscard]] QDateTime fromDateTime() const;

    /**
     * @brief Retrieves the user-selected ending timestamp.
     * @return QDateTime representing the termination of the requested track interval.
     */
    [[nodiscard]] QDateTime toDateTime() const;

private slots:
    /**
     * @brief Validates the current date/time inputs and updates the UI state accordingly.
     * @note Enables or disables the Send button and displays an alert if From > To.
     */
    void validateRange();

    /**
     * @brief Sets the temporal query interval to the past 1 hour up to current time.
     */
    void setPresetPast1Hour();

    /**
     * @brief Sets the temporal query interval to the past 6 hours up to current time.
     */
    void setPresetPast6Hours();

    /**
     * @brief Sets the temporal query interval to the past 24 hours up to current time.
     */
    void setPresetPast24Hours();

    /**
     * @brief Sets the temporal query interval from midnight (00:00:00) today to current time.
     */
    void setPresetToday();

private:
    /**
     * @brief Assembles and configures all child widgets, layouts, and input components.
     */
    void setupUi();

    /**
     * @brief Binds widget signals to corresponding validation and preset slots.
     */
    void setupConnections();

    // =========================================================================
    // UI Member Variables
    // =========================================================================

    /// Editable date & time picker for the starting interval.
    QDateTimeEdit *m_fromDateTimeEdit{nullptr};

    /// Editable date & time picker for the ending interval.
    QDateTimeEdit *m_toDateTimeEdit{nullptr};

    /// Dynamic label displaying validation status or temporal ordering errors.
    QLabel *m_validationLabel{nullptr};

    /// Read-only label indicating the target UDP IP and port.
    QLabel *m_targetInfoLabel{nullptr};

    /// Button to set 1-hour interval preset.
    QPushButton *m_preset1HrBtn{nullptr};

    /// Button to set 6-hour interval preset.
    QPushButton *m_preset6HrBtn{nullptr};

    /// Button to set 24-hour interval preset.
    QPushButton *m_preset24HrBtn{nullptr};

    /// Button to set today's interval preset.
    QPushButton *m_presetTodayBtn{nullptr};

    /// Primary tactical button triggering request submission.
    QPushButton *m_sendButton{nullptr};

    /// Dismissal button closing the dialog without submitting.
    QPushButton *m_cancelButton{nullptr};
};

} // namespace GISApp::UI::Tracks

#endif // TRACKREQUESTDIALOG_H

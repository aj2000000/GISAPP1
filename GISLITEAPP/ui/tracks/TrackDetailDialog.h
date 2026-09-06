/**
 * @file TrackDetailDialog.h
 * @brief Modal dialog presenting canonical tactical track properties and real-time telemetry.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef TRACKDETAILDIALOG_H
#define TRACKDETAILDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include "TacticalTrack.h"

namespace GISApp::UI::Tracks {

/**
 * @class TrackDetailDialog
 * @brief Tactical inspector dialog displaying comprehensive canonical STRUCT_TRACK attributes.
 *
 * Architectural Role:
 * - Resides strictly in the UI Presentation layer (MVC).
 * - Serves as a decoupled view component instantiated and managed by TrackController.
 * - Presents the canonical wire telemetry (`STRUCT_TRACK`) of a single tactical track entity.
 * - Utilizes `FieldKeyValueMapper` to transform raw numeric enum codes into human-readable tactical descriptions.
 * - Receives live telemetry updates pushed by TrackController::onTrackUpdated without any direct coupling
 *   to database repositories or network handlers.
 */
class TrackDetailDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs TrackDetailDialog for a specific track entity.
     * @param[in] track Initial track domain entity.
     * @param[in] parent Optional parent widget.
     */
    explicit TrackDetailDialog(const GISApp::Domain::Tracks::TacticalTrack &track,
                               QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~TrackDetailDialog() override = default;

    /**
     * @brief Returns the identifier of the track being inspected.
     * @return Integer track ID.
     */
    [[nodiscard]] int trackId() const { return m_trackId; }

    /**
     * @brief Updates all visual fields to reflect a new track state.
     * @param[in] track Updated tactical track domain entity.
     */
    void updateTrackData(const GISApp::Domain::Tracks::TacticalTrack &track);

private:
    /**
     * @brief Builds the tactical dark UI layout, headers, property grids, and close buttons.
     */
    void setupUi();

    /**
     * @brief Helper to create a two-column styled key-value label pair.
     * @param[in] labelText Descriptive property key text.
     * @param[out] valueLabelPtr Pointer to the QLabel displaying the dynamic value.
     * @return Container widget or layout row.
     */
    QWidget* createPropertyRow(const QString &labelText, QLabel **valueLabelPtr);

    int m_trackId;                                      ///< Track identifier being inspected.
    GISApp::Domain::Tracks::TacticalTrack m_track;      ///< Current track domain model state.

    // UI Widgets
    QLabel *m_titleLabel{nullptr};
    QLabel *m_identityBadge{nullptr};
    QLabel *m_trackIdLabel{nullptr};
    QLabel *m_callsignLabel{nullptr};
    QLabel *m_symbolLabel{nullptr};

    // Location & Kinematics
    QLabel *m_latLabel{nullptr};
    QLabel *m_lonLabel{nullptr};
    QLabel *m_heightLabel{nullptr};
    QLabel *m_dirLabel{nullptr};

    // Attributes (FieldKeyValueMapper)
    QLabel *m_domainTypeLabel{nullptr};
    QLabel *m_subTypeLabel{nullptr};
    QLabel *m_classificationLabel{nullptr};
    QLabel *m_strengthLabel{nullptr};
    QLabel *m_actTypeLabel{nullptr};
    QLabel *m_actSubTypeLabel{nullptr};
    QLabel *m_actClassificationLabel{nullptr};

    // System & Sources
    QLabel *m_sysTypeLabel{nullptr};
    QLabel *m_sourcesLabel{nullptr};

    // Timing & Remarks
    QLabel *m_reportTimeLabel{nullptr};
    QLabel *m_remarksLabel{nullptr};

    QPushButton *m_closeButton{nullptr};
};

} // namespace GISApp::UI::Tracks

#endif // TRACKDETAILDIALOG_H

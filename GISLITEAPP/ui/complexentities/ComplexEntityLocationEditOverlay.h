/**
 * @file ComplexEntityLocationEditOverlay.h
 * @brief Header definition for ComplexEntityLocationEditOverlay floating tactical ribbon.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the **Presentation UI Layer** (`ui/complexentities/`).
 * - Implements the **Tactical HUD / Ribbon Overlay Pattern** floating directly over the
 *   map canvas (`MapViewContainer`) to allow interactive relocation and vertex editing of complex entities.
 * - Supports dual modes:
 *   1. **Move Entire Entity (Rigid Translation)**: shifts all constituent points by identical
 *      geodetic offset $(\Delta\text{lat}, \Delta\text{lon})$, preserving 100% relative geometry.
 *   2. **Edit Control Points (Geometry Modification)**: selects, moves, adds, or deletes individual
 *      control vertices $(P_1, P_2, \dots, P_n)$, reshaping the spline/boundary in real time.
 * - Integrates with @ref GISApp::Controllers::ComplexEntities::ComplexEntityController which
 *   captures map clicks and pushes live preview updates to @ref GISApp::Services::ComplexEntities::ComplexEntityService.
 */

#ifndef COMPLEXENTITYLOCATIONEDITOVERLAY_H
#define COMPLEXENTITYLOCATIONEDITOVERLAY_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ComplexEntity.h"

namespace GISApp::UI::ComplexEntities {

/**
 * @class ComplexEntityLocationEditOverlay
 * @brief Modeless floating tactical ribbon widget for translating complex entities or modifying control points.
 */
class ComplexEntityLocationEditOverlay : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief Editing mode enumeration.
     */
    enum class EditMode {
        MoveEntireEntity,   ///< Rigid translation preserving 100% relative geometry
        EditControlPoints   ///< Move, add, or delete individual vertices
    };
    Q_ENUM(EditMode)

    /**
     * @brief Constructs the ComplexEntityLocationEditOverlay ribbon widget.
     * @param[in] parent Optional parent widget (typically MapViewContainer).
     */
    explicit ComplexEntityLocationEditOverlay(QWidget *parent = nullptr);

    /**
     * @brief Virtual destructor ensuring clean deallocation of UI elements.
     */
    virtual ~ComplexEntityLocationEditOverlay() override = default;

    /**
     * @brief Activates the overlay ribbon for the specified ComplexEntity in the specified mode.
     * @param[in] entity The complex entity to edit.
     * @param[in] mode Initial editing mode (MoveEntireEntity or EditControlPoints).
     */
    void startEditing(const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
                      EditMode mode = EditMode::MoveEntireEntity);

    /**
     * @brief Closes and dismisses the ribbon overlay.
     */
    void stopEditing();

    /**
     * @brief Checks if the location editing ribbon is currently active.
     * @return True if actively editing an entity, false otherwise.
     */
    [[nodiscard]] bool isEditingActive() const { return m_active; }

    /**
     * @brief Retrieves the 32-bit identifier of the entity currently being edited.
     * @return Entity ID or 0 if inactive.
     */
    [[nodiscard]] quint32 entityId() const { return m_entityId; }

    /**
     * @brief Retrieves the active editing mode.
     * @return Current EditMode.
     */
    [[nodiscard]] EditMode editMode() const { return m_editMode; }

    /**
     * @brief Switches the active editing mode.
     * @param[in] mode Target EditMode.
     */
    void setEditMode(EditMode mode);

    /**
     * @brief Checks if the 'Click Map to Move / Place' interactive mode is enabled.
     * @return True if map click placement is active.
     */
    [[nodiscard]] bool isMapClickModeActive() const;

    /**
     * @brief Retrieves the active selected control point index (0-based).
     * @return Index of selected vertex.
     */
    [[nodiscard]] int selectedPointIndex() const { return m_selectedPointIndex; }

    /**
     * @brief Selects a specific control point by 0-based index.
     * @param[in] index Vertex index [0..points.size()-1].
     */
    void setSelectedPointIndex(int index);

    /**
     * @brief Synchronizes the ribbon with an updated entity (e.g. after point add/remove/move).
     * @param[in] entity Updated entity domain model.
     */
    void updateEntityData(const GISApp::Domain::ComplexEntities::ComplexEntity &entity);

    /**
     * @brief Computes and returns the cumulative translation delta in degrees (Move mode).
     * @param[out] outDeltaLat Cumulative latitude shift in decimal degrees.
     * @param[out] outDeltaLon Cumulative longitude shift in decimal degrees.
     */
    void getCumulativeDelta(double &outDeltaLat, double &outDeltaLon) const;

    /**
     * @brief Handles a map canvas coordinate click according to the active mode:
     * - MoveEntireEntity: computes offset to place entity anchor at clicked coordinate.
     * - EditControlPoints: updates the selected control vertex coordinate.
     * @param[in] clickedLat Target latitude clicked on map.
     * @param[in] clickedLon Target longitude clicked on map.
     */
    void handleMapCoordinateClicked(double clickedLat, double clickedLon);

    /**
     * @brief Saves the current overlay state onto the undo stack.
     * @note Public so the controller can snapshot before drag operations.
     */
    void saveUndoSnapshot();

signals:
    /**
     * @brief Emitted whenever the entity is shifted by an incremental delta in MoveEntireEntity mode.
     * @param[in] entityId The target entity ID.
     * @param[in] deltaLat Latitude shift in degrees to apply across all points.
     * @param[in] deltaLon Longitude shift in degrees to apply across all points.
     */
    void positionShifted(quint32 entityId, double deltaLat, double deltaLon);

    /**
     * @brief Emitted when an individual control point is modified in EditControlPoints mode.
     * @param[in] entityId The target entity ID.
     * @param[in] pointIndex 0-based vertex index.
     * @param[in] newLat New latitude in degrees.
     * @param[in] newLon New longitude in degrees.
     */
    void controlPointModified(quint32 entityId, int pointIndex, double newLat, double newLon);

    /**
     * @brief Emitted when operator requests inserting a new control point.
     * @param[in] entityId The target entity ID.
     * @param[in] afterIndex Insert after this vertex index (-1 for end).
     * @param[in] lat Latitude for new point.
     * @param[in] lon Longitude for new point.
     */
    void controlPointAdded(quint32 entityId, int afterIndex, double lat, double lon);

    /**
     * @brief Emitted when operator requests deleting a control point.
     * @param[in] entityId The target entity ID.
     * @param[in] pointIndex 0-based vertex index to remove.
     */
    void controlPointRemoved(quint32 entityId, int pointIndex);

    /**
     * @brief Emitted when an active control point index is selected in the ribbon.
     * @param[in] index 0-based vertex index.
     */
    void controlPointSelected(int index);

    /**
     * @brief Emitted when editing mode changes.
     * @param[in] mode New active EditMode.
     */
    void editModeChanged(EditMode mode);

    /**
     * @brief Emitted when the operator confirms and commits the edits.
     * @param[in] entityId The target entity ID to save.
     */
    void saveRequested(quint32 entityId);

    /**
     * @brief Emitted when the operator requests resetting the entity to its original state.
     * @param[in] entityId The target entity ID.
     */
    void resetRequested(quint32 entityId);

    /**
     * @brief Emitted when the operator cancels editing, reverting all changes.
     * @param[in] entityId The target entity ID.
     */
    void cancelRequested(quint32 entityId);

    /**
     * @brief Emitted when the operator triggers an undo action to revert the last edit.
     * @param[in] entityId The target entity ID.
     */
    void undoRequested(quint32 entityId);

    /**
     * @brief Emitted after a rotation or bulk operation modifies all control points at once.
     * @param[in] entityId The target entity ID.
     * @param[in] newPoints Complete updated point set after the bulk modification.
     */
    void allPointsModified(quint32 entityId, const QVector<STRUCT_LOCATION> &newPoints);

private slots:
    void onModeMoveToggled(bool checked);
    void onModePointsToggled(bool checked);
    void onPointComboChanged(int index);
    void onPrevPointClicked();
    void onNextPointClicked();
    void onAddPointClicked();
    void onRemovePointClicked();
    void onNudgeNorth();
    void onNudgeSouth();
    void onNudgeEast();
    void onNudgeWest();
    void onResetClicked();
    void onSaveClicked();
    void onCancelClicked();
    void onUndoClicked();
    void onRotateCW();
    void onRotateCCW();

private:
    void setupUi();
    void updateTelemetryDisplay();
    void rebuildPointCombo();
    [[nodiscard]] double currentStepMeters() const;
    void applyIncrementalShift(double deltaLat, double deltaLon);
    void updateUndoBtnState();
    void applyRotation(double angleDegrees);
    [[nodiscard]] double currentRotationStepDegrees() const;

    // --- State Variables ---
    bool m_active{false};                                           ///< True when ribbon is actively editing
    EditMode m_editMode{EditMode::MoveEntireEntity};                ///< Active editing mode
    quint32 m_entityId{0};                                          ///< Identifier of entity being edited
    UINT_8 m_entityType{1};                                         ///< Visual discriminator code
    QString m_entityName;                                           ///< Designation name
    QVector<STRUCT_LOCATION> m_currentPoints;                       ///< Cached current points
    int m_selectedPointIndex{0};                                    ///< Active vertex index in EditControlPoints mode
    double m_originalAnchorLat{0.0};                                ///< Initial reference anchor latitude
    double m_originalAnchorLon{0.0};                                ///< Initial reference anchor longitude
    double m_currentAnchorLat{0.0};                                 ///< Current reference anchor latitude
    double m_currentAnchorLon{0.0};                                 ///< Current reference anchor longitude
    double m_cumulativeDeltaLat{0.0};                               ///< Total latitude translation applied
    double m_cumulativeDeltaLon{0.0};                               ///< Total longitude translation applied

    // --- Undo Stack ---
    /** @brief Snapshot of overlay state for undo operations. */
    struct UndoSnapshot {
        QVector<STRUCT_LOCATION> points;    ///< Full copy of all control points at snapshot time
        int selectedPointIndex;              ///< Active vertex index at snapshot time
        double cumulativeDeltaLat;           ///< Cumulative latitude shift at snapshot time
        double cumulativeDeltaLon;           ///< Cumulative longitude shift at snapshot time
        double currentAnchorLat;             ///< Anchor latitude at snapshot time
        double currentAnchorLon;             ///< Anchor longitude at snapshot time
    };
    QVector<UndoSnapshot> m_undoStack;       ///< LIFO stack of state snapshots for undo
    static constexpr int MAX_UNDO_DEPTH = 50; ///< Maximum undo history depth

    // --- UI Controls ---
    QLabel *m_titleLabel{nullptr};                                  ///< Entity designation & ID badge
    QLabel *m_typeBadge{nullptr};                                   ///< Pill badge showing entity type name
    QToolButton *m_modeMoveBtn{nullptr};                            ///< Switch to Move Entire Entity mode
    QToolButton *m_modePointsBtn{nullptr};                          ///< Switch to Edit Control Points mode
    QWidget *m_pointsWidgetContainer{nullptr};                      ///< Container for point selection & add/del tools
    QComboBox *m_pointCombo{nullptr};                               ///< Point selector combo
    QPushButton *m_prevPtBtn{nullptr};                              ///< Select previous point
    QPushButton *m_nextPtBtn{nullptr};                              ///< Select next point
    QPushButton *m_addPtBtn{nullptr};                               ///< Add control point
    QPushButton *m_removePtBtn{nullptr};                            ///< Remove control point
    QLabel *m_anchorLabel{nullptr};                                 ///< Active anchor or point coordinates
    QLabel *m_deltaLabel{nullptr};                                  ///< Offset readout or point index readout
    QToolButton *m_mapClickBtn{nullptr};                            ///< Toggle for interactive map canvas placement
    QComboBox *m_stepCombo{nullptr};                                ///< Metric nudge increment selector
    QPushButton *m_northBtn{nullptr};                               ///< Nudge North
    QPushButton *m_southBtn{nullptr};                               ///< Nudge South
    QPushButton *m_westBtn{nullptr};                                ///< Nudge West
    QPushButton *m_eastBtn{nullptr};                                ///< Nudge East
    QPushButton *m_undoBtn{nullptr};                                ///< Undo last editing operation
    QPushButton *m_resetBtn{nullptr};                               ///< Reset to origin
    QPushButton *m_saveBtn{nullptr};                                ///< Commit changes to repository
    QPushButton *m_cancelBtn{nullptr};                              ///< Discard changes and close
    QComboBox *m_rotStepCombo{nullptr};                              ///< Rotation angle step selector (degrees)
    QPushButton *m_cwBtn{nullptr};                                   ///< Rotate clockwise around centroid
    QPushButton *m_ccwBtn{nullptr};                                  ///< Rotate counter-clockwise around centroid
};

} // namespace GISApp::UI::ComplexEntities

#endif // COMPLEXENTITYLOCATIONEDITOVERLAY_H

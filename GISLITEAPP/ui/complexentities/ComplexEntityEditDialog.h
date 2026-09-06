/**
 * @file ComplexEntityEditDialog.h
 * @brief Modeless tactical editing dialog for modifying ComplexEntity attributes, symbology, and coordinates.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYEDITDIALOG_H
#define COMPLEXENTITYEDITDIALOG_H

#include <QDialog>
#include <QVector>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QToolButton>
#include <QTableWidget>
#include <QGroupBox>

#include "ComplexEntity.h"

namespace GISApp::UI::ComplexEntities {

/**
 * @class ComplexEntityEditDialog
 * @brief Floating tactical overlay editor for ComplexEntity domain objects.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the **Presentation UI Layer** (`ui/complexentities/`).
 * - Implements the **View / Editor Dialog Pattern** allowing the operator to adjust
 *   telemetry, annotations, echelons, line styles, and location points.
 * - Dynamically adapts visible sections according to the chosen entity type:
 *   - Type 7 (Formation Boundary): Flank annotations, Echelon, Line style, Allegiance.
 *   - Type 8 (Deployment Area): Echelon, Line style (blank annotations & SP3/4 per doctrine).
 *   - Types 1-6: Standard 4-way annotations and operational parameters.
 * - Supports interactive map coordinate picking via @ref pickCoordinateRequested signal.
 * - Communicates saves reactively via @ref entitySaved signal.
 */
class ComplexEntityEditDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs ComplexEntityEditDialog populated with existing entity data.
     * @param[in] entity ComplexEntity instance to edit.
     * @param[in] parent Optional parent widget (default: nullptr for modeless floating overlay).
     */
    explicit ComplexEntityEditDialog(const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
                                     QWidget *parent = nullptr);

    /**
     * @brief Destructor releasing UI resources.
     */
    virtual ~ComplexEntityEditDialog() override;

    /**
     * @brief Retrieves the active entity identifier being edited.
     * @return 32-bit unsigned entity ID.
     */
    [[nodiscard]] quint32 entityId() const { return m_entityId; }

    /**
     * @brief Updates dialog contents with fresh entity data from live telemetry.
     * @param[in] entity Updated domain model instance.
     */
    void updateEntityData(const GISApp::Domain::ComplexEntities::ComplexEntity &entity);

    /**
     * @brief Injects a picked coordinate from map canvas into the points table.
     * @param[in] latitude Geodetic latitude [-90.0, 90.0].
     * @param[in] longitude Geodetic longitude [-180.0, 180.0].
     */
    void addPickedCoordinate(double latitude, double longitude);

    /**
     * @brief Checks if map coordinate picking mode is currently active.
     * @return True if pick button is toggled active, false otherwise.
     */
    [[nodiscard]] bool isPickModeActive() const;

signals:
    /**
     * @brief Emitted when the operator confirms edits by clicking 'Save Changes'.
     * @param[in] updatedEntity Modified ComplexEntity domain model with validated fields.
     */
    void entitySaved(const GISApp::Domain::ComplexEntities::ComplexEntity &updatedEntity);

    /**
     * @brief Emitted when 'Pick from Map' toggle state changes.
     * @param[in] active True if picking is enabled, false if disabled.
     */
    void pickCoordinateRequested(bool active);

    /**
     * @brief Emitted when operator requests moving the entire entity on the map overlay ribbon.
     * @param[in] entityId Target entity ID.
     */
    void moveOnOverlayRequested(quint32 entityId);

    /**
     * @brief Emitted when operator requests editing individual control points on the map overlay ribbon.
     * @param[in] entityId Target entity ID.
     */
    void editControlPointsRequested(quint32 entityId);

    /**
     * @brief Emitted when user requests centering the camera on the edited entity.
     * @param[in] lat Target center latitude.
     * @param[in] lon Target center longitude.
     */
    void centerRequested(double lat, double lon);

public slots:
    /**
     * @brief Handles external map coordinate click during active picking mode.
     * @param[in] lat Clicked latitude.
     * @param[in] lon Clicked longitude.
     */
    void onMapCoordinateClicked(double lat, double lon);

private slots:
    void onEntityTypeChanged(int index);
    void onAddPointClicked();
    void onRemovePointClicked();
    void onMoveUpPointClicked();
    void onMoveDownPointClicked();
    void onPickModeToggled(bool checked);
    void onSaveClicked();
    void onCenterClicked();

private:
    void setupUi();
    void populateFields();
    void updateAdaptiveSections(int entityType);
    void loadPointsTable(const QVector<STRUCT_LOCATION> &points);
    [[nodiscard]] QVector<STRUCT_LOCATION> collectPointsFromTable() const;

    quint32 m_entityId{0};                                          ///< Entity identifier
    GISApp::Domain::ComplexEntities::ComplexEntity m_originalEntity; ///< Original domain snapshot

    // UI Widgets - Header
    QLabel *m_titleLabel{nullptr};                                  ///< Header title with entity ID & name
    QLabel *m_typeBadge{nullptr};                                   ///< Type classification badge

    // UI Widgets - General Info
    QLineEdit *m_nameEdit{nullptr};                                 ///< Designated name input
    QComboBox *m_typeCombo{nullptr};                                ///< Entity type selector
    QLineEdit *m_remarksEdit{nullptr};                              ///< Operational remarks input

    // UI Widgets - Annotations Section
    QGroupBox *m_annotGroup{nullptr};                               ///< Annotations group container
    QWidget   *m_leftAnnotContainer{nullptr};                       ///< Left / Top flank container
    QWidget   *m_rightAnnotContainer{nullptr};                      ///< Right / Bottom flank container
    QWidget   *m_topAnnotContainer{nullptr};                        ///< Top annotation container
    QWidget   *m_bottomAnnotContainer{nullptr};                     ///< Bottom annotation container
    QLabel    *m_leftAnnotLabel{nullptr};                           ///< Label for left annotation
    QLabel    *m_rightAnnotLabel{nullptr};                          ///< Label for right annotation
    QLineEdit *m_leftAnnotEdit{nullptr};                            ///< Left annotation input
    QLineEdit *m_rightAnnotEdit{nullptr};                           ///< Right annotation input
    QLineEdit *m_topAnnotEdit{nullptr};                             ///< Top annotation input
    QLineEdit *m_bottomAnnotEdit{nullptr};                          ///< Bottom annotation input

    // UI Widgets - Parameters Section
    QGroupBox *m_paramGroup{nullptr};                               ///< Parameters group container
    QWidget   *m_sp1Container{nullptr};                             ///< Special param 1 container
    QWidget   *m_sp2Container{nullptr};                             ///< Special param 2 container
    QWidget   *m_sp3Container{nullptr};                             ///< Special param 3 container
    QWidget   *m_sp4Container{nullptr};                             ///< Special param 4 container
    QLabel    *m_sp1Label{nullptr};                                 ///< Param 1 label
    QLabel    *m_sp2Label{nullptr};                                 ///< Param 2 label
    QLabel    *m_sp3Label{nullptr};                                 ///< Param 3 label
    QLabel    *m_sp4Label{nullptr};                                 ///< Param 4 label

    // Tactical Combos for SP1/SP2/SP3
    QComboBox *m_echelonCombo{nullptr};                             ///< Echelon selector (for Type 7 & 8)
    QComboBox *m_lineStyleCombo{nullptr};                           ///< Line style selector (for Type 7 & 8)
    QComboBox *m_allegianceCombo{nullptr};                          ///< Allegiance selector (for Type 7)
    QSpinBox  *m_sp1Spin{nullptr};                                  ///< Generic SP1 spinbox (Types 1-6)
    QSpinBox  *m_sp2Spin{nullptr};                                  ///< Generic SP2 spinbox (Types 1-6)
    QSpinBox  *m_sp3Spin{nullptr};                                  ///< Generic SP3 spinbox (Types 1-6)
    QSpinBox  *m_sp4Spin{nullptr};                                  ///< Generic SP4 spinbox (Types 1-6)

    // UI Widgets - Points Table & Toolbar
    QGroupBox    *m_pointsGroup{nullptr};                           ///< Points group container
    QTableWidget *m_pointsTable{nullptr};                           ///< Editable location points table
    QPushButton  *m_addPointBtn{nullptr};                           ///< Add point button
    QPushButton  *m_removePointBtn{nullptr};                        ///< Remove point button
    QPushButton  *m_moveUpBtn{nullptr};                             ///< Move point up button
    QPushButton  *m_moveDownBtn{nullptr};                           ///< Move point down button
    QToolButton  *m_pickCoordBtn{nullptr};                          ///< Pick from map canvas toggle button

    // UI Widgets - Action Footer
    QPushButton *m_centerBtn{nullptr};                              ///< Center camera button
    QPushButton *m_saveBtn{nullptr};                                ///< Commit edits button
    QPushButton *m_cancelBtn{nullptr};                              ///< Discard edits button
};

} // namespace GISApp::UI::ComplexEntities

#endif // COMPLEXENTITYEDITDIALOG_H

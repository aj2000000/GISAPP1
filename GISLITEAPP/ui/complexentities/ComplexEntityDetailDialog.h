/**
 * @file ComplexEntityDetailDialog.h
 * @brief Modal dialog presenting comprehensive canonical complex entity properties, spatial geometry, and telemetry.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYDETAILDIALOG_H
#define COMPLEXENTITYDETAILDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTableWidget>

#include "ComplexEntity.h"

namespace GISApp::UI::ComplexEntities {

/**
 * @class ComplexEntityDetailDialog
 * @brief Tactical inspector dialog displaying comprehensive canonical STRUCT_COMPLEX_ENTITY attributes.
 *
 * Architectural Role & Responsibilities:
 * - Resides strictly in the UI Presentation layer (MVC pattern).
 * - Serves as an interactive inspector view instantiated and managed by `ComplexEntityController`.
 * - Displays all canonical fields of `STRUCT_COMPLEX_ENTITY` (Message ID 905):
 *   - Core identification and visual classification (Types 1–7: Point, Line, Polygon, Text Only, Custom Image, Custom Painter, Formation Boundary).
 *   - Primary geodetic location and calculated arithmetic centroid / bounding box coordinates.
 *   - Altitude / height and bearing / direction.
 *   - 4-way spatial annotations (Top, Bottom, Left, Right).
 *   - Special operational parameters 1 through 4 with domain-specific contextual decoding (e.g. Echelon indicators).
 *   - Geodetic location points table (`QVector<STRUCT_LOCATION>`) with proportional column layout.
 *   - Dynamic entity details key-value table (`STRUCT_DETAILS`).
 *   - Temporal report timestamp and operational remarks.
 * - Dynamically adapts table heights and column widths to prevent visual clumping and empty space.
 * - Receives live telemetry updates pushed via updateEntityData() without direct database coupling.
 * - Centralizes all visual styling via ThemeManager object names.
 */
class ComplexEntityDetailDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs ComplexEntityDetailDialog for a specific complex entity.
     * @param[in] entity Initial entity domain object.
     * @param[in] parent Optional parent widget.
     */
    explicit ComplexEntityDetailDialog(const GISApp::Domain::ComplexEntities::ComplexEntity &entity,
                                       QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~ComplexEntityDetailDialog() override = default;

    /**
     * @brief Returns the numerical identifier of the entity being inspected.
     * @return 32-bit unsigned entity identifier.
     */
    [[nodiscard]] quint32 entityId() const { return m_entityId; }

    /**
     * @brief Updates all visual fields to reflect fresh entity state in real-time.
     * @param[in] entity Updated complex entity domain model.
     */
    void updateEntityData(const GISApp::Domain::ComplexEntities::ComplexEntity &entity);

signals:
    /**
     * @brief Emitted when user requests centering the camera on this entity.
     * @param[in] lat Target center latitude in decimal degrees.
     * @param[in] lon Target center longitude in decimal degrees.
     */
    void centerRequested(double lat, double lon);

    /**
     * @brief Emitted when operator clicks 'Edit Entity' to open the editing overlay.
     * @param[in] entityId Numerical entity ID to edit.
     */
    void editRequested(quint32 entityId);

private slots:
    /**
     * @brief Handles click on the 'Center on Map' action button.
     */
    void onCenterClicked();

    /**
     * @brief Handles click on the 'Copy Summary' action button.
     */
    void onCopySummaryClicked();

    /**
     * @brief Handles click on the 'Edit Entity' action button.
     */
    void onEditClicked();

private:
    /**
     * @brief Builds layout, property grids, dynamic tables, headers, and action buttons.
     */
    void setupUi();

    /**
     * @brief Helper to construct styled key-value label rows.
     * @param[in] labelText Descriptive title.
     * @param[out] valueLabelPtr Output pointer to dynamic value label.
     * @return Container widget.
     */
    QWidget* createPropertyRow(const QString &labelText, QLabel **valueLabelPtr);

    /**
     * @brief Adjusts table height dynamically based on its row count to eliminate empty vertical space.
     * @param[in,out] table Target table widget.
     * @param[in] minHeight Minimum permissible height in pixels.
     * @param[in] maxHeight Maximum permissible height in pixels before scrolling.
     */
    static void adjustTableHeight(QTableWidget *table, int minHeight, int maxHeight);

    quint32 m_entityId{0};                                          ///< Entity identifier
    GISApp::Domain::ComplexEntities::ComplexEntity m_entity;       ///< Current domain model

    // UI Widgets - Header
    QLabel *m_titleLabel{nullptr};                                  ///< Header title with icon and designated name
    QLabel *m_typeBadge{nullptr};                                   ///< Visual type status badge

    // UI Widgets - Identification & Metrics
    QLabel *m_idLabel{nullptr};                                     ///< Numerical ID label
    QLabel *m_nameLabel{nullptr};                                   ///< Designated name label
    QLabel *m_typeDescLabel{nullptr};                               ///< Descriptive visual type label
    QLabel *m_primaryLocLabel{nullptr};                             ///< Primary location coordinate (Lat, Lon)
    QLabel *m_centroidLabel{nullptr};                               ///< Computed centroid coordinate (Lat, Lon)
    QLabel *m_primaryAltLabel{nullptr};                             ///< Primary altitude / height MSL
    QLabel *m_primaryDirLabel{nullptr};                             ///< Primary heading / bearing
    QLabel *m_pointsCountLabel{nullptr};                            ///< Number of location points label
    QLabel *m_detailsCountLabel{nullptr};                           ///< Number of details label
    QLabel *m_updatedLabel{nullptr};                                ///< Last updated timestamp label
    QLabel *m_remarksLabel{nullptr};                                ///< Operational remarks label

    // UI Widgets - 4-Way Spatial Annotations
    QGroupBox *m_annotGroup{nullptr};                               ///< Annotations group box
    QWidget *m_topAnnotContainer{nullptr};                          ///< Top annotation container
    QWidget *m_bottomAnnotContainer{nullptr};                       ///< Bottom annotation container
    QWidget *m_leftAnnotContainer{nullptr};                         ///< Left annotation container
    QWidget *m_rightAnnotContainer{nullptr};                        ///< Right annotation container
    QLabel *m_topAnnotationLabel{nullptr};                          ///< Top spatial annotation
    QLabel *m_bottomAnnotationLabel{nullptr};                       ///< Bottom spatial annotation
    QLabel *m_leftAnnotationLabel{nullptr};                         ///< Left spatial annotation
    QLabel *m_rightAnnotationLabel{nullptr};                        ///< Right spatial annotation

    // UI Widgets - Special Parameters
    QGroupBox *m_paramGroup{nullptr};                               ///< Special parameters group box
    QWidget *m_param1Container{nullptr};                            ///< Param 1 container
    QWidget *m_param2Container{nullptr};                            ///< Param 2 container
    QWidget *m_param3Container{nullptr};                            ///< Param 3 container
    QWidget *m_param4Container{nullptr};                            ///< Param 4 container
    QLabel *m_param1Label{nullptr};                                 ///< Special parameter 1
    QLabel *m_param2Label{nullptr};                                 ///< Special parameter 2
    QLabel *m_param3Label{nullptr};                                 ///< Special parameter 3
    QLabel *m_param4Label{nullptr};                                 ///< Special parameter 4

    // UI Widgets - Location Points Table
    QGroupBox *m_pointsGroup{nullptr};                              ///< Points group box
    QTableWidget *m_pointsTable{nullptr};                           ///< Geodetic location points table

    // UI Widgets - Dynamic Details Table
    QGroupBox *m_detailsGroup{nullptr};                             ///< Details group box
    QTableWidget *m_detailsTable{nullptr};                          ///< Key-Value STRUCT_DETAILS table

    // UI Widgets - Actions
    QPushButton *m_centerBtn{nullptr};                              ///< Center camera button
    QPushButton *m_copyBtn{nullptr};                                ///< Copy summary button
    QPushButton *m_closeBtn{nullptr};                               ///< Dialog close button
};

} // namespace GISApp::UI::ComplexEntities

#endif // COMPLEXENTITYDETAILDIALOG_H

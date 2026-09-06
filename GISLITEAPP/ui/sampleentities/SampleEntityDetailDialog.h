/**
 * @file SampleEntityDetailDialog.h
 * @brief Modal dialog presenting canonical sample entity properties and real-time telemetry.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYDETAILDIALOG_H
#define SAMPLEENTITYDETAILDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

#include "sampleentity.h"

namespace GISApp::UI::SampleEntities {

/**
 * @class SampleEntityDetailDialog
 * @brief Tactical inspector dialog displaying comprehensive canonical STRUCT_SAMPLE_ENTITY attributes.
 *
 * Architectural Role:
 * - Resides strictly in the UI Presentation layer (MVC).
 * - Displays telemetry data of a single SampleEntity (Message ID 904).
 * - Receives live updates pushed via updateEntityData() without direct database coupling.
 * - Styled consistently with application theme via ThemeManager.
 */
class SampleEntityDetailDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs SampleEntityDetailDialog for a specific sample entity.
     * @param[in] entity Initial entity domain object.
     * @param[in] parent Optional parent widget.
     */
    explicit SampleEntityDetailDialog(const GISApp::Domain::SampleEntities::SampleEntity &entity,
                                      QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~SampleEntityDetailDialog() override = default;

    /**
     * @brief Returns the identifier of the entity being inspected.
     * @return Integer entity ID.
     */
    [[nodiscard]] int entityId() const { return m_entityId; }

    /**
     * @brief Updates all visual fields to reflect fresh entity state in real-time.
     * @param[in] entity Updated sample entity domain model.
     */
    void updateEntityData(const GISApp::Domain::SampleEntities::SampleEntity &entity);

private:
    /**
     * @brief Builds layout, property grids, headers, and close button.
     */
    void setupUi();

    /**
     * @brief Helper to construct styled key-value label rows.
     * @param[in] labelText Descriptive title.
     * @param[out] valueLabelPtr Output pointer to dynamic value label.
     * @return Container widget.
     */
    QWidget* createPropertyRow(const QString &labelText, QLabel **valueLabelPtr);

    int m_entityId;                                              ///< Entity identifier
    GISApp::Domain::SampleEntities::SampleEntity m_entity;      ///< Current domain model

    // UI Widgets
    QLabel *m_titleLabel{nullptr};
    QLabel *m_typeBadge{nullptr};

    QLabel *m_idLabel{nullptr};
    QLabel *m_nameLabel{nullptr};
    QLabel *m_typeDescLabel{nullptr};

    QLabel *m_latLabel{nullptr};
    QLabel *m_lonLabel{nullptr};
    QLabel *m_heightLabel{nullptr};
    QLabel *m_dirLabel{nullptr};

    QLabel *m_reportTimeLabel{nullptr};
    QLabel *m_remarksLabel{nullptr};
};

} // namespace GISApp::UI::SampleEntities

#endif // SAMPLEENTITYDETAILDIALOG_H

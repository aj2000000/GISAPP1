/**
 * @file ComplexEntityTableModel.h
 * @brief Table model for rendering Complex Entities using Qt Model-View Architecture.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYTABLEMODEL_H
#define COMPLEXENTITYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <optional>

#include "ComplexEntity.h"
#include "IComplexEntityRepository.h"

namespace GISApp::UIModels::ComplexEntities {

/**
 * @class ComplexEntityTableModel
 * @brief High-performance table model displaying complex entities received via UDP Message ID 905.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the **UI Models Layer** (`src/ui_models/complexentities/`).
 * - Implements Qt Model-View Architecture (`QAbstractTableModel`).
 * - Subscribes to `IComplexEntityRepository` signals (`complexEntitiesUpdated`, `complexEntityUpserted`,
 *   `complexEntityRemoved`) to keep table views automatically synchronized.
 */
class ComplexEntityTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColumnId = 0,          ///< Unique entity identifier (Id)
        ColumnName,            ///< Entity callsign/name (Name)
        ColumnType,            ///< Entity type (Point, Line, Polygon, etc.)
        ColumnPointsCount,     ///< Number of coordinate points
        ColumnPrimaryCoords,   ///< Primary/centroid coordinates
        ColumnAnnotations,     ///< Top/Bottom/Left/Right summary
        ColumnSpecialParams,   ///< SP1..SP4 summary
        ColumnDetailsCount,    ///< Number of dynamic key-value details
        ColumnReportTime,      ///< Detection/report timestamp
        ColumnRemarks,         ///< Operational remarks
        ColumnCount            ///< Total column count
    };

    enum ComplexEntityModelRoles {
        EntityIdRole = Qt::UserRole + 1,  ///< Raw integer entity ID
        RawLatitudeRole,                  ///< Primary latitude
        RawLongitudeRole                  ///< Primary longitude
    };

    explicit ComplexEntityTableModel(GISApp::Repositories::IComplexEntityRepository *repo = nullptr,
                                     QObject *parent = nullptr);
    virtual ~ComplexEntityTableModel() override = default;

    void setRepository(GISApp::Repositories::IComplexEntityRepository *repo);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    [[nodiscard]] std::optional<GISApp::Domain::ComplexEntities::ComplexEntity> entityAt(int row) const;
    [[nodiscard]] GISApp::Domain::ComplexEntities::ComplexEntity getEntityAt(int row) const {
        auto opt = entityAt(row);
        return opt.has_value() ? opt.value() : GISApp::Domain::ComplexEntities::ComplexEntity{};
    }
    [[nodiscard]] int rowForEntityId(int entityId) const;

public slots:
    void reloadData();

private:
    GISApp::Repositories::IComplexEntityRepository *m_repo{nullptr};
    QVector<GISApp::Domain::ComplexEntities::ComplexEntity> m_entities;
};

} // namespace GISApp::UIModels::ComplexEntities

#endif // COMPLEXENTITYTABLEMODEL_H

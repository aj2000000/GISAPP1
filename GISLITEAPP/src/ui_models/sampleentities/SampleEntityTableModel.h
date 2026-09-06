/**
 * @file SampleEntityTableModel.h
 * @brief Table model for rendering sample entities using Qt Model-View Architecture.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYTABLEMODEL_H
#define SAMPLEENTITYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QColor>
#include <QFont>
#include <optional>

#include "sampleentity.h"
#include "ISampleEntityRepository.h"

namespace GISApp::UIModels::SampleEntities {

/**
 * @class SampleEntityTableModel
 * @brief High-performance table model displaying sample entities received via UDP Message ID 904.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the UI Models layer (`src/ui_models/sampleentities/`).
 * - Implements the **Qt Model-View Architecture** (`QAbstractTableModel`).
 * - Connects directly to `ISampleEntityRepository` signals (`sampleEntitiesUpdated`,
 *   `sampleEntityUpserted`, `sampleEntityRemoved`) to automatically keep table views synchronized
 *   without polling or thread contention.
 */
class SampleEntityTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Logical columns represented by the model reflecting SampleEntity attributes.
     */
    enum Column {
        ColumnId = 0,          ///< Unique entity identifier (Id)
        ColumnName,            ///< Entity callsign/name (Name)
        ColumnType,            ///< Entity type identifier (type)
        ColumnLatitude,        ///< Geodetic latitude in decimal degrees
        ColumnLongitude,       ///< Geodetic longitude in decimal degrees
        ColumnHeight,          ///< Elevation / altitude above MSL in meters
        ColumnDirection,       ///< Bearing / heading direction in degrees
        ColumnReportTime,      ///< Timestamp of the most recent telemetry packet
        ColumnRemarks,         ///< Operational remarks or free-text notes
        ColumnCount            ///< Total number of columns
    };

    /**
     * @brief Custom user roles for programmatic extraction of raw, unformatted values.
     */
    enum SampleEntityModelRoles {
        EntityIdRole = Qt::UserRole + 1,  ///< Raw integer entity ID
        RawLatitudeRole,                  ///< Raw double latitude in degrees
        RawLongitudeRole,                 ///< Raw double longitude in degrees
        RawHeightRole,                    ///< Raw double height in meters
        RawDirectionRole,                 ///< Raw double direction in degrees
        EntityObjectRole                  ///< Full SampleEntity instance via QVariant
    };

    /**
     * @brief Constructs SampleEntityTableModel with an optional entity repository.
     * @param[in] repo Optional pointer to ISampleEntityRepository.
     * @param[in] parent Optional parent QObject for lifecycle management.
     */
    explicit SampleEntityTableModel(GISApp::Repositories::ISampleEntityRepository *repo = nullptr,
                                    QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~SampleEntityTableModel() override = default;

    /**
     * @brief Attaches a new repository instance to the model, rebinding change signals.
     * @param[in] repo Pointer to ISampleEntityRepository.
     */
    void setRepository(GISApp::Repositories::ISampleEntityRepository *repo);

    /**
     * @brief Provides access to the currently bound repository.
     * @return Pointer to active ISampleEntityRepository.
     */
    [[nodiscard]] GISApp::Repositories::ISampleEntityRepository* repository() const { return m_repo; }

    /**
     * @brief Returns the total row count corresponding to active entities.
     * @param[in] parent Parent index for hierarchical models (unused in flat table).
     * @return Number of cached entities.
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns the fixed column count.
     * @param[in] parent Parent index (unused in flat table).
     * @return ColumnCount enum value.
     */
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Retrieves tabular display values and decoration roles.
     * @param[in] index Model index identifying row and column.
     * @param[in] role Qt display or custom user role.
     * @return Formatted QVariant value.
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Provides column title labels and alignment roles for horizontal headers.
     * @param[in] section Column or row index.
     * @param[in] orientation Horizontal or vertical header orientation.
     * @param[in] role Display or text alignment role.
     * @return Header title string or alignment flag.
     */
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    /**
     * @brief Retrieves the domain SampleEntity for a given table row.
     * @param[in] row Zero-based row index.
     * @return SampleEntity value object.
     */
    [[nodiscard]] GISApp::Domain::SampleEntities::SampleEntity getEntityAt(int row) const;

    /**
     * @brief Finds the table row corresponding to a given entity ID.
     * @param[in] entityId Unique integer identifier.
     * @return Row index if present, or -1 if not found.
     */
    [[nodiscard]] int findRowByEntityId(int entityId) const;

public slots:
    /**
     * @brief Synchronously refreshes the internal cache from the repository.
     */
    void reloadEntities();

private slots:
    /**
     * @brief Slot triggered when the repository signals batch entity changes.
     */
    void onEntitiesUpdated();

private:
    /**
     * @brief Connects change notification signals from the active repository.
     */
    void setupRepositoryConnections();

    /// Pointer to the underlying sample entity repository
    GISApp::Repositories::ISampleEntityRepository *m_repo{nullptr};

    /// Cached list of sample entities for high-performance table view queries
    QVector<GISApp::Domain::SampleEntities::SampleEntity> m_entities;
};

} // namespace GISApp::UIModels::SampleEntities

#endif // SAMPLEENTITYTABLEMODEL_H

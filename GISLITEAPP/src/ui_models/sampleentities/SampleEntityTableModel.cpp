/**
 * @file SampleEntityTableModel.cpp
 * @brief Implementation of SampleEntityTableModel displaying canonical SampleEntity attributes.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "SampleEntityTableModel.h"

namespace GISApp::UIModels::SampleEntities {

/**
 * @brief Constructs SampleEntityTableModel and connects to repository signals.
 * @param[in] repo Injected repository pointer.
 * @param[in] parent Optional parent QObject.
 */
SampleEntityTableModel::SampleEntityTableModel(GISApp::Repositories::ISampleEntityRepository *repo,
                                               QObject *parent)
    : QAbstractTableModel(parent)
    , m_repo(repo)
{
    setupRepositoryConnections();
    reloadEntities();
}

/**
 * @brief Replaces active repository and reloads table model.
 * @param[in] repo Pointer to new ISampleEntityRepository.
 */
void SampleEntityTableModel::setRepository(GISApp::Repositories::ISampleEntityRepository *repo)
{
    if (m_repo == repo) {
        return;
    }

    if (m_repo) {
        disconnect(m_repo, &GISApp::Repositories::ISampleEntityRepository::sampleEntitiesUpdated,
                   this, &SampleEntityTableModel::onEntitiesUpdated);
    }

    m_repo = repo;
    setupRepositoryConnections();
    reloadEntities();
}

/**
 * @brief Subscribes to repository modification signals.
 */
void SampleEntityTableModel::setupRepositoryConnections()
{
    if (!m_repo) {
        return;
    }

    connect(m_repo, &GISApp::Repositories::ISampleEntityRepository::sampleEntitiesUpdated,
            this, &SampleEntityTableModel::onEntitiesUpdated);
}

/**
 * @brief Fetches all entities from the repository and notifies attached views.
 */
void SampleEntityTableModel::reloadEntities()
{
    beginResetModel();
    if (m_repo) {
        m_entities = m_repo->getAllSampleEntities();
    } else {
        m_entities.clear();
    }
    endResetModel();
}

/**
 * @brief Handler for batch updates signaled by the repository.
 */
void SampleEntityTableModel::onEntitiesUpdated()
{
    reloadEntities();
}

/**
 * @brief Retrieves entity value object at specific row index.
 */
GISApp::Domain::SampleEntities::SampleEntity SampleEntityTableModel::getEntityAt(int row) const
{
    if (row >= 0 && row < m_entities.size()) {
        return m_entities.at(row);
    }
    return GISApp::Domain::SampleEntities::SampleEntity(0, 0, 0.0, 0.0);
}

/**
 * @brief Finds the zero-based table row for an entity ID.
 */
int SampleEntityTableModel::findRowByEntityId(int entityId) const
{
    for (int i = 0; i < m_entities.size(); ++i) {
        if (static_cast<int>(m_entities.at(i).Id()) == entityId) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Returns total number of active entity rows.
 */
int SampleEntityTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entities.size();
}

/**
 * @brief Returns total number of table columns.
 */
int SampleEntityTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

/**
 * @brief Returns header text and styling for columns.
 */
QVariant SampleEntityTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnId:         return tr("Entity ID");
        case ColumnName:       return tr("Entity Name");
        case ColumnType:       return tr("Type");
        case ColumnLatitude:   return tr("Latitude");
        case ColumnLongitude:  return tr("Longitude");
        case ColumnHeight:     return tr("Height (m)");
        case ColumnDirection:  return tr("Direction (°)");
        case ColumnReportTime: return tr("Report Time (UTC)");
        case ColumnRemarks:    return tr("Remarks");
        default: break;
        }
    } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return section + 1;
    } else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignCenter);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

/**
 * @brief Retrieves cell display values and custom user role data.
 */
QVariant SampleEntityTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entities.size()) {
        return QVariant();
    }

    const auto &entity = m_entities.at(index.row());
    const int col = index.column();

    // Raw value extraction via custom user roles
    if (role == EntityIdRole) {
        return entity.Id();
    } else if (role == RawLatitudeRole) {
        return entity.location().latatitude;
    } else if (role == RawLongitudeRole) {
        return entity.location().longitude;
    } else if (role == RawHeightRole) {
        return entity.location().height;
    } else if (role == RawDirectionRole) {
        return entity.location().dir;
    }

    // Display string formatting
    if (role == Qt::DisplayRole) {
        switch (col) {
        case ColumnId:
            return entity.Id();
        case ColumnName:
            return entity.Name();
        case ColumnType:
            return entity.type();
        case ColumnLatitude:
            return QString::asprintf("%.5f°", entity.location().latatitude);
        case ColumnLongitude:
            return QString::asprintf("%.5f°", entity.location().longitude);
        case ColumnHeight:
            return QString::asprintf("%.0f m", entity.location().height);
        case ColumnDirection:
            return QString::asprintf("%.0f°", entity.location().dir);
        case ColumnReportTime:
            return entity.reportTime().isValid()
                ? entity.reportTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"))
                : QStringLiteral("--:--:--");
        case ColumnRemarks:
            return entity.remarks();
        default:
            return QVariant();
        }
    }

    // Text alignment per column type
    if (role == Qt::TextAlignmentRole) {
        switch (col) {
        case ColumnId:
        case ColumnType:
        case ColumnLatitude:
        case ColumnLongitude:
        case ColumnHeight:
        case ColumnDirection:
        case ColumnReportTime:
            return QVariant(Qt::AlignCenter);
        case ColumnName:
        case ColumnRemarks:
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    return QVariant();
}

} // namespace GISApp::UIModels::SampleEntities

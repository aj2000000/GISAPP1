/**
 * @file ComplexEntityTableModel.cpp
 * @brief Implementation of ComplexEntityTableModel Qt item model.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntityTableModel.h"

namespace GISApp::UIModels::ComplexEntities {

ComplexEntityTableModel::ComplexEntityTableModel(GISApp::Repositories::IComplexEntityRepository *repo,
                                                 QObject *parent)
    : QAbstractTableModel(parent)
{
    setRepository(repo);
}

void ComplexEntityTableModel::setRepository(GISApp::Repositories::IComplexEntityRepository *repo)
{
    if (m_repo) {
        disconnect(m_repo, nullptr, this, nullptr);
    }

    m_repo = repo;

    if (m_repo) {
        connect(m_repo, &GISApp::Repositories::IComplexEntityRepository::complexEntitiesUpdated,
                this, &ComplexEntityTableModel::reloadData);
        connect(m_repo, &GISApp::Repositories::IComplexEntityRepository::complexEntityUpserted,
                this, &ComplexEntityTableModel::reloadData);
        connect(m_repo, &GISApp::Repositories::IComplexEntityRepository::complexEntityRemoved,
                this, &ComplexEntityTableModel::reloadData);
    }

    reloadData();
}

void ComplexEntityTableModel::reloadData()
{
    beginResetModel();
    m_entities = m_repo ? m_repo->getAllComplexEntities() : QVector<GISApp::Domain::ComplexEntities::ComplexEntity>();
    endResetModel();
}

int ComplexEntityTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_entities.size();
}

int ComplexEntityTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant ComplexEntityTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entities.size()) {
        return QVariant();
    }

    const auto &ent = m_entities.at(index.row());

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case ColumnId:
        case ColumnPointsCount:
        case ColumnDetailsCount:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        case ColumnType:
            return static_cast<int>(Qt::AlignCenter);
        default:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == EntityIdRole) {
        return ent.id();
    }
    if (role == RawLatitudeRole) {
        return ent.primaryLocation().latatitude;
    }
    if (role == RawLongitudeRole) {
        return ent.primaryLocation().longitude;
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColumnId:
            return ent.id();
        case ColumnName:
            return ent.name().isEmpty() ? QStringLiteral("CMPLX-%1").arg(ent.id()) : ent.name();
        case ColumnType:
            return GISApp::Domain::ComplexEntities::ComplexEntity::entityTypeToString(ent.entityType());
        case ColumnPointsCount:
            return ent.noOfLocationPoints();
        case ColumnPrimaryCoords: {
            const auto loc = ent.primaryLocation();
            return QStringLiteral("%1°, %2°")
                   .arg(loc.latatitude, 0, 'f', 4)
                   .arg(loc.longitude, 0, 'f', 4);
        }
        case ColumnAnnotations: {
            QStringList parts;
            if (!ent.topAnnotation().isEmpty()) parts << QStringLiteral("T:%1").arg(ent.topAnnotation());
            if (!ent.bottomAnnotation().isEmpty()) parts << QStringLiteral("B:%1").arg(ent.bottomAnnotation());
            if (!ent.leftAnnotation().isEmpty()) parts << QStringLiteral("L:%1").arg(ent.leftAnnotation());
            if (!ent.rightAnnotation().isEmpty()) parts << QStringLiteral("R:%1").arg(ent.rightAnnotation());
            return parts.isEmpty() ? QStringLiteral("-") : parts.join(QStringLiteral(" | "));
        }
        case ColumnSpecialParams:
            return QStringLiteral("[%1, %2, %3, %4]")
                   .arg(ent.specialParam1())
                   .arg(ent.specialParam2())
                   .arg(ent.specialParam3())
                   .arg(ent.specialParam4());
        case ColumnDetailsCount:
            return ent.noOfDetails();
        case ColumnReportTime:
            return ent.reportTime().isValid() ? ent.reportTime().toString("dd/MM/yyyy HH:mm:ss") : QStringLiteral("-");
        case ColumnRemarks:
            return ent.remarks().isEmpty() ? QStringLiteral("-") : ent.remarks();
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant ComplexEntityTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnId:            return tr("ID");
        case ColumnName:          return tr("Entity Name");
        case ColumnType:          return tr("Type");
        case ColumnPointsCount:   return tr("Points");
        case ColumnPrimaryCoords: return tr("Primary Coords");
        case ColumnAnnotations:   return tr("Annotations");
        case ColumnSpecialParams: return tr("Special Params");
        case ColumnDetailsCount:  return tr("Details");
        case ColumnReportTime:    return tr("Report Time");
        case ColumnRemarks:       return tr("Remarks");
        default:                  return QVariant();
        }
    }
    return QVariant();
}

std::optional<GISApp::Domain::ComplexEntities::ComplexEntity> ComplexEntityTableModel::entityAt(int row) const
{
    if (row >= 0 && row < m_entities.size()) {
        return m_entities.at(row);
    }
    return std::nullopt;
}

int ComplexEntityTableModel::rowForEntityId(int entityId) const
{
    for (int i = 0; i < m_entities.size(); ++i) {
        if (static_cast<int>(m_entities.at(i).id()) == entityId) {
            return i;
        }
    }
    return -1;
}

} // namespace GISApp::UIModels::ComplexEntities

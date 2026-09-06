/**
 * @file SqliteComplexEntityRepository.cpp
 * @brief Implementation of SqliteComplexEntityRepository with thread-safe caching and SQLite persistence.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "SqliteComplexEntityRepository.h"
#include "DatabaseManager.h"

#include <QWriteLocker>
#include <QReadLocker>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>
#include <QDebug>

namespace GISApp::Repositories::ComplexEntities {

SqliteComplexEntityRepository::SqliteComplexEntityRepository(QObject *parent)
    : IComplexEntityRepository(parent)
{
    ensureTableExists();
    loadFromDatabase();
}

void SqliteComplexEntityRepository::upsertComplexEntity(const Domain::ComplexEntities::ComplexEntity &entity)
{
    const int id = static_cast<int>(entity.id());
    {
        QWriteLocker locker(&m_lock);
        m_entities.insert(id, entity);
    }

    saveEntityToDatabase(entity);

    emit complexEntityUpserted(id);
    emit complexEntitiesUpdated();
}

void SqliteComplexEntityRepository::upsertComplexEntities(const QVector<Domain::ComplexEntities::ComplexEntity> &entities)
{
    if (entities.isEmpty()) {
        return;
    }

    QVector<int> updatedIds;
    updatedIds.reserve(entities.size());

    {
        QWriteLocker locker(&m_lock);
        for (const auto &ent : entities) {
            const int id = static_cast<int>(ent.id());
            m_entities.insert(id, ent);
            updatedIds.append(id);
        }
    }

    saveEntitiesBatch(entities);

    for (int id : updatedIds) {
        emit complexEntityUpserted(id);
    }
    emit complexEntitiesUpdated();
}

QVector<Domain::ComplexEntities::ComplexEntity> SqliteComplexEntityRepository::getAllComplexEntities() const
{
    QReadLocker locker(&m_lock);
    return m_entities.values();
}

std::optional<Domain::ComplexEntities::ComplexEntity> SqliteComplexEntityRepository::getComplexEntityById(int id) const
{
    QReadLocker locker(&m_lock);
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it.value();
    }
    return std::nullopt;
}

bool SqliteComplexEntityRepository::removeComplexEntity(int id)
{
    bool found = false;
    {
        QWriteLocker locker(&m_lock);
        found = (m_entities.remove(id) > 0);
    }

    if (found) {
        deleteEntityFromDatabase(id);
        emit complexEntityRemoved(id);
        emit complexEntitiesUpdated();
    }
    return found;
}

void SqliteComplexEntityRepository::clearComplexEntities()
{
    {
        QWriteLocker locker(&m_lock);
        m_entities.clear();
    }

    clearDatabaseEntities();
    emit complexEntitiesUpdated();
}

int SqliteComplexEntityRepository::count() const
{
    QReadLocker locker(&m_lock);
    return m_entities.size();
}

void SqliteComplexEntityRepository::ensureTableExists()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    const QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS complex_entities ("
        "  entity_id INTEGER PRIMARY KEY,"
        "  entity_name TEXT,"
        "  entity_type INTEGER,"
        "  no_of_location_points INTEGER,"
        "  location_points TEXT,"
        "  left_annotation TEXT,"
        "  right_annotation TEXT,"
        "  top_annotation TEXT,"
        "  bottom_annotation TEXT,"
        "  special_param1 INTEGER,"
        "  special_param2 INTEGER,"
        "  special_param3 INTEGER,"
        "  special_param4 INTEGER,"
        "  no_of_details INTEGER,"
        "  details TEXT,"
        "  report_time TEXT,"
        "  remarks TEXT,"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
    );

    if (!query.exec(sql)) {
        qWarning() << "[SqliteComplexEntityRepository] Failed to verify/create complex_entities table:"
                   << query.lastError().text();
    }
}

void SqliteComplexEntityRepository::loadFromDatabase()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT entity_id, entity_name, entity_type, location_points, "
            "       left_annotation, right_annotation, top_annotation, bottom_annotation, "
            "       special_param1, special_param2, special_param3, special_param4, "
            "       details, report_time, remarks "
            "FROM complex_entities;"))) {
        qWarning() << "[SqliteComplexEntityRepository] Failed to query complex_entities:"
                   << query.lastError().text();
        return;
    }

    QWriteLocker locker(&m_lock);
    while (query.next()) {
        UINT_32 id          = query.value(0).toUInt();
        QString name        = query.value(1).toString();
        UINT_8 type         = static_cast<UINT_8>(query.value(2).toUInt());
        QString locPointsJson = query.value(3).toString();
        QString leftAnn     = query.value(4).toString();
        QString rightAnn    = query.value(5).toString();
        QString topAnn      = query.value(6).toString();
        QString bottomAnn   = query.value(7).toString();
        UINT_16 sp1         = static_cast<UINT_16>(query.value(8).toUInt());
        UINT_16 sp2         = static_cast<UINT_16>(query.value(9).toUInt());
        UINT_16 sp3         = static_cast<UINT_16>(query.value(10).toUInt());
        UINT_16 sp4         = static_cast<UINT_16>(query.value(11).toUInt());
        QString detailsJson = query.value(12).toString();
        QDateTime rptTime   = query.value(13).toDateTime();
        QString remarks     = query.value(14).toString();

        Domain::ComplexEntities::ComplexEntity ent(id, name, type);
        ent.loadLocationPointsFromJson(locPointsJson);
        ent.setLeftAnnotation(leftAnn);
        ent.setRightAnnotation(rightAnn);
        ent.setTopAnnotation(topAnn);
        ent.setBottomAnnotation(bottomAnn);
        ent.setSpecialParam1(sp1);
        ent.setSpecialParam2(sp2);
        ent.setSpecialParam3(sp3);
        ent.setSpecialParam4(sp4);
        ent.loadDetailsFromJson(detailsJson);
        if (rptTime.isValid()) {
            ent.setReportTime(rptTime);
        }
        ent.setRemarks(remarks);

        m_entities.insert(static_cast<int>(id), ent);
    }

    qInfo() << "[SqliteComplexEntityRepository] Restored" << m_entities.size()
            << "complex entities from SQLite database.";
}

void SqliteComplexEntityRepository::saveEntityToDatabase(const Domain::ComplexEntities::ComplexEntity &entity)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO complex_entities ("
        "  entity_id, entity_name, entity_type, no_of_location_points, location_points, "
        "  left_annotation, right_annotation, top_annotation, bottom_annotation, "
        "  special_param1, special_param2, special_param3, special_param4, "
        "  no_of_details, details, report_time, remarks, updated_at"
        ") VALUES ("
        "  :id, :name, :type, :no_pts, :pts, :l_ann, :r_ann, :t_ann, :b_ann, "
        "  :sp1, :sp2, :sp3, :sp4, :no_det, :det, :rpt, :rem, CURRENT_TIMESTAMP"
        ") ON CONFLICT(entity_id) DO UPDATE SET "
        "  entity_name = excluded.entity_name,"
        "  entity_type = excluded.entity_type,"
        "  no_of_location_points = excluded.no_of_location_points,"
        "  location_points = excluded.location_points,"
        "  left_annotation = excluded.left_annotation,"
        "  right_annotation = excluded.right_annotation,"
        "  top_annotation = excluded.top_annotation,"
        "  bottom_annotation = excluded.bottom_annotation,"
        "  special_param1 = excluded.special_param1,"
        "  special_param2 = excluded.special_param2,"
        "  special_param3 = excluded.special_param3,"
        "  special_param4 = excluded.special_param4,"
        "  no_of_details = excluded.no_of_details,"
        "  details = excluded.details,"
        "  report_time = excluded.report_time,"
        "  remarks = excluded.remarks,"
        "  updated_at = CURRENT_TIMESTAMP;"
    ));

    query.bindValue(QStringLiteral(":id"), static_cast<int>(entity.id()));
    query.bindValue(QStringLiteral(":name"), entity.name());
    query.bindValue(QStringLiteral(":type"), static_cast<int>(entity.entityType()));
    query.bindValue(QStringLiteral(":no_pts"), static_cast<int>(entity.noOfLocationPoints()));
    query.bindValue(QStringLiteral(":pts"), entity.locationPointsToJson());
    query.bindValue(QStringLiteral(":l_ann"), entity.leftAnnotation());
    query.bindValue(QStringLiteral(":r_ann"), entity.rightAnnotation());
    query.bindValue(QStringLiteral(":t_ann"), entity.topAnnotation());
    query.bindValue(QStringLiteral(":b_ann"), entity.bottomAnnotation());
    query.bindValue(QStringLiteral(":sp1"), static_cast<int>(entity.specialParam1()));
    query.bindValue(QStringLiteral(":sp2"), static_cast<int>(entity.specialParam2()));
    query.bindValue(QStringLiteral(":sp3"), static_cast<int>(entity.specialParam3()));
    query.bindValue(QStringLiteral(":sp4"), static_cast<int>(entity.specialParam4()));
    query.bindValue(QStringLiteral(":no_det"), entity.noOfDetails());
    query.bindValue(QStringLiteral(":det"), entity.detailsToJson());
    query.bindValue(QStringLiteral(":rpt"), entity.reportTime().isValid() ? entity.reportTime().toString(Qt::ISODate) : QString());
    query.bindValue(QStringLiteral(":rem"), entity.remarks());

    if (!query.exec()) {
        qWarning() << "[SqliteComplexEntityRepository] Failed to save entity" << entity.id()
                   << ":" << query.lastError().text();
    }
}

void SqliteComplexEntityRepository::saveEntitiesBatch(const QVector<Domain::ComplexEntities::ComplexEntity> &entities)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    db.transaction();
    for (const auto &ent : entities) {
        saveEntityToDatabase(ent);
    }
    db.commit();
}

void SqliteComplexEntityRepository::deleteEntityFromDatabase(int entityId)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM complex_entities WHERE entity_id = :id;"));
    query.bindValue(QStringLiteral(":id"), entityId);

    if (!query.exec()) {
        qWarning() << "[SqliteComplexEntityRepository] Failed to delete entity" << entityId
                   << "from database:" << query.lastError().text();
    }
}

void SqliteComplexEntityRepository::clearDatabaseEntities()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("DELETE FROM complex_entities;"))) {
        qWarning() << "[SqliteComplexEntityRepository] Failed to clear complex_entities table:"
                   << query.lastError().text();
    }
}

} // namespace GISApp::Repositories::ComplexEntities

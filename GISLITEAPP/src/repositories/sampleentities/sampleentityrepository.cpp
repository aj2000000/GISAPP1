/**
 * @file sampleentityrepository.cpp
 * @brief Implementation of SampleEntityRepository with in-memory caching and SQLite persistence.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "sampleentityrepository.h"
#include "DatabaseManager.h"

#include <QWriteLocker>
#include <QReadLocker>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSqlDatabase>
#include <QDebug>

namespace GISApp::Repositories::SampleEntities {

/**
 * @brief Constructs SampleEntityRepository, validates SQLite table, and loads saved entities.
 * @param[in] parent Optional QObject parent ownership.
 */
SampleEntityRepository::SampleEntityRepository(QObject *parent)
    : ISampleEntityRepository(parent)
{
    ensureTableExists();
    loadFromDatabase();
}

/**
 * @brief Thread-safe insert or update of a single entity.
 * @param[in] sampleEntity Domain entity to store.
 */
void SampleEntityRepository::upsertSampleEntity(const Domain::SampleEntities::SampleEntity &sampleEntity)
{
    const int id = static_cast<int>(sampleEntity.Id());
    {
        QWriteLocker locker(&m_lock);
        m_entities.insert(id, sampleEntity);
    }

    saveEntityToDatabase(sampleEntity);

    emit sampleEntityUpserted(id);
    emit sampleEntitiesUpdated();
}

/**
 * @brief Thread-safe batch insert or update of entities in memory and database transaction.
 * @param[in] sampleEntities Collection of domain entities.
 */
void SampleEntityRepository::upsertSampleEntities(const QVector<Domain::SampleEntities::SampleEntity> &sampleEntities)
{
    if (sampleEntities.isEmpty()) {
        return;
    }

    {
        QWriteLocker locker(&m_lock);
        for (const auto &entity : sampleEntities) {
            m_entities.insert(static_cast<int>(entity.Id()), entity);
        }
    }

    saveEntitiesToDatabase(sampleEntities);

    emit sampleEntitiesUpdated();
}

/**
 * @brief Returns snapshot of all active entities.
 * @return Thread-safe copy of all SampleEntities.
 */
QVector<Domain::SampleEntities::SampleEntity> SampleEntityRepository::getAllSampleEntities() const
{
    QReadLocker locker(&m_lock);
    return m_entities.values().toVector();
}

/**
 * @brief Looks up entity by ID.
 * @param[in] id Numerical entity ID.
 * @return Optional entity or nullopt if absent.
 */
std::optional<Domain::SampleEntities::SampleEntity> SampleEntityRepository::getSampleEntityById(int id) const
{
    QReadLocker locker(&m_lock);
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it.value();
    }
    return std::nullopt;
}

/**
 * @brief Removes an entity by ID from memory and SQLite database.
 * @param[in] id Numerical entity ID.
 * @return True if removed.
 */
bool SampleEntityRepository::removeSampleEntity(int id)
{
    bool removed = false;
    {
        QWriteLocker locker(&m_lock);
        removed = (m_entities.remove(id) > 0);
    }

    if (removed) {
        deleteEntityFromDatabase(id);
        emit sampleEntityRemoved(id);
        emit sampleEntitiesUpdated();
    }

    return removed;
}

/**
 * @brief Clears all entities in memory and purges SQLite table.
 */
void SampleEntityRepository::clearSampleEntity()
{
    {
        QWriteLocker locker(&m_lock);
        m_entities.clear();
    }

    clearDatabaseEntities();

    emit sampleEntitiesUpdated();
}

/**
 * @brief Returns count of active entities.
 * @return Integer entity count.
 */
int SampleEntityRepository::count() const
{
    QReadLocker locker(&m_lock);
    return m_entities.size();
}

// =============================================================================
// SQLite Persistence Subsystem
// =============================================================================

/**
 * @brief Creates the `sample_entities` table in SQLite if not already present.
 */
void SampleEntityRepository::ensureTableExists()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    const QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS sample_entities ("
        "  entity_id INTEGER PRIMARY KEY,"
        "  entity_name TEXT,"
        "  entity_type INTEGER,"
        "  latitude REAL,"
        "  longitude REAL,"
        "  height REAL,"
        "  dir REAL,"
        "  report_time TEXT,"
        "  remarks TEXT,"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        );

    if (!query.exec(sql)) {
        qWarning() << "[SampleEntityRepository] Failed to verify/create sample_entities table:"
                   << query.lastError().text();
    }
}

/**
 * @brief Restores persisted entities from SQLite database into memory cache on boot.
 */
void SampleEntityRepository::loadFromDatabase()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        qDebug() << "[SampleEntityRepository] Database not open, skipping loadFromDatabase";
        return;
    }

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT entity_id, entity_name, entity_type, latitude, longitude, height, dir, "
            "report_time, remarks FROM sample_entities;"))) {
        qWarning() << "[SampleEntityRepository] Failed to query sample_entities:"
                   << query.lastError().text();
        return;
    }

    QWriteLocker locker(&m_lock);
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        int type = query.value(2).toInt();
        double lat = query.value(3).toDouble();
        double lon = query.value(4).toDouble();
        double height = query.value(5).toDouble();
        double dir = query.value(6).toDouble();
        QDateTime reportTime = query.value(7).toDateTime();
        QString remarks = query.value(8).toString();

        Domain::SampleEntities::SampleEntity entity(id, type, lat, lon, height, dir);
        entity.setName(name);
        entity.setRemarks(remarks);
        if (reportTime.isValid()) {
            entity.setReportTime(reportTime);
        }

        m_entities.insert(id, entity);
    }

    qInfo() << "[SampleEntityRepository] Restored" << m_entities.size()
            << "sample entities from SQLite database.";
}

/**
 * @brief Saves or updates a single entity into the SQLite table using upsert conflict resolution.
 * @param[in] entity SampleEntity domain entity.
 */
void SampleEntityRepository::saveEntityToDatabase(const Domain::SampleEntities::SampleEntity &entity)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO sample_entities ("
        "  entity_id, entity_name, entity_type, latitude, longitude, height, dir, "
        "  report_time, remarks, updated_at"
        ") VALUES ("
        "  :entity_id, :entity_name, :entity_type, :latitude, :longitude, :height, :dir, "
        "  :report_time, :remarks, CURRENT_TIMESTAMP"
        ") ON CONFLICT(entity_id) DO UPDATE SET "
        "  entity_name = excluded.entity_name,"
        "  entity_type = excluded.entity_type,"
        "  latitude = excluded.latitude,"
        "  longitude = excluded.longitude,"
        "  height = excluded.height,"
        "  dir = excluded.dir,"
        "  report_time = excluded.report_time,"
        "  remarks = excluded.remarks,"
        "  updated_at = CURRENT_TIMESTAMP;"
        ));

    query.bindValue(":entity_id", static_cast<int>(entity.Id()));
    query.bindValue(":entity_name", entity.Name());
    query.bindValue(":entity_type", static_cast<int>(entity.type()));
    query.bindValue(":latitude", entity.location().latatitude);
    query.bindValue(":longitude", entity.location().longitude);
    query.bindValue(":height", entity.location().height);
    query.bindValue(":dir", entity.location().dir);
    query.bindValue(":report_time", entity.reportTime().toString(Qt::ISODate));
    query.bindValue(":remarks", entity.remarks());

    if (!query.exec()) {
        qWarning() << "[SampleEntityRepository] Failed to upsert entity" << entity.Id()
        << "into database:" << query.lastError().text();
    }
}

/**
 * @brief Batch persists a list of entities within a single atomic SQLite transaction.
 * @param[in] entities Collection of domain entities.
 */
void SampleEntityRepository::saveEntitiesToDatabase(const QVector<Domain::SampleEntities::SampleEntity> &entities)
{
    if (entities.isEmpty()) {
        return;
    }

    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    db.transaction();
    for (const auto &entity : entities) {
        saveEntityToDatabase(entity);
    }
    db.commit();
}

/**
 * @brief Deletes an entity row from SQLite by its ID.
 * @param[in] entityId Numerical entity ID.
 */
void SampleEntityRepository::deleteEntityFromDatabase(int entityId)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM sample_entities WHERE entity_id = :id;"));
    query.bindValue(":id", entityId);

    if (!query.exec()) {
        qWarning() << "[SampleEntityRepository] Failed to delete entity" << entityId
                   << "from database:" << query.lastError().text();
    }
}

/**
 * @brief Purges all records from the `sample_entities` SQLite table.
 */
void SampleEntityRepository::clearDatabaseEntities()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("DELETE FROM sample_entities;"))) {
        qWarning() << "[SampleEntityRepository] Failed to clear sample_entities table:"
                   << query.lastError().text();
    }
}

} // namespace GISApp::Repositories::SampleEntities

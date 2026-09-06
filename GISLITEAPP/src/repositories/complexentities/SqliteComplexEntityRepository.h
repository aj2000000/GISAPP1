/**
 * @file SqliteComplexEntityRepository.h
 * @brief Thread-safe SQLite repository with in-memory caching for ComplexEntity domain models.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SQLITECOMPLEXENTITYREPOSITORY_H
#define SQLITECOMPLEXENTITYREPOSITORY_H

#include <QReadWriteLock>
#include <QMap>

#include "IComplexEntityRepository.h"

namespace GISApp::Repositories::ComplexEntities {

/**
 * @class SqliteComplexEntityRepository
 * @brief Concrete SQLite repository implementing IComplexEntityRepository with thread-safe in-memory caching.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the **Repository Implementation Layer** (`src/repositories/complexentities/`).
 * - Implements `GISApp::Repositories::IComplexEntityRepository`.
 * - Combines an in-memory `QMap` cache protected by `QReadWriteLock` for nanosecond concurrent reads
 *   with transactional persistence into an SQLite database (`complex_entities` table).
 * - Ensures thread safety across UDP network ingestion background threads and Qt UI threads.
 */
class SqliteComplexEntityRepository : public IComplexEntityRepository
{
    Q_OBJECT

public:
    /**
     * @brief Constructs SqliteComplexEntityRepository, verifies SQLite table, and warms in-memory cache.
     * @param[in] parent Optional parent QObject for lifecycle management.
     */
    explicit SqliteComplexEntityRepository(QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~SqliteComplexEntityRepository() override = default;

    // --- IComplexEntityRepository Interface Implementation ---

    void upsertComplexEntity(const Domain::ComplexEntities::ComplexEntity &entity) override;
    void upsertComplexEntities(const QVector<Domain::ComplexEntities::ComplexEntity> &entities) override;
    [[nodiscard]] QVector<Domain::ComplexEntities::ComplexEntity> getAllComplexEntities() const override;
    [[nodiscard]] std::optional<Domain::ComplexEntities::ComplexEntity> getComplexEntityById(int id) const override;
    bool removeComplexEntity(int id) override;
    void clearComplexEntities() override;
    [[nodiscard]] int count() const override;

private:
    /**
     * @brief Verifies and creates the `complex_entities` table in SQLite if it does not exist.
     */
    void ensureTableExists();

    /**
     * @brief Restores persisted entities from SQLite database into memory cache on initialization.
     */
    void loadFromDatabase();

    /**
     * @brief Saves or updates a single entity into the SQLite table using upsert conflict resolution.
     * @param[in] entity ComplexEntity domain object.
     */
    void saveEntityToDatabase(const Domain::ComplexEntities::ComplexEntity &entity);

    /**
     * @brief Saves multiple entities to the SQLite database within a single atomic transaction.
     * @param[in] entities Collection of domain entities.
     */
    void saveEntitiesBatch(const QVector<Domain::ComplexEntities::ComplexEntity> &entities);

    /**
     * @brief Deletes an entity row from SQLite by its ID.
     * @param[in] entityId Numerical entity ID.
     */
    void deleteEntityFromDatabase(int entityId);

    /**
     * @brief Purges all records from the `complex_entities` SQLite table.
     */
    void clearDatabaseEntities();

    /// Reader-writer lock guaranteeing thread safety across concurrent readers and writers
    mutable QReadWriteLock m_lock;

    /// Primary in-memory index mapping entity ID to ComplexEntity instance
    QMap<int, Domain::ComplexEntities::ComplexEntity> m_entities;
};

} // namespace GISApp::Repositories::ComplexEntities

#endif // SQLITECOMPLEXENTITYREPOSITORY_H

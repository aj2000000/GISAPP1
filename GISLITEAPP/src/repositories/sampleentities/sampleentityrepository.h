/**
 * @file sampleentityrepository.h
 * @brief Thread-safe repository for SampleEntity domain models with SQLite persistence.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYREPOSITORY_H
#define SAMPLEENTITYREPOSITORY_H

#include "ISampleEntityRepository.h"
#include <QHash>
#include <QReadWriteLock>

namespace GISApp::Repositories::SampleEntities {

/**
 * @class SampleEntityRepository
 * @brief Thread-safe repository managing SampleEntity objects with dual-layer storage (In-Memory + SQLite).
 *
 * Architectural Role & Design Patterns:
 * - Implements the **Repository Pattern** conforming to `ISampleEntityRepository`.
 * - **In-Memory Cache**: Uses `QHash<int, SampleEntity>` protected by `QReadWriteLock` for high-throughput
 *   concurrent reads by Map/UI rendering controllers and thread-safe writes by UDP message handlers.
 * - **Persistent Storage**: Automatically synchronizes state changes to the SQLite database (`gislite.db`)
 *   via `DatabaseManager`.
 * - **Auto Schema Migration**: Verifies and creates the `sample_entities` SQLite table automatically on first boot.
 * - **Observer Notifications**: Broadcasts Qt signals (`sampleEntityUpserted`, `sampleEntitiesUpdated`,
 *   `sampleEntityRemoved`) to decouple storage from UI presenters.
 */
class SampleEntityRepository : public ISampleEntityRepository
{
    Q_OBJECT

public:
    /**
     * @brief Constructs SampleEntityRepository and restores persisted entities from SQLite database.
     * @param[in] parent Optional QObject parent ownership.
     */
    explicit SampleEntityRepository(QObject *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~SampleEntityRepository() override = default;

    /**
     * @brief Inserts or updates a single SampleEntity in memory and persists to SQLite.
     * @param[in] sampleEntity Domain entity instance to upsert.
     * @note Thread-safe. Emits sampleEntityUpserted and sampleEntitiesUpdated signals.
     */
    void upsertSampleEntity(const Domain::SampleEntities::SampleEntity &sampleEntity) override;

    /**
     * @brief Batch inserts or updates a collection of SampleEntities in a single database transaction.
     * @param[in] sampleEntities Collection of domain entities.
     * @note Thread-safe. Emits sampleEntitiesUpdated signal upon completion.
     */
    void upsertSampleEntities(const QVector<Domain::SampleEntities::SampleEntity> &sampleEntities) override;

    /**
     * @brief Retrieves all active SampleEntities currently maintained in repository.
     * @return Thread-safe copy of all SampleEntities as a QVector.
     */
    [[nodiscard]] QVector<Domain::SampleEntities::SampleEntity> getAllSampleEntities() const override;

    /**
     * @brief Looks up a specific SampleEntity by its unique numerical identifier.
     * @param[in] id Numerical entity ID.
     * @return std::optional containing the entity if present, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<Domain::SampleEntities::SampleEntity> getSampleEntityById(int id) const override;

    /**
     * @brief Removes a SampleEntity by ID from memory cache and SQLite database.
     * @param[in] id Numerical entity ID to remove.
     * @return True if entity was found and removed, false otherwise.
     */
    bool removeSampleEntity(int id) override;

    /**
     * @brief Purges all SampleEntities from memory cache and clears the SQLite table.
     */
    void clearSampleEntity() override;

    /**
     * @brief Retrieves the current count of managed entities.
     * @return Total active entity count.
     */
    [[nodiscard]] int count() const override;

private:
    /**
     * @brief Ensures the `sample_entities` SQLite table exists; creates it if missing.
     */
    void ensureTableExists();

    /**
     * @brief Restores persisted SampleEntities from SQLite database into memory cache on startup.
     */
    void loadFromDatabase();

    /**
     * @brief Persists or updates a single SampleEntity in the SQLite database.
     * @param[in] entity SampleEntity domain instance.
     */
    void saveEntityToDatabase(const Domain::SampleEntities::SampleEntity &entity);

    /**
     * @brief Batch persists a list of SampleEntities within a single SQLite transaction.
     * @param[in] entities Collection of domain entities.
     */
    void saveEntitiesToDatabase(const QVector<Domain::SampleEntities::SampleEntity> &entities);

    /**
     * @brief Deletes an entity record from the SQLite database by ID.
     * @param[in] entityId Numerical entity ID.
     */
    void deleteEntityFromDatabase(int entityId);

    /**
     * @brief Purges all records from the `sample_entities` SQLite table.
     */
    void clearDatabaseEntities();

    // =========================================================================
    // Member Variables
    // =========================================================================

    /// Synchronization lock allowing multiple concurrent readers and exclusive writers.
    mutable QReadWriteLock m_lock;

    /// In-memory synchronized hash map indexing SampleEntities by their integer ID.
    QHash<int, Domain::SampleEntities::SampleEntity> m_entities;
};

} // namespace GISApp::Repositories::SampleEntities

#endif // SAMPLEENTITYREPOSITORY_H

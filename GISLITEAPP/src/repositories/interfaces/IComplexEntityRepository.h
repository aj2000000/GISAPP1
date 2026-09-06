/**
 * @file IComplexEntityRepository.h
 * @brief Abstract repository interface defining persistence contracts for ComplexEntity domain models.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef ICOMPLEXENTITYREPOSITORY_H
#define ICOMPLEXENTITYREPOSITORY_H

#include <QObject>
#include <QVector>
#include <optional>
#include "ComplexEntity.h"

namespace GISApp::Repositories {

/**
 * @class IComplexEntityRepository
 * @brief Abstract interface defining CRUD and event contracts for ComplexEntity persistence.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the **Repository Interfaces Layer** (`src/repositories/interfaces/`).
 * - Implements the **Repository Pattern** (DDD) to isolate domain and application services
 *   from concrete database storage technologies (e.g. SQLite, PostgreSQL, Memory).
 * - Declares Qt signals for reactive event-driven propagation of data changes to services and views.
 */
class IComplexEntityRepository : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs IComplexEntityRepository with optional parent QObject.
     * @param[in] parent Optional parent for Qt memory management.
     */
    explicit IComplexEntityRepository(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief Virtual destructor ensuring clean polymorphic teardown.
     */
    virtual ~IComplexEntityRepository() override = default;

    /**
     * @brief Inserts or updates an individual complex entity in the repository.
     * @param[in] entity Domain entity to persist.
     */
    virtual void upsertComplexEntity(const Domain::ComplexEntities::ComplexEntity &entity) = 0;

    /**
     * @brief Batch inserts or updates multiple complex entities atomically.
     * @param[in] entities Collection of domain entities.
     */
    virtual void upsertComplexEntities(const QVector<Domain::ComplexEntities::ComplexEntity> &entities) = 0;

    /**
     * @brief Retrieves all active complex entities currently in the repository.
     * @return QVector of ComplexEntity domain objects.
     */
    [[nodiscard]] virtual QVector<Domain::ComplexEntities::ComplexEntity> getAllComplexEntities() const = 0;

    /**
     * @brief Looks up a complex entity by its unique identifier.
     * @param[in] id Numerical entity ID.
     * @return std::optional containing entity if found, or std::nullopt.
     */
    [[nodiscard]] virtual std::optional<Domain::ComplexEntities::ComplexEntity> getComplexEntityById(int id) const = 0;

    /**
     * @brief Removes an entity by its identifier.
     * @param[in] id Numerical entity ID to remove.
     * @return True if removed, false if not found.
     */
    virtual bool removeComplexEntity(int id) = 0;

    /**
     * @brief Removes all complex entities from cache and persistent storage.
     */
    virtual void clearComplexEntities() = 0;

    /**
     * @brief Returns the total number of managed complex entities.
     * @return Integer entity count.
     */
    [[nodiscard]] virtual int count() const = 0;

signals:
    /**
     * @brief Signal emitted whenever the collection of complex entities changes.
     */
    void complexEntitiesUpdated();

    /**
     * @brief Signal emitted when a specific entity is inserted or updated.
     * @param[in] entityId Identifier of the modified entity.
     */
    void complexEntityUpserted(int entityId);

    /**
     * @brief Signal emitted when a specific entity is removed.
     * @param[in] entityId Identifier of the deleted entity.
     */
    void complexEntityRemoved(int entityId);
};

} // namespace GISApp::Repositories

#endif // ICOMPLEXENTITYREPOSITORY_H

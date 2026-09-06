/**
 * @file SampleEntityService.h
 * @brief Domain application service providing business logic and event distribution for SampleEntities.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef SAMPLEENTITYSERVICE_H
#define SAMPLEENTITYSERVICE_H

#include <QObject>
#include <QVector>
#include <optional>
#include "ISampleEntityRepository.h"
#include "sampleentity.h"

namespace GISApp::Services::SampleEntities {

/**
 * @class SampleEntityService
 * @brief Application service coordinating business logic and lifecycle events for SampleEntity domain models.
 *
 * Architectural Role:
 * - Resides strictly in the **Domain / Application Service Layer**.
 * - Subscribes to `ISampleEntityRepository` update signals and translates them into domain events.
 * - Entirely UI-agnostic (no dependency on QWidget, QMapLibre, or rendering pipelines).
 * - Exposes clean querying and mutation APIs consumed by presentation controllers (e.g. `SampleEntityController`).
 */
class SampleEntityService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs SampleEntityService bound to a sample entity repository.
     * @param[in] entityRepo Pointer to the data repository instance.
     * @param[in] parent Optional QObject parent ownership.
     */
    explicit SampleEntityService(GISApp::Repositories::ISampleEntityRepository *entityRepo,
                                 QObject *parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    virtual ~SampleEntityService() override = default;

    /**
     * @brief Retrieves all active SampleEntities from the repository.
     * @return QVector of SampleEntity domain objects.
     */
    [[nodiscard]] QVector<GISApp::Domain::SampleEntities::SampleEntity> getAllEntities() const;

    /**
     * @brief Looks up an individual SampleEntity by its integer ID.
     * @param[in] entityId Unique numerical identifier.
     * @return std::optional containing the entity if found, or std::nullopt.
     */
    [[nodiscard]] std::optional<GISApp::Domain::SampleEntities::SampleEntity> getEntityById(int entityId) const;

    /**
     * @brief Returns the total count of active sample entities.
     * @return Integer count.
     */
    [[nodiscard]] int entityCount() const;

    /**
     * @brief Deletes an entity from repository by ID.
     * @param[in] entityId Numerical entity ID.
     * @return True if removed, false otherwise.
     */
    bool deleteEntity(int entityId);

signals:
    /**
     * @brief Emitted when the entire collection of sample entities is refreshed or batch-updated.
     * @param[in] entities Updated collection of all active sample entities.
     */
    void entitiesUpdated(const QVector<GISApp::Domain::SampleEntities::SampleEntity> &entities);

    /**
     * @brief Emitted when a specific entity receives a position or attribute update.
     * @param[in] entity The updated domain entity.
     */
    void entityUpdated(const GISApp::Domain::SampleEntities::SampleEntity &entity);

    /**
     * @brief Emitted when an entity is removed from tracking.
     * @param[in] entityId Numerical identifier of the removed entity.
     */
    void entityRemoved(int entityId);

private slots:
    /**
     * @brief Slot triggered when the underlying repository updates its entity collection.
     */
    void onRepositoryEntitiesUpdated();

private:
    /// Injected data source interface.
    GISApp::Repositories::ISampleEntityRepository *m_entityRepo{nullptr};
};

} // namespace GISApp::Services::SampleEntities

#endif // SAMPLEENTITYSERVICE_H

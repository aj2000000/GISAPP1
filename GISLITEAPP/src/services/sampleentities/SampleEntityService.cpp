/**
 * @file SampleEntityService.cpp
 * @brief Implementation of domain SampleEntityService.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "SampleEntityService.h"
#include <QDebug>

namespace GISApp::Services::SampleEntities {

/**
 * @brief Constructs SampleEntityService and establishes signal forwarding from the repository.
 * @param[in] entityRepo Injected repository pointer.
 * @param[in] parent Optional QObject parent.
 */
SampleEntityService::SampleEntityService(GISApp::Repositories::ISampleEntityRepository *entityRepo,
                                         QObject *parent)
    : QObject(parent)
    , m_entityRepo(entityRepo)
{
    if (m_entityRepo) {
        // Forward collection batch changes
        connect(m_entityRepo, &GISApp::Repositories::ISampleEntityRepository::sampleEntitiesUpdated,
                this, &SampleEntityService::onRepositoryEntitiesUpdated);

        // Forward single-entity upserts
        connect(m_entityRepo, &GISApp::Repositories::ISampleEntityRepository::sampleEntityUpserted,
                this, [this](int entityId) {
                    auto entityOpt = getEntityById(entityId);
                    if (entityOpt.has_value()) {
                        emit entityUpdated(entityOpt.value());
                    }
                });

        // Forward single-entity removals
        connect(m_entityRepo, &GISApp::Repositories::ISampleEntityRepository::sampleEntityRemoved,
                this, &SampleEntityService::entityRemoved);
    }
}

/**
 * @brief Retrieves all entities from the repository.
 */
QVector<GISApp::Domain::SampleEntities::SampleEntity> SampleEntityService::getAllEntities() const
{
    return m_entityRepo ? m_entityRepo->getAllSampleEntities()
                        : QVector<GISApp::Domain::SampleEntities::SampleEntity>{};
}

/**
 * @brief Retrieves an entity by its integer ID.
 */
std::optional<GISApp::Domain::SampleEntities::SampleEntity> SampleEntityService::getEntityById(int entityId) const
{
    return m_entityRepo ? m_entityRepo->getSampleEntityById(entityId) : std::nullopt;
}

/**
 * @brief Returns total active entities count.
 */
int SampleEntityService::entityCount() const
{
    return m_entityRepo ? m_entityRepo->count() : 0;
}

/**
 * @brief Deletes an entity through the repository.
 */
bool SampleEntityService::deleteEntity(int entityId)
{
    return m_entityRepo ? m_entityRepo->removeSampleEntity(entityId) : false;
}

/**
 * @brief Handles repository batch updates and broadcasts domain signal.
 */
void SampleEntityService::onRepositoryEntitiesUpdated()
{
    const auto entities = getAllEntities();
    qDebug() << "[SampleEntityService] Domain entities updated (" << entities.size()
             << "entities). Broadcasting entitiesUpdated.";
    emit entitiesUpdated(entities);
}

} // namespace GISApp::Services::SampleEntities

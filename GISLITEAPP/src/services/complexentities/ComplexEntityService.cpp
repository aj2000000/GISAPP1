/**
 * @file ComplexEntityService.cpp
 * @brief Implementation of ComplexEntityService business logic and event propagation.
 * @author GISLITE Development Team
 * @date 2026
 */

#include "ComplexEntityService.h"

#include <algorithm>
#include <cmath>

namespace GISApp::Services::ComplexEntities {

ComplexEntityService::ComplexEntityService(GISApp::Repositories::IComplexEntityRepository *entityRepo,
                                           QObject *parent)
    : QObject(parent)
    , m_repo(entityRepo)
{
    if (m_repo) {
        connect(m_repo, &GISApp::Repositories::IComplexEntityRepository::complexEntitiesUpdated,
                this, [this]() {
                    emit entitiesUpdated(getAllEntities());
                });

        connect(m_repo, &GISApp::Repositories::IComplexEntityRepository::complexEntityUpserted,
                this, &ComplexEntityService::entityUpserted);

        connect(m_repo, &GISApp::Repositories::IComplexEntityRepository::complexEntityRemoved,
                this, &ComplexEntityService::entityRemoved);
    }
}

QVector<GISApp::Domain::ComplexEntities::ComplexEntity> ComplexEntityService::getAllEntities() const
{
    if (!m_repo) {
        return {};
    }
    return m_repo->getAllComplexEntities();
}

std::optional<GISApp::Domain::ComplexEntities::ComplexEntity> ComplexEntityService::getEntityById(int entityId) const
{
    if (!m_repo) {
        return std::nullopt;
    }
    return m_repo->getComplexEntityById(entityId);
}

int ComplexEntityService::entityCount() const
{
    return m_repo ? m_repo->count() : 0;
}

bool ComplexEntityService::deleteEntity(int entityId)
{
    return m_repo ? m_repo->removeComplexEntity(entityId) : false;
}

bool ComplexEntityService::updateEntity(const GISApp::Domain::ComplexEntities::ComplexEntity &entity)
{
    if (!m_repo) {
        return false;
    }
    m_repo->upsertComplexEntity(entity);
    return true;
}

bool ComplexEntityService::calculateEntitiesCenter(double &outLat, double &outLon, double &outZoom) const
{
    const auto entities = getAllEntities();
    if (entities.isEmpty()) {
        return false;
    }

    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;
    bool foundAny = false;

    for (const auto &ent : entities) {
        for (const auto &pt : ent.locationPoints()) {
            minLat = std::min(minLat, pt.latatitude);
            maxLat = std::max(maxLat, pt.latatitude);
            minLon = std::min(minLon, pt.longitude);
            maxLon = std::max(maxLon, pt.longitude);
            foundAny = true;
        }
    }

    if (!foundAny) {
        return false;
    }

    outLat = (minLat + maxLat) / 2.0;
    outLon = (minLon + maxLon) / 2.0;

    double dLat = std::abs(maxLat - minLat);
    double dLon = std::abs(maxLon - minLon);
    double maxSpan = std::max(dLat, dLon);

    if (maxSpan < 0.05) {
        outZoom = 12.0;
    } else if (maxSpan < 0.2) {
        outZoom = 10.0;
    } else if (maxSpan < 1.0) {
        outZoom = 8.0;
    } else if (maxSpan < 5.0) {
        outZoom = 6.0;
    } else {
        outZoom = 4.0;
    }

    return true;
}

} // namespace GISApp::Services::ComplexEntities

/**
 * @file ComplexEntityService.h
 * @brief Application domain service managing business workflows and events for Complex Entities.
 * @author GISLITE Development Team
 * @date 2026
 */

#ifndef COMPLEXENTITYSERVICE_H
#define COMPLEXENTITYSERVICE_H

#include <QObject>
#include <QVector>
#include <optional>

#include "IComplexEntityRepository.h"
#include "ComplexEntity.h"

namespace GISApp::Services::ComplexEntities {

/**
 * @class ComplexEntityService
 * @brief Domain application service orchestrating business rules and events for ComplexEntity instances.
 *
 * Architectural Role & Design Patterns:
 * - Resides in the **Domain Application Service Layer** (`src/services/complexentities/`).
 * - Bridges between data persistence repositories (`IComplexEntityRepository`) and presentation controllers.
 * - Entirely UI-agnostic: has no dependency on Qt GUI widgets, dialogs, or MapLibre rendering components.
 * - Dispatches domain update signals to UI controllers and reactive models.
 */
class ComplexEntityService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs ComplexEntityService with a data repository dependency.
     * @param[in] entityRepo Pointer to IComplexEntityRepository.
     * @param[in] parent Optional parent QObject.
     */
    explicit ComplexEntityService(GISApp::Repositories::IComplexEntityRepository *entityRepo,
                                  QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~ComplexEntityService() override = default;

    /**
     * @brief Retrieves all active complex entities from repository.
     * @return QVector of ComplexEntity domain models.
     */
    [[nodiscard]] QVector<GISApp::Domain::ComplexEntities::ComplexEntity> getAllEntities() const;

    /**
     * @brief Retrieves an individual entity by its identifier.
     * @param[in] entityId Numerical entity ID.
     * @return std::optional containing entity if found, or std::nullopt.
     */
    [[nodiscard]] std::optional<GISApp::Domain::ComplexEntities::ComplexEntity> getEntityById(int entityId) const;

    /**
     * @brief Returns total active complex entity count.
     * @return Integer count.
     */
    [[nodiscard]] int entityCount() const;

    /**
     * @brief Deletes an entity by its identifier.
     * @param[in] entityId Numerical entity ID.
     * @return True if deleted, false if not found.
     */
    bool deleteEntity(int entityId);

    /**
     * @brief Updates or persists an existing complex entity into repository.
     * @param[in] entity Updated ComplexEntity domain instance.
     * @return True if saved successfully, false otherwise.
     */
    bool updateEntity(const GISApp::Domain::ComplexEntities::ComplexEntity &entity);

    /**
     * @brief Calculates combined geographic bounding box / center of all active complex entities.
     * @param[out] outLat Center latitude.
     * @param[out] outLon Center longitude.
     * @param[out] outZoom Suggested camera zoom level.
     * @return True if at least one entity had valid coordinates, false otherwise.
     */
    bool calculateEntitiesCenter(double &outLat, double &outLon, double &outZoom) const;

signals:
    /**
     * @brief Emitted when the full collection of complex entities changes.
     * @param[in] entities Updated collection.
     */
    void entitiesUpdated(const QVector<GISApp::Domain::ComplexEntities::ComplexEntity> &entities);

    /**
     * @brief Emitted when an individual entity is inserted or updated.
     * @param[in] entityId ID of updated entity.
     */
    void entityUpserted(int entityId);

    /**
     * @brief Emitted when an entity is removed.
     * @param[in] entityId ID of deleted entity.
     */
    void entityRemoved(int entityId);

private:
    GISApp::Repositories::IComplexEntityRepository *m_repo{nullptr};
};

} // namespace GISApp::Services::ComplexEntities

#endif // COMPLEXENTITYSERVICE_H

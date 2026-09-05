/**
 * @file ILayerRepository.h
 * @brief Header definition for ILayerRepository data access interface.
 */

#ifndef ILAYERREPOSITORY_H
#define ILAYERREPOSITORY_H

#include <QString>
#include <QVector>
#include <QMap>

namespace GISApp::Domain::Layers {
class MapLayer;
}

namespace GISApp::Repositories {

/**
 * @class ILayerRepository
 * @brief Abstract repository interface defining persistence contracts for GIS layers.
 *
 * Implements the Repository pattern to decouple layer data persistence
 * (SQLite, project files, or remote servers) from the business logic and UI controllers.
 */
class ILayerRepository
{
public:
    virtual ~ILayerRepository() = default;

    /**
     * @brief Prefetches all saved layers from persistent storage sorted by z-order ascending.
     * @return QVector of dynamically allocated MapLayer pointers. Caller takes ownership.
     */
    [[nodiscard]] virtual QVector<GISApp::Domain::Layers::MapLayer*> getAllLayers() = 0;

    /**
     * @brief Retrieves an individual layer by its unique identifier.
     * @param[in] id Layer ID string.
     * @return Pointer to MapLayer if found, nullptr otherwise. Caller takes ownership.
     */
    [[nodiscard]] virtual GISApp::Domain::Layers::MapLayer* getLayerById(const QString &id) = 0;

    /**
     * @brief Inserts or updates an individual layer configuration in the database.
     * @param[in] layer Pointer to MapLayer entity to persist.
     * @return True if saved successfully, false on error.
     */
    virtual bool saveLayer(const GISApp::Domain::Layers::MapLayer *layer) = 0;

    /**
     * @brief Updates the visibility flag of a specific layer in the database.
     * @param[in] layerId Unique layer identifier.
     * @param[in] isVisible Boolean visibility flag.
     * @return True if database updated, false on failure.
     */
    virtual bool updateVisibility(const QString &layerId, bool isVisible) = 0;

    /**
     * @brief Updates the z-order rank of an individual layer.
     * @param[in] layerId Unique layer identifier.
     * @param[in] zOrder New integer rendering rank.
     * @return True if updated, false on failure.
     */
    virtual bool updateZOrder(const QString &layerId, int zOrder) = 0;

    /**
     * @brief Batch updates z-order ranks within an atomic database transaction.
     * @param[in] orderMap Map associating layer IDs with their new z-order indices.
     * @return True if transaction committed successfully, false if rolled back.
     */
    virtual bool updateZOrders(const QMap<QString, int> &orderMap) = 0;

    /**
     * @brief Deletes a layer record from the persistent store.
     * @param[in] layerId Unique layer identifier to delete.
     * @return True if deleted, false on failure.
     */
    virtual bool deleteLayer(const QString &layerId) = 0;

    /**
     * @brief Purges legacy mock layers and ensures the active BaseMap layer group exists.
     * @return True if cleaned up successfully, false on database error.
     */
    virtual bool purgeLegacyLayers() = 0;

    /**
     * @brief Ensures a fixed/mandatory application layer exists in persistent storage.
     *
     * Checks the persistent store by unique layer identifier:
     * - If the layer already exists, leaves it completely untouched (preserving user-configured
     *   z-order, visibility, opacity, and custom settings).
     * - If missing, determines the topmost z-order (MAX(z_order) + 1) and creates the layer
     *   associated with the specified group_name at the top of the stack.
     *
     * @param[in] prototype Prototype MapLayer entity defining id, name, groupName, type, and sourceUri.
     * @return True if the layer already exists or was successfully created; false on database error.
     * @note Thread safety depends on the underlying repository implementation.
     */
    virtual bool ensureLayerExists(const GISApp::Domain::Layers::MapLayer &prototype) = 0;

    /**
     * @brief Ensures a collection of fixed/mandatory application layers exist in persistent storage.
     *
     * Iterates through the provided layer prototypes, verifying each one exists or creating it on top.
     *
     * @param[in] prototypes Vector of prototype MapLayer entities.
     * @return True if all fixed layers exist or were created successfully; false if any failed.
     */
    virtual bool ensureFixedLayers(const QVector<GISApp::Domain::Layers::MapLayer> &prototypes) = 0;
};

} // namespace GISApp::Repositories

#endif // ILAYERREPOSITORY_H

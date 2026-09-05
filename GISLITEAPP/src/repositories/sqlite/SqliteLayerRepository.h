/**
 * @file SqliteLayerRepository.h
 * @brief Header definition for SqliteLayerRepository SQLite layer repository implementation.
 */

#ifndef SQLITELAYERREPOSITORY_H
#define SQLITELAYERREPOSITORY_H

#include "ILayerRepository.h"

namespace GISApp::Repositories::Sqlite {

/**
 * @class SqliteLayerRepository
 * @brief SQLite implementation of the ILayerRepository interface.
 *
 * SqliteLayerRepository executes SQL queries on the `layers` table managed by DatabaseManager.
 * Features:
 * - Queries and prefetches layers sorted by z-order ascending.
 * - Updates visibility and z-order directly when the user toggles checkboxes or moves layers up/down.
 * - Seeds default baseline tactical layers on first startup if the database is unpopulated.
 */
class SqliteLayerRepository : public ILayerRepository
{
public:
    /**
     * @brief Constructs SqliteLayerRepository.
     */
    SqliteLayerRepository();

    /**
     * @brief Destructor.
     */
    virtual ~SqliteLayerRepository() override = default;

    /**
     * @brief Prefetches all saved layers ordered by z-order ascending.
     * @return QVector of dynamically allocated MapLayer pointers.
     */
    [[nodiscard]] QVector<GISApp::Domain::Layers::MapLayer*> getAllLayers() override;

    /**
     * @brief Retrieves an individual layer by ID.
     * @param[in] id Layer ID.
     * @return Pointer to MapLayer if found, nullptr otherwise.
     */
    [[nodiscard]] GISApp::Domain::Layers::MapLayer* getLayerById(const QString &id) override;

    /**
     * @brief Persists or updates layer metadata in SQLite.
     * @param[in] layer Pointer to MapLayer.
     * @return True if successful.
     */
    bool saveLayer(const GISApp::Domain::Layers::MapLayer *layer) override;

    /**
     * @brief Updates layer visibility flag in SQLite.
     * @param[in] layerId Layer ID.
     * @param[in] isVisible Boolean visibility flag.
     * @return True if updated.
     */
    bool updateVisibility(const QString &layerId, bool isVisible) override;

    /**
     * @brief Updates layer rendering z-order index in SQLite.
     * @param[in] layerId Layer ID.
     * @param[in] zOrder New integer rendering rank.
     * @return True if updated.
     */
    bool updateZOrder(const QString &layerId, int zOrder) override;

    /**
     * @brief Atomically updates multiple layer z-orders within a SQL transaction.
     * @param[in] orderMap Map associating layer ID with new z-order.
     * @return True if transaction committed successfully.
     */
    bool updateZOrders(const QMap<QString, int> &orderMap) override;

    /**
     * @brief Deletes a layer record from SQLite.
     * @param[in] layerId Layer ID.
     * @return True if deleted.
     */
    bool deleteLayer(const QString &layerId) override;

    /**
     * @brief Purges legacy mock layers and ensures the active BaseMap layer group exists.
     * @return True if cleaned up successfully, false on database error.
     */
    bool purgeLegacyLayers() override;

private:
    /**
     * @brief Populates the database with initial baseline BaseMap layer if empty.
     */
    void seedDefaultLayers();
};

} // namespace GISApp::Repositories::Sqlite

#endif // SQLITELAYERREPOSITORY_H

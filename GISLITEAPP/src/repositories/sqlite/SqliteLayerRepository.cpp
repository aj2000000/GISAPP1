/**
 * @file SqliteLayerRepository.cpp
 * @brief Implementation of SqliteLayerRepository SQLite layer repository.
 */

#include "SqliteLayerRepository.h"
#include "DatabaseManager.h"
#include "MapLayer.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDebug>

namespace GISApp::Repositories::Sqlite {

SqliteLayerRepository::SqliteLayerRepository()
{
}

QVector<GISApp::Domain::Layers::MapLayer*> SqliteLayerRepository::getAllLayers()
{
    QVector<GISApp::Domain::Layers::MapLayer*> layers;
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();

    if (!db.isOpen()) {
        qWarning() << "[SqliteLayerRepository] Database not open during getAllLayers";
        return layers;
    }

    // Purge any legacy mock layers and ensure BaseMap group layer exists
    purgeLegacyLayers();

    QSqlQuery query(db);
    if (!query.exec("SELECT id, name, group_name, layer_type, source_uri, z_order, is_visible, opacity, config_json "
                    "FROM layers ORDER BY z_order ASC;")) {
        qWarning() << "[SqliteLayerRepository] Failed to query layers:" << query.lastError().text();
        return layers;
    }

    while (query.next()) {
        QString id         = query.value(0).toString();
        QString name       = query.value(1).toString();
        QString groupName  = query.value(2).toString();
        QString typeStr    = query.value(3).toString();
        QString uri        = query.value(4).toString();
        int zOrder         = query.value(5).toInt();
        bool isVisible     = query.value(6).toBool();
        double opacity     = query.value(7).toDouble();
        QString configJson = query.value(8).toString();

        auto *layer = new GISApp::Domain::Layers::MapLayer(
            id, name, GISApp::Domain::Layers::MapLayer::stringToLayerType(typeStr), uri);

        layer->setGroupName(groupName);
        layer->setZOrder(zOrder);
        layer->setVisible(isVisible);
        layer->setOpacity(opacity);
        layer->setConfigJson(configJson);

        layers.append(layer);
    }

    return layers;
}

GISApp::Domain::Layers::MapLayer* SqliteLayerRepository::getLayerById(const QString &id)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return nullptr;

    QSqlQuery query(db);
    query.prepare("SELECT id, name, group_name, layer_type, source_uri, z_order, is_visible, opacity, config_json "
                  "FROM layers WHERE id = :id;");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        QString name       = query.value(1).toString();
        QString groupName  = query.value(2).toString();
        QString typeStr    = query.value(3).toString();
        QString uri        = query.value(4).toString();
        int zOrder         = query.value(5).toInt();
        bool isVisible     = query.value(6).toBool();
        double opacity     = query.value(7).toDouble();
        QString configJson = query.value(8).toString();

        auto *layer = new GISApp::Domain::Layers::MapLayer(
            id, name, GISApp::Domain::Layers::MapLayer::stringToLayerType(typeStr), uri);

        layer->setGroupName(groupName);
        layer->setZOrder(zOrder);
        layer->setVisible(isVisible);
        layer->setOpacity(opacity);
        layer->setConfigJson(configJson);

        return layer;
    }

    return nullptr;
}

bool SqliteLayerRepository::saveLayer(const GISApp::Domain::Layers::MapLayer *layer)
{
    if (!layer) return false;

    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO layers "
                  "(id, name, group_name, layer_type, source_uri, z_order, is_visible, opacity, config_json, updated_at) "
                  "VALUES (:id, :name, :group_name, :layer_type, :source_uri, :z_order, :is_visible, :opacity, :config_json, CURRENT_TIMESTAMP);");

    query.bindValue(":id", layer->id());
    query.bindValue(":name", layer->name());
    query.bindValue(":group_name", layer->groupName());
    query.bindValue(":layer_type", GISApp::Domain::Layers::MapLayer::layerTypeToString(layer->layerType()));
    query.bindValue(":source_uri", layer->sourceUri());
    query.bindValue(":z_order", layer->zOrder());
    query.bindValue(":is_visible", layer->isVisible() ? 1 : 0);
    query.bindValue(":opacity", layer->opacity());
    query.bindValue(":config_json", layer->configJson());

    if (!query.exec()) {
        qWarning() << "[SqliteLayerRepository] Failed to save layer:" << query.lastError().text();
        return false;
    }
    return true;
}

bool SqliteLayerRepository::updateVisibility(const QString &layerId, bool isVisible)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("UPDATE layers SET is_visible = :visible, updated_at = CURRENT_TIMESTAMP WHERE id = :id;");
    query.bindValue(":visible", isVisible ? 1 : 0);
    query.bindValue(":id", layerId);

    if (!query.exec()) {
        qWarning() << "[SqliteLayerRepository] Failed to update visibility:" << query.lastError().text();
        return false;
    }
    return true;
}

bool SqliteLayerRepository::updateZOrder(const QString &layerId, int zOrder)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("UPDATE layers SET z_order = :z_order, updated_at = CURRENT_TIMESTAMP WHERE id = :id;");
    query.bindValue(":z_order", zOrder);
    query.bindValue(":id", layerId);

    if (!query.exec()) {
        qWarning() << "[SqliteLayerRepository] Failed to update z-order:" << query.lastError().text();
        return false;
    }
    return true;
}

bool SqliteLayerRepository::updateZOrders(const QMap<QString, int> &orderMap)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return false;

    if (!db.transaction()) {
        qWarning() << "[SqliteLayerRepository] Failed to begin transaction for updateZOrders";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE layers SET z_order = :z_order, updated_at = CURRENT_TIMESTAMP WHERE id = :id;");

    for (auto it = orderMap.constBegin(); it != orderMap.constEnd(); ++it) {
        query.bindValue(":id", it.key());
        query.bindValue(":z_order", it.value());
        if (!query.exec()) {
            qWarning() << "[SqliteLayerRepository] Failed in transaction on layer:" << it.key()
                       << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    return db.commit();
}

bool SqliteLayerRepository::deleteLayer(const QString &layerId)
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("DELETE FROM layers WHERE id = :id;");
    query.bindValue(":id", layerId);

    if (!query.exec()) {
        qWarning() << "[SqliteLayerRepository] Failed to delete layer:" << query.lastError().text();
        return false;
    }
    return true;
}

bool SqliteLayerRepository::purgeLegacyLayers()
{
    QSqlDatabase db = GISApp::Database::DatabaseManager::instance().database();
    if (!db.isOpen()) return false;

    // Purge legacy mock/hallucinated layers
    QSqlQuery query(db);
    query.exec("DELETE FROM layers WHERE id IN ('layer_dark_base', 'layer_borders', 'layer_topography', "
               "'layer_military_grid', 'layer_satellite', 'layer_tactical_tracks') "
               "OR group_name IN ('Base Maps', 'Terrain');");

    // Ensure the real BaseMap group layer exists
    QSqlQuery checkQuery(db);
    checkQuery.exec("SELECT COUNT(*) FROM layers WHERE group_name = 'BaseMap';");
    if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
        seedDefaultLayers();
    }

    return true;
}

void SqliteLayerRepository::seedDefaultLayers()
{
    qInfo() << "[SqliteLayerRepository] Seeding active BaseMap layer into SQLite...";

    // Real active MapLibre background layer matching local/offline tactical dark style
    GISApp::Domain::Layers::MapLayer baseLayer(
        "background",
        "Tactical Dark Base",
        GISApp::Domain::Layers::LayerType::Tile,
        "local://resources/map/styles/tactical_dark_fallback.json"
    );
    baseLayer.setGroupName("BaseMap");
    baseLayer.setZOrder(0);
    baseLayer.setVisible(true);
    baseLayer.setOpacity(1.0);

    saveLayer(&baseLayer);

    qInfo() << "[SqliteLayerRepository] BaseMap layer seeded successfully.";
}

} // namespace GISApp::Repositories::Sqlite

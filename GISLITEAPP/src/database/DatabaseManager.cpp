/**
 * @file DatabaseManager.cpp
 * @brief Implementation of DatabaseManager SQLite connection and schema manager.
 */

#include "DatabaseManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace GISApp::Database {

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager s_instance;
    return s_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_connectionName("GISAppPrimaryConnection")
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::initialize(const QString &dbPath)
{
    if (isOpen()) {
        return true;
    }

    if (dbPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir;
        if (!dir.exists(dataDir)) {
            dir.mkpath(dataDir);
        }
        m_dbPath = dataDir + "/gislite.db";
    } else {
        m_dbPath = dbPath;
    }

    qInfo() << "[DatabaseManager] Opening SQLite database at:" << m_dbPath;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(m_dbPath);

    if (!db.open()) {
        qCritical() << "[DatabaseManager] Failed to open SQLite database:" << db.lastError().text();
        return false;
    }

    // Enable foreign keys and write-ahead logging (WAL) for performance
    {
        QSqlQuery pragmaQuery(db);
        pragmaQuery.exec("PRAGMA foreign_keys = ON;");
        pragmaQuery.exec("PRAGMA journal_mode = WAL;");
    }

    if (!createTables()) {
        qCritical() << "[DatabaseManager] Schema initialization failed.";
        return false;
    }

    qInfo() << "[DatabaseManager] Database initialized and tables verified.";
    return true;
}

void DatabaseManager::close()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        {
            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(m_connectionName);
        qInfo() << "[DatabaseManager] Database closed.";
    }
}

bool DatabaseManager::isOpen() const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return false;
    }
    return QSqlDatabase::database(m_connectionName).isOpen();
}

QSqlDatabase DatabaseManager::database() const
{
    return QSqlDatabase::database(m_connectionName);
}

bool DatabaseManager::createTables()
{
    QSqlDatabase db = database();

    // 1. Create layers table for persisting layer hierarchy, z-order, and visibility
    const QString createLayersTableSql =
        "CREATE TABLE IF NOT EXISTS layers ("
        "  id TEXT PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  group_name TEXT DEFAULT '',"
        "  layer_type TEXT NOT NULL,"
        "  source_uri TEXT,"
        "  z_order INTEGER NOT NULL,"
        "  is_visible INTEGER DEFAULT 1,"
        "  opacity REAL DEFAULT 1.0,"
        "  config_json TEXT DEFAULT '{}',"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    {
        QSqlQuery query(db);
        if (!query.exec(createLayersTableSql)) {
            qCritical() << "[DatabaseManager] Failed to create layers table:" << query.lastError().text();
            return false;
        }
    }

    // 2. Validate tracks table schema and migrate if obsolete columns are detected
    bool shouldRecreateTracks = false;
    {
        QSqlQuery schemaQuery(db);
        if (schemaQuery.exec(QStringLiteral("PRAGMA table_info(tracks);"))) {
            bool hasTrackName = false;
            bool hasObsoleteSpeed = false;
            int columnCount = 0;
            while (schemaQuery.next()) {
                ++columnCount;
                const QString colName = schemaQuery.value(1).toString();
                if (colName == QStringLiteral("track_name")) {
                    hasTrackName = true;
                } else if (colName == QStringLiteral("speed")) {
                    hasObsoleteSpeed = true;
                }
            }
            if (columnCount > 0 && (!hasTrackName || hasObsoleteSpeed)) {
                shouldRecreateTracks = true;
            }
        }
    }

    if (shouldRecreateTracks) {
        qInfo() << "[DatabaseManager] Detected obsolete tracks schema. Recreating tracks table with canonical wire schema...";
        QSqlQuery dropQuery(db);
        if (!dropQuery.exec(QStringLiteral("DROP TABLE tracks;"))) {
            qWarning() << "[DatabaseManager] Failed to drop obsolete tracks table:" << dropQuery.lastError().text();
        }
    }

    // Create tracks table for persisting tactical track telemetry
    const QString createTracksTableSql =
        "CREATE TABLE IF NOT EXISTS tracks ("
        "  track_id INTEGER PRIMARY KEY,"
        "  track_name TEXT NOT NULL,"
        "  latitude REAL NOT NULL,"
        "  longitude REAL NOT NULL,"
        "  height REAL DEFAULT 0.0,"
        "  dir REAL DEFAULT 0.0,"
        "  track_identity INTEGER DEFAULT 0,"
        "  attr_type INTEGER DEFAULT 0,"
        "  attr_sub_type INTEGER DEFAULT 0,"
        "  attr_classification INTEGER DEFAULT 0,"
        "  attr_strength INTEGER DEFAULT 1,"
        "  attr_act_type INTEGER DEFAULT 0,"
        "  attr_act_sub_type INTEGER DEFAULT 0,"
        "  attr_act_classification INTEGER DEFAULT 0,"
        "  sys_track_type INTEGER DEFAULT 1,"
        "  symbol_code TEXT DEFAULT '',"
        "  remarks TEXT DEFAULT '',"
        "  report_time DATETIME,"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    {
        QSqlQuery query(db);
        if (!query.exec(createTracksTableSql)) {
            qCritical() << "[DatabaseManager] Failed to create tracks table:" << query.lastError().text();
            return false;
        }
    }

    // 3. Create app_settings table for user preferences, active theme, and state
    const QString createAppSettingsTableSql =
        "CREATE TABLE IF NOT EXISTS app_settings ("
        "  key TEXT PRIMARY KEY,"
        "  value TEXT NOT NULL,"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    {
        QSqlQuery query(db);
        if (!query.exec(createAppSettingsTableSql)) {
            qCritical() << "[DatabaseManager] Failed to create app_settings table:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::setSetting(const QString &key, const QString &value)
{
    if (!isOpen()) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral(
        "INSERT INTO app_settings (key, value, updated_at) VALUES (:key, :value, CURRENT_TIMESTAMP) "
        "ON CONFLICT(key) DO UPDATE SET value = excluded.value, updated_at = CURRENT_TIMESTAMP;"
    ));
    query.bindValue(QStringLiteral(":key"), key);
    query.bindValue(QStringLiteral(":value"), value);

    if (!query.exec()) {
        qWarning() << "[DatabaseManager] Failed to save setting:" << key << query.lastError().text();
        return false;
    }
    return true;
}

QString DatabaseManager::getSetting(const QString &key, const QString &defaultValue) const
{
    if (!isOpen()) {
        return defaultValue;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT value FROM app_settings WHERE key = :key LIMIT 1;"));
    query.bindValue(QStringLiteral(":key"), key);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return defaultValue;
}

} // namespace GISApp::Database


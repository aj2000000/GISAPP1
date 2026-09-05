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
    QSqlQuery pragmaQuery(db);
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");
    pragmaQuery.exec("PRAGMA journal_mode = WAL;");

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
    QSqlQuery query(db);

    // Create layers table for persisting layer hierarchy, z-order, and visibility
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

    if (!query.exec(createLayersTableSql)) {
        qCritical() << "[DatabaseManager] Failed to create layers table:" << query.lastError().text();
        return false;
    }

    return true;
}

} // namespace GISApp::Database

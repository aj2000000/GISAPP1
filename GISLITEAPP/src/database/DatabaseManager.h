/**
 * @file DatabaseManager.h
 * @brief Header definition for DatabaseManager SQLite connection and schema manager.
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

namespace GISApp::Database {

/**
 * @class DatabaseManager
 * @brief Centralized SQLite connection lifecycle and schema migration manager.
 *
 * DatabaseManager manages the application's embedded SQLite database (gislite.db).
 * It ensures the database connection is cleanly opened, validates tables, and applies
 * migrations (e.g. creating the `layers` table for layer ordering and prefetch).
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Retrieves the singleton instance of DatabaseManager.
     * @return Reference to DatabaseManager singleton.
     */
    static DatabaseManager& instance();

    /**
     * @brief Initializes the database connection and runs schema creation.
     * @param[in] dbPath Optional explicit path. If empty, defaults to AppDataLocation/gislite.db.
     * @return True if initialized successfully, false on critical SQL error.
     */
    bool initialize(const QString &dbPath = QString());

    /**
     * @brief Closes the database connection cleanly upon application shutdown.
     */
    void close();

    /**
     * @brief Checks if the database connection is open and active.
     * @return True if open, false otherwise.
     */
    [[nodiscard]] bool isOpen() const;

    /**
     * @brief Retrieves the underlying Qt SQL database handle.
     * @return QSqlDatabase instance.
     */
    [[nodiscard]] QSqlDatabase database() const;

    /**
     * @brief Retrieves the resolved file path to the SQLite database.
     * @return Database file path string.
     */
    [[nodiscard]] QString databasePath() const { return m_dbPath; }

    /**
     * @brief Saves a key-value setting into the persistent database.
     * @param[in] key Setting key identifier.
     * @param[in] value Setting value string.
     * @return True if stored successfully, false otherwise.
     */
    bool setSetting(const QString &key, const QString &value);

    /**
     * @brief Retrieves a key-value setting from persistent storage.
     * @param[in] key Setting key identifier.
     * @param[in] defaultValue Value to return if key does not exist.
     * @return Stored value string or defaultValue.
     */
    [[nodiscard]] QString getSetting(const QString &key, const QString &defaultValue = QString()) const;

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    virtual ~DatabaseManager() override;

    Q_DISABLE_COPY(DatabaseManager)

    /**
     * @brief Executes DDL SQL statements to create necessary database tables.
     * @return True if schema created or already exists, false on failure.
     */
    bool createTables();

    /// Absolute file system path to the active SQLite file
    QString m_dbPath;

    /// Connection name tag for QSqlDatabase
    QString m_connectionName;
};

} // namespace GISApp::Database

#endif // DATABASEMANAGER_H

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QObject>
#include <QString>

/**
 * @class DatabaseManager
 * @brief Manages MySQL database connections and query execution
 *
 * This class provides a wrapper around QSqlDatabase for managing MySQL connections.
 * It handles connection lifecycle, query execution, and error reporting through signals.
 * Thread-safe when using separate connection names for each thread.
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    /**
     * @brief Establishes connection to MySQL database
     * @param hostname Database server address (e.g., "localhost")
     * @param databaseName Name of the database to connect to
     * @param username Database user credentials
     * @param password Database password
     * @param port MySQL server port (default: 3306)
     * @param connectionName Unique identifier for this connection (important for multi-threading)
     * @return true if connection successful, false otherwise
     * @note Emits connectionStatusChanged() signal on success/failure
     */
    bool connectToDatabase(const QString &hostname, const QString &databaseName,
                          const QString &username, const QString &password,
                          int port = 3306, const QString &connectionName = "QMYSQL");

    /**
     * @brief Closes the database connection
     * @note Emits connectionStatusChanged(false) signal
     */
    void disconnectFromDatabase();

    /**
     * @brief Check if database connection is active
     * @return true if database is connected and open
     */
    bool isConnected() const;

    /**
     * @brief Executes a SQL query without return values (INSERT, UPDATE, DELETE)
     * @param query SQL query string to execute
     * @return true if query executed successfully, false on error
     * @note Use this for INSERT, UPDATE, DELETE operations. For SELECT, use prepareQuery()
     */
    bool executeQuery(const QString &query);

    /**
     * @brief Prepares a SQL query for execution (typically SELECT statements)
     * @param query SQL query string to prepare
     * @return QSqlQuery object ready for execution
     * @note Use this for SELECT operations where you need to iterate results
     */
    QSqlQuery prepareQuery(const QString &query);

    /**
     * @brief Gets the last error message from database operations
     * @return Error message string, empty if no error
     */
    QString getLastError() const;

    /**
     * @brief Gets the underlying QSqlDatabase object
     * @return Reference to the QSqlDatabase instance
     * @note Use this when you need direct access to the database object
     */
    QSqlDatabase getDatabase() const;

private:
    QSqlDatabase m_database;  ///< Qt database connection object
    QString m_lastError;      ///< Stores the last error message for retrieval

    /**
     * @brief Internal method to store error messages
     * @param error Error message to store
     */
    void setLastError(const QString &error);

signals:
    /**
     * @brief Emitted when database connection status changes
     * @param connected true if connected, false if disconnected
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief Emitted when a database error occurs
     * @param error Error message describing what went wrong
     */
    void errorOccurred(const QString &error);
};

#endif // DATABASEMANAGER_H
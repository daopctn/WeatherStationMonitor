#include "DatabaseManager.h"
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    disconnectFromDatabase();
}

bool DatabaseManager::connectToDatabase(const QString &hostname, const QString &databaseName,
                                       const QString &username, const QString &password,
                                       int port, const QString &connectionName)
{
    // Close existing connection if already open
    if (m_database.isOpen()) {
        disconnectFromDatabase();
    }

    // Create new MySQL database connection with unique name
    // Connection name is important for multi-threading - each thread needs unique name
    m_database = QSqlDatabase::addDatabase("QMYSQL", connectionName);
    m_database.setHostName(hostname);
    m_database.setDatabaseName(databaseName);
    m_database.setUserName(username);
    m_database.setPassword(password);
    m_database.setPort(port);

    // Attempt to establish connection
    bool connected = m_database.open();

    if (!connected) {
        // Connection failed - log error and notify listeners
        QString error = QString("Failed to connect to database: %1").arg(m_database.lastError().text());
        setLastError(error);
        emit errorOccurred(error);
        qDebug() << "Database connection failed:" << error;
    } else {
        qDebug() << "Successfully connected to database";
        setLastError("");
    }

    // Notify listeners of connection status change
    emit connectionStatusChanged(connected);
    return connected;
}

void DatabaseManager::disconnectFromDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
        qDebug() << "Disconnected from database";
        emit connectionStatusChanged(false);
    }
}

bool DatabaseManager::isConnected() const
{
    return m_database.isOpen();
}

bool DatabaseManager::executeQuery(const QString &query)
{
    // Verify database connection is active before executing query
    if (!isConnected()) {
        QString error = "Database is not connected";
        setLastError(error);
        emit errorOccurred(error);
        return false;
    }

    // Create query object bound to this database connection
    QSqlQuery sqlQuery(m_database);
    bool success = sqlQuery.exec(query);

    if (!success) {
        // Query execution failed - capture and report error details
        QString error = QString("Query execution failed: %1").arg(sqlQuery.lastError().text());
        setLastError(error);
        emit errorOccurred(error);
        qDebug() << "Query failed:" << error;
    } else {
        // Clear any previous errors on successful execution
        setLastError("");
    }

    return success;
}

QSqlQuery DatabaseManager::prepareQuery(const QString &query)
{
    // Create query object for this database connection
    QSqlQuery sqlQuery(m_database);

    // Check connection status before preparing query
    if (!isConnected()) {
        QString error = "Database is not connected";
        setLastError(error);
        emit errorOccurred(error);
        return sqlQuery;  // Return empty query object
    }

    // Prepare the query (parse and validate SQL without executing)
    // This is useful for SELECT statements where you'll iterate results
    if (!sqlQuery.prepare(query)) {
        QString error = QString("Query preparation failed: %1").arg(sqlQuery.lastError().text());
        setLastError(error);
        emit errorOccurred(error);
        qDebug() << "Query preparation failed:" << error;
    } else {
        setLastError("");
    }

    return sqlQuery;
}

QString DatabaseManager::getLastError() const
{
    return m_lastError;
}

QSqlDatabase DatabaseManager::getDatabase() const
{
    return m_database;
}

void DatabaseManager::setLastError(const QString &error)
{
    m_lastError = error;
}
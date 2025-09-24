#include "DatabaseManager.h"
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    m_database = QSqlDatabase::addDatabase("QMYSQL");
}

DatabaseManager::~DatabaseManager()
{
    disconnectFromDatabase();
}

bool DatabaseManager::connectToDatabase(const QString &hostname, const QString &databaseName,
                                       const QString &username, const QString &password, int port)
{
    if (m_database.isOpen()) {
        disconnectFromDatabase();
    }

    m_database.setHostName(hostname);
    m_database.setDatabaseName(databaseName);
    m_database.setUserName(username);
    m_database.setPassword(password);
    m_database.setPort(port);

    bool connected = m_database.open();

    if (!connected) {
        QString error = QString("Failed to connect to database: %1").arg(m_database.lastError().text());
        setLastError(error);
        emit errorOccurred(error);
        qDebug() << "Database connection failed:" << error;
    } else {
        qDebug() << "Successfully connected to database";
        setLastError("");
    }

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
    if (!isConnected()) {
        QString error = "Database is not connected";
        setLastError(error);
        emit errorOccurred(error);
        return false;
    }

    QSqlQuery sqlQuery(m_database);
    bool success = sqlQuery.exec(query);

    if (!success) {
        QString error = QString("Query execution failed: %1").arg(sqlQuery.lastError().text());
        setLastError(error);
        emit errorOccurred(error);
        qDebug() << "Query failed:" << error;
    } else {
        setLastError("");
    }

    return success;
}

QSqlQuery DatabaseManager::prepareQuery(const QString &query)
{
    QSqlQuery sqlQuery(m_database);

    if (!isConnected()) {
        QString error = "Database is not connected";
        setLastError(error);
        emit errorOccurred(error);
        return sqlQuery;
    }

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

void DatabaseManager::setLastError(const QString &error)
{
    m_lastError = error;
}
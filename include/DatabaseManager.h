#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QObject>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool connectToDatabase(const QString &hostname, const QString &databaseName,
                          const QString &username, const QString &password,
                          int port = 3306);
    void disconnectFromDatabase();
    bool isConnected() const;

    bool executeQuery(const QString &query);
    QSqlQuery prepareQuery(const QString &query);

    QString getLastError() const;

private:
    QSqlDatabase m_database;
    QString m_lastError;

    void setLastError(const QString &error);

signals:
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);
};

#endif // DATABASEMANAGER_H
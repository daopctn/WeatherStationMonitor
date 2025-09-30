/*
    - inherit QThread correctly
    - Create DB connection
    - insert data into DB from share vector every 10 seconds
    - Use mutex to protect the shared vector
    - Use signal to notify MainWindow when new data is inserted
*/
#ifndef DATABASETHREAD_H
#define DATABASETHREAD_H

#include <QThread>
#include <QMutex>
#include <QVector>
#include "DatabaseManager.h"
#include "WeatherData.h"
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class DatabaseThread : public QThread
{
    Q_OBJECT
public:
    explicit DatabaseThread(DatabaseManager *dbManager, QVector<WeatherData> &weatherDataVector, QMutex &mutex, QObject *parent = nullptr);
    ~DatabaseThread();
    void run() override;
    void stop();

signals:
    void insertDataDone();
    void errorOccurred(const QString &error);

private:
    DatabaseManager *m_dbManager;
    bool m_running;
    void insertDataIntoDatabase();
};

#endif // DATABASETHREAD_H
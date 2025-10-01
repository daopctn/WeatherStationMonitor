// thread manager to handle all threads
#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>
#include <functional>
#include <DatabaseThread.h>
#include <WeatherWorker.h>
#include <DatabaseManager.h>
#include <WeatherData.h>

class ThreadManager : public QObject
{
    Q_OBJECT

public:
    explicit ThreadManager(QObject *parent = nullptr);
    ~ThreadManager();
    void startThreads();
    void stopThreads();
    void waitForThreads();

signals:
    void allThreadsFinished();

private:
    DatabaseManager *databaseManager;
    // create 5 worker and 1 database thread
    WeatherWorker *zoccaWorker;
    WeatherData *lastestZoccaData = nullptr;
    WeatherWorker *romeWorker;
    WeatherData *lastestRomeData = nullptr;
    WeatherWorker *parisWorker;
    WeatherData *lastestParisData = nullptr;
    WeatherWorker *londonWorker;
    WeatherData *lastestLondonData = nullptr;
    WeatherWorker *newYorkWorker;
    WeatherData *lastestNewYorkData = nullptr;
    DatabaseThread *databaseThread;
    QVector<WeatherData> weatherDataVector;
    QMutex mutex;
};

#endif // THREADMANAGER_H
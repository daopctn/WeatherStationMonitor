/**
 * @file ThreadManager.h
 * @brief Thread manager to coordinate all worker threads in the weather monitoring system
 */

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

/**
 * @class ThreadManager
 * @brief Manages lifecycle of all weather monitoring threads
 *
 * This class creates and manages 5 WeatherWorker threads (one per monitored city)
 * and 1 DatabaseThread for persisting data to MySQL. It handles:
 * - Thread creation and initialization
 * - Loading latest weather data from database to prevent duplicate API calls
 * - Coordinating thread shutdown and cleanup
 * - Building API URLs from configuration instead of hardcoded values
 *
 * The manager uses a shared QVector with QMutex for thread-safe data sharing
 * between workers and the database thread.
 */
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
    /**
     * @brief Helper function to load latest weather data from a specific location table
     * @param query QSqlQuery object to use for database access
     * @param tableName Name of the database table (e.g., "london", "paris")
     * @param data Pointer to WeatherData struct to populate with results
     * @note This function reduces code duplication by centralizing query logic
     */
    void loadLatestDataForLocation(QSqlQuery &query, const QString &tableName, WeatherData *data);
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
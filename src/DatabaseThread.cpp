#include "DatabaseThread.h"
#include <QThread>
#include <QRegularExpression>

// Store shared data references as member variables
static QVector<WeatherData> *s_weatherDataVector = nullptr;
static QMutex *s_mutex = nullptr;

DatabaseThread::DatabaseThread(DatabaseManager *dbManager, QVector<WeatherData> &weatherDataVector, QMutex &mutex, QObject *parent)
    : QThread(parent), m_dbManager(dbManager), m_running(false)
{
    // Store references to shared data
    s_weatherDataVector = &weatherDataVector;
    s_mutex = &mutex;
}

DatabaseThread::~DatabaseThread()
{
    qDebug() << "DatabaseThread destructor called.";
    stop();
    wait();
}

void DatabaseThread::run()
{
    m_running = true;

    // Verify database connection
    if (!m_dbManager || !m_dbManager->isConnected())
    {
        emit errorOccurred("Database is not connected");
        return;
    }

    while (m_running)
    {
        // Insert data into database
        insertDataIntoDatabase();

        // Sleep for 15 minutes, but check m_running every second for responsive shutdown
        for (int i = 0; i < 10 && m_running; ++i)
        {
            QThread::sleep(1); // Sleep 1 second at a time
        }
    }
}

void DatabaseThread::stop()
{
    m_running = false;
}

void DatabaseThread::insertDataIntoDatabase()
{
    if (!s_weatherDataVector || !s_mutex || !m_dbManager)
    {
        emit errorOccurred("Invalid shared data or database manager");
        return;
    }

    // Lock the mutex to protect shared vector access
    s_mutex->lock();

    // Copy data from shared vector
    QVector<WeatherData> dataCopy = *s_weatherDataVector;

    // Clear the shared vector after copying
    s_weatherDataVector->clear();

    // Unlock the mutex
    s_mutex->unlock();

    // Insert each weather data entry into database
    for (const WeatherData &data : dataCopy)
    {
        // Sanitize table name - allow only alphanumeric and underscore, convert to lowercase
        QString sanitizedTableName = data.locationName.toLower();
        sanitizedTableName.replace(QRegularExpression("[^a-z0-9_]"), "_");

        QString query = QString("INSERT INTO %1 (temperature, humidity, timestamp) "
                                "VALUES (%2, %3, %4)")
                            .arg(sanitizedTableName)
                            .arg(data.temperature)
                            .arg(data.humidity)
                            .arg(data.timestamp);

        if (!m_dbManager->executeQuery(query))
        {
            emit errorOccurred("Failed to insert data: " + m_dbManager->getLastError());
            qDebug() << "Database insert error:" << m_dbManager->getLastError();
        }
        else
        {
            qDebug() << "Inserted weather data for" << data.locationName
                     << "- Temp:" << data.temperature << "C, Humidity:" << data.humidity << "%";
        }
    }

    if (!dataCopy.isEmpty())
    {
        emit insertDataDone();
    }
}
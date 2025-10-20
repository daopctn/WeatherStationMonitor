#include <ThreadManager.h>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
ThreadManager::ThreadManager(QObject *parent)
    : QObject(parent)
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir); // ensure exists
    QString configPath = configDir + "/config.json";
    qDebug() << "Using config file at:" << configPath;
    QFile file(configPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open config file";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
    {
        qWarning() << "Invalid JSON format";
        return;
    }

    QJsonObject obj = doc.object();

    // Extract database configuration
    QJsonObject dbConfig = obj.value("Database").toObject();
    QString m_hostname = dbConfig.value("host").toString();
    QString m_databaseName = dbConfig.value("name").toString();
    QString m_username = dbConfig.value("user").toString();
    QString m_password = dbConfig.value("password").toString();

    // Extract Weather API configuration
    QJsonObject weatherAPIConfig = obj.value("WeatherAPI").toObject();
    QString apiKey = weatherAPIConfig.value("api_key").toString();
    QString baseUrl = weatherAPIConfig.value("base_url").toString();
    QJsonObject locations = weatherAPIConfig.value("locations").toObject();

    // Create a quick database connection to get the lastest data of each table
    // database connection
    databaseManager = new DatabaseManager(this);
    bool connection = databaseManager->connectToDatabase(
        m_hostname,
        m_databaseName,
        m_username,
        m_password,
        3306,
        "ThreadManager_connection");

    if (connection)
    {
        qDebug() << "Database connected successfully.";
    }
    else
    {
        qDebug() << "Database connection failed:" << databaseManager->getLastError();
    }

    lastestLondonData = new WeatherData();
    lastestNewYorkData = new WeatherData();
    lastestParisData = new WeatherData();
    lastestRomeData = new WeatherData();
    lastestZoccaData = new WeatherData();

    // dummy data
    lastestLondonData->locationName = "London";
    lastestLondonData->temperature = 0.0;
    lastestLondonData->humidity = 0.0;
    lastestLondonData->timestamp = 0;

    lastestNewYorkData->locationName = "New York";
    lastestNewYorkData->temperature = 0.0;
    lastestNewYorkData->humidity = 0.0;
    lastestNewYorkData->timestamp = 0;

    lastestParisData->locationName = "Paris";
    lastestParisData->temperature = 0.0;
    lastestParisData->humidity = 0.0;
    lastestParisData->timestamp = 0;

    lastestRomeData->locationName = "Rome";
    lastestRomeData->temperature = 0.0;
    lastestRomeData->humidity = 0.0;
    lastestRomeData->timestamp = 0;

    lastestZoccaData->locationName = "Zocca";
    lastestZoccaData->temperature = 0.0;
    lastestZoccaData->humidity = 0.0;
    lastestZoccaData->timestamp = 0;

    // Query latest data from database for each location
    // This prevents duplicate API calls by checking timestamp before fetching new data
    QSqlDatabase db = databaseManager->getDatabase();
    QSqlQuery q(db);

    loadLatestDataForLocation(q, "london", lastestLondonData);
    loadLatestDataForLocation(q, "new_york", lastestNewYorkData);
    loadLatestDataForLocation(q, "paris", lastestParisData);
    loadLatestDataForLocation(q, "rome", lastestRomeData);
    loadLatestDataForLocation(q, "zocca", lastestZoccaData);

    // Build API URLs from configuration (no hardcoded coordinates or API keys)
    // Format: https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=KEY
    auto buildWeatherURL = [&](const QString& locationKey) -> QString {
        QJsonObject loc = locations.value(locationKey).toObject();
        double lat = loc.value("latitude").toDouble();
        double lon = loc.value("longitude").toDouble();
        return QString("%1/weather?lat=%2&lon=%3&appid=%4")
            .arg(baseUrl)
            .arg(lat)
            .arg(lon)
            .arg(apiKey);
    };

    // Create weather worker threads for each location
    zoccaWorker = new WeatherWorker(buildWeatherURL("zocca"), weatherDataVector, mutex, this);
    zoccaWorker->lastestData = lastestZoccaData;
    connect(zoccaWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Zocca WeatherWorker thread finished."; });

    romeWorker = new WeatherWorker(buildWeatherURL("rome"), weatherDataVector, mutex, this);
    romeWorker->lastestData = lastestRomeData;
    connect(romeWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Rome WeatherWorker thread finished."; });

    parisWorker = new WeatherWorker(buildWeatherURL("paris"), weatherDataVector, mutex, this);
    parisWorker->lastestData = lastestParisData;
    connect(parisWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Paris WeatherWorker thread finished."; });

    londonWorker = new WeatherWorker(buildWeatherURL("london"), weatherDataVector, mutex, this);
    londonWorker->lastestData = lastestLondonData;
    connect(londonWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "London WeatherWorker thread finished."; });

    newYorkWorker = new WeatherWorker(buildWeatherURL("new_york"), weatherDataVector, mutex, this);
    newYorkWorker->lastestData = lastestNewYorkData;
    connect(newYorkWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "New York WeatherWorker thread finished."; });

    // database thread
    databaseThread = new DatabaseThread(databaseManager, weatherDataVector, mutex, this);
    connect(databaseThread, &DatabaseThread::finished, this, []()
            { qDebug() << "DatabaseThread finished."; });
    databaseThread->start();

    zoccaWorker->start();
    romeWorker->start();
    parisWorker->start();
    londonWorker->start();
    newYorkWorker->start();
}

ThreadManager::~ThreadManager()
{
    stopThreads();
    waitForThreads();
}

void ThreadManager::connectWeatherUpdates(QObject *receiver, const char *slot)
{
    if (!receiver || !slot)
    {
        qWarning() << "connectWeatherUpdates: Invalid receiver or slot parameter";
        return;
    }

    // Connect all weather workers' weatherDataUpdated signals to the receiver's slot
    // Qt will automatically use QueuedConnection for cross-thread signals
    if (zoccaWorker)
    {
        connect(zoccaWorker, SIGNAL(weatherDataUpdated(const WeatherData &)), receiver, slot);
    }
    if (romeWorker)
    {
        connect(romeWorker, SIGNAL(weatherDataUpdated(const WeatherData &)), receiver, slot);
    }
    if (parisWorker)
    {
        connect(parisWorker, SIGNAL(weatherDataUpdated(const WeatherData &)), receiver, slot);
    }
    if (londonWorker)
    {
        connect(londonWorker, SIGNAL(weatherDataUpdated(const WeatherData &)), receiver, slot);
    }
    if (newYorkWorker)
    {
        connect(newYorkWorker, SIGNAL(weatherDataUpdated(const WeatherData &)), receiver, slot);
    }
}

void ThreadManager::startThreads()
{
    if (zoccaWorker && !zoccaWorker->isRunning())
    {
        zoccaWorker->start();
    }
    if (romeWorker && !romeWorker->isRunning())
    {
        romeWorker->start();
    }
    if (parisWorker && !parisWorker->isRunning())
    {
        parisWorker->start();
    }
    if (londonWorker && !londonWorker->isRunning())
    {
        londonWorker->start();
    }
    if (newYorkWorker && !newYorkWorker->isRunning())
    {
        newYorkWorker->start();
    }
    if (databaseThread && !databaseThread->isRunning())
    {
        databaseThread->start();
    }
}

void ThreadManager::stopThreads()
{
    if (zoccaWorker)
    {
        zoccaWorker->stop();
    }
    if (romeWorker)
    {
        romeWorker->stop();
    }
    if (parisWorker)
    {
        parisWorker->stop();
    }
    if (londonWorker)
    {
        londonWorker->stop();
    }
    if (newYorkWorker)
    {
        newYorkWorker->stop();
    }
    if (databaseThread)
    {
        databaseThread->stop();
    }
}

void ThreadManager::waitForThreads()
{
    if (zoccaWorker)
    {
        zoccaWorker->wait();
        delete zoccaWorker;
        zoccaWorker = nullptr;
    }
    if (romeWorker)
    {
        romeWorker->wait();
        delete romeWorker;
        romeWorker = nullptr;
    }
    if (parisWorker)
    {
        parisWorker->wait();
        delete parisWorker;
        parisWorker = nullptr;
    }
    if (londonWorker)
    {
        londonWorker->wait();
        delete londonWorker;
        londonWorker = nullptr;
    }
    if (newYorkWorker)
    {
        newYorkWorker->wait();
        delete newYorkWorker;
        newYorkWorker = nullptr;
    }
    if (databaseThread)
    {
        databaseThread->wait();
        delete databaseThread;
        databaseThread = nullptr;
    }

    // free latest data
    if (lastestLondonData)
    {
        delete lastestLondonData;
        lastestLondonData = nullptr;
    }
    if (lastestNewYorkData)
    {
        delete lastestNewYorkData;
        lastestNewYorkData = nullptr;
    }
    if (lastestParisData)
    {
        delete lastestParisData;
        lastestParisData = nullptr;
    }
    if (lastestRomeData)
    {
        delete lastestRomeData;
        lastestRomeData = nullptr;
    }
    if (lastestZoccaData)
    {
        delete lastestZoccaData;
        lastestZoccaData = nullptr;
    }
}

void ThreadManager::loadLatestDataForLocation(QSqlQuery &query, const QString &tableName, WeatherData *data)
{
    // Prepare query to fetch most recent weather record from specified table
    // Orders by ID descending to get latest entry, limits to 1 result
    QString queryStr = QString("SELECT temperature, humidity, timestamp FROM %1 ORDER BY id DESC LIMIT 1").arg(tableName);
    query.prepare(queryStr);

    if (query.exec() && query.next())
    {
        // Successfully retrieved data - populate WeatherData struct
        data->temperature = query.value(0).toDouble();
        data->humidity = query.value(1).toDouble();
        data->timestamp = query.value(2).toLongLong();

        // Capitalize first letter of location name for display
        QString displayName = tableName;
        displayName[0] = displayName[0].toUpper();
        // Handle underscore in "new_york" -> "New York"
        displayName.replace('_', ' ');
        if (displayName.contains(' ')) {
            QStringList parts = displayName.split(' ');
            for (QString &part : parts) {
                part[0] = part[0].toUpper();
            }
            displayName = parts.join(' ');
        }
        data->locationName = displayName;

        qDebug() << "Latest" << displayName << "Data:"
                 << " Temp:" << data->temperature
                 << " Humidity:" << data->humidity
                 << " Time:" << data->timestamp;
    }
    else
    {
        // No data found in table - this is normal for first run
        qDebug() << "No data found in" << tableName << "table.";
    }

    // Clean up query for next use
    query.finish();
}

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
    // Database
    QJsonObject dbConfig = obj.value("Database").toObject();
    QString m_hostname = dbConfig.value("host").toString();
    QString m_databaseName = dbConfig.value("name").toString();
    QString m_username = dbConfig.value("user").toString();
    QString m_password = dbConfig.value("password").toString();

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

    // query lastest data
    QSqlDatabase db = databaseManager->getDatabase();
    QSqlQuery q(db);
    q.prepare("SELECT temperature, humidity, timestamp FROM london ORDER BY id DESC LIMIT 1");
    if (q.exec() && q.next())
    {
        lastestLondonData->locationName = "London";
        lastestLondonData->temperature = q.value(0).toDouble();
        lastestLondonData->humidity = q.value(1).toDouble();
        lastestLondonData->timestamp = q.value(2).toLongLong();
        qDebug() << "Lastest London Data:"
                 << " Temp:" << lastestLondonData->temperature
                 << " Humidity:" << lastestLondonData->humidity
                 << " Time:" << lastestLondonData->timestamp;
    }
    else
    {
        qDebug() << "No data found in london table.";
    }
    q.finish();
    q.prepare("SELECT temperature, humidity, timestamp FROM new_york ORDER BY id DESC LIMIT 1");
    if (q.exec() && q.next())
    {
        lastestNewYorkData->locationName = "New York";
        lastestNewYorkData->temperature = q.value(0).toDouble();
        lastestNewYorkData->humidity = q.value(1).toDouble();
        lastestNewYorkData->timestamp = q.value(2).toLongLong();
        qDebug() << "Lastest New York Data:"
                 << " Temp:" << lastestNewYorkData->temperature
                 << " Humidity:" << lastestNewYorkData->humidity
                 << " Time:" << lastestNewYorkData->timestamp;
    }
    else
    {
        qDebug() << "No data found in new_york table.";
    }
    q.finish();
    q.prepare("SELECT temperature, humidity, timestamp FROM paris ORDER BY id DESC LIMIT 1");
    if (q.exec() && q.next())
    {
        lastestParisData->locationName = "Paris";
        lastestParisData->temperature = q.value(0).toDouble();
        lastestParisData->humidity = q.value(1).toDouble();
        lastestParisData->timestamp = q.value(2).toLongLong();
        qDebug() << "Lastest Paris Data:"
                 << " Temp:" << lastestParisData->temperature
                 << " Humidity:" << lastestParisData->humidity
                 << " Time:" << lastestParisData->timestamp;
    }
    else
    {
        qDebug() << "No data found in paris table.";
    }
    q.finish();
    q.prepare("SELECT temperature, humidity, timestamp FROM rome ORDER BY id DESC LIMIT 1");
    if (q.exec() && q.next())
    {
        lastestRomeData->locationName = "Rome";
        lastestRomeData->temperature = q.value(0).toDouble();
        lastestRomeData->humidity = q.value(1).toDouble();
        lastestRomeData->timestamp = q.value(2).toLongLong();
        qDebug() << "Lastest Rome Data:"
                 << " Temp:" << lastestRomeData->temperature
                 << " Humidity:" << lastestRomeData->humidity
                 << " Time:" << lastestRomeData->timestamp;
    }
    else
    {
        qDebug() << "No data found in rome table.";
    }
    q.finish();
    q.prepare("SELECT temperature, humidity, timestamp FROM zocca ORDER BY id DESC LIMIT 1");
    if (q.exec() && q.next())
    {
        lastestZoccaData->locationName = "Zocca";
        lastestZoccaData->temperature = q.value(0).toDouble();
        lastestZoccaData->humidity = q.value(1).toDouble();
        lastestZoccaData->timestamp = q.value(2).toLongLong();
        qDebug() << "Lastest Zocca Data:"
                 << " Temp:" << lastestZoccaData->temperature
                 << " Humidity:" << lastestZoccaData->humidity
                 << " Time:" << lastestZoccaData->timestamp;
    }
    else
    {
        qDebug() << "No data found in zocca table.";
    }
    q.finish();

    zoccaWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=a37d50cf573ace59c09175f7f0e7f164", weatherDataVector, mutex, this);
    zoccaWorker->lastestData = lastestZoccaData;
    connect(zoccaWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Zocca WeatherWorker thread finished."; });

    romeWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=41.89&lon=12.49&appid=a37d50cf573ace59c09175f7f0e7f164", weatherDataVector, mutex, this);
    romeWorker->lastestData = lastestRomeData;
    connect(romeWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Rome WeatherWorker thread finished."; });

    parisWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=48.85&lon=2.35&appid=a37d50cf573ace59c09175f7f0e7f164", weatherDataVector, mutex, this);
    parisWorker->lastestData = lastestParisData;
    connect(parisWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Paris WeatherWorker thread finished."; });

    londonWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=51.51&lon=-0.13&appid=a37d50cf573ace59c09175f7f0e7f164", weatherDataVector, mutex, this);
    londonWorker->lastestData = lastestLondonData;
    connect(londonWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "London WeatherWorker thread finished."; });

    newYorkWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=40.71&lon=-74.01&appid=a37d50cf573ace59c09175f7f0e7f164", weatherDataVector, mutex, this);
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

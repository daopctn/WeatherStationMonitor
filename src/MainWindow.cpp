#include "../include/MainWindow.h"
#include <QDebug>
#include <QFile>
#include <PythonBridge.h>
#include <WeatherWorker.h>
#include <DatabaseThread.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), pythonBridge(new PythonBridge())
{
    // ui->setupUi(this);
    // setWindowTitle("Weather Station Monitor");

    // QFile file("/home/daopctn/Projects/WeatherStationMonitor/config.json");

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
    m_hostname = dbConfig.value("host").toString();
    m_databaseName = dbConfig.value("name").toString();
    m_username = dbConfig.value("user").toString();
    m_password = dbConfig.value("password").toString();

    // Create a quick database connection to get the lastest data of each table
    // database connection
    databaseManager = new DatabaseManager(this);
    bool connection = databaseManager->connectToDatabase(
        m_hostname,
        m_databaseName,
        m_username,
        m_password);

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
    QSqlQuery q("SELECT temperature, humidity, timestamp FROM london ORDER BY id DESC LIMIT 1");
    if (q.next())
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

    // // weather fetcher
    // weatherFetcher = new WeatherFetcher(this, databaseManager, pythonBridge);

    // // // activate timer
    // m_fetchTimer = new QTimer(this);
    // connect(m_fetchTimer, &QTimer::timeout, this, &MainWindow::fetchWeatherForAllLocations);
    // // m_fetchTimer->start(3600000); // fetch every 60 minutes
    // // m_fetchTimer->start(60000);   // fetch every 1 minute
    // m_fetchTimer->start(10000); // fetch every 10 seconds (for testing)
    // // signals and slots

    // connect(weatherFetcher, &WeatherFetcher::insertDataDone,
    //         this, &MainWindow::onInsertDataDone);
    // connect(weatherFetcher, &WeatherFetcher::errorOccurred,
    //         this, &MainWindow::onInsertDataDone);
    // connect(ui->pushButton, &QPushButton::clicked,
    //         this, &MainWindow::onButtonClicked);
    // MainWindow::onButtonClicked();

    // Testing the WeatherWorker class

    QList<Location> locations = {
        {44.34, 10.99, "Zocca"},
        {41.89, 12.49, "Rome"},
        {48.85, 2.35, "Paris"},
        {51.51, -0.13, "London"},
        {40.71, -74.01, "New York"}};

    // QString apiURL = "https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=a37d50cf573ace59c09175f7f0e7f164";
    // m_weatherWorker = new WeatherWorker(apiURL, m_weatherDataVector, m_mutex, this);
    // connect(m_weatherWorker, &WeatherWorker::finished, this, []()
    //         { qDebug() << "WeatherWorker thread finished."; });
    // m_weatherWorker->start();

    zoccaWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=a37d50cf573ace59c09175f7f0e7f164", m_weatherDataVector, m_mutex, this);
    zoccaWorker->lastestData = lastestZoccaData;
    connect(zoccaWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Zocca WeatherWorker thread finished."; });

    romeWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=41.89&lon=12.49&appid=a37d50cf573ace59c09175f7f0e7f164", m_weatherDataVector, m_mutex, this);
    romeWorker->lastestData = lastestRomeData;
    connect(romeWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Rome WeatherWorker thread finished."; });

    parisWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=48.85&lon=2.35&appid=a37d50cf573ace59c09175f7f0e7f164", m_weatherDataVector, m_mutex, this);
    parisWorker->lastestData = lastestParisData;
    connect(parisWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "Paris WeatherWorker thread finished."; });

    londonWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=51.51&lon=-0.13&appid=a37d50cf573ace59c09175f7f0e7f164", m_weatherDataVector, m_mutex, this);
    londonWorker->lastestData = lastestLondonData;
    connect(londonWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "London WeatherWorker thread finished."; });

    newYorkWorker = new WeatherWorker("https://api.openweathermap.org/data/2.5/weather?lat=40.71&lon=-74.01&appid=a37d50cf573ace59c09175f7f0e7f164", m_weatherDataVector, m_mutex, this);
    newYorkWorker->lastestData = lastestNewYorkData;
    connect(newYorkWorker, &WeatherWorker::finished, this, []()
            { qDebug() << "New York WeatherWorker thread finished."; });

    // database thread
    DatabaseThread *dbThread = new DatabaseThread(databaseManager, m_weatherDataVector, m_mutex, this);
    connect(dbThread, &DatabaseThread::finished, this, []()
            { qDebug() << "DatabaseThread finished."; });
    dbThread->start();

    zoccaWorker->start();
    romeWorker->start();
    parisWorker->start();
    londonWorker->start();
    newYorkWorker->start();
}

void MainWindow::fetchWeatherForAllLocations()
{
    // qDebug() << "Fetching weather for all locations...";
    QList<Location> locations = {
        {44.34, 10.99, "Zocca"},
        {41.89, 12.49, "Rome"},
        {48.85, 2.35, "Paris"},
        {51.51, -0.13, "London"},
        {40.71, -74.01, "New York"}};

    weatherFetcher->fetchMultipleWeather(locations);
}

MainWindow::~MainWindow()
{
    // // Safe delete ui
    // if (ui)
    // {
    //     delete ui;
    //     ui = nullptr;
    // }

    // // Safe cleanup of database manager
    // if (databaseManager)
    // {
    //     if (databaseManager->isConnected())
    //     {
    //         databaseManager->disconnectFromDatabase();
    //     }
    //     delete databaseManager;
    //     databaseManager = nullptr;
    // }

    // // Safe delete weather fetcher
    // if (weatherFetcher)
    // {
    //     delete weatherFetcher;
    //     weatherFetcher = nullptr;
    // }

    // free weather workers
}

void MainWindow::onInsertDataDone()
{
}

void MainWindow::onErrorOccurred(const QString &error)
{
    // ui->label_4->setText("Error: " + error);
}

void MainWindow::onButtonClicked()
{
    // update ui
    if (databaseManager->isConnected())
    {
        QStringList tables = {"london", "new_york", "paris", "rome", "zocca"};
        int row = 0;
        ui->tableWidget->setRowCount(0);
        for (const QString &tbl : tables)
        {
            QSqlQuery q("SELECT temperature, humidity, time FROM " + tbl + " ORDER BY time DESC LIMIT 1");
            qDebug() << "Querying table:" << tbl;
            if (q.next())
            {
                ui->tableWidget->insertRow(row);
                ui->tableWidget->setItem(row, 0, new QTableWidgetItem(tbl));
                ui->tableWidget->setItem(row, 1, new QTableWidgetItem(q.value(0).toString()));
                ui->tableWidget->setItem(row, 2, new QTableWidgetItem(q.value(1).toString()));
                pythonBridge->calculateAverageData(
                    m_hostname,
                    m_databaseName,
                    m_username,
                    m_password,
                    tbl,
                    m_avgTemperature,
                    m_avgHumidity);
                double avgTemp = m_avgTemperature;
                double avgHumidity = m_avgHumidity;
                ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(avgTemp) + " °C"));
                ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(avgHumidity) + " %"));
                ui->tableWidget->setItem(row, 5, new QTableWidgetItem(q.value(2).toString()));
                row++;
            }
        }
    }
    else
    {
        qDebug() << "Database not connected.";
    }
}
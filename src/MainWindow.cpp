#include "../include/MainWindow.h"
#include <QDebug>
#include <QFile>
#include <PythonBridge.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), pythonBridge(new PythonBridge())
{
    ui->setupUi(this);
    setWindowTitle("Weather Station Monitor");

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

    // weather fetcher
    weatherFetcher = new WeatherFetcher(this, databaseManager, pythonBridge);

    // // activate timer
    m_fetchTimer = new QTimer(this);
    connect(m_fetchTimer, &QTimer::timeout, this, &MainWindow::fetchWeatherForAllLocations);
    m_fetchTimer->start(3600000); // fetch every 60 seconds

    // signals and slots

    connect(weatherFetcher, &WeatherFetcher::insertDataDone,
            this, &MainWindow::onInsertDataDone);
    connect(weatherFetcher, &WeatherFetcher::errorOccurred,
            this, &MainWindow::onInsertDataDone);
    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::onButtonClicked);
    // MainWindow::onButtonClicked();
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
    delete ui;
    if (databaseManager->isConnected())
    {
        databaseManager->disconnectFromDatabase();
    }
    delete databaseManager;
    delete weatherFetcher;
}

void MainWindow::onInsertDataDone()
{
}

void MainWindow::onErrorOccurred(const QString &error)
{
    ui->label_4->setText("Error: " + error);
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
        ui->label_4->setText("DB not connected");
        ui->label_5->setText("");
        ui->label_6->setText("");
    }
}
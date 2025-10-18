#include "../include/MainWindow.h"
#include <QDebug>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <PythonBridge.h>

// Anonymous namespace for helper functions
namespace {
    QString getWeatherIconPath(int weatherId, long long timestamp,
                               long long sunrise, long long sunset)
    {
        // Determine day or night
        bool isDay = (timestamp >= sunrise && timestamp < sunset);

        // Map weatherId to OpenWeatherMap icon code
        QString iconCode;
        if (weatherId >= 200 && weatherId < 300) {
            iconCode = "11";  // Thunderstorm
        } else if (weatherId >= 300 && weatherId < 400) {
            iconCode = "09";  // Drizzle
        } else if (weatherId >= 500 && weatherId < 600) {
            iconCode = "10";  // Rain
        } else if (weatherId >= 600 && weatherId < 700) {
            iconCode = "13";  // Snow
        } else if (weatherId >= 700 && weatherId < 800) {
            iconCode = "50";  // Atmosphere (fog, mist, etc.)
        } else if (weatherId == 800) {
            iconCode = "01";  // Clear sky
        } else if (weatherId == 801) {
            iconCode = "02";  // Few clouds (11-25%)
        } else if (weatherId == 802) {
            iconCode = "03";  // Scattered clouds (25-50%)
        } else if (weatherId >= 803 && weatherId <= 804) {
            iconCode = "04";  // Broken/Overcast clouds (51-100%)
        } else {
            iconCode = "01";  // Default to clear sky
        }

        // Determine day/night suffix
        QString dayNight = isDay ? "d" : "n";

        // Return resource path
        return QString(":/weather_icons/%1%2@2x.png").arg(iconCode, dayNight);
    }

    /**
     * @brief Updates UI widgets for a specific location with weather data
     * Reduces code duplication by centralizing UI update logic for all 5 locations
     */
    void updateLocationUI(Ui::MainWindow *ui, int locationIndex,
                         double temperature, const QString &description,
                         double humidity, double windSpeed, int pressure,
                         const QString &dateStr, const QString &timeStr,
                         int weatherId, long long unixTime,
                         long long sunrise, long long sunset)
    {
        // Get appropriate weather icon
        QString iconPath = getWeatherIconPath(weatherId, unixTime, sunrise, sunset);
        QPixmap weatherIcon(iconPath);
        QPixmap scaledIcon = weatherIcon.scaled(MainWindow::WEATHER_ICON_SIZE, MainWindow::WEATHER_ICON_SIZE,
                                                Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Update UI widgets based on location index
        switch (locationIndex) {
        case 0:
            ui->temp0->setText(QString::number(temperature, 'f', 1) + "°C");
            ui->describe0->setText(description);
            ui->humLabel0->setText(QString::number(humidity, 'f', 0) + "%");
            ui->windLabel0->setText(QString::number(windSpeed, 'f', 1) + " km/h");
            ui->pressLabel0->setText(QString::number(pressure) + " hPa");
            ui->date0->setText(dateStr);
            ui->time0->setText(timeStr);
            ui->icon0->setPixmap(scaledIcon);
            break;
        case 1:
            ui->temp1->setText(QString::number(temperature, 'f', 1) + "°C");
            ui->describe1->setText(description);
            ui->humLabel1->setText(QString::number(humidity, 'f', 0) + "%");
            ui->windLabel1->setText(QString::number(windSpeed, 'f', 1) + " km/h");
            ui->pressLabel1->setText(QString::number(pressure) + " hPa");
            ui->date1->setText(dateStr);
            ui->time1->setText(timeStr);
            ui->icon1->setPixmap(scaledIcon);
            break;
        case 2:
            ui->temp2->setText(QString::number(temperature, 'f', 1) + "°C");
            ui->describe2->setText(description);
            ui->humLabel2->setText(QString::number(humidity, 'f', 0) + "%");
            ui->windLabel2->setText(QString::number(windSpeed, 'f', 1) + " km/h");
            ui->pressLabel2->setText(QString::number(pressure) + " hPa");
            ui->date2->setText(dateStr);
            ui->time2->setText(timeStr);
            ui->icon2->setPixmap(scaledIcon);
            break;
        case 3:
            ui->temp3->setText(QString::number(temperature, 'f', 1) + "°C");
            ui->describe3->setText(description);
            ui->humLabel3->setText(QString::number(humidity, 'f', 0) + "%");
            ui->windLabel3->setText(QString::number(windSpeed, 'f', 1) + " km/h");
            ui->pressLabel3->setText(QString::number(pressure) + " hPa");
            ui->date3->setText(dateStr);
            ui->time3->setText(timeStr);
            ui->icon3->setPixmap(scaledIcon);
            break;
        case 4:
            ui->temp4->setText(QString::number(temperature, 'f', 1) + "°C");
            ui->describe4->setText(description);
            ui->humLabel4->setText(QString::number(humidity, 'f', 0) + "%");
            ui->windLabel4->setText(QString::number(windSpeed, 'f', 1) + " km/h");
            ui->pressLabel4->setText(QString::number(pressure) + " hPa");
            ui->date4->setText(dateStr);
            ui->time4->setText(timeStr);
            ui->icon4->setPixmap(scaledIcon);
            break;
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), pythonBridge(new PythonBridge()), m_dbManager(nullptr)
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

    // Create persistent database connection
    m_dbManager = new DatabaseManager();
    bool connected = m_dbManager->connectToDatabase(
        m_hostname,
        m_databaseName,
        m_username,
        m_password,
        3306,
        "MainWindow_connection");

    if (!connected)
    {
        qWarning() << "Failed to connect to database:" << m_dbManager->getLastError();
    }

    // Thread manager
    threadManager = new ThreadManager(this);
    threadManager->startThreads();

    // Khởi động spinner
    m_spinner = new Spinner(this);
    m_spinner->start();

    // Setup data update timer to refresh UI with latest weather data
    dataUpdateTimer = new QTimer(this);
    connect(dataUpdateTimer, &QTimer::timeout, this, &MainWindow::onButtonClicked);
    dataUpdateTimer->start(UI_UPDATE_INTERVAL_MS);

    // Update UI immediately on startup
    onButtonClicked();
}

MainWindow::~MainWindow()
{
    // Safe delete ui
    if (ui)
    {
        delete ui;
        ui = nullptr;
    }

    // Close database connection
    if (m_dbManager)
    {
        m_dbManager->disconnectFromDatabase();
        delete m_dbManager;
        m_dbManager = nullptr;
    }

    // free ThreadManager
    if (threadManager)
    {
        threadManager->stopThreads();
        threadManager->waitForThreads();
        delete threadManager;
        threadManager = nullptr;
    }

    // free python bridge
    if (pythonBridge)
    {
        delete pythonBridge;
        pythonBridge = nullptr;
    }

    // Dừng spinner
    if (m_spinner)
    {
        m_spinner->stop();
        m_spinner->wait();
        delete m_spinner;
        m_spinner = nullptr;
    }
}

void MainWindow::onButtonClicked()
{
    // qDebug() << "onButtonClicked() called - Updating UI...";

    // Pause spinner during operation
    if (m_spinner)
    {
        m_spinner->pause();
    }

    // Check if database connection is valid
    if (!m_dbManager)
    {
        qWarning() << "Database manager not initialized";
        if (m_spinner)
        {
            m_spinner->resume();
        }
        return;
    }

    QSqlDatabase db = m_dbManager->getDatabase();

    // Verify connection is still alive
    if (!db.isOpen())
    {
        qWarning() << "Database connection is not open";
        if (m_spinner)
        {
            m_spinner->resume();
        }
        return;
    }

    // Update UI for each location
    QStringList tables = {"zocca", "rome", "paris", "new_york", "london"};

    for (int i = 0; i < tables.size(); ++i)
    {
        QString tableName = tables[i];

        // Query the latest weather data for this location
        QSqlQuery query(db);
        QString queryStr = QString("SELECT temperature, humidity, pressure, windSpeed, description, timestamp, weather_id, sunrise, sunset "
                                   "FROM %1 ORDER BY timestamp DESC LIMIT 1")
                               .arg(tableName);

        if (!query.exec(queryStr))
        {
            qDebug() << "Query failed for" << tableName << ":" << query.lastError().text();
            continue;
        }

        if (query.next())
        {
            // Retrieve data from database
            double temperatureKelvin = query.value(0).toDouble();
            double humidity = query.value(1).toDouble();
            int pressure = query.value(2).toInt();
            double windSpeed = query.value(3).toDouble();
            QString description = query.value(4).toString();
            long long unixTime = query.value(5).toLongLong();
            int weatherId = query.value(6).toInt();
            long long sunrise = query.value(7).toLongLong();
            long long sunset = query.value(8).toLongLong();

            // Convert Kelvin to Celsius
            double temperature = temperatureKelvin - 273.15;

            // Convert Unix timestamp to formatted date/time strings
            QDateTime dateTime = QDateTime::fromSecsSinceEpoch(unixTime);
            QString dateStr = dateTime.toString("dd/MM/yyyy");
            QString timeStr = dateTime.toString("hh:mm:ss");

            // Update UI for this location using helper function
            updateLocationUI(ui, i, temperature, description, humidity, windSpeed,
                           pressure, dateStr, timeStr, weatherId, unixTime, sunrise, sunset);
        }
    }

    // Resume spinner after operation
    if (m_spinner)
    {
        m_spinner->resume();
    }
}
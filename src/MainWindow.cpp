#include "../include/MainWindow.h"
#include <QDebug>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDate>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QMargins>
#include <QToolTip>
#include <QCursor>
#include <QVBoxLayout>
#include <QLabel>
#include <QtCharts/QLegend>
#include <PythonBridge.h>

// Initialize location configuration data (shared across all methods to follow DRY principle)
const QVector<MainWindow::LocationInfo> MainWindow::LOCATIONS = {
    {"zocca", "Zocca", 7200, 0},         // UTC+2 (Europe/Rome timezone)
    {"rome", "Rome", 7200, 1},           // UTC+2 (Europe/Rome timezone)
    {"paris", "Paris", 7200, 2},         // UTC+2 (Europe/Paris timezone)
    {"new_york", "New York", -14400, 3}, // UTC-4 (America/New_York with DST)
    {"london", "London", 3600, 4}        // UTC+1 (Europe/London with BST)
};

// Anonymous namespace for helper functions
namespace
{
    QString getWeatherIconPath(int weatherId, long long timestamp,
                               long long sunrise, long long sunset)
    {
        // Determine day or night
        bool isDay = (timestamp >= sunrise && timestamp < sunset);

        // Map weatherId to OpenWeatherMap icon code
        QString iconCode;
        if (weatherId >= 200 && weatherId < 300)
        {
            iconCode = "11"; // Thunderstorm
        }
        else if (weatherId >= 300 && weatherId < 400)
        {
            iconCode = "09"; // Drizzle
        }
        else if (weatherId >= 500 && weatherId < 600)
        {
            iconCode = "10"; // Rain
        }
        else if (weatherId >= 600 && weatherId < 700)
        {
            iconCode = "13"; // Snow
        }
        else if (weatherId >= 700 && weatherId < 800)
        {
            iconCode = "50"; // Atmosphere (fog, mist, etc.)
        }
        else if (weatherId == 800)
        {
            iconCode = "01"; // Clear sky
        }
        else if (weatherId == 801)
        {
            iconCode = "02"; // Few clouds (11-25%)
        }
        else if (weatherId == 802)
        {
            iconCode = "03"; // Scattered clouds (25-50%)
        }
        else if (weatherId >= 803 && weatherId <= 804)
        {
            iconCode = "04"; // Broken/Overcast clouds (51-100%)
        }
        else
        {
            iconCode = "01"; // Default to clear sky
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
        switch (locationIndex)
        {
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
    : QMainWindow(parent), ui(new Ui::MainWindow), pythonBridge(new PythonBridge()), m_dbManager(nullptr), m_tempSeries{nullptr, nullptr, nullptr, nullptr, nullptr}, m_humSeries{nullptr, nullptr, nullptr, nullptr, nullptr}, m_tempMarkers{nullptr, nullptr, nullptr, nullptr, nullptr}, m_humMarkers{nullptr, nullptr, nullptr, nullptr, nullptr}, m_zeroLine{nullptr, nullptr, nullptr, nullptr, nullptr}, m_axisX{nullptr, nullptr, nullptr, nullptr, nullptr}, m_timezoneOffsets{0, 0, 0, 0, 0}, m_hasVirtualPoint{false, false, false, false, false}, m_patternContainers{nullptr, nullptr, nullptr, nullptr, nullptr}
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

    // Connect weather workers to receive real-time updates
    threadManager->connectWeatherUpdates(this, SLOT(onWeatherDataUpdated(const WeatherData &)));

    // Khởi động spinner
    m_spinner = new Spinner(this);
    m_spinner->start();

    // Update UI immediately on startup with existing database data
    // After this, UI will be updated in real-time via weatherDataUpdated signals
    refreshWeatherUI();

    // Initialize charts with historical data
    initializeCharts();

    // Update statistics labels on startup
    updateStatisticsLabels();
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

void MainWindow::refreshWeatherUI()
{
    // Pause spinner during UI refresh operation
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

    // Update UI for each location using centralized configuration
    for (const LocationInfo &location : LOCATIONS)
    {
        // Query the latest weather data for this location
        QSqlQuery query(db);
        QString queryStr = QString("SELECT temperature, humidity, pressure, windSpeed, description, timestamp, weather_id, sunrise, sunset "
                                   "FROM %1 ORDER BY timestamp DESC LIMIT 1")
                               .arg(location.tableName);

        if (!query.exec(queryStr))
        {
            qDebug() << "Query failed for" << location.tableName << ":" << query.lastError().text();
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

            // Convert Unix timestamp to local time using timezone offset
            QDateTime dateTimeUTC = QDateTime::fromSecsSinceEpoch(unixTime, Qt::UTC);
            QDateTime dateTimeLocal = dateTimeUTC.addSecs(location.timezoneOffset);
            QString dateStr = dateTimeLocal.toString("dd/MM/yyyy");
            QString timeStr = dateTimeLocal.toString("hh:mm:ss");

            // Update UI for this location using helper function
            updateLocationUI(ui, location.uiIndex, temperature, description, humidity, windSpeed,
                             pressure, dateStr, timeStr, weatherId, unixTime, sunrise, sunset);
        }
    }

    // Resume spinner after operation
    if (m_spinner)
    {
        m_spinner->resume();
    }
}

void MainWindow::initializeCharts()
{
    // Map UI indices to chart view widgets
    QChartView *chartViews[] = {
        ui->chartViewZocca,   // Index 0
        ui->chartViewRome,    // Index 1
        ui->chartViewParis,   // Index 2
        ui->chartViewNewYork, // Index 3
        ui->chartViewLondon   // Index 4
    };

    constexpr int numChartViews = sizeof(chartViews) / sizeof(chartViews[0]);

    // Initialize charts using centralized location configuration
    for (const LocationInfo &location : LOCATIONS)
    {
        if (location.uiIndex < 0 || location.uiIndex >= numChartViews)
        {
            qWarning() << "Invalid uiIndex" << location.uiIndex << "for location" << location.displayName;
            continue;
        }
        setupChart(chartViews[location.uiIndex], location.tableName,
                   location.displayName, location.timezoneOffset, location.uiIndex);
    }
}

void MainWindow::onWeatherDataUpdated(const WeatherData &data)
{
    // Pause spinner during UI update
    if (m_spinner)
    {
        m_spinner->pause();
    }

    // Find location configuration by display name
    const LocationInfo *location = nullptr;
    for (const LocationInfo &loc : LOCATIONS)
    {
        if (loc.displayName == data.locationName)
        {
            location = &loc;
            break;
        }
    }

    if (!location)
    {
        qWarning() << "Unknown location:" << data.locationName;
        if (m_spinner)
        {
            m_spinner->resume();
        }
        return;
    }

    // Convert Kelvin to Celsius
    double temperature = data.temperature - 273.15;

    // Convert Unix timestamp to local time using timezone offset
    QDateTime dateTimeUTC = QDateTime::fromSecsSinceEpoch(data.timestamp, Qt::UTC);
    QDateTime dateTimeLocal = dateTimeUTC.addSecs(location->timezoneOffset);
    QString dateStr = dateTimeLocal.toString("dd/MM/yyyy");
    QString timeStr = dateTimeLocal.toString("hh:mm:ss");

    // Update UI for this location using helper function
    updateLocationUI(ui, location->uiIndex, temperature, data.description, data.humidity, data.windSpeed,
                     data.pressure, dateStr, timeStr, data.weatherId, data.timestamp, data.sunrise, data.sunset);

    // Update chart in real-time with new data point
    updateChartRealtime(location->uiIndex, data);

    qDebug() << "Real-time UI update for" << data.locationName << "- Temp:" << temperature << "°C";

    // Resume spinner after operation
    if (m_spinner)
    {
        m_spinner->resume();
    }
}

void MainWindow::setupChart(QChartView *chartView, const QString &tableName, const QString &locationName, int timezoneOffset, int locationIndex)
{
    if (!m_dbManager || !m_dbManager->isConnected())
    {
        qWarning() << "Database not connected, cannot setup chart for" << locationName;
        return;
    }

    // Store timezone offset for this location
    if (locationIndex >= 0 && locationIndex < 5)
    {
        m_timezoneOffsets[locationIndex] = timezoneOffset;
    }

    QSqlDatabase db = m_dbManager->getDatabase();

    // Query last 100 records ordered by timestamp ascending (oldest first)
    QSqlQuery query(db);
    QString queryStr = QString("SELECT temperature, humidity, timestamp FROM %1 ORDER BY timestamp DESC LIMIT 100")
                           .arg(tableName);

    if (!query.exec(queryStr))
    {
        qDebug() << "Chart query failed for" << tableName << ":" << query.lastError().text();
        return;
    }

    // Create series for temperature and humidity
    QLineSeries *tempSeries = new QLineSeries();
    tempSeries->setName("Temperature (°C)");

    // Make the line thicker and more visible
    QPen tempPen(QColor(230, 126, 34)); // Orange color
    tempPen.setWidth(3);
    tempSeries->setPen(tempPen);

    QLineSeries *humSeries = new QLineSeries();
    humSeries->setName("Humidity (%)");

    // Make the line thicker and more visible
    QPen humPen(QColor(41, 128, 185)); // Blue color
    humPen.setWidth(3);
    humSeries->setPen(humPen);

    // Store series references for real-time updates
    if (locationIndex >= 0 && locationIndex < 5)
    {
        m_tempSeries[locationIndex] = tempSeries;
        m_humSeries[locationIndex] = humSeries;
    }

    // Collect data points (they come in reverse order, so we need to reverse)
    QList<QPointF> tempPoints;
    QList<QPointF> humPoints;
    long long firstTimestamp = 0;
    long long lastTimestamp = 0;

    while (query.next())
    {
        double temperatureKelvin = query.value(0).toDouble();
        double humidity = query.value(1).toDouble();
        long long unixTime = query.value(2).toLongLong();

        // Track first and last timestamps for date range
        if (firstTimestamp == 0)
        {
            lastTimestamp = unixTime; // First record is actually the latest (DESC order)
        }
        firstTimestamp = unixTime; // Last record will be the oldest

        // Convert Kelvin to Celsius
        double temperature = temperatureKelvin - 273.15;

        // Convert Unix timestamp to milliseconds for QDateTime
        qint64 timestampMs = unixTime * 1000LL;

        tempPoints.prepend(QPointF(timestampMs, temperature));
        humPoints.prepend(QPointF(timestampMs, humidity));
    }

    // Add points to line series (only if we have 2 or more points to draw a line)
    if (tempPoints.size() >= 2)
    {
        for (const QPointF &point : tempPoints)
        {
            tempSeries->append(point);
        }

        for (const QPointF &point : humPoints)
        {
            humSeries->append(point);
        }
    }
    else if (tempPoints.size() == 1)
    {
        // For single point, don't add to line series - markers will show it
        m_hasVirtualPoint[locationIndex] = true; // Flag that we're in "marker-only" mode
        qDebug() << "Single point mode for location index" << locationIndex << "- showing marker only";
    }

    // Create scatter series for markers at data points with tooltips
    QScatterSeries *tempMarkers = new QScatterSeries();
    tempMarkers->setName("");                           // Don't show in legend
    tempMarkers->setMarkerSize(8);                      // Larger markers for easier hovering
    tempMarkers->setColor(QColor(230, 126, 34));        // Orange to match temp line
    tempMarkers->setBorderColor(QColor(255, 255, 255)); // White border for visibility
    tempMarkers->setMarkerShape(QScatterSeries::MarkerShapeCircle);

    for (const QPointF &point : tempPoints)
    {
        tempMarkers->append(point);
    }

    // Store marker series reference for real-time updates
    if (locationIndex >= 0 && locationIndex < 5)
    {
        m_tempMarkers[locationIndex] = tempMarkers;
    }

    // Enable tooltips on temperature markers
    connect(tempMarkers, &QScatterSeries::hovered, [timezoneOffset](const QPointF &point, bool state)
            {
        if (state) {
            // Mouse is over the point - show tooltip
            QDateTime dtUTC = QDateTime::fromMSecsSinceEpoch(point.x(), Qt::UTC);
            QDateTime dtLocal = dtUTC.addSecs(timezoneOffset);  // Apply timezone offset
            QString timestamp = dtLocal.toString("dd/MM/yyyy hh:mm:ss");
            double tempCelsius = point.y();

            QToolTip::showText(QCursor::pos(),
                QString("Temperature: %1°C\n%2")
                .arg(tempCelsius, 0, 'f', 1)
                .arg(timestamp));
        } else {
            // Mouse moved away - hide tooltip
            QToolTip::hideText();
        } });

    QScatterSeries *humMarkers = new QScatterSeries();
    humMarkers->setName("");                           // Don't show in legend
    humMarkers->setMarkerSize(8);                      // Larger markers for easier hovering
    humMarkers->setColor(QColor(41, 128, 185));        // Blue to match humidity line
    humMarkers->setBorderColor(QColor(255, 255, 255)); // White border for visibility
    humMarkers->setMarkerShape(QScatterSeries::MarkerShapeCircle);

    for (const QPointF &point : humPoints)
    {
        humMarkers->append(point);
    }

    // Store marker series reference for real-time updates
    if (locationIndex >= 0 && locationIndex < 5)
    {
        m_humMarkers[locationIndex] = humMarkers;
    }

    // Enable tooltips on humidity markers
    connect(humMarkers, &QScatterSeries::hovered, [timezoneOffset](const QPointF &point, bool state)
            {
        if (state) {
            // Mouse is over the point - show tooltip
            QDateTime dtUTC = QDateTime::fromMSecsSinceEpoch(point.x(), Qt::UTC);
            QDateTime dtLocal = dtUTC.addSecs(timezoneOffset);  // Apply timezone offset
            QString timestamp = dtLocal.toString("dd/MM/yyyy hh:mm:ss");
            double humidity = point.y();

            QToolTip::showText(QCursor::pos(),
                QString("Humidity: %1%\n%2")
                .arg(humidity, 0, 'f', 1)
                .arg(timestamp));
        } else {
            // Mouse moved away - hide tooltip
            QToolTip::hideText();
        } });

    // Create zero reference line for temperature axis
    QLineSeries *zeroLine = new QLineSeries();
    zeroLine->setName(""); // Don't show in legend
    if (!tempPoints.isEmpty())
    {
        // Span the entire X-axis range
        qint64 minX = tempPoints.first().x();
        qint64 maxX = tempPoints.last().x();
        zeroLine->append(QPointF(minX, 0));
        zeroLine->append(QPointF(maxX, 0));
    }
    QPen zeroPen(QColor(128, 128, 128)); // Gray color
    zeroPen.setWidth(1);
    zeroPen.setStyle(Qt::DashLine); // Dashed line
    zeroLine->setPen(zeroPen);

    // Store zero line reference for real-time updates
    if (locationIndex >= 0 && locationIndex < 5)
    {
        m_zeroLine[locationIndex] = zeroLine;
    }

    // Create chart
    QChart *chart = new QChart();
    chart->addSeries(tempSeries);
    chart->addSeries(humSeries);
    chart->addSeries(zeroLine);    // Add zero reference line
    chart->addSeries(tempMarkers); // Add markers on top of lines
    chart->addSeries(humMarkers);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Set chart title with location name - white text to match theme
    chart->setTitle(locationName);
    QFont titleFont;
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    chart->setTitleFont(titleFont);
    chart->setTitleBrush(QBrush(Qt::white)); // White title

    // Make chart background transparent - the QChartView provides the frosted glass effect
    chart->setBackgroundBrush(QBrush(Qt::transparent));
    chart->setBackgroundPen(QPen(Qt::transparent));
    chart->setBackgroundRoundness(0);

    // Keep plot area transparent
    chart->setPlotAreaBackgroundVisible(false);

    // Remove all margins to maximize chart space
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setMargins(QMargins(0, 0, 0, 0));

    // Create X-axis - no grid, no labels (tooltips will show exact timestamps)
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setLabelsVisible(false);   // Hide labels
    axisX->setGridLineVisible(false); // Hide grid lines
    axisX->setLineVisible(false);     // Hide axis line itself

    // Store X-axis reference for real-time updates
    if (locationIndex >= 0 && locationIndex < 5)
    {
        m_axisX[locationIndex] = axisX;
    }

    // Set compact font for Y-axis to minimize space
    QFont axisFont;
    axisFont.setPointSize(7);

    chart->addAxis(axisX, Qt::AlignBottom);
    tempSeries->attachAxis(axisX);
    humSeries->attachAxis(axisX);
    zeroLine->attachAxis(axisX); // Attach zero line to X-axis
    tempMarkers->attachAxis(axisX);
    humMarkers->attachAxis(axisX);

    // Create Y-axis for Temperature (left side) - orange text to match theme
    QValueAxis *axisYTemp = new QValueAxis();
    axisYTemp->setLabelFormat("%.0f");
    axisYTemp->setLabelsFont(axisFont);
    axisYTemp->setLinePenColor(QColor(230, 126, 34)); // Subtle orange line
    axisYTemp->setLabelsColor(QColor(230, 126, 34));  // Orange labels

    QFont tempFont = axisYTemp->labelsFont(); // Bold font for emphasis
    tempFont.setPointSize(10);
    tempFont.setBold(true);
    axisYTemp->setLabelsFont(tempFont);

    axisYTemp->setGridLineVisible(false); // Remove grid lines
    axisYTemp->setRange(-30, 60);         // Fixed range: -30°C to 60°C

    chart->addAxis(axisYTemp, Qt::AlignLeft);
    tempSeries->attachAxis(axisYTemp);
    zeroLine->attachAxis(axisYTemp);    // Attach zero line to temperature Y-axis
    tempMarkers->attachAxis(axisYTemp); // Attach markers to same axis

    // Create Y-axis for Humidity (right side) - blue text to match theme
    QValueAxis *axisYHum = new QValueAxis();
    axisYHum->setLabelFormat("%.0f");
    axisYHum->setLabelsFont(axisFont);
    axisYHum->setLinePenColor(QColor(41, 128, 185)); // Subtle blue line
    axisYHum->setLabelsColor(QColor(41, 128, 185));  // Blue labels

    QFont humFont = axisYHum->labelsFont();
    humFont.setPointSize(10);
    humFont.setBold(true);
    axisYHum->setLabelsFont(humFont);

    axisYHum->setGridLineVisible(false); // Remove grid lines
    axisYHum->setRange(0, 100);          // Fixed range: 0% to 100%

    chart->addAxis(axisYHum, Qt::AlignRight);
    humSeries->attachAxis(axisYHum);
    humMarkers->attachAxis(axisYHum); // Attach markers to same axis

    // Hide legend - we'll use a shared legend for all charts
    chart->legend()->setVisible(false);

    // Apply chart to view - stylesheet handles the frosted glass styling
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(0, 0, 0, 0); // Remove view margins

    qDebug() << "Chart initialized for" << locationName << "with" << tempPoints.size() << "data points";
}

void MainWindow::updateChartRealtime(int locationIndex, const WeatherData &data)
{
    // Validate location index
    if (locationIndex < 0 || locationIndex >= 5)
    {
        qWarning() << "Invalid location index for chart update:" << locationIndex;
        return;
    }

    // Ensure series and axis exist for this location
    if (!m_tempSeries[locationIndex] || !m_humSeries[locationIndex] ||
        !m_tempMarkers[locationIndex] || !m_humMarkers[locationIndex] ||
        !m_zeroLine[locationIndex] || !m_axisX[locationIndex])
    {
        qWarning() << "Chart series or axis not initialized for location index:" << locationIndex;
        return;
    }

    // Convert Kelvin to Celsius
    double temperature = data.temperature - 273.15;

    // Convert Unix timestamp to milliseconds for QDateTime
    qint64 timestampMs = data.timestamp * 1000LL;

    // Create new data points
    QPointF tempPoint(timestampMs, temperature);
    QPointF humPoint(timestampMs, data.humidity);

    // Get current marker count BEFORE adding new point
    int markerCountBefore = m_tempMarkers[locationIndex]->count();

    // Maintain 100-point rolling window for markers
    if (markerCountBefore >= 100)
    {
        // Remove oldest point (first point)
        m_tempMarkers[locationIndex]->remove(0);
        m_humMarkers[locationIndex]->remove(0);

        // Also remove from line series if it exists there
        if (m_tempSeries[locationIndex]->count() > 0)
        {
            m_tempSeries[locationIndex]->remove(0);
            m_humSeries[locationIndex]->remove(0);
        }

        markerCountBefore = 99; // After removal
    }

    // Always add to marker series (markers show for any number of points)
    m_tempMarkers[locationIndex]->append(tempPoint);
    m_humMarkers[locationIndex]->append(humPoint);

    // Handle line series based on point count BEFORE this addition
    if (markerCountBefore == 0)
    {
        // This is the first point - only show marker, no line
        m_hasVirtualPoint[locationIndex] = true;
        qDebug() << "First point - marker-only mode for location index" << locationIndex;
    }
    else if (markerCountBefore == 1 && m_hasVirtualPoint[locationIndex])
    {
        // This is the second point - now create the line with both points
        QPointF firstPoint = m_tempMarkers[locationIndex]->at(0);
        QPointF firstHumPoint = m_humMarkers[locationIndex]->at(0);

        m_tempSeries[locationIndex]->append(firstPoint);
        m_humSeries[locationIndex]->append(firstHumPoint);
        m_tempSeries[locationIndex]->append(tempPoint);
        m_humSeries[locationIndex]->append(humPoint);

        m_hasVirtualPoint[locationIndex] = false;
        qDebug() << "Second point arrived - created line for location index" << locationIndex;
    }
    else if (markerCountBefore >= 1)
    {
        // We already have a line - just add this point to it
        m_tempSeries[locationIndex]->append(tempPoint);
        m_humSeries[locationIndex]->append(humPoint);
    }

    // Update X-axis range and zero line to show new data
    // Use marker series for range calculation (always has real data)
    if (m_tempMarkers[locationIndex]->count() > 0)
    {
        qint64 minX = m_tempMarkers[locationIndex]->at(0).x();
        qint64 maxX = m_tempMarkers[locationIndex]->at(m_tempMarkers[locationIndex]->count() - 1).x();

        // Only add padding if we have exactly 1 point (to make it visible)
        if (m_tempMarkers[locationIndex]->count() == 1)
        {
            // Add ±30 minutes padding (1800000 ms) around the single point
            qint64 padding = 1800000;
            minX -= padding;
            maxX += padding;
        }
        // For 2+ points, use the actual data range without padding

        // Update X-axis range to display the data window
        QDateTime minDateTime = QDateTime::fromMSecsSinceEpoch(minX, Qt::UTC);
        QDateTime maxDateTime = QDateTime::fromMSecsSinceEpoch(maxX, Qt::UTC);
        m_axisX[locationIndex]->setRange(minDateTime, maxDateTime);

        // Clear and update zero line to span the range
        m_zeroLine[locationIndex]->clear();
        m_zeroLine[locationIndex]->append(QPointF(minX, 0));
        m_zeroLine[locationIndex]->append(QPointF(maxX, 0));
    }

    qDebug() << "Chart updated for location index" << locationIndex
             << "- Temp:" << temperature << "°C, Humidity:" << data.humidity << "%"
             << "- Total markers:" << m_tempMarkers[locationIndex]->count()
             << ", Line points:" << m_tempSeries[locationIndex]->count();
}

void MainWindow::updateStatisticsLabels()
{
    qDebug() << "Updating statistics labels...";

    if (!pythonBridge)
    {
        qWarning() << "PythonBridge not initialized!";
        return;
    }

    // Get JSON data from Python
    QString jsonString = pythonBridge->getStatisticsJson(
        m_hostname,
        m_databaseName,
        m_username,
        m_password);

    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (doc.isNull() || !doc.isObject())
    {
        qWarning() << "Failed to parse JSON statistics";
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray locations = root["locations"].toArray();

    // Process each location
    for (int i = 0; i < locations.size(); ++i)
    {
        QJsonObject loc = locations[i].toObject();
        QString locationName = loc["location"].toString();

        // Check for errors
        if (loc.contains("error"))
        {
            qWarning() << "Error for location" << locationName << ":" << loc["error"].toString();
            continue;
        }

        // Get statistics objects
        QJsonObject temp = loc["temperature"].toObject();
        QJsonObject humidity = loc["humidity"].toObject();
        QJsonObject pressure = loc["pressure"].toObject();
        QJsonObject wind = loc["wind"].toObject();
        QJsonObject weatherConditions = loc["weather_conditions"].toObject();
        QJsonObject timeRange = loc["time_range"].toObject();

        // Determine label suffix based on location name
        QString suffix;
        if (locationName == "ZOCCA")
            suffix = "Zocca";
        else if (locationName == "ROME")
            suffix = "Rome";
        else if (locationName == "PARIS")
            suffix = "Paris";
        else if (locationName == "LONDON")
            suffix = "London";
        else if (locationName == "NEW_YORK")
            suffix = "NewYork";
        else
            continue;

        // Update Location label
        QLabel *locationLabel = this->findChild<QLabel *>(QString("statsLocation%1").arg(suffix));
        if (locationLabel)
        {
            locationLabel->setText(locationName);
        }

        // Update Temperature labels
        QLabel *tempRangeLabel = this->findChild<QLabel *>(QString("statsTempRange%1").arg(suffix));
        if (tempRangeLabel && temp.contains("min") && temp.contains("max"))
        {
            tempRangeLabel->setText(QString("Value: %1°C → %2°C")
                                        .arg(temp["min"].toDouble(), 0, 'f', 1)
                                        .arg(temp["max"].toDouble(), 0, 'f', 1));
        }

        QLabel *tempMeanLabel = this->findChild<QLabel *>(QString("statsTempMean%1").arg(suffix));
        if (tempMeanLabel && temp.contains("mean"))
        {
            tempMeanLabel->setText(QString("Mean: %1°C")
                                       .arg(temp["mean"].toDouble(), 0, 'f', 1));
        }

        QLabel *tempStdDevLabel = this->findChild<QLabel *>(QString("statsTempStdDev%1").arg(suffix));
        if (tempStdDevLabel && temp.contains("std_dev"))
        {
            tempStdDevLabel->setText(QString("σ=%1°C")
                                         .arg(temp["std_dev"].toDouble(), 0, 'f', 1));
        }

        // Update Humidity labels
        QLabel *humidityRangeLabel = this->findChild<QLabel *>(QString("statsHumRange%1").arg(suffix));
        if (humidityRangeLabel && humidity.contains("min") && humidity.contains("max"))
        {
            humidityRangeLabel->setText(QString("Value: %1% → %2%")
                                            .arg(humidity["min"].toDouble(), 0, 'f', 0)
                                            .arg(humidity["max"].toDouble(), 0, 'f', 0));
        }

        QLabel *humidityMeanLabel = this->findChild<QLabel *>(QString("statsHumMean%1").arg(suffix));
        if (humidityMeanLabel && humidity.contains("mean"))
        {
            humidityMeanLabel->setText(QString("Mean: %1%")
                                           .arg(humidity["mean"].toDouble(), 0, 'f', 0));
        }

        // Update Pressure labels
        QLabel *pressureRangeLabel = this->findChild<QLabel *>(QString("statsPressRange%1").arg(suffix));
        if (pressureRangeLabel && pressure.contains("min") && pressure.contains("max"))
        {
            pressureRangeLabel->setText(QString("Value: %1 → %2 hPa")
                                            .arg(pressure["min"].toDouble(), 0, 'f', 0)
                                            .arg(pressure["max"].toDouble(), 0, 'f', 0));
        }

        QLabel *pressureMeanLabel = this->findChild<QLabel *>(QString("statsPressMean%1").arg(suffix));
        if (pressureMeanLabel && pressure.contains("mean"))
        {
            pressureMeanLabel->setText(QString("Mean: %1 hPa")
                                           .arg(pressure["mean"].toDouble(), 0, 'f', 0));
        }

        QLabel *pressureTrendLabel = this->findChild<QLabel *>(QString("statsPressTrend%1").arg(suffix));
        if (pressureTrendLabel && pressure.contains("trend"))
        {
            QString trend = pressure["trend"].toString();
            QString displayText;
            QString color;

            if (trend == "stable")
            {
                displayText = "→ stable";
                color = "#cccccc"; // gray
            }
            else if (trend.startsWith("increasing (") && trend.endsWith(")"))
            {
                QString slopeStr = trend.mid(12, trend.length() - 13); // extract inside ()
                displayText = QString("↑ %1").arg(slopeStr);
                color = "#4CAF50"; // green
            }
            else if (trend.startsWith("decreasing (") && trend.endsWith(")"))
            {
                QString slopeStr = trend.mid(12, trend.length() - 13);
                displayText = QString("↓ %1").arg(slopeStr);
                color = "#FF5252"; // red
            }
            else
            {
                displayText = trend;
                color = "white"; // fallback
            }

            // Apply colored text using rich text (HTML)
            pressureTrendLabel->setText(
                QString("<span style='color:%1;'>Trend: %2</span>")
                    .arg(color, displayText));
        }

        // Update Wind labels
        QLabel *windValueLabel = this->findChild<QLabel *>(QString("statsWindValue%1").arg(suffix));
        if (windValueLabel && wind.contains("mean_speed"))
        {
            windValueLabel->setText(QString("Mean: %1 m/s")
                                        .arg(wind["mean_speed"].toDouble(), 0, 'f', 1));
        }

        QLabel *windGustLabel = this->findChild<QLabel *>(QString("statsWindGust%1").arg(suffix));
        if (windGustLabel && wind.contains("max_speed"))
        {
            windGustLabel->setText(QString("Gust: %1 m/s")
                                       .arg(wind["max_speed"].toDouble(), 0, 'f', 1));
        }

        // ========== Weather Patterns (Conditions Distribution) ==========
        // Update weather pattern labels with percentages
        updateWeatherPatterns(suffix, weatherConditions);

        // ========== Multi-Period Comparison (24h vs 7 days) ==========
        // Get period comparison for this location
        QString tableName;
        if (locationName == "ZOCCA")
            tableName = "zocca";
        else if (locationName == "ROME")
            tableName = "rome";
        else if (locationName == "PARIS")
            tableName = "paris";
        else if (locationName == "LONDON")
            tableName = "london";
        else if (locationName == "NEW_YORK")
            tableName = "new_york";

        if (!tableName.isEmpty())
        {
            QString periodJson = pythonBridge->getPeriodComparisonJson(
                m_hostname,
                m_databaseName,
                m_username,
                m_password,
                tableName);

            QJsonDocument periodDoc = QJsonDocument::fromJson(periodJson.toUtf8());
            if (!periodDoc.isNull() && periodDoc.isObject())
            {
                QJsonObject periodObj = periodDoc.object();

                if (!periodObj.contains("error"))
                {
                    QJsonObject comparison = periodObj["comparison"].toObject();
                    QString trendSummary = comparison["trend_summary"].toString();

                    // Parse trend_summary to extract temperature and pressure parts
                    // Format: "Temperature: stable, Pressure: stable" or
                    //         "Temperature: Warming trend (+X°C vs 7-day avg), Pressure: rising (...)"
                    QString tempTrend = "N/A";
                    QString pressTrend = "N/A";

                    // Split by finding "Pressure: " to handle commas in the trend text
                    int pressureIndex = trendSummary.indexOf("Pressure: ");
                    if (pressureIndex > 0)
                    {
                        tempTrend = trendSummary.left(pressureIndex).trimmed();
                        // Remove trailing comma if present
                        if (tempTrend.endsWith(","))
                        {
                            tempTrend = tempTrend.left(tempTrend.length() - 1).trimmed();
                        }
                        pressTrend = trendSummary.mid(pressureIndex).trimmed();
                    }
                    else
                    {
                        tempTrend = trendSummary;
                    }

                    // Update Multi-Period labels
                    QLabel *multiPeriodTempLabel = this->findChild<QLabel *>(QString("statsMultiPeriodTemp%1").arg(suffix));
                    if (multiPeriodTempLabel)
                    {
                        multiPeriodTempLabel->setText(tempTrend);
                    }

                    QLabel *multiPeriodPressLabel = this->findChild<QLabel *>(QString("statsMultiPeriodPress%1").arg(suffix));
                    if (multiPeriodPressLabel)
                    {
                        multiPeriodPressLabel->setText(pressTrend);
                    }

                    // Optionally update the title with the time range
                    QLabel *multiPeriodTitleLabel = this->findChild<QLabel *>(QString("statsMultiPeriodTitle%1").arg(suffix));
                    if (multiPeriodTitleLabel)
                    {
                        multiPeriodTitleLabel->setText("Multi-period");
                    }
                }
            }
        }
    }

    // ========== Cross-Location Insights ==========
    // Get comparative statistics across all locations
    QString comparisonJson = pythonBridge->getLocationComparisonJson(
        m_hostname,
        m_databaseName,
        m_username,
        m_password,
        24); // 24 hours

    QJsonDocument comparisonDoc = QJsonDocument::fromJson(comparisonJson.toUtf8());
    if (!comparisonDoc.isNull() && comparisonDoc.isObject())
    {
        QJsonObject comparisonObj = comparisonDoc.object();

        if (!comparisonObj.contains("error"))
        {
            // Extract temperature data
            QJsonObject temperature = comparisonObj["temperature"].toObject();
            QJsonObject warmest = temperature["warmest"].toObject();
            QJsonObject coldest = temperature["coldest"].toObject();
            double tempDifference = temperature["difference"].toDouble();

            // Extract humidity data
            QJsonObject humidity = comparisonObj["humidity"].toObject();
            QJsonObject mostHumid = humidity["most_humid"].toObject();
            QJsonObject leastHumid = humidity["least_humid"].toObject();

            // Extract pressure data
            QJsonObject pressure = comparisonObj["pressure"].toObject();
            QJsonObject highPressure = pressure["highest"].toObject();
            QJsonObject lowPressure = pressure["lowest"].toObject();

            // Update Temperature labels
            QLabel *tempSpreadLabel = this->findChild<QLabel *>("label_2");
            if (tempSpreadLabel)
            {
                // Convert Kelvin to Celsius for the difference
                tempSpreadLabel->setText(QString("Temp Spread: %1°C  ")
                                             .arg(tempDifference, 0, 'f', 1));
            }

            QLabel *warmestLabel = this->findChild<QLabel *>("label_3");
            if (warmestLabel)
            {
                double warmestTemp = warmest["value"].toDouble() - 273.15; // Convert K to C
                warmestLabel->setText(QString("Warmest: %1 (%2°C)  ")
                                          .arg(warmest["location"].toString())
                                          .arg(warmestTemp, 0, 'f', 1));
            }

            QLabel *coldestLabel = this->findChild<QLabel *>("label_4");
            if (coldestLabel)
            {
                double coldestTemp = coldest["value"].toDouble() - 273.15; // Convert K to C
                coldestLabel->setText(QString("Coldest: %1 (%2°C)  ")
                                          .arg(coldest["location"].toString())
                                          .arg(coldestTemp, 0, 'f', 1));
            }

            // Update Humidity labels
            QLabel *humidityRangeLabel = this->findChild<QLabel *>("label_5");
            if (humidityRangeLabel)
            {
                double humRange = mostHumid["value"].toDouble() - leastHumid["value"].toDouble();
                humidityRangeLabel->setText(QString("Humidity Range: %1% ")
                                                .arg(humRange, 0, 'f', 0));
            }

            QLabel *mostHumidLabel = this->findChild<QLabel *>("label_7");
            if (mostHumidLabel)
            {
                mostHumidLabel->setText(QString("Most Humid: %1 (%2%)")
                                            .arg(mostHumid["location"].toString())
                                            .arg(mostHumid["value"].toDouble(), 0, 'f', 0));
            }

            QLabel *driestLabel = this->findChild<QLabel *>("label_6");
            if (driestLabel)
            {
                driestLabel->setText(QString("Driest: %1 (%2%)")
                                         .arg(leastHumid["location"].toString())
                                         .arg(leastHumid["value"].toDouble(), 0, 'f', 0));
            }

            // Update Pressure labels
            QLabel *pressureRangeLabel = this->findChild<QLabel *>("label_8");
            if (pressureRangeLabel)
            {
                double pressRange = highPressure["value"].toDouble() - lowPressure["value"].toDouble();
                pressureRangeLabel->setText(QString("Pressure Range: %1 hPa ")
                                                .arg(pressRange, 0, 'f', 0));
            }

            QLabel *highPressureLabel = this->findChild<QLabel *>("label_10");
            if (highPressureLabel)
            {
                highPressureLabel->setText(QString("High: %1 (%2 hPa)")
                                               .arg(highPressure["location"].toString())
                                               .arg(highPressure["value"].toDouble(), 0, 'f', 0));
            }

            QLabel *lowPressureLabel = this->findChild<QLabel *>("label_9");
            if (lowPressureLabel)
            {
                lowPressureLabel->setText(QString("Low:  %1 (%2 hPa)")
                                              .arg(lowPressure["location"].toString())
                                              .arg(lowPressure["value"].toDouble(), 0, 'f', 0));
            }

            qDebug() << "Cross-location insights updated successfully.";
        }
        else
        {
            qWarning() << "Error in comparison data:" << comparisonObj["error"].toString();
        }
    }

    qDebug() << "Statistics labels updated successfully.";
}

void MainWindow::updateWeatherPatterns(const QString &locationSuffix, const QJsonObject &weatherConditions)
{
    // Map location suffix to index
    int locationIndex = -1;
    if (locationSuffix == "Zocca") locationIndex = 0;
    else if (locationSuffix == "Rome") locationIndex = 1;
    else if (locationSuffix == "Paris") locationIndex = 2;
    else if (locationSuffix == "NewYork") locationIndex = 3;
    else if (locationSuffix == "London") locationIndex = 4;

    if (locationIndex < 0 || locationIndex >= 5)
    {
        qWarning() << "Invalid location suffix:" << locationSuffix;
        return;
    }

    // Get or create the pattern container
    QWidget *patternContainer = m_patternContainers[locationIndex];
    QVBoxLayout *patternLayout = nullptr;

    if (!patternContainer)
    {
        // First time - find the existing statsPatternValueXXX label and replace it
        QLabel *patternValueLabel = this->findChild<QLabel *>(QString("statsPatternValue%1").arg(locationSuffix));
        if (!patternValueLabel)
        {
            qWarning() << "Could not find pattern value label for" << locationSuffix;
            return;
        }

        // Get the parent layout
        QLayout *parentLayout = patternValueLabel->parentWidget()->layout();
        if (!parentLayout)
        {
            qWarning() << "No parent layout found for pattern label";
            return;
        }

        // Remove the old single-label widget
        parentLayout->removeWidget(patternValueLabel);
        patternValueLabel->setParent(nullptr);
        patternValueLabel->deleteLater();

        // Create a new container widget
        patternContainer = new QWidget();
        patternLayout = new QVBoxLayout(patternContainer);
        patternLayout->setContentsMargins(0, 0, 0, 0);
        patternLayout->setSpacing(2);

        // Add the container to the parent layout
        parentLayout->addWidget(patternContainer);

        // Store reference
        m_patternContainers[locationIndex] = patternContainer;
    }
    else
    {
        // Container exists - just clear its layout
        patternLayout = qobject_cast<QVBoxLayout*>(patternContainer->layout());
        if (!patternLayout)
        {
            qWarning() << "Pattern container has no layout!";
            return;
        }

        // Clear existing widgets
        QLayoutItem *item;
        while ((item = patternLayout->takeAt(0)) != nullptr)
        {
            delete item->widget();
            delete item;
        }
    }

    // Extract conditions from JSON
    QJsonObject conditions = weatherConditions["conditions"].toObject();

    if (conditions.isEmpty())
    {
        // No data - show placeholder
        QLabel *noDataLabel = new QLabel("No data available");
        noDataLabel->setStyleSheet("color: #888888;");
        patternLayout->addWidget(noDataLabel);
    }
    else
    {
        // Create a list of conditions sorted by percentage (descending)
        QList<QPair<QString, double>> conditionList;
        for (auto it = conditions.begin(); it != conditions.end(); ++it)
        {
            QString conditionName = it.key();
            QJsonObject conditionData = it.value().toObject();
            double percentage = conditionData["percentage"].toDouble();
            conditionList.append(qMakePair(conditionName, percentage));
        }

        // Sort by percentage (descending)
        std::sort(conditionList.begin(), conditionList.end(),
                  [](const QPair<QString, double> &a, const QPair<QString, double> &b) {
                      return a.second > b.second;
                  });

        // Add labels for each condition
        for (const auto &pair : conditionList)
        {
            QString conditionName = pair.first;
            double percentage = pair.second;

            // Create label with condition name and percentage
            QLabel *conditionLabel = new QLabel(QString("%1 (%2%)")
                                                    .arg(conditionName)
                                                    .arg(percentage, 0, 'f', 1));

            // Style the label based on percentage (highlight more common conditions)
            if (percentage >= 50.0)
            {
                conditionLabel->setStyleSheet("font-weight: bold; color: #4CAF50;"); // Green for high percentage
            }
            else if (percentage >= 25.0)
            {
                conditionLabel->setStyleSheet("color: #FFC107;"); // Yellow for medium percentage
            }
            else
            {
                conditionLabel->setStyleSheet("color: #cccccc;"); // Gray for low percentage
            }

            patternLayout->addWidget(conditionLabel);
        }
    }

    qDebug() << "Weather patterns updated for" << locationSuffix << "with" << conditions.size() << "conditions";
}
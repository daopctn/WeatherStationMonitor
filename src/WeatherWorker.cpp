#include "WeatherWorker.h"
#include <QTime>

WeatherWorker::WeatherWorker(const QString &apiURL, QVector<WeatherData> &weatherDataVector, QMutex &mutex, QObject *parent)
    : QThread(parent), m_apiURL(apiURL), m_weatherDataVector(weatherDataVector), m_mutex(mutex), m_running(false)
{
}

WeatherWorker::~WeatherWorker()
{
    qDebug() << "WeatherWorker destructor called.";
    stop();
    wait(); // Ensure the thread has finished
}

void WeatherWorker::run()
{
    // Create the network manager IN the worker thread
    QNetworkAccessManager networkManager;
    m_networkManager = &networkManager;

    // Connect the signal AFTER moving to thread context
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WeatherWorker::onNetworkReply,
            Qt::DirectConnection); // Important: Use DirectConnection since we're in the same thread

    // Setup timer to fetch weather data periodically
    QTimer timer;
    connect(&timer, &QTimer::timeout,
            this, &WeatherWorker::fetchWeatherData,
            Qt::DirectConnection); // DirectConnection for same thread

    timer.start(WEATHER_FETCH_INTERVAL_MS);

    m_running = true;

    // Fetch data immediately
    fetchWeatherData();

    // Start the event loop
    exec();

    // Cleanup happens automatically when stack objects go out of scope
    m_networkManager = nullptr;
}

void WeatherWorker::stop()
{
    // qDebug() << "Stopping WeatherWorker thread...";
    m_running = false;
    quit(); // Exit the event loop
}

void WeatherWorker::fetchWeatherData()
{
    if (!m_networkManager || !m_running)
    {
        qWarning() << "NetworkManager not initialized or worker stopped";
        return;
    }

    QNetworkRequest request(m_apiURL);
    // This now happens in the correct thread context
    QNetworkReply *reply = m_networkManager->get(request);

    // Alternative: Connect directly to the reply
    // connect(reply, &QNetworkReply::finished, this, [this, reply]()
    //         { onNetworkReply(reply); });
}

void WeatherWorker::onNetworkReply(QNetworkReply *reply)
{
    // Check for network errors (timeout, DNS failure, connection refused, etc.)
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater();  // Clean up to prevent memory leak
        return;
    }

    // Read all response data and clean up the reply object
    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    // Print raw response for debugging (commented out for production)
    // qDebug() << "Raw API Response:" << responseData;

    // Parse JSON response from OpenWeatherMap API
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isObject())
    {
        qWarning() << "Invalid JSON response";
        return;
    }

    // Validate that response contains required data structure
    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("main") || !jsonObj["main"].isObject())
    {
        qWarning() << "JSON does not contain 'main' object";
        return;
    }

    // Extract temperature, pressure, humidity from "main" object
    // Note: Temperature is in Kelvin from API
    QJsonObject mainObj = jsonObj["main"].toObject();

    // Validate temperature field exists and has reasonable value
    if (!mainObj.contains("temp"))
    {
        qWarning() << "Missing 'temp' field in API response";
        return;
    }
    double temperature = mainObj.value("temp").toDouble();
    // Sanity check: temperature should be in Kelvin (valid range: 173-373K = -100°C to 100°C)
    if (temperature < 173.0 || temperature > 373.0)
    {
        qWarning() << "Invalid temperature value:" << temperature << "K (expected 173-373K)";
        return;
    }

    // Validate pressure (typical range: 870-1085 hPa)
    int pressure = mainObj.value("pressure").toInt(0);
    if (pressure < 800 || pressure > 1200)
    {
        qWarning() << "Suspicious pressure value:" << pressure << "hPa (expected 800-1200 hPa)";
        // Don't return - pressure anomalies can be valid in extreme weather
    }

    // Validate humidity (must be 0-100%)
    double humidity = mainObj.value("humidity").toDouble(0.0);
    if (humidity < 0.0 || humidity > 100.0)
    {
        qWarning() << "Invalid humidity value:" << humidity << "% (expected 0-100%)";
        humidity = qBound(0.0, humidity, 100.0); // Clamp to valid range
    }

    // Extract wind speed from "wind" object (in m/s)
    // Validate wind object exists
    if (!jsonObj.contains("wind") || !jsonObj["wind"].isObject())
    {
        qWarning() << "Missing or invalid 'wind' object in API response";
        return;
    }
    QJsonObject windObj = jsonObj["wind"].toObject();
    double windSpeed = windObj.value("speed").toDouble(0.0);
    // Sanity check: wind speed should be reasonable (0-100 m/s, hurricane max ~90 m/s)
    if (windSpeed < 0.0 || windSpeed > 150.0)
    {
        qWarning() << "Invalid wind speed value:" << windSpeed << "m/s (expected 0-150 m/s)";
        windSpeed = qBound(0.0, windSpeed, 150.0); // Clamp to valid range
    }

    // Extract weather ID from "weather" array (used for icon mapping)
    // Weather ID determines weather condition (e.g., 800=clear, 804=clouds)
    int weatherId = -1;
    if (jsonObj.contains("weather") && jsonObj["weather"].isArray())
    {
        QJsonArray weatherArray = jsonObj["weather"].toArray();
        if (!weatherArray.isEmpty())
        {
            QJsonObject weatherObj = weatherArray[0].toObject();
            weatherId = weatherObj.value("id").toInt();
        }
    }

    // Extract human-readable weather description (e.g., "overcast clouds")
    QString description;
    if (jsonObj.contains("weather") && jsonObj["weather"].isArray())
    {
        QJsonArray weatherArray = jsonObj["weather"].toArray();
        if (!weatherArray.isEmpty())
        {
            QJsonObject weatherObj = weatherArray[0].toObject();
            description = weatherObj.value("description").toString();
        }
    }

    // Extract timestamps: current time, sunrise, and sunset (all in UNIX epoch seconds)
    // These are used to determine day/night for icon selection
    // Validate timestamp exists
    if (!jsonObj.contains("dt"))
    {
        qWarning() << "Missing 'dt' (timestamp) field in API response";
        return;
    }
    long long unixTime = jsonObj.value("dt").toVariant().toLongLong();
    // Sanity check: timestamp should be reasonable (after year 2000 and before year 2100)
    long long minTimestamp = 946684800;  // 2000-01-01
    long long maxTimestamp = 4102444800; // 2100-01-01
    if (unixTime < minTimestamp || unixTime > maxTimestamp)
    {
        qWarning() << "Invalid timestamp value:" << unixTime << "(expected" << minTimestamp << "-" << maxTimestamp << ")";
        return;
    }

    // Validate sys object exists for sunrise/sunset
    if (!jsonObj.contains("sys") || !jsonObj["sys"].isObject())
    {
        qWarning() << "Missing or invalid 'sys' object in API response";
        return;
    }
    QJsonObject sysObj = jsonObj["sys"].toObject();
    long long sunriseTime = sysObj.value("sunrise").toVariant().toLongLong(0);
    long long sunsetTime = sysObj.value("sunset").toVariant().toLongLong(0);

    // Validate sunrise/sunset times are reasonable
    if (sunriseTime < minTimestamp || sunriseTime > maxTimestamp)
    {
        qWarning() << "Invalid sunrise time:" << sunriseTime;
        sunriseTime = unixTime; // Fallback to current time
    }
    if (sunsetTime < minTimestamp || sunsetTime > maxTimestamp)
    {
        qWarning() << "Invalid sunset time:" << sunsetTime;
        sunsetTime = unixTime; // Fallback to current time
    }

    // Only process if this data is newer than what we already have (prevent duplicates)
    if (unixTime > lastestData->timestamp)
    {
        // Create new weather data struct with parsed values
        WeatherData newData;
        newData.locationName = jsonObj.value("name").toString();
        newData.temperature = temperature;
        newData.pressure = pressure;
        newData.humidity = humidity;
        newData.windSpeed = windSpeed;
        newData.weatherId = weatherId;
        newData.description = description;
        newData.timestamp = unixTime;
        newData.sunrise = sunriseTime;
        newData.sunset = sunsetTime;

        // Update latest data cache for duplicate detection
        lastestData->locationName = newData.locationName;
        lastestData->temperature = newData.temperature;
        lastestData->pressure = newData.pressure;
        lastestData->humidity = newData.humidity;
        lastestData->windSpeed = newData.windSpeed;
        lastestData->weatherId = newData.weatherId;
        lastestData->description = newData.description;
        lastestData->timestamp = newData.timestamp;
        lastestData->sunrise = sunriseTime;
        lastestData->sunset = sunsetTime;

        // Print parsed data for debugging
        qDebug() << "";
        qDebug() << "Parsed Data - Thread ID:" << QThread::currentThreadId()
                 << " Location:" << newData.locationName
                 << " Temp:" << newData.temperature
                 << " Humidity:" << newData.humidity
                 << " Time:" << newData.timestamp;

        // Add new data to shared vector (thread-safe with mutex)
        m_mutex.lock();
        m_weatherDataVector.append(newData);
        m_mutex.unlock();

        // Notify listeners that new weather data is available (send copy of data for real-time UI update)
        emit weatherDataUpdated(newData);
    }
    else
    {
        // Data is older than what we have - skip to avoid duplicates in database
        qDebug() << "";
        qDebug() << "Thread ID:" << QThread::currentThreadId()
                 << "Received older data. Ignoring update."
                 << " New timestamp:" << unixTime
                 << " Last timestamp:" << lastestData->timestamp;
    }
}

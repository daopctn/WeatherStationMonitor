#include "../include/WeatherFetcher.h"
#include <QJsonParseError>
#include <QDebug>

WeatherFetcher::WeatherFetcher(QObject *parent, DatabaseManager *databaseManager, PythonBridge *pythonBridge)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)), m_databaseManager(databaseManager), m_pythonBridge(pythonBridge)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WeatherFetcher::onRequestFinished);
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

    // WeatherAPI
    QJsonObject apiObj = obj["WeatherAPI"].toObject();
    QString baseUrl = apiObj["base_url"].toString();
    QString apiKey = apiObj["api_key"].toString();
    int timeout = apiObj["timeout"].toInt();
}

WeatherFetcher::~WeatherFetcher()
{
    delete m_networkManager;
}

void WeatherFetcher::fetchMultipleWeather(const QList<Location> &locations)
{

    for (const Location &location : locations)
    {
        // qDebug() << "Preparing request for location:" << location.name << "Lat:" << location.lat << "Lon:" << location.lon;
        QString apiUrl = QString("%1/weather?lat=%2&lon=%3&appid=%4")
                             .arg(m_baseUrl)
                             .arg(location.lat)
                             .arg(location.lon)
                             .arg(m_apiKey);
        qDebug() << "Fetching weather for" << location.name << "from URL:" << apiUrl;
        QNetworkRequest request(apiUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = m_networkManager->get(request);
        m_replyLocationMap[reply] = location.name;
    }
}

void WeatherFetcher::onRequestFinished(QNetworkReply *reply)
{
    QString locationName = m_replyLocationMap.value(reply, "Unknown");
    m_replyLocationMap.remove(reply);

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    // qDebug() << "API Response:" << data;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        emit errorOccurred("JSON parse error: " + parseError.errorString());
        return;
    }

    QJsonObject rootObject = doc.object();
    QJsonObject mainObject = rootObject["main"].toObject();

    if (mainObject.contains("temp") && mainObject.contains("humidity"))
    {
        double kelvinTemperature = mainObject["temp"].toDouble();
        double humidity = mainObject["humidity"].toDouble();

        // Process temperature through Python (convert from Kelvin to Celsius)
        double celsiusTemperature = m_pythonBridge->convertKelvinToCelsius(kelvinTemperature);
        insertData(celsiusTemperature, humidity, locationName);
        emit insertDataDone();
    }
    else
    {
        emit errorOccurred("Temperature or humidity data not found in response");
    }
}

void WeatherFetcher::insertData(double temperature, double humidity, const QString &locationName)
{
    if (m_databaseManager && m_databaseManager->isConnected())
    {
        // Sanitize location name for table name (remove spaces, special chars)
        QString sanitizedLocationName = locationName.toLower().replace(" ", "_").replace("-", "_");
        qDebug() << "Original location:" << locationName << "Sanitized:" << sanitizedLocationName;

        // Create table if not exists
        QString createTableQuery = QString(
                                       "CREATE TABLE IF NOT EXISTS `%1` ("
                                       "id INT AUTO_INCREMENT PRIMARY KEY, "
                                       "temperature DECIMAL(5,2), "
                                       "humidity DECIMAL(5,2), "
                                       "time TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                                       ")")
                                       .arg(sanitizedLocationName);

        if (!m_databaseManager->executeQuery(createTableQuery))
        {
            emit errorOccurred("Failed to create table for location " + locationName + ": " + m_databaseManager->getLastError());
            return;
        }

        // Insert data into location-specific table
        qDebug() << "Inserting temperature:" << temperature << "humidity:" << humidity << "for location:" << locationName;
        QString insertQuery = QString("INSERT INTO `%1` (temperature, humidity, time) VALUES (%2, %3, NOW())")
                                  .arg(sanitizedLocationName)
                                  .arg(temperature)
                                  .arg(humidity);

        if (!m_databaseManager->executeQuery(insertQuery))
        {
            emit errorOccurred("Failed to insert data into " + locationName + " table: " + m_databaseManager->getLastError());
        }
    }
    else
    {
        qDebug() << "Database not connected";
        emit errorOccurred("Database not connected");
    }
}

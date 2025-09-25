#include "../include/WeatherFetcher.h"
#include <QJsonParseError>
#include <QDebug>

WeatherFetcher::WeatherFetcher(QObject *parent, DatabaseManager *databaseManager, PythonBridge *pythonBridge)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_databaseManager(databaseManager)
    , m_pythonBridge(pythonBridge)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WeatherFetcher::onRequestFinished);
}

// WeatherFetcher::~WeatherFetcher()
// {
//     delete m_pythonBridge;
// }

void WeatherFetcher::fetchWeather()
{
    QString apiKey = qgetenv("WEATHER_API_KEY");
    QString baseUrl = qgetenv("WEATHER_API_BASE_URL");

    if (baseUrl.isEmpty()) {
        baseUrl = "https://api.openweathermap.org/data/2.5";
    }

    if (apiKey.isEmpty()) {
        emit errorOccurred("Weather API key not configured. Please set WEATHER_API_KEY environment variable.");
        return;
    }

    QString apiUrl = QString("%1/weather?lat=44.34&lon=10.99&appid=%2").arg(baseUrl, apiKey);

    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_networkManager->get(request);
}

void WeatherFetcher::onRequestFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("JSON parse error: " + parseError.errorString());
        return;
    }

    QJsonObject rootObject = doc.object();
    QJsonObject mainObject = rootObject["main"].toObject();

    if (mainObject.contains("temp")) {
        double kelvinTemperature = mainObject["temp"].toDouble();

        // Process temperature through Python (convert from Kelvin to Celsius)
        double celsiusTemperature = m_pythonBridge->convertKelvinToCelsius(kelvinTemperature);
        insertData(celsiusTemperature);        
        emit insertDataDone();
    } else {
        emit errorOccurred("Temperature data not found in response");
    }
}

void WeatherFetcher::insertData( double temperature)
{
    if (m_databaseManager && m_databaseManager->isConnected()) {
        qDebug() << "Inserting temperature:" << temperature;
        QString queryStr = QString("INSERT INTO test_location (value, time) VALUES (%1, NOW())")
                               .arg(temperature);
        if (!m_databaseManager->executeQuery(queryStr)) {
            emit errorOccurred("Failed to insert data: " + m_databaseManager->getLastError());
        }
    } else {
        qDebug() << "Database not connected";
        emit errorOccurred("Database not connected");
    }
}

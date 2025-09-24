#include "../include/WeatherFetcher.h"
#include <QJsonParseError>
#include <QDebug>

WeatherFetcher::WeatherFetcher(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_apiUrl("https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=a37d50cf573ace59c09175f7f0e7f164")
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WeatherFetcher::onRequestFinished);
}

void WeatherFetcher::fetchWeather()
{
    QNetworkRequest request(m_apiUrl);
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
        double temperature = mainObject["temp"].toDouble();
        emit temperatureReceived(temperature);
    } else {
        emit errorOccurred("Temperature data not found in response");
    }
}
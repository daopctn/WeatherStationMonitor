#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include "PythonBridge.h"

class WeatherFetcher : public QObject
{
    Q_OBJECT

public:
    explicit WeatherFetcher(QObject *parent = nullptr);
    ~WeatherFetcher();
    void fetchWeather();

signals:
    void temperatureReceived(double temperature);
    void errorOccurred(const QString &error);

private slots:
    void onRequestFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiUrl;
    PythonBridge *m_pythonBridge;
};

#endif // WEATHERFETCHER_H
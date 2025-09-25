#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include "PythonBridge.h"
#include "DatabaseManager.h"

class WeatherFetcher : public QObject
{
    Q_OBJECT

public:
    explicit WeatherFetcher(QObject *parent = nullptr, DatabaseManager *databaseManager = nullptr, PythonBridge *pythonBridge = nullptr);
    // ~WeatherFetcher();
    void fetchWeather();

signals:
    void insertDataDone();
    void errorOccurred(const QString &error);

private slots:
    void onRequestFinished(QNetworkReply *reply);
    void insertData(double temperature);

private:
    QNetworkAccessManager *m_networkManager;
    PythonBridge *m_pythonBridge;
    DatabaseManager *m_databaseManager;

};

#endif // WEATHERFETCHER_H
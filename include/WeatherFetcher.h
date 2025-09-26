#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include "PythonBridge.h"
#include "DatabaseManager.h"

struct Location
{
    double lat;
    double lon;
    QString name;
};

class WeatherFetcher : public QObject
{
    Q_OBJECT

public:
    explicit WeatherFetcher(QObject *parent = nullptr, DatabaseManager *databaseManager = nullptr, PythonBridge *pythonBridge = nullptr);
    ~WeatherFetcher();
    void fetchMultipleWeather(const QList<Location> &locations);

signals:
    void insertDataDone();
    void errorOccurred(const QString &error);

private slots:
    void onRequestFinished(QNetworkReply *reply);
    void insertData(double temperature, double humidity, const QString &locationName);

private:
    QNetworkAccessManager *m_networkManager;
    PythonBridge *m_pythonBridge;
    DatabaseManager *m_databaseManager;
    QMap<QNetworkReply *, QString> m_replyLocationMap;
    QString m_apiKey;
    QString m_baseUrl;
};

#endif // WEATHERFETCHER_H
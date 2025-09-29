/*
    - Create WeatherWorker inherit from QThread
    - Each thread create its own QNetworkAccessManager in its run() method
    - Fetch weather data from OpenWeatherMap API
    - Parse JSON response to extract temperature, humidity, and weather description
    - Will modify a shared Queue of WeatherData
    - Use QMutex to protect the shared Queue
    - Request every 10 seconds for test and 30 minutes for production
*/

#ifndef WEATHERWORKER_H
#define WEATHERWORKER_H

#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutex>
#include <QQueue>
#include "WeatherData.h"

class WeatherWorker : public QThread
{
    Q_OBJECT
public:
    explicit WeatherWorker(const QString &apiKey, const QString &city, QQueue<WeatherData> &dataQueue, QMutex &mutex, QObject *parent = nullptr);
    ~WeatherWorker();
    void run() override;
    void stop();
signals:
    void weatherDataUpdated();
private slots:
    void onNetworkReply(QNetworkReply *reply);

private:
    QString m_apiKey;
    QString m_city;
    QQueue<WeatherData> &m_dataQueue;
    QMutex &m_mutex;
    bool m_running;
};

#endif // WEATHERWORKER_H
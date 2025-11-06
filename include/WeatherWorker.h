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
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <QVector>
#include <QUrl>
#include <QTime>
#include "WeatherData.h"

class WeatherWorker : public QThread
{
    Q_OBJECT
public:
    explicit WeatherWorker(const QString &apiURL, QVector<WeatherData> &weatherDataVector, QMutex &mutex, QObject *parent = nullptr);
    ~WeatherWorker();
    void run() override;
    void stop();
    WeatherData *lastestData = nullptr;

    // Configuration Constants
    static constexpr int WEATHER_FETCH_INTERVAL_MS = 300000;  ///< Fetch weather data every 5 minutes (300,000 ms)
    static constexpr int MAX_RETRY_ATTEMPTS = 3;              ///< Maximum number of retry attempts for failed requests
    static constexpr int BASE_RETRY_DELAY_MS = 2000;          ///< Base delay for exponential backoff (2 seconds)
    static constexpr int NETWORK_TIMEOUT_MS = 10000;          ///< Network request timeout (10 seconds)

signals:
    void weatherDataUpdated(const WeatherData &data);
    void errorOccurred(const QString &errorMessage);          ///< Emitted when an error occurs

private slots:
    void onNetworkReply(QNetworkReply *reply);

private:
    QString m_apiURL;
    QVector<WeatherData> &m_weatherDataVector;
    QMutex &m_mutex;
    QNetworkAccessManager *m_networkManager;
    bool m_running;
    int m_consecutiveFailures;                                  ///< Track consecutive failures for rate limiting

    void fetchWeatherData();

    /**
     * @brief Implements exponential backoff delay
     * @param attempt Current attempt number (0-indexed)
     * @note Delays: 2s, 4s, 8s for attempts 0, 1, 2
     */
    void exponentialBackoff(int attempt);
};

#endif // WEATHERWORKER_H
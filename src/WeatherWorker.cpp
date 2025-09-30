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
    if (m_networkManager)
    {
        m_networkManager->deleteLater();
        m_networkManager = nullptr;
    }
}

void WeatherWorker::run()
{
    m_networkManager = new QNetworkAccessManager();
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &WeatherWorker::onNetworkReply);

    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &WeatherWorker::fetchWeatherData);
    timer->start(5000); // 5 seconds for testing

    m_running = true;

    // Fetch data immediately, then every 5 seconds via timer
    fetchWeatherData();

    // Start the event loop - this will process Qt signals and slots
    exec();

    // Cleanup when event loop exits
    timer->stop();
    delete timer;
}

void WeatherWorker::stop()
{
    // qDebug() << "Stopping WeatherWorker thread...";
    m_running = false;
    quit(); // Exit the event loop
}

void WeatherWorker::fetchWeatherData()
{
    if (!m_networkManager)
    {
        qWarning() << "NetworkManager not initialized";
        return;
    }

    // qDebug() << "Fetching weather data from:" << m_apiURL;
    QNetworkRequest request(m_apiURL);
    m_networkManager->get(request);
    // qDebug() << "Request sent, waiting for reply...";
}

void WeatherWorker::onNetworkReply(QNetworkReply *reply)
{
    qDebug() << "onNetworkReply called!";
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    // Print raw response for debugging
    // qDebug() << "Raw API Response:" << responseData;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isObject())
    {
        qWarning() << "Invalid JSON response";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("main") || !jsonObj["main"].isObject())
    {
        qWarning() << "JSON does not contain 'main' object";
        return;
    }

    QJsonObject mainObj = jsonObj["main"].toObject();
    double temperature = mainObj.value("temp").toDouble();
    double humidity = mainObj.value("humidity").toDouble();

    WeatherData data;
    data.temperature = temperature;
    data.humidity = humidity;
    data.timestamp = QTime::currentTime();

    // Print parsed data for debugging
    qDebug() << "Parsed Weather Data:";
    qDebug() << "  Temperature:" << data.temperature << "K";
    qDebug() << "  Humidity:" << data.humidity << "%";
    qDebug() << "  Timestamp:" << data.timestamp.toString();

    m_mutex.lock();
    m_weatherDataVector.append(data);
    qDebug() << "  Total data points collected:" << m_weatherDataVector.size();
    m_mutex.unlock();

    emit weatherDataUpdated();
}

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

    QTimer timer;
    connect(&timer, &QTimer::timeout,
            this, &WeatherWorker::fetchWeatherData,
            Qt::DirectConnection); // DirectConnection for same thread

    // 5 minutes for production

    timer.start(300000); // 300,000 ms = 5 minutes

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

    // Convert UNIX timestamp (dt) to QDateTime, then extract QTime
    long long unixTime = jsonObj.value("dt").toVariant().toLongLong();

    if (unixTime > lastestData->timestamp)
    {
        WeatherData newData;
        newData.locationName = jsonObj.value("name").toString();
        newData.temperature = temperature;
        newData.humidity = humidity;
        newData.timestamp = unixTime;

        lastestData->locationName = newData.locationName;
        lastestData->temperature = newData.temperature;
        lastestData->humidity = newData.humidity;
        lastestData->timestamp = newData.timestamp;

        // Print parsed data for debugging with thread id in 1 qDebug line

        qDebug() << "Parsed Data - Thread ID:" << QThread::currentThreadId()
                 << " Location:" << newData.locationName
                 << " Temp:" << newData.temperature
                 << " Humidity:" << newData.humidity
                 << " Time:" << newData.timestamp;
        m_mutex.lock();
        m_weatherDataVector.append(newData);
        m_mutex.unlock();
        emit weatherDataUpdated();
    }
    else
    {
        qDebug() << "Thread ID:" << QThread::currentThreadId()
                 << "Received older data. Ignoring update."
                 << " New timestamp:" << unixTime
                 << " Last timestamp:" << lastestData->timestamp;
    }
}

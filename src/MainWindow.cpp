#include "../include/MainWindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), weatherFetcher(new WeatherFetcher(this)), databaseManager(new DatabaseManager(this))
{
    ui->setupUi(this);
    setWindowTitle("Weather Station Monitor");

    connect(weatherFetcher, &WeatherFetcher::temperatureReceived,
            this, &MainWindow::onTemperatureReceived);
    connect(weatherFetcher, &WeatherFetcher::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(ui->pushButton, &QPushButton::clicked,
            weatherFetcher, &WeatherFetcher::fetchWeather);

    testDatabaseConnection();
    weatherFetcher->fetchWeather();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTemperatureReceived(double temperature)
{
    // Temperature is now already converted from Kelvin to Celsius by Python processor
    ui->label_4->setText(QString::number(temperature, 'f', 1) + "°C (Processed by Python)");
    m_lastFetchTime = QTime::currentTime();
    // debug
    qDebug() << "Temperature received (Celsius):" << temperature << "at" << m_lastFetchTime.toString("HH:mm:ss");
    ui->label_5->setText("Last updated at: " + m_lastFetchTime.toString("HH:mm:ss"));
}

void MainWindow::onErrorOccurred(const QString &error)
{
    ui->label_4->setText("Error: " + error);
}

void MainWindow::testDatabaseConnection()
{
    qDebug() << "Testing database connection...";

    bool connected = databaseManager->connectToDatabase(
        "localhost",
        "weather_db",
        "daopctn",
        "dao02112003"
    );

    if (connected) {
        qDebug() << "Database connection test: SUCCESS";

        bool queryResult = databaseManager->executeQuery(
            "INSERT INTO test_location (value, time) VALUES (17.5, NOW())");

        if (queryResult) {
            qDebug() << "Sample data inserted successfully";
        } else {
            qDebug() << "Failed to insert data:" << databaseManager->getLastError();
        }
    } else {
        qDebug() << "Database connection test: FAILED -" << databaseManager->getLastError();
    }
}
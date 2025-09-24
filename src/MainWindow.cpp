#include "../include/MainWindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), weatherFetcher(new WeatherFetcher(this))
{
    ui->setupUi(this);
    setWindowTitle("Weather Station Monitor");

    connect(weatherFetcher, &WeatherFetcher::temperatureReceived,
            this, &MainWindow::onTemperatureReceived);
    connect(weatherFetcher, &WeatherFetcher::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(ui->pushButton, &QPushButton::clicked,
            weatherFetcher, &WeatherFetcher::fetchWeather);

    weatherFetcher->fetchWeather();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTemperatureReceived(double temperature)
{
    ui->label_4->setText(QString::number(temperature, 'f', 1) + "°C");
    m_lastFetchTime = QTime::currentTime();
    // debug
    qDebug() << "Temperature received:" << temperature << "at" << m_lastFetchTime.toString("HH:mm:ss");
    ui->label_5->setText("Last updated at: " + m_lastFetchTime.toString("HH:mm:ss"));
}

void MainWindow::onErrorOccurred(const QString &error)
{
    ui->label_4->setText("Error: " + error);
}
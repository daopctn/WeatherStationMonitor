#include "../include/MainWindow.h"
#include <QDebug>
#include <PythonBridge.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), pythonBridge(new PythonBridge())
{
    ui->setupUi(this);
    setWindowTitle("Weather Station Monitor");

    // database connection
    databaseManager = new DatabaseManager(this);
    QString dbHost = qgetenv("DB_HOST");
    QString dbName = qgetenv("DB_NAME");
    QString dbUser = qgetenv("DB_USER");
    QString dbPassword = qgetenv("DB_PASSWORD");

    if (dbHost.isEmpty()) dbHost = "localhost";
    if (dbName.isEmpty()) dbName = "weather_db";
    if (dbUser.isEmpty()) dbUser = "daopctn";

    bool connection = databaseManager->connectToDatabase(
        dbHost,
        dbName,
        dbUser,
        dbPassword);

    if (connection)
    {
        qDebug() << "Database connected successfully.";
    }
    else
    {
        qDebug() << "Database connection failed:" << databaseManager->getLastError();
    }

    // weather fetcher
    weatherFetcher = new WeatherFetcher(this, databaseManager, pythonBridge);

    // // activate timer
    m_fetchTimer = new QTimer(this);
    connect(m_fetchTimer, &QTimer::timeout, weatherFetcher, &WeatherFetcher::fetchWeather);
    m_fetchTimer->start(60000); // fetch every 60 seconds

    // signals and slots

    connect(weatherFetcher, &WeatherFetcher::insertDataDone,
            this, &MainWindow::onInsertDataDone);
    connect(weatherFetcher, &WeatherFetcher::errorOccurred,
            this, &MainWindow::onInsertDataDone);
    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::onButtonClicked);
    MainWindow::onButtonClicked();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (databaseManager->isConnected())
    {
        databaseManager->disconnectFromDatabase();
    }
    delete databaseManager;
    delete weatherFetcher;
}

void MainWindow::onInsertDataDone()
{
    QString dbHost = qgetenv("DB_HOST");
    QString dbName = qgetenv("DB_NAME");
    QString dbUser = qgetenv("DB_USER");
    QString dbPassword = qgetenv("DB_PASSWORD");

    if (dbHost.isEmpty()) dbHost = "localhost";
    if (dbName.isEmpty()) dbName = "weather_db";
    if (dbUser.isEmpty()) dbUser = "daopctn";

    double a = pythonBridge->calculateAverageTemperature(
        dbHost,
        dbName,
        dbUser,
        dbPassword);
    // print pythonBridge avg
    qDebug() << "Average temperature from DB (Python): " << a;
}

void MainWindow::onErrorOccurred(const QString &error)
{
    ui->label_4->setText("Error: " + error);
}

void MainWindow::onButtonClicked()
{
    // update ui
    if (databaseManager->isConnected())
    {
        QSqlQuery query = databaseManager->prepareQuery("SELECT value, time FROM test_location ORDER BY time DESC LIMIT 1");
        if (query.exec() && query.next())
        {
            double latestTemp = query.value(0).toDouble();
            QString latestTime = query.value(1).toString();
            ui->label_4->setText(QString::number(latestTemp) + " °C");
            ui->label_5->setText(latestTime);
        }
        else
        {
            ui->label_4->setText("No data");
            ui->label_5->setText("");
        }

        QString dbHost = qgetenv("DB_HOST");
        QString dbName = qgetenv("DB_NAME");
        QString dbUser = qgetenv("DB_USER");
        QString dbPassword = qgetenv("DB_PASSWORD");

        if (dbHost.isEmpty()) dbHost = "localhost";
        if (dbName.isEmpty()) dbName = "weather_db";
        if (dbUser.isEmpty()) dbUser = "daopctn";

        double avgTemp = pythonBridge->calculateAverageTemperature(
            dbHost,
            dbName,
            dbUser,
            dbPassword);
        ui->label_6->setText(QString::number(avgTemp) + " °C");
    }
    else
    {
        ui->label_4->setText("DB not connected");
        ui->label_5->setText("");
        ui->label_6->setText("");
    }
}
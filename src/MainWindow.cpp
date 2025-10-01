#include "../include/MainWindow.h"
#include <QDebug>
#include <QFile>
#include <PythonBridge.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), pythonBridge(new PythonBridge())
{
    ui->setupUi(this);
    setWindowTitle("Weather Station Monitor");

    // QFile file("/home/daopctn/Projects/WeatherStationMonitor/config.json");
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir); // ensure exists
    QString configPath = configDir + "/config.json";
    qDebug() << "Using config file at:" << configPath;
    QFile file(configPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open config file";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
    {
        qWarning() << "Invalid JSON format";
        return;
    }

    QJsonObject obj = doc.object();
    // Database
    QJsonObject dbConfig = obj.value("Database").toObject();
    m_hostname = dbConfig.value("host").toString();
    m_databaseName = dbConfig.value("name").toString();
    m_username = dbConfig.value("user").toString();
    m_password = dbConfig.value("password").toString();

    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::onButtonClicked);

    // Thread manager
    threadManager = new ThreadManager(this);
    threadManager->startThreads();

    // Khởi động spinner
    m_spinner = new Spinner(this);
    m_spinner->start();
}

MainWindow::~MainWindow()
{
    // Safe delete ui
    if (ui)
    {
        delete ui;
        ui = nullptr;
    }

    // free ThreadManager
    if (threadManager)
    {
        threadManager->stopThreads();
        threadManager->waitForThreads();
        delete threadManager;
        threadManager = nullptr;
    }

    // free python bridge
    if (pythonBridge)
    {
        delete pythonBridge;
        pythonBridge = nullptr;
    }

    // Dừng spinner
    if (m_spinner)
    {
        m_spinner->stop();
        m_spinner->wait();
        delete m_spinner;
        m_spinner = nullptr;
    }
}

void MainWindow::onButtonClicked()
{
    // Pause spinner during operation
    if (m_spinner)
    {
        m_spinner->pause();
    }

    // Create database connection using DatabaseManager
    DatabaseManager dbManager;
    bool connected = dbManager.connectToDatabase(
        m_hostname,
        m_databaseName,
        m_username,
        m_password,
        3306,
        "MainWindow_connection");

    if (!connected)
    {
        qDebug() << "Database connection failed:" << dbManager.getLastError();
        return;
    }

    QSqlDatabase db = dbManager.getDatabase();

    // update ui
    QStringList tables = {"london", "new_york", "paris", "rome", "zocca"};
    int row = 0;
    ui->tableWidget->setRowCount(0);
    for (const QString &tbl : tables)
    {
        QSqlQuery q(db);
        q.prepare("SELECT temperature, humidity, timestamp FROM " + tbl + " ORDER BY id DESC LIMIT 1");
        if (q.exec() && q.next())
        {
            ui->tableWidget->insertRow(row);
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(tbl));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(q.value(0).toString()));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(q.value(1).toString()));
            pythonBridge->calculateAverageData(
                m_hostname,
                m_databaseName,
                m_username,
                m_password,
                tbl,
                m_avgTemperature,
                m_avgHumidity);
            qDebug() << tbl << "- Temp:" << q.value(0).toString() << "Humidity:" << q.value(1).toString() << "Time:" << q.value(2).toString() << "- Avg Temp:" << m_avgTemperature << "Avg Humidity:" << m_avgHumidity;
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(m_avgTemperature) + " °C"));
            ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(m_avgHumidity) + " %"));
            ui->tableWidget->setItem(row, 5, new QTableWidgetItem(q.value(2).toString()));
            row++;
        }
    }

    dbManager.disconnectFromDatabase();

    // Resume spinner after operation
    if (m_spinner)
    {
        m_spinner->resume();
    }
}
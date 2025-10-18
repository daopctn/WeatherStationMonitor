#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include "ui/ui_MainWindow.h"
#include "DatabaseManager.h"
#include "PythonBridge.h"
#include "WeatherWorker.h"
#include "WeatherData.h"
#include "DatabaseThread.h"
#include "Spinner.h"
#include "ThreadManager.h"

/**
 * @class MainWindow
 * @brief Main application window for the Weather Station Monitor
 *
 * This is the primary UI class that displays weather data for 5 monitored cities.
 * It handles:
 * - Loading configuration from JSON file
 * - Establishing database connection
 * - Managing thread lifecycle via ThreadManager
 * - Periodic UI updates with latest weather data from database
 * - Dynamic weather icon display based on conditions and day/night
 *
 * The UI refreshes every 5 seconds to display the most recent weather data
 * stored in the MySQL database by the background worker threads.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // UI Configuration Constants
    static constexpr int UI_UPDATE_INTERVAL_MS = 5000;      ///< UI refresh interval (5 seconds)
    static constexpr int WEATHER_ICON_SIZE = 80;            ///< Weather icon display size in pixels

private slots:

    void onButtonClicked();

private:
    QTimer *dataUpdateTimer;
    Ui::MainWindow *ui;
    PythonBridge *pythonBridge;
    double m_avgTemperature;
    double m_avgHumidity;
    QString m_hostname;
    QString m_databaseName;
    QString m_username;
    QString m_password;

    Spinner *m_spinner;

    ThreadManager *threadManager;
    DatabaseManager *m_dbManager;  // Persistent database connection
};

#endif // MAINWINDOW_H
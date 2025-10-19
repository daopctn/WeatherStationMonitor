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
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

QT_CHARTS_USE_NAMESPACE

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
    /**
     * @brief Refreshes UI with latest weather data from database
     * @note Called automatically by QTimer every UI_UPDATE_INTERVAL_MS (5 seconds)
     */
    void refreshWeatherUI();

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

    /**
     * @brief Initializes all chart views with data from database
     * @note Called once during MainWindow construction
     */
    void initializeCharts();

    /**
     * @brief Creates and populates a chart for a specific location
     * @param chartView The QChartView widget to populate
     * @param tableName Database table name for the location
     * @param locationName Display name for the chart title
     * @param timezoneOffset Timezone offset in seconds from UTC
     */
    void setupChart(QChartView *chartView, const QString &tableName, const QString &locationName, int timezoneOffset);
};

#endif // MAINWINDOW_H
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
 * - Real-time UI updates via signals from WeatherWorker threads
 * - Dynamic weather icon display based on conditions and day/night
 *
 * The UI updates in real-time whenever new weather data arrives from the API,
 * using Qt's signal/slot mechanism to receive data from background worker threads.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @struct LocationInfo
     * @brief Stores configuration for a weather monitoring location
     */
    struct LocationInfo
    {
        QString tableName;      ///< Database table name (e.g., "zocca", "new_york")
        QString displayName;    ///< Display name for UI (e.g., "Zocca", "New York")
        int timezoneOffset;     ///< Timezone offset in seconds from UTC
        int uiIndex;            ///< Index for UI widget mapping (0-4)
    };

    /// Centralized location configuration to avoid duplication across methods
    static const QVector<LocationInfo> LOCATIONS;

    // UI Configuration Constants
    static constexpr int WEATHER_ICON_SIZE = 80;            ///< Weather icon display size in pixels

private slots:
    /**
     * @brief Refreshes UI with latest weather data from database for all locations
     * @note Called once during startup to populate initial UI state
     */
    void refreshWeatherUI();

    /**
     * @brief Handles real-time weather data updates from worker threads
     * @param data The new weather data received from API
     * @note This slot receives signals directly from WeatherWorker threads
     */
    void onWeatherDataUpdated(const WeatherData &data);

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

    // Chart series references for real-time updates (indexed by location.uiIndex)
    QLineSeries *m_tempSeries[5];      ///< Temperature line series for each location
    QLineSeries *m_humSeries[5];       ///< Humidity line series for each location
    QScatterSeries *m_tempMarkers[5];  ///< Temperature marker series for each location
    QScatterSeries *m_humMarkers[5];   ///< Humidity marker series for each location
    QLineSeries *m_zeroLine[5];        ///< Zero reference line for each location
    QDateTimeAxis *m_axisX[5];         ///< X-axis (time) for each chart - needed to update range
    int m_timezoneOffsets[5];          ///< Timezone offsets for tooltip display
    bool m_hasVirtualPoint[5];         ///< Tracks if chart has a virtual second point for single-point display

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
     * @param locationIndex Index of the location (0-4) for storing series references
     */
    void setupChart(QChartView *chartView, const QString &tableName, const QString &locationName, int timezoneOffset, int locationIndex);

    /**
     * @brief Updates a chart in real-time with new weather data
     * @param locationIndex Index of the location (0-4)
     * @param data The new weather data to add to the chart
     * @note Maintains a rolling window of 100 data points
     */
    void updateChartRealtime(int locationIndex, const WeatherData &data);
};

#endif // MAINWINDOW_H
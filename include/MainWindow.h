#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTime>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include "ui/ui_MainWindow.h"
#include "WeatherFetcher.h"
#include "DatabaseManager.h"
#include "PythonBridge.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onInsertDataDone();
    void onErrorOccurred(const QString &error);
    // void testDatabaseConnection();
    void onButtonClicked();
    void fetchWeatherForAllLocations();

private:
    Ui::MainWindow *ui;
    DatabaseManager *databaseManager;
    WeatherFetcher *weatherFetcher;
    PythonBridge *pythonBridge;
    QTime m_lastFetchTime;
    QTimer *m_fetchTimer;
    double m_avgTemperature;
    double m_avgHumidity;
    // const QString &host,
    //                                              const QString &database,
    //                                              const QString &username,
    //                                              const QString &password,
    QString m_hostname;
    QString m_databaseName;
    QString m_username;
    QString m_password;
};

#endif // MAINWINDOW_H
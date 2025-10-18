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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
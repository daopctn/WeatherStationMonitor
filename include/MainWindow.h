#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTime>
#include <QTimer>
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

private:
    Ui::MainWindow *ui;
    DatabaseManager *databaseManager;
    WeatherFetcher *weatherFetcher;
    PythonBridge *pythonBridge;
    QTime m_lastFetchTime;
    QTimer *m_fetchTimer;
};

#endif // MAINWINDOW_H
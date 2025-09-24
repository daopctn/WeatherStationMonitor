#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTime>
#include "ui/ui_MainWindow.h"
#include "WeatherFetcher.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTemperatureReceived(double temperature);
    void onErrorOccurred(const QString &error);

private:
    Ui::MainWindow *ui;
    WeatherFetcher *weatherFetcher;
    QTime m_lastFetchTime;
};

#endif // MAINWINDOW_H
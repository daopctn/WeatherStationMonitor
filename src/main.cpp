#include <QApplication>
#include "../include/MainWindow.h"
#include "../include/WeatherData.h"
#include <iostream>

// Undefine Qt macros that conflict with Python
#undef slots
#include "../include/PythonBridge.h"
#include <Python.h>
#define slots Q_SLOTS

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Register WeatherData type for cross-thread signal/slot communication
    qRegisterMetaType<WeatherData>("WeatherData");

    MainWindow window;
    window.show();

    return app.exec();
}
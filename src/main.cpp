#include <QApplication>
#include "../include/MainWindow.h"
#include <iostream>

// Undefine Qt macros that conflict with Python
#undef slots
#include "../include/PythonBridge.h"
#include <Python.h>
#define slots Q_SLOTS

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    MainWindow window;
    window.show();

    return app.exec();
}
#include "PythonBridge.h"
#include <iostream>
#include <QDebug>
// Undefine Qt macros that conflict with Python
#undef slots
#include <Python.h>
#define slots Q_SLOTS

PythonBridge::PythonBridge() : m_initialized(false)
{
    initialize();
}

PythonBridge::~PythonBridge()
{
    finalize();
}

bool PythonBridge::initialize()
{
    if (m_initialized)
    {
        return true;
    }

    Py_Initialize();
    if (!Py_IsInitialized())
    {
        std::cerr << "Failed to initialize Python" << std::endl;
        return false;
    }

    m_initialized = true;
    return true;
}

void PythonBridge::finalize()
{
    if (m_initialized)
    {
        Py_Finalize();
        m_initialized = false;
    }
}

double PythonBridge::convertKelvinToCelsius(double kelvinTemp)
{
    if (!m_initialized)
    {
        std::cerr << "Python not initialized!" << std::endl;
        return 0.0;
    }

    // Use Python to do the conversion
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("parent_dir = os.path.dirname(os.getcwd())");
    PyRun_SimpleString("python_path = os.path.join(parent_dir, 'python')");
    PyRun_SimpleString("print(f'Adding Python path: {python_path}')");
    PyRun_SimpleString("sys.path.insert(0, python_path)");

    // Import our processor module and call the function
    PyObject *pModule = PyImport_ImportModule("processor");
    if (pModule == nullptr)
    {
        PyErr_Print();
        std::cerr << "CRITICAL ERROR: Failed to import processor module!" << std::endl;
        return 0.0;
    }

    PyObject *pFunc = PyObject_GetAttrString(pModule, "kelvin_to_celsius");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        PyErr_Print();
        std::cerr << "CRITICAL ERROR: Cannot find kelvin_to_celsius function!" << std::endl;
        Py_DECREF(pModule);
        return 0.0;
    }

    // Call the Python function with the Kelvin temperature
    PyObject *pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(kelvinTemp));

    PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

    double result = 0.0;
    if (pValue != nullptr && PyFloat_Check(pValue))
    {
        result = PyFloat_AsDouble(pValue);
        std::cout << "Python converted " << kelvinTemp << "K to " << result << "°C" << std::endl;
    }
    else
    {
        PyErr_Print();
        std::cerr << "CRITICAL ERROR: Python function call failed!" << std::endl;
    }

    // Cleanup
    Py_XDECREF(pValue);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    return result;
}
double PythonBridge::calculateAverageTemperature(const QString &host,
                                                 const QString &database,
                                                 const QString &username,
                                                 const QString &password,
                                                 const QString &tbl)
{
    if (!m_initialized)
    {
        std::cerr << "Python not initialized!" << std::endl;
        return 0.0;
    }

    // Setup Python path
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("parent_dir = os.path.dirname(os.getcwd())");
    PyRun_SimpleString("python_path = os.path.join(parent_dir, 'python')");
    PyRun_SimpleString("sys.path.insert(0, python_path)");

    // Import processor module
    PyObject *pModule = PyImport_ImportModule("processor");
    if (pModule == nullptr)
    {
        PyErr_Print();
        std::cerr << "Failed to import processor module!" << std::endl;
        return 0.0;
    }

    // Get calculate_average_from_db function
    PyObject *pFunc = PyObject_GetAttrString(pModule, "calculate_average_from_db");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        PyErr_Print();
        std::cerr << "Cannot find calculate_average_from_db function!" << std::endl;
        Py_DECREF(pModule);
        return 0.0;
    }

    // Create arguments tuple with database credentials
    PyObject *pArgs = PyTuple_New(5);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(host.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(database.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(username.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(password.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 4, PyUnicode_FromString(tbl.toUtf8().constData()));

    // Call Python function
    PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

    double result = 0.0;
    if (pValue != nullptr && PyFloat_Check(pValue))
    {
        result = PyFloat_AsDouble(pValue);
        std::cout << "Python calculated average temperature: " << result << "°C" << std::endl;
    }
    else
    {
        PyErr_Print();
        std::cerr << "Python function call failed!" << std::endl;
    }

    // Cleanup
    Py_XDECREF(pValue);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    return result;
}

double PythonBridge::calculateAverageHumidity(const QString &host,
                                              const QString &database,
                                              const QString &username,
                                              const QString &password,
                                              const QString &tbl)
{
    if (!m_initialized)
    {
        std::cerr << "Python not initialized!" << std::endl;
        return 0.0;
    }

    // Setup Python path
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("parent_dir = os.path.dirname(os.getcwd())");
    PyRun_SimpleString("python_path = os.path.join(parent_dir, 'python')");
    PyRun_SimpleString("sys.path.insert(0, python_path)");

    // Import processor module
    PyObject *pModule = PyImport_ImportModule("processor");
    if (pModule == nullptr)
    {
        PyErr_Print();
        std::cerr << "Failed to import processor module!" << std::endl;
        return 0.0;
    }

    // Get calculate_average_humidity_from_db function
    PyObject *pFunc = PyObject_GetAttrString(pModule, "calculate_average_humidity_from_db");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        PyErr_Print();
        std::cerr << "Cannot find calculate_average_humidity_from_db function!" << std::endl;
        Py_DECREF(pModule);
        return 0.0;
    }

    // Create arguments tuple with database credentials
    PyObject *pArgs = PyTuple_New(5);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(host.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(database.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(username.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(password.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 4, PyUnicode_FromString(tbl.toUtf8().constData()));

    // Call Python function
    PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

    double result = 0.0;
    if (pValue != nullptr && PyFloat_Check(pValue))
    {
        result = PyFloat_AsDouble(pValue);
        std::cout << "Python calculated average humidity: " << result << "%" << std::endl;
    }
    else
    {
        PyErr_Print();
        std::cerr << "Python function call failed!" << std::endl;
    }
    // Cleanup
    Py_XDECREF(pValue);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    return result;
}

void PythonBridge::calculateAverageData(const QString &host,
                                        const QString &database,
                                        const QString &username,
                                        const QString &password,
                                        const QString &tbl,
                                        double &avgTemp,
                                        double &avgHumidity)
{
    if (!m_initialized)
    {
        std::cerr << "Python not initialized!" << std::endl;
        avgTemp = 0.0;
        avgHumidity = 0.0;
        return;
    }

    // Setup Python path
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("parent_dir = os.path.dirname(os.getcwd())");
    PyRun_SimpleString("python_path = os.path.join(parent_dir, 'python')");
    PyRun_SimpleString("sys.path.insert(0, python_path)");

    // Import processor module
    PyObject *pModule = PyImport_ImportModule("processor");
    if (pModule == nullptr)
    {
        PyErr_Print();
        std::cerr << "Failed to import processor module!" << std::endl;
        avgTemp = 0.0;
        avgHumidity = 0.0;
        return;
    }

    // Get calculate_both_averages_from_db function
    PyObject *pFunc = PyObject_GetAttrString(pModule, "calculate_both_averages_from_db");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        PyErr_Print();
        std::cerr << "Cannot find calculate_both_averages_from_db function!" << std::endl;
        Py_DECREF(pModule);
        avgTemp = 0.0;
        avgHumidity = 0.0;
        return;
    }

    // Create arguments tuple with database credentials
    PyObject *pArgs = PyTuple_New(5);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(host.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(database.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(username.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(password.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 4, PyUnicode_FromString(tbl.toUtf8().constData()));

    // Call Python function
    PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

    // Initialize defaults
    avgTemp = 0.0;
    avgHumidity = 0.0;

    // Extract tuple values
    if (pValue != nullptr && PyTuple_Check(pValue) && PyTuple_Size(pValue) == 2)
    {
        PyObject *tempObj = PyTuple_GetItem(pValue, 0);
        PyObject *humidityObj = PyTuple_GetItem(pValue, 1);

        if (tempObj && PyFloat_Check(tempObj))
        {
            avgTemp = PyFloat_AsDouble(tempObj);
        }

        if (humidityObj && PyFloat_Check(humidityObj))
        {
            avgHumidity = PyFloat_AsDouble(humidityObj);
        }

        std::cout << "Python calculated averages - Temperature: " << avgTemp << "°C, Humidity: " << avgHumidity << "%" << std::endl;
    }
    else
    {
        PyErr_Print();
        std::cerr << "Python function call failed or returned invalid tuple!" << std::endl;
    }

    // Cleanup
    Py_XDECREF(pValue);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);
}
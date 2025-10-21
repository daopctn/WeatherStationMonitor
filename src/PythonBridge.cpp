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
        qDebug() << "ERROR: Failed to initialize Python";
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
        Py_XDECREF(pFunc);
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

QString PythonBridge::getStatisticsJson(const QString &host,
                                         const QString &database,
                                         const QString &username,
                                         const QString &password)
{
    if (!m_initialized)
    {
        return QString("{\"error\": \"Python not initialized!\"}");
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
        return QString("{\"error\": \"Failed to import processor module!\"}");
    }

    // Get generate_all_statistics_json function
    PyObject *pFunc = PyObject_GetAttrString(pModule, "generate_all_statistics_json");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        PyErr_Print();
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
        return QString("{\"error\": \"Cannot find generate_all_statistics_json function!\"}");
    }

    // Create arguments tuple with database credentials
    PyObject *pArgs = PyTuple_New(4);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(host.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(database.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(username.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(password.toUtf8().constData()));

    // Call Python function - it returns JSON string
    PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

    QString result;
    if (pValue == nullptr)
    {
        PyErr_Print();
        result = "{\"error\": \"Python function call failed!\"}";
    }
    else if (PyUnicode_Check(pValue))
    {
        // Python function returns JSON string
        const char *output = PyUnicode_AsUTF8(pValue);
        if (output)
        {
            result = QString::fromUtf8(output);
        }
        else
        {
            result = "{\"error\": \"Could not decode Python string\"}";
        }
    }
    else
    {
        result = "{\"error\": \"Python function did not return a string\"}";
    }

    // Cleanup
    Py_XDECREF(pValue);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    return result;
}

QString PythonBridge::getPeriodComparisonJson(const QString &host,
                                               const QString &database,
                                               const QString &username,
                                               const QString &password,
                                               const QString &table)
{
    if (!m_initialized)
    {
        return QString("{\"error\": \"Python not initialized!\"}");
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
        return QString("{\"error\": \"Failed to import processor module!\"}");
    }

    // Get compare_periods function
    PyObject *pFunc = PyObject_GetAttrString(pModule, "compare_periods");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        PyErr_Print();
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
        return QString("{\"error\": \"Cannot find compare_periods function!\"}");
    }

    // Create arguments tuple with database credentials and table name
    PyObject *pArgs = PyTuple_New(5);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(host.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(database.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(username.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(password.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 4, PyUnicode_FromString(table.toUtf8().constData()));

    // Call Python function - it returns JSON string
    PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

    QString result;
    if (pValue == nullptr)
    {
        PyErr_Print();
        result = "{\"error\": \"Python function call failed!\"}";
    }
    else if (PyUnicode_Check(pValue))
    {
        // Python function returns JSON string
        const char *output = PyUnicode_AsUTF8(pValue);
        if (output)
        {
            result = QString::fromUtf8(output);
        }
        else
        {
            result = "{\"error\": \"Could not decode Python string\"}";
        }
    }
    else
    {
        result = "{\"error\": \"Python function did not return a string\"}";
    }

    // Cleanup
    Py_XDECREF(pValue);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    return result;
}

QString PythonBridge::getLocationComparisonJson(const QString &host,
                                                 const QString &database,
                                                 const QString &username,
                                                 const QString &password,
                                                 int hours)
{
    if (!m_initialized)
    {
        return QString("{\"error\": \"Python not initialized!\"}");
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
        return QString("{\"error\": \"Failed to import processor module!\"}");
    }

    // Get compare_all_locations function
    PyObject *pFunc = PyObject_GetAttrString(pModule, "compare_all_locations");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        PyErr_Print();
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
        return QString("{\"error\": \"Cannot find compare_all_locations function!\"}");
    }

    // Create arguments tuple with database credentials and hours
    PyObject *pArgs = PyTuple_New(5);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(host.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(database.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(username.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(password.toUtf8().constData()));
    PyTuple_SetItem(pArgs, 4, PyLong_FromLong(hours));

    // Call Python function - it returns JSON string
    PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

    QString result;
    if (pValue == nullptr)
    {
        PyErr_Print();
        result = "{\"error\": \"Python function call failed!\"}";
    }
    else if (PyUnicode_Check(pValue))
    {
        // Python function returns JSON string
        const char *output = PyUnicode_AsUTF8(pValue);
        if (output)
        {
            result = QString::fromUtf8(output);
        }
        else
        {
            result = "{\"error\": \"Could not decode Python string\"}";
        }
    }
    else
    {
        result = "{\"error\": \"Python function did not return a string\"}";
    }

    // Cleanup
    Py_XDECREF(pValue);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    return result;
}
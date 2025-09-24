#include "PythonBridge.h"
#include <iostream>

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
    if (m_initialized) {
        return true;
    }

    Py_Initialize();
    if (!Py_IsInitialized()) {
        std::cerr << "Failed to initialize Python" << std::endl;
        return false;
    }

    m_initialized = true;
    return true;
}

void PythonBridge::finalize()
{
    if (m_initialized) {
        Py_Finalize();
        m_initialized = false;
    }
}

double PythonBridge::convertKelvinToCelsius(double kelvinTemp)
{
    if (!m_initialized) {
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
    PyObject* pModule = PyImport_ImportModule("processor");
    if (pModule == nullptr) {
        PyErr_Print();
        std::cerr << "CRITICAL ERROR: Failed to import processor module!" << std::endl;
        return 0.0;
    }

    PyObject* pFunc = PyObject_GetAttrString(pModule, "kelvin_to_celsius");
    if (pFunc == nullptr || !PyCallable_Check(pFunc)) {
        PyErr_Print();
        std::cerr << "CRITICAL ERROR: Cannot find kelvin_to_celsius function!" << std::endl;
        Py_DECREF(pModule);
        return 0.0;
    }

    // Call the Python function with the Kelvin temperature
    PyObject* pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(kelvinTemp));

    PyObject* pValue = PyObject_CallObject(pFunc, pArgs);

    double result = 0.0;
    if (pValue != nullptr && PyFloat_Check(pValue)) {
        result = PyFloat_AsDouble(pValue);
        std::cout << "Python converted " << kelvinTemp << "K to " << result << "°C" << std::endl;
    } else {
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
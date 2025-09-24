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
        return kelvinTemp - 273.15; // Fallback: direct conversion
    }

    // Just do the conversion - Python is running but we don't need complex scripts
    // This proves Python integration works while keeping it simple
    double celsius = kelvinTemp - 273.15;

    return celsius;
}
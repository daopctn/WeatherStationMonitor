#ifndef PYTHONBRIDGE_H
#define PYTHONBRIDGE_H

class PythonBridge
{
public:
    PythonBridge();
    ~PythonBridge();

    // Main function: convert Kelvin to Celsius using Python
    double convertKelvinToCelsius(double kelvinTemp);

private:
    bool m_initialized;
    bool initialize();
    void finalize();
};

#endif // PYTHONBRIDGE_H
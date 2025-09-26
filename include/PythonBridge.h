#ifndef PYTHONBRIDGE_H
#define PYTHONBRIDGE_H
#include <DatabaseManager.h>

class PythonBridge
{
public:
    PythonBridge();
    ~PythonBridge();

    // Main function: convert Kelvin to Celsius using Python
    double convertKelvinToCelsius(double kelvinTemp);
    double calculateAverageTemperature(const QString &host,
                                       const QString &database,
                                       const QString &username,
                                       const QString &password,
                                       const QString &tbl);
    double calculateAverageHumidity(const QString &host,
                                    const QString &database,
                                    const QString &username,
                                    const QString &password,
                                    const QString &tbl);
    
    void calculateAverageData(const QString &host,
                              const QString &database,
                              const QString &username,
                              const QString &password,
                              const QString &tbl,
                              double &avgTemp,
                              double &avgHumidity);
    

private:
    bool m_initialized;
    bool initialize();
    void finalize();
};

#endif // PYTHONBRIDGE_H
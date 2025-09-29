#ifndef WEAtHERDATA_H
#define WEATHERDATA_H

#include <QTime>
struct WeatherData {
    double temperature; // in Celsius
    double humidity;    // in percentage
    QTime timestamp;   // Unix timestamp
};

#endif // WEATHERDATA_H
#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QTime>
struct WeatherData
{
    double temperature; // in Celsius
    double humidity;    // in percentage
    QTime timestamp;    // time of the reading
};

#endif // WEATHERDATA_H
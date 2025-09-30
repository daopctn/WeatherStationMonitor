#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QDateTime>
struct WeatherData
{
    QString locationName;
    double temperature;  // in Celsius
    double humidity;     // in percentage
    long long timestamp; // time of the reading
};

#endif // WEATHERDATA_H
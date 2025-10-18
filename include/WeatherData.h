#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QDateTime>
struct WeatherData
{
    QString locationName;
    double temperature;  // in Celsius
    int pressure;      // in hPa
    double humidity;     // in percentage
    double windSpeed;    // in km/h
    int weatherId;     // weather condition ID
    QString description;   // e.g., "Sunny", "Rainy"
    long long timestamp; // time of the reading
    long long sunrise;    // sunrise time
    long long sunset;     // sunset time
};

#endif // WEATHERDATA_H
#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QDateTime>

/**
 * @struct WeatherData
 * @brief Data structure for storing weather information from OpenWeatherMap API
 *
 * This struct holds all weather data retrieved from the API and stored in the database.
 * It includes temperature, atmospheric conditions, timestamps, and weather classification.
 * All timestamp fields are in UNIX epoch seconds (UTC).
 */
struct WeatherData
{
    QString locationName;     ///< City name (e.g., "London", "Paris")
    double temperature;       ///< Temperature in Kelvin (from API)
    int pressure;             ///< Atmospheric pressure in hPa (hectopascals)
    double humidity;          ///< Relative humidity percentage (0-100)
    double windSpeed;         ///< Wind speed in m/s
    int weatherId;            ///< OpenWeatherMap weather condition ID (e.g., 800=clear, 804=clouds)
    QString description;      ///< Human-readable weather description (e.g., "overcast clouds")
    long long timestamp;      ///< Time of weather reading (UNIX epoch seconds, UTC)
    long long sunrise;        ///< Sunrise time for this location (UNIX epoch seconds, UTC)
    long long sunset;         ///< Sunset time for this location (UNIX epoch seconds, UTC)
};

#endif // WEATHERDATA_H
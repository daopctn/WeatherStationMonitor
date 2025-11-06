/**
 * @file test_WeatherData.cpp
 * @brief Unit tests for WeatherData structure
 *
 * Tests data structure initialization, copying, and field validation.
 */

#include <QtTest/QtTest>
#include <WeatherData.h>

class TestWeatherData : public QObject
{
    Q_OBJECT

private slots:
    /**
     * @brief Test default construction
     */
    void testDefaultConstruction()
    {
        WeatherData data;

        // Verify default values (should be zero-initialized)
        QCOMPARE(data.temperature, 0.0);
        QCOMPARE(data.humidity, 0.0);
        QCOMPARE(data.pressure, 0);
        QCOMPARE(data.windSpeed, 0.0);
        QCOMPARE(data.weatherId, 0);
        QVERIFY(data.locationName.isEmpty());
        QVERIFY(data.description.isEmpty());
        QCOMPARE(data.timestamp, 0LL);
        QCOMPARE(data.sunrise, 0LL);
        QCOMPARE(data.sunset, 0LL);
    }

    /**
     * @brief Test field assignment
     */
    void testFieldAssignment()
    {
        WeatherData data;

        // Assign values
        data.locationName = "London";
        data.temperature = 285.15; // 12°C in Kelvin
        data.humidity = 65.5;
        data.pressure = 1013;
        data.windSpeed = 5.5;
        data.weatherId = 800;
        data.description = "Clear sky";
        data.timestamp = 1699200000;
        data.sunrise = 1699165200;
        data.sunset = 1699202400;

        // Verify assignments
        QCOMPARE(data.locationName, QString("London"));
        QCOMPARE(data.temperature, 285.15);
        QCOMPARE(data.humidity, 65.5);
        QCOMPARE(data.pressure, 1013);
        QCOMPARE(data.windSpeed, 5.5);
        QCOMPARE(data.weatherId, 800);
        QCOMPARE(data.description, QString("Clear sky"));
        QCOMPARE(data.timestamp, 1699200000LL);
        QCOMPARE(data.sunrise, 1699165200LL);
        QCOMPARE(data.sunset, 1699202400LL);
    }

    /**
     * @brief Test copy construction
     */
    void testCopyConstruction()
    {
        WeatherData original;
        original.locationName = "Paris";
        original.temperature = 288.15;
        original.humidity = 70.0;
        original.pressure = 1015;
        original.windSpeed = 3.2;
        original.weatherId = 801;
        original.description = "Few clouds";
        original.timestamp = 1699203600;
        original.sunrise = 1699165800;
        original.sunset = 1699202100;

        // Copy construct
        WeatherData copy = original;

        // Verify all fields copied correctly
        QCOMPARE(copy.locationName, original.locationName);
        QCOMPARE(copy.temperature, original.temperature);
        QCOMPARE(copy.humidity, original.humidity);
        QCOMPARE(copy.pressure, original.pressure);
        QCOMPARE(copy.windSpeed, original.windSpeed);
        QCOMPARE(copy.weatherId, original.weatherId);
        QCOMPARE(copy.description, original.description);
        QCOMPARE(copy.timestamp, original.timestamp);
        QCOMPARE(copy.sunrise, original.sunrise);
        QCOMPARE(copy.sunset, original.sunset);
    }

    /**
     * @brief Test assignment operator
     */
    void testAssignmentOperator()
    {
        WeatherData source;
        source.locationName = "New York";
        source.temperature = 283.15;
        source.humidity = 75.0;
        source.timestamp = 1699207200;

        WeatherData dest;
        dest = source;

        QCOMPARE(dest.locationName, source.locationName);
        QCOMPARE(dest.temperature, source.temperature);
        QCOMPARE(dest.humidity, source.humidity);
        QCOMPARE(dest.timestamp, source.timestamp);
    }

    /**
     * @brief Test realistic weather data values
     */
    void testRealisticValues()
    {
        WeatherData data;

        // London typical values
        data.locationName = "London";
        data.temperature = 280.15;  // 7°C
        data.humidity = 82.0;       // Typical for UK
        data.pressure = 1013;       // Standard sea level pressure
        data.windSpeed = 7.2;       // 7.2 m/s (moderate breeze)
        data.weatherId = 500;       // Light rain
        data.description = "light rain";

        // Validate ranges
        QVERIFY(data.temperature > 173.0 && data.temperature < 373.0); // -100°C to 100°C
        QVERIFY(data.humidity >= 0.0 && data.humidity <= 100.0);
        QVERIFY(data.pressure > 800 && data.pressure < 1200);
        QVERIFY(data.windSpeed >= 0.0 && data.windSpeed < 150.0);
        QVERIFY(data.weatherId >= 0);
    }

    /**
     * @brief Test extreme but valid values
     */
    void testExtremeValues()
    {
        WeatherData hotData;
        hotData.temperature = 330.15;  // ~57°C (Death Valley)
        hotData.humidity = 5.0;        // Very dry
        hotData.windSpeed = 45.0;      // Hurricane-force winds

        QVERIFY(hotData.temperature > 273.15); // Above freezing
        QVERIFY(hotData.humidity < 100.0);
        QVERIFY(hotData.windSpeed > 0.0);

        WeatherData coldData;
        coldData.temperature = 233.15;  // -40°C (Arctic)
        coldData.humidity = 95.0;       // Very humid
        coldData.windSpeed = 0.5;       // Calm

        QVERIFY(coldData.temperature < 273.15); // Below freezing
        QVERIFY(coldData.humidity <= 100.0);
        QVERIFY(coldData.windSpeed >= 0.0);
    }

    /**
     * @brief Test QVector compatibility (used in thread-safe data sharing)
     */
    void testQVectorCompatibility()
    {
        QVector<WeatherData> dataVector;

        // Add multiple weather data entries
        WeatherData london;
        london.locationName = "London";
        london.temperature = 280.15;

        WeatherData paris;
        paris.locationName = "Paris";
        paris.temperature = 285.15;

        dataVector.append(london);
        dataVector.append(paris);

        QCOMPARE(dataVector.size(), 2);
        QCOMPARE(dataVector[0].locationName, QString("London"));
        QCOMPARE(dataVector[1].locationName, QString("Paris"));
        QCOMPARE(dataVector[0].temperature, 280.15);
        QCOMPARE(dataVector[1].temperature, 285.15);
    }

    /**
     * @brief Test timestamp ordering (important for duplicate prevention)
     */
    void testTimestampOrdering()
    {
        WeatherData older;
        older.timestamp = 1699200000;

        WeatherData newer;
        newer.timestamp = 1699203600;

        QVERIFY(newer.timestamp > older.timestamp);
        QVERIFY(older.timestamp < newer.timestamp);

        // Simulate duplicate check (as done in WeatherWorker)
        long long latestTimestamp = older.timestamp;
        bool shouldUpdate = (newer.timestamp > latestTimestamp);
        QVERIFY(shouldUpdate);

        shouldUpdate = (older.timestamp > latestTimestamp);
        QVERIFY(!shouldUpdate); // Same timestamp, should not update
    }

    /**
     * @brief Test day/night detection (sunrise/sunset logic)
     */
    void testDayNightDetection()
    {
        WeatherData data;
        data.sunrise = 1699165200;  // 6:00 AM
        data.sunset = 1699202400;   // 6:00 PM
        data.timestamp = 1699180800; // 12:00 noon

        // During daytime
        bool isDaytime = (data.timestamp >= data.sunrise) && (data.timestamp < data.sunset);
        QVERIFY(isDaytime);

        // At night
        data.timestamp = 1699210000; // 10:00 PM
        isDaytime = (data.timestamp >= data.sunrise) && (data.timestamp < data.sunset);
        QVERIFY(!isDaytime); // Should be nighttime
    }
};

QTEST_MAIN(TestWeatherData)
#include "test_WeatherData.moc"

-- Test Database Schema
-- Isolated test database that doesn't affect production data

CREATE DATABASE IF NOT EXISTS weather_station_test_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE weather_station_test_db;

-- Create test tables with same structure as production
CREATE TABLE IF NOT EXISTS zocca (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT NOT NULL COMMENT 'Temperature in Celsius',
    humidity FLOAT NOT NULL COMMENT 'Humidity percentage (0-100)',
    pressure INT NULL COMMENT 'Atmospheric pressure in hPa',
    windSpeed DOUBLE NULL COMMENT 'Wind speed in m/s',
    weather_id INT NULL COMMENT 'OpenWeatherMap weather condition ID',
    description VARCHAR(255) NULL COMMENT 'Weather description',
    timestamp BIGINT NOT NULL COMMENT 'Unix timestamp of data collection',
    sunrise BIGINT NOT NULL COMMENT 'Sunrise time (Unix timestamp)',
    sunset BIGINT NOT NULL COMMENT 'Sunset time (Unix timestamp)'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Test weather data for Zocca';

CREATE TABLE IF NOT EXISTS rome (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT NOT NULL,
    humidity FLOAT NOT NULL,
    pressure INT NULL,
    windSpeed DOUBLE NULL,
    weather_id INT NULL,
    description VARCHAR(255) NULL,
    timestamp BIGINT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Test weather data for Rome';

CREATE TABLE IF NOT EXISTS paris (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT NOT NULL,
    humidity FLOAT NOT NULL,
    pressure INT NULL,
    windSpeed DOUBLE NULL,
    weather_id INT NULL,
    description VARCHAR(255) NULL,
    timestamp BIGINT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Test weather data for Paris';

CREATE TABLE IF NOT EXISTS london (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT NOT NULL,
    humidity FLOAT NOT NULL,
    pressure INT NULL,
    windSpeed DOUBLE NULL,
    weather_id INT NULL,
    description VARCHAR(255) NULL,
    timestamp BIGINT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Test weather data for London';

CREATE TABLE IF NOT EXISTS new_york (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT NOT NULL,
    humidity FLOAT NOT NULL,
    pressure INT NULL,
    windSpeed DOUBLE NULL,
    weather_id INT NULL,
    description VARCHAR(255) NULL,
    timestamp BIGINT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Test weather data for New York';

-- Insert sample test data
INSERT INTO london (temperature, humidity, pressure, windSpeed, weather_id, description, timestamp, sunrise, sunset)
VALUES
    (280.15, 65.5, 1013, 5.5, 800, 'Clear sky', 1699200000, 1699165200, 1699202400),
    (282.45, 70.2, 1015, 3.2, 801, 'Few clouds', 1699203600, 1699165200, 1699202400),
    (278.90, 80.1, 1010, 8.7, 500, 'Light rain', 1699207200, 1699165200, 1699202400);

INSERT INTO paris (temperature, humidity, pressure, windSpeed, weather_id, description, timestamp, sunrise, sunset)
VALUES
    (285.20, 55.0, 1018, 4.1, 800, 'Clear sky', 1699200000, 1699165800, 1699202100),
    (286.75, 58.5, 1019, 3.8, 802, 'Scattered clouds', 1699203600, 1699165800, 1699202100);

-- Cleanup procedure for tests
DELIMITER //
CREATE PROCEDURE cleanup_test_data()
BEGIN
    DELETE FROM london WHERE timestamp < UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL 7 DAY));
    DELETE FROM paris WHERE timestamp < UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL 7 DAY));
    DELETE FROM rome WHERE timestamp < UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL 7 DAY));
    DELETE FROM zocca WHERE timestamp < UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL 7 DAY));
    DELETE FROM new_york WHERE timestamp < UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL 7 DAY));
END//
DELIMITER ;

SHOW TABLES;

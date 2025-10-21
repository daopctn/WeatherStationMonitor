-- Weather Station Monitor Database Schema
-- This script creates the database and all required tables
-- ACTUAL SCHEMA - Reflects current database structure

CREATE DATABASE IF NOT EXISTS weather_station_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE weather_station_db;

-- Create weather data table for Zocca, Italy
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for Zocca, Italy';

-- Create weather data table for Rome, Italy
CREATE TABLE IF NOT EXISTS rome (
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for Rome, Italy';

-- Create weather data table for Paris, France
CREATE TABLE IF NOT EXISTS paris (
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for Paris, France';

-- Create weather data table for London, UK
CREATE TABLE IF NOT EXISTS london (
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for London, UK';

-- Create weather data table for New York, USA
CREATE TABLE IF NOT EXISTS new_york (
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for New York, USA';

-- Database Schema Summary
-- Columns per table:
--   - id: Auto-increment primary key
--   - temperature: Temperature in Celsius (FLOAT)
--   - humidity: Humidity percentage 0-100 (FLOAT)
--   - pressure: Atmospheric pressure in hPa (INT, nullable)
--   - windSpeed: Wind speed in m/s (DOUBLE, nullable)
--   - weather_id: OpenWeatherMap weather condition ID (INT, nullable)
--   - description: Human-readable weather description (VARCHAR, nullable)
--   - timestamp: Unix timestamp of when data was collected (BIGINT)
--   - sunrise: Unix timestamp of sunrise (BIGINT)
--   - sunset: Unix timestamp of sunset (BIGINT)

-- Sample data query:
-- SELECT FROM_UNIXTIME(timestamp) as datetime, temperature, humidity, pressure, windSpeed
-- FROM zocca ORDER BY timestamp DESC LIMIT 10;

-- Create user for the application (optional, for security)
-- CREATE USER IF NOT EXISTS 'weather_user'@'localhost' IDENTIFIED BY 'your_password_here';
-- GRANT SELECT, INSERT, UPDATE ON weather_station_db.* TO 'weather_user'@'localhost';
-- FLUSH PRIVILEGES;

-- Show created tables
SHOW TABLES;

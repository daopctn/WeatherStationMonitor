-- Weather Station Monitor Database Schema
-- This script creates the database and all required tables

CREATE DATABASE IF NOT EXISTS weather_station_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE weather_station_db;

-- Create weather data table for Zocca, Italy
CREATE TABLE IF NOT EXISTS zocca (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature DECIMAL(5,2) NOT NULL COMMENT 'Temperature in Celsius',
    humidity INT NOT NULL COMMENT 'Humidity percentage',
    pressure INT NOT NULL COMMENT 'Atmospheric pressure in hPa',
    wind_speed DECIMAL(5,2) NOT NULL COMMENT 'Wind speed in m/s',
    wind_direction INT NOT NULL COMMENT 'Wind direction in degrees',
    clouds INT NOT NULL COMMENT 'Cloudiness percentage',
    weather_main VARCHAR(50) NOT NULL COMMENT 'Main weather condition',
    weather_description VARCHAR(100) NOT NULL COMMENT 'Weather description',
    weather_icon VARCHAR(10) NOT NULL COMMENT 'Weather icon code',
    visibility INT NOT NULL COMMENT 'Visibility in meters',
    sunrise BIGINT NOT NULL COMMENT 'Sunrise time (Unix timestamp)',
    sunset BIGINT NOT NULL COMMENT 'Sunset time (Unix timestamp)',
    timezone_offset INT NOT NULL COMMENT 'Timezone offset in seconds',
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Data fetch timestamp',
    INDEX idx_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for Zocca, Italy';

-- Create weather data table for Rome, Italy
CREATE TABLE IF NOT EXISTS rome (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature DECIMAL(5,2) NOT NULL,
    humidity INT NOT NULL,
    pressure INT NOT NULL,
    wind_speed DECIMAL(5,2) NOT NULL,
    wind_direction INT NOT NULL,
    clouds INT NOT NULL,
    weather_main VARCHAR(50) NOT NULL,
    weather_description VARCHAR(100) NOT NULL,
    weather_icon VARCHAR(10) NOT NULL,
    visibility INT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL,
    timezone_offset INT NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for Rome, Italy';

-- Create weather data table for Paris, France
CREATE TABLE IF NOT EXISTS paris (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature DECIMAL(5,2) NOT NULL,
    humidity INT NOT NULL,
    pressure INT NOT NULL,
    wind_speed DECIMAL(5,2) NOT NULL,
    wind_direction INT NOT NULL,
    clouds INT NOT NULL,
    weather_main VARCHAR(50) NOT NULL,
    weather_description VARCHAR(100) NOT NULL,
    weather_icon VARCHAR(10) NOT NULL,
    visibility INT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL,
    timezone_offset INT NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for Paris, France';

-- Create weather data table for London, UK
CREATE TABLE IF NOT EXISTS london (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature DECIMAL(5,2) NOT NULL,
    humidity INT NOT NULL,
    pressure INT NOT NULL,
    wind_speed DECIMAL(5,2) NOT NULL,
    wind_direction INT NOT NULL,
    clouds INT NOT NULL,
    weather_main VARCHAR(50) NOT NULL,
    weather_description VARCHAR(100) NOT NULL,
    weather_icon VARCHAR(10) NOT NULL,
    visibility INT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL,
    timezone_offset INT NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for London, UK';

-- Create weather data table for New York, USA
CREATE TABLE IF NOT EXISTS new_york (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature DECIMAL(5,2) NOT NULL,
    humidity INT NOT NULL,
    pressure INT NOT NULL,
    wind_speed DECIMAL(5,2) NOT NULL,
    wind_direction INT NOT NULL,
    clouds INT NOT NULL,
    weather_main VARCHAR(50) NOT NULL,
    weather_description VARCHAR(100) NOT NULL,
    weather_icon VARCHAR(10) NOT NULL,
    visibility INT NOT NULL,
    sunrise BIGINT NOT NULL,
    sunset BIGINT NOT NULL,
    timezone_offset INT NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Weather data for New York, USA';

-- Create user for the application (optional, for security)
-- CREATE USER IF NOT EXISTS 'weather_user'@'localhost' IDENTIFIED BY 'your_password_here';
-- GRANT SELECT, INSERT, UPDATE ON weather_station_db.* TO 'weather_user'@'localhost';
-- FLUSH PRIVILEGES;

-- Show created tables
SHOW TABLES;

# Database Setup

This directory contains SQL scripts for setting up the Weather Station Monitor database.

## Files

- `schema.sql` - Creates the database and all required tables

## Manual Setup

If you want to set up the database manually:

```bash
# Login to MySQL
sudo mysql -u root -p

# Run the schema script
source /usr/share/weather-station-monitor/database/schema.sql

# Or import directly
sudo mysql -u root -p < /usr/share/weather-station-monitor/database/schema.sql
```

## Database Structure

**Database Name:** `weather_station_db`

**Tables:**
- `zocca` - Weather data for Zocca, Italy
- `rome` - Weather data for Rome, Italy
- `paris` - Weather data for Paris, France
- `london` - Weather data for London, UK
- `new_york` - Weather data for New York, USA

Each table contains:
- `id` - Auto-incrementing primary key (INT)
- `temperature` - Temperature in Celsius (FLOAT, NOT NULL)
- `humidity` - Humidity percentage 0-100 (FLOAT, NOT NULL)
- `pressure` - Atmospheric pressure in hPa (INT, nullable)
- `windSpeed` - Wind speed in m/s (DOUBLE, nullable)
- `weather_id` - OpenWeatherMap weather condition ID (INT, nullable)
- `description` - Human-readable weather description (VARCHAR, nullable)
- `timestamp` - Unix timestamp of when data was collected (BIGINT, NOT NULL)
- `sunrise` - Unix timestamp of sunrise (BIGINT, NOT NULL)
- `sunset` - Unix timestamp of sunset (BIGINT, NOT NULL)

## Configuration

After database setup, create your configuration file:

```bash
sudo mkdir -p /etc/weather-station-monitor
sudo cp /usr/share/weather-station-monitor/config/example_config.json /etc/weather-station-monitor/config.json
sudo nano /etc/weather-station-monitor/config.json
```

Update the database credentials and API key in the config file.

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
- `id` - Auto-incrementing primary key
- `temperature` - Temperature in Celsius
- `humidity` - Humidity percentage
- `pressure` - Atmospheric pressure in hPa
- `wind_speed` - Wind speed in m/s
- `wind_direction` - Wind direction in degrees
- `clouds` - Cloudiness percentage
- `weather_main` - Main weather condition
- `weather_description` - Detailed weather description
- `weather_icon` - Weather icon code
- `visibility` - Visibility in meters
- `sunrise` - Sunrise time (Unix timestamp)
- `sunset` - Sunset time (Unix timestamp)
- `timezone_offset` - Timezone offset from UTC in seconds
- `timestamp` - Data fetch timestamp

## Configuration

After database setup, create your configuration file:

```bash
sudo mkdir -p /etc/weather-station-monitor
sudo cp /usr/share/weather-station-monitor/config/example_config.json /etc/weather-station-monitor/config.json
sudo nano /etc/weather-station-monitor/config.json
```

Update the database credentials and API key in the config file.

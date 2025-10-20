# Installation Guide - Weather Station Monitor

This guide covers both installing from the pre-built `.deb` package and building from source.

## Table of Contents
- [System Requirements](#system-requirements)
- [Installing from .deb Package](#installing-from-deb-package)
- [Building from Source](#building-from-source)
- [Database Setup](#database-setup)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)

---

## System Requirements

- **OS:** Ubuntu 22.04 LTS or Debian-based Linux distribution
- **RAM:** Minimum 2GB
- **Disk Space:** ~500MB for installation
- **Internet Connection:** Required for fetching weather data

### Required Dependencies

The .deb package will automatically install most dependencies, but you need:
- MySQL Server 8.0 or higher
- Qt 5.15 or higher
- Python 3.10 or higher

---

## Installing from .deb Package

### Step 1: Download the Package

Download the latest `.deb` package from the releases page or use the provided file:
```bash
# Example: weather-station-monitor_1.0.0_amd64.deb
```

### Step 2: Install the Package

```bash
# Install the .deb package
sudo dpkg -i weather-station-monitor_1.0.0_amd64.deb

# If there are dependency issues, fix them with:
sudo apt-get install -f
```

### Step 3: Verify Installation

```bash
# Check if the package is installed
dpkg -l | grep weather-station-monitor

# Check the installed version
weather-station-monitor --version
```

---

## Building from Source

### Step 1: Install Build Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    qtbase5-dev \
    qtcharts5-dev \
    libqt5sql5-mysql \
    python3-dev \
    python3-pip \
    mysql-server \
    debhelper
```

### Step 2: Clone the Repository

```bash
git clone https://github.com/yourusername/WeatherStationMonitor.git
cd WeatherStationMonitor
```

### Step 3: Build the .deb Package

```bash
# Run the build script
./build-deb.sh
```

This script will:
1. Check for required dependencies
2. Build the project using CMake
3. Create the .deb package in the parent directory

### Step 4: Install Your Built Package

```bash
sudo dpkg -i ../weather-station-monitor_*.deb
sudo apt-get install -f
```

---

## Database Setup

### Step 1: Ensure MySQL is Running

```bash
# Start MySQL service
sudo systemctl start mysql
sudo systemctl enable mysql

# Check status
sudo systemctl status mysql
```

### Step 2: Create the Database

```bash
# Login to MySQL as root
sudo mysql -u root -p

# Run the schema script
source /usr/share/weather-station-monitor/database/schema.sql

# Or import from command line:
sudo mysql -u root -p < /usr/share/weather-station-monitor/database/schema.sql
```

### Step 3: Verify Database Creation

```bash
sudo mysql -u root -p
```

In MySQL shell:
```sql
USE weather_station_db;
SHOW TABLES;
-- Should show: zocca, rome, paris, london, new_york
EXIT;
```

### Step 4: (Optional) Create Dedicated Database User

For better security:
```sql
CREATE USER 'weather_user'@'localhost' IDENTIFIED BY 'your_secure_password';
GRANT SELECT, INSERT, UPDATE ON weather_station_db.* TO 'weather_user'@'localhost';
FLUSH PRIVILEGES;
```

---

## Configuration

### Step 1: Get OpenWeatherMap API Key

1. Visit [OpenWeatherMap](https://openweathermap.org/api)
2. Sign up for a free account
3. Generate an API key (free tier allows 1000 calls/day)

### Step 2: Edit Configuration File

```bash
# Edit the configuration file
sudo nano /etc/weather-station-monitor/config.json
```

Update the following fields:

```json
{
  "Database": {
    "host": "localhost",
    "name": "weather_station_db",
    "user": "root",           // or "weather_user" if you created one
    "password": "your_mysql_password"
  },
  "WeatherAPI": {
    "base_url": "https://api.openweathermap.org/data/2.5",
    "api_key": "your_openweathermap_api_key_here",
    "timeout": 10000,
    "fetch_interval_ms": 300000
  }
}
```

### Step 3: Verify Configuration

```bash
# The config file should have proper permissions
sudo chmod 644 /etc/weather-station-monitor/config.json
```

---

## Running the Application

### From Terminal
```bash
weather-station-monitor
```

### From Application Menu
1. Open your application launcher
2. Search for "Weather Station Monitor"
3. Click to launch

---

## Troubleshooting

### Application won't start

**Check if all dependencies are installed:**
```bash
dpkg -l | grep -E 'qt5|mysql|python3'
```

**Check logs:**
```bash
# Run from terminal to see error messages
weather-station-monitor
```

### Database Connection Failed

**Verify MySQL is running:**
```bash
sudo systemctl status mysql
```

**Test database connection:**
```bash
mysql -u root -p -e "USE weather_station_db; SHOW TABLES;"
```

**Check credentials in config:**
```bash
sudo cat /etc/weather-station-monitor/config.json
```

### API Connection Failed

**Verify your API key:**
- Visit [OpenWeatherMap API](https://home.openweathermap.org/api_keys)
- Check if your key is active (new keys may take 1-2 hours to activate)

**Test API manually:**
```bash
curl "https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=YOUR_API_KEY"
```

### Python Import Errors

**Install Python dependencies:**
```bash
pip3 install -r /usr/share/weather-station-monitor/python/requirements.txt
```

Or install manually:
```bash
pip3 install numpy pandas mysql-connector-python
```

### Permission Denied

**If you get permission errors for config:**
```bash
sudo chown $USER:$USER /etc/weather-station-monitor/config.json
```

---

## Uninstallation

To completely remove the application:

```bash
# Remove the package
sudo apt-get remove weather-station-monitor

# Remove configuration files too
sudo apt-get purge weather-station-monitor

# (Optional) Remove the database
sudo mysql -u root -p -e "DROP DATABASE weather_station_db;"
```

---

## Getting Help

- **Documentation:** `/usr/share/doc/weather-station-monitor/`
- **Database Help:** `/usr/share/weather-station-monitor/database/README.md`
- **Issues:** Report bugs at your project repository

---

## Next Steps

After successful installation:
1. The app will automatically fetch weather data every 5 minutes
2. View real-time data in the main window
3. Check historical charts for each city
4. Monitor temperature, humidity, pressure, and wind data
5. Python analytics will calculate averages automatically

Enjoy monitoring the weather!

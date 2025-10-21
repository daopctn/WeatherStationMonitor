# Weather Station Monitor

<div align="center">

**Real-time weather monitoring for multiple cities with beautiful charts and analytics**

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![Qt](https://img.shields.io/badge/Qt-5.15-green)
![License](https://img.shields.io/badge/license-MIT-blue)

</div>

## Overview

Weather Station Monitor is a professional desktop application that provides real-time weather monitoring for multiple cities worldwide. Built with Qt5 and powered by OpenWeatherMap API, it offers beautiful visualizations, historical data tracking, and Python-powered analytics.

### Key Features

- **Multi-City Monitoring** - Track 5 cities simultaneously (Zocca, Rome, Paris, London, New York)
- **Real-Time Updates** - Automatic data refresh every 5 minutes
- **Interactive Charts** - Beautiful temperature and humidity charts with historical data
- **Advanced Statistics** - Comprehensive analytics including trends, patterns, and multi-period comparisons
- **Cross-Location Insights** - Compare weather conditions across all cities in real-time
- **Weather Pattern Analysis** - Distribution analysis of weather conditions with percentages
- **Trend Detection** - Automatic identification of increasing/decreasing/stable trends
- **Comprehensive Data** - Temperature, humidity, pressure, wind speed, gust, visibility, and comfort index
- **Python Analytics** - Advanced statistical calculations including rate of change and comfort analysis
- **Database Persistence** - MySQL backend for reliable data storage with historical tracking
- **Modern UI** - Clean Qt5-based interface with weather icons and three comprehensive tabs
- **Multi-Threaded** - Concurrent API calls for fast data fetching

---

## Screenshots

![Weather Overview Tab](docs/Overview%20tab.png)
![Interactive Charts Tab](docs/Charts%20tab.png)
![Advanced Statistics Tab](docs/statistics%20tab.png)
---

## Installation

### Quick Install (Ubuntu/Debian)

```bash
# Download the .deb package
# Install it
sudo dpkg -i weather-station-monitor_1.0.0_amd64.deb
sudo apt-get install -f

# Set up the database
sudo mysql -u root -p < /usr/share/weather-station-monitor/database/schema.sql

# Configure the application
sudo nano /etc/weather-station-monitor/config.json
# Add your database credentials and OpenWeatherMap API key

# Run the application
weather-station-monitor
```

### Detailed Installation

See [INSTALL.md](docs/INSTALL.md) for comprehensive installation instructions including:
- Building from source
- Database setup
- Configuration details
- Troubleshooting

---

## Quick Start

### 1. Get an API Key

1. Visit [OpenWeatherMap](https://openweathermap.org/api)
2. Create a free account
3. Generate an API key (Free tier: 1000 calls/day)

### 2. Configure the Application

Edit `/etc/weather-station-monitor/config.json` with your database credentials and OpenWeatherMap API key. See the [Configuration](#configuration) section below for full options and examples.

### 3. Launch

From terminal:
```bash
weather-station-monitor
```

Or find it in your application menu under "Science" or search for "Weather Station Monitor"

---

## Usage

### Main Window Tabs

The application features three comprehensive tabs for different views of your weather data:

#### Weather Overview Tab

Real-time weather data for all 5 cities in individual panels:
- **Temperature** (°C) with current readings
- **Humidity** (%) levels
- **Pressure** (hPa) with trend indicators
- **Wind** speed (m/s) and direction
- **Weather conditions** with descriptive icons
- **Sunrise/Sunset** times
- **Visibility** and other meteorological data

#### Interactive Charts Tab

Historical data visualization for each city:
- **Temperature Chart** - Historical temperature data with trend lines
- **Humidity Chart** - Historical humidity data with time-based plotting
- **Real-time Updates** - Charts update automatically as new data arrives
- **Rolling Data Window** - Maintains 100 most recent data points for optimal performance
- **Interactive Navigation** - Zoom and pan through historical data
- **Dual Series Display** - Temperature and humidity plotted simultaneously

#### Advanced Statistics Tab

Comprehensive analytics and insights:
- **Location Statistics** - Detailed stats for each city (min/max/mean values, trends)
- **Weather Patterns** - Distribution analysis of weather conditions with percentages
- **Multi-Period Comparison** - Compare 24-hour vs 7-day averages and trends
- **Trend Detection** - Automatic identification of increasing/decreasing/stable patterns
- **Cross-Location Insights** - Compare conditions across all cities simultaneously
- **Rate of Change** - Calculate how quickly conditions are changing per hour
- **Comfort Index** - Human comfort analysis based on temperature and humidity

### Auto-Refresh System

The application provides real-time data updates, automatically fetching new weather data every 5 minutes in the background. All fetched data is stored in the MySQL database for historical tracking and advanced analytics. The last update time is displayed in the status bar.

---

## Architecture

### Technology Stack

- **Frontend:** Qt 5.15 (C++ with Qt Charts for visualization)
- **Backend:** MySQL 8.0 (persistent data storage)
- **Analytics:** Python 3.10 with NumPy, Pandas (advanced statistical analysis)
- **API:** OpenWeatherMap REST API (real-time weather data)
- **Build System:** CMake (cross-platform compilation)
- **Packaging:** Debian (.deb package format)

### Components

```
┌─────────────────────────────────────────────────┐
│           Qt5 GUI (MainWindow)                  │
│   ┌─────────────────────────────────────────┐   │
│   │ Weather Overview │ Charts │ Statistics │   │
│   └─────────────────────────────────────────┘   │
├─────────────────────────────────────────────────┤
│        ThreadManager (Multi-threaded API)      │
├─────────────────────────────────────────────────┤
│ DatabaseManager │ PythonBridge │ StatisticsEngine │
├─────────────────────────────────────────────────┤
│     MySQL DB    │  Python Analytics (NumPy/Pandas) │
└─────────────────────────────────────────────────┘
```

**Key Components:**
- **Qt5 GUI**: Three-tab interface with real-time updates and visual loading indicators
- **ThreadManager**: Advanced threading with 5 concurrent WeatherWorkers + 1 DatabaseThread
- **DatabaseManager**: MySQL connection and data persistence with thread-safe operations
- **PythonBridge**: Seamless C++ to Python integration for advanced analytics
- **StatisticsEngine**: Comprehensive calculations (trends, patterns, rate of change, comfort index)
- **Spinner System**: Visual feedback during data operations and API calls

---

## Advanced Analytics

Weather Station Monitor includes comprehensive Python-powered analytics for in-depth weather analysis.

### Statistical Calculations

- **Basic Statistics**: Min, max, mean, and standard deviation for all parameters
- **Trend Analysis**: Linear regression to detect increasing/decreasing/stable trends
- **Rate of Change**: Hourly rate calculations for temperature and pressure changes
- **Comfort Index**: Human comfort analysis based on temperature-humidity combinations

### Weather Pattern Analysis

- **Condition Distribution**: Percentage breakdown of weather conditions (clear, cloudy, rain, etc.)
- **Pattern Recognition**: Most common weather patterns for each location
- **Historical Trends**: Long-term weather pattern analysis

### Multi-Period Comparisons

- **24-Hour vs 7-Day Analysis**: Compare recent vs longer-term averages
- **Trend Summaries**: Human-readable summaries of temperature and pressure changes
- **Weather Stability**: Assess how consistent conditions have been over time

### Cross-Location Insights

- **Comparative Extremes**: Identify warmest/coldest cities, most/least humid locations
- **Pressure Ranges**: Analyze atmospheric pressure variations across cities
- **Temperature Spread**: Calculate temperature differences between locations

### Data Processing Features

- **Pandas Integration**: Time-series analysis and data manipulation
- **NumPy Calculations**: Efficient numerical computations for large datasets
- **Real-time Processing**: Analytics update automatically with new data
- **Error Handling**: Robust processing with graceful error recovery

### Advanced Technical Features

#### Multi-Threaded Architecture
- **5 Concurrent Weather Workers**: Independent threads for each city to prevent blocking
- **Dedicated Database Thread**: Separate thread for all database operations
- **Thread-Safe Data Sharing**: Mutex-protected data vectors between threads
- **Real-time UI Updates**: Signal/slot mechanism for live interface updates

#### Dynamic Weather Icon System
- **Day/Night Awareness**: Icons change based on sunrise/sunset times for each location
- **Weather Condition Mapping**: 15+ weather conditions with appropriate visual indicators
- **Time Zone Support**: Proper local time calculations for all 5 cities
- **Resource Management**: Efficient icon loading and caching

#### Visual Feedback System
- **Loading Spinner**: Animated indicator during API calls and data processing
- **Pause/Resume Capability**: Smart spinner management during UI operations
- **Non-blocking UI**: Background operations don't freeze the interface

#### Configuration System
- **Environment Variables**: Secure credential management via environment variables
- **JSON Configuration**: Flexible config file with fallback defaults
- **Runtime Configuration**: Settings loaded at startup with validation

---

## Configuration

### Config File Location

- **System-wide:** `/etc/weather-station-monitor/config.json`
- **Example:** `/usr/share/weather-station-monitor/config/example_config.json`

### Configuration Options

```json
{
  "Database": {
    "host": "localhost",        // MySQL host
    "name": "weather_station_db", // Database name
    "user": "your_user",         // MySQL username
    "password": "your_password"  // MySQL password
  },
  "WeatherAPI": {
    "base_url": "https://api.openweathermap.org/data/2.5",
    "api_key": "your_key",       // OpenWeatherMap API key
    "timeout": 10000,            // Request timeout (ms)
    "fetch_interval_ms": 300000, // Auto-refresh interval (5 min)
    "locations": {
      // Predefined cities with coordinates
    }
  }
}
```

---

## Development

### Building from Source

```bash
# Install dependencies
sudo apt-get install qtbase5-dev qtcharts5-dev python3-dev cmake

# Clone and build
git clone https://github.com/daopctn/WeatherStationMonitor.git
cd WeatherStationMonitor
./build-deb.sh
```

### Project Structure

```
WeatherStationMonitor/
├── src/              # C++ source files
├── include/          # Header files
├── ui/               # Qt Designer UI files
├── python/           # Python analytics scripts
├── resources/        # Icons and resources
├── database/         # SQL schema files
├── debian/           # Debian packaging files
└── docs/             # Documentation
```

### Dependencies

See [python/requirements.txt](python/requirements.txt) for Python dependencies.

---

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

---

## Troubleshooting

### Common Issues

**Database connection fails:**
- Check MySQL is running: `sudo systemctl status mysql`
- Verify credentials in config file
- Ensure database exists: `sudo mysql -e "SHOW DATABASES;"`

**API calls fail:**
- Verify API key is active (may take 1-2 hours for new keys)
- Check internet connection
- Test API manually with curl

**Application won't start:**
- Check dependencies: `dpkg -l | grep weather-station-monitor`
- Run from terminal to see error messages
- Check Qt installation

See [INSTALL.md](docs/INSTALL.md) for detailed troubleshooting.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- Weather data provided by [OpenWeatherMap](https://openweathermap.org/)
- Weather icons from OpenWeatherMap
- Built with [Qt Framework](https://www.qt.io/)

---

## Support

- **Documentation:** [INSTALL.md](docs/INSTALL.md)
- **Issues:** [GitHub Issues](https://github.com/daopctn/WeatherStationMonitor/issues)
- **Email:** daopctn@gmail.com

---

<div align="center">

Made with ❤️ using Qt, Python, and MySQL

</div>

# Quick Start Guide - Weather Station Monitor

Get up and running in 5 minutes!

## Prerequisites

- Ubuntu 22.04 or Debian-based Linux
- MySQL Server installed
- Internet connection

## 1. Build the Package (1 minute)

```bash
# Make the build script executable (first time only)
chmod +x build-deb.sh

# Build the .deb package
./build-deb.sh
```

The script will:
- ✅ Check dependencies
- ✅ Install missing tools
- ✅ Build the package
- ✅ Show you the output location

## 2. Install the Package (30 seconds)

```bash
# Install the .deb package (it will be in the parent directory)
sudo dpkg -i ../weather-station-monitor_*.deb

# Fix any dependency issues (if needed)
sudo apt-get install -f
```

## 3. Set Up MySQL Database (1 minute)

```bash
# Ensure MySQL is running
sudo systemctl start mysql

# Create the database
sudo mysql -u root -p < /usr/share/weather-station-monitor/database/schema.sql
```

Enter your MySQL root password when prompted.

## 4. Get API Key (2 minutes)

1. Go to [OpenWeatherMap](https://openweathermap.org/api)
2. Click "Sign Up" (free account)
3. After logging in, go to "API keys" tab
4. Copy your API key (or generate a new one)

**Note:** New API keys may take 1-2 hours to activate.

## 5. Configure the App (30 seconds)

```bash
# Edit the configuration file
sudo nano /etc/weather-station-monitor/config.json
```

Update these two sections:

```json
{
  "Database": {
    "password": "your_mysql_password"  ← Change this
  },
  "WeatherAPI": {
    "api_key": "your_api_key_here"     ← Change this
  }
}
```

Save with `Ctrl+O`, then exit with `Ctrl+X`.

## 6. Run the App!

```bash
weather-station-monitor
```

Or find it in your applications menu: **Science → Weather Station Monitor**

## What You Should See

After launching:
1. **Three main tabs**: Weather Overview, Interactive Charts, and Advanced Statistics
2. **Weather Overview tab**: 5 city panels showing current weather data for Zocca, Rome, Paris, London, and New York
3. **Interactive Charts tab**: Temperature and humidity graphs for each city with historical data
4. **Advanced Statistics tab**: Comprehensive analytics including trends, patterns, and cross-location comparisons
5. **Real-time updates** - Data refreshes every 5 minutes automatically
6. **Weather icons** - Visual indicators for current conditions

## First Run Checklist

- [ ] Application window opens without errors
- [ ] Loading spinner appears during initial data fetch
- [ ] You see weather data for all 5 cities in the Overview tab
- [ ] Weather icons display correctly (day/night aware)
- [ ] Charts show historical data in the Interactive Charts tab
- [ ] Advanced Statistics tab loads with trend analysis
- [ ] Status bar shows "Last updated: [time]"
- [ ] No error messages in terminal output

## Troubleshooting

### "Can't connect to MySQL database"
```bash
# Check if MySQL is running
sudo systemctl status mysql

# If not running, start it
sudo systemctl start mysql
```

### "API Key Invalid"
- Wait 1-2 hours if you just created the key
- Double-check the key in config.json
- Test manually:
  ```bash
  curl "https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=YOUR_KEY"
  ```

### "Python import errors"
```bash
pip3 install numpy pandas mysql-connector-python
```

### Application won't start
```bash
# Run from terminal to see error messages
weather-station-monitor

# Check if installed correctly
dpkg -l | grep weather-station-monitor
```

## Next Steps

- **Add more cities:** Edit the database and configuration
- **View historical data:** Check the MySQL database tables
- **Customize refresh rate:** Edit `fetch_interval_ms` in config.json
- **Export data:** Use MySQL queries to export CSV files

## Common Commands

```bash
# Start the app
weather-station-monitor

# Check database
sudo mysql -u root -p weather_station_db

# View logs
journalctl -u mysql

# Reinstall if needed
sudo apt-get reinstall weather-station-monitor
```

## File Locations

- **Config:** `/etc/weather-station-monitor/config.json`
- **Database scripts:** `/usr/share/weather-station-monitor/database/`
- **Docs:** `/usr/share/doc/weather-station-monitor/`
- **Executable:** `/usr/bin/weather-station-monitor`

## Getting Help

📖 **Full Documentation:** [INSTALL.md](INSTALL.md)
🔧 **Packaging Guide:** [PACKAGING.md](PACKAGING.md)
💾 **Database Help:** `/usr/share/weather-station-monitor/database/README.md`

---

**Enjoy monitoring the weather! 🌤️**

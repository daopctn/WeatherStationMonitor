import numpy as np
import pandas as pd
import mysql.connector
from mysql.connector import Error
from datetime import datetime, timedelta
import json


def calculate_both_averages_from_db(host, database, username, password, table):
    """Calculate both temperature and humidity averages with single connection
    LEGACY FUNCTION - Kept for backwards compatibility"""
    connection = None
    cursor = None
    try:
        connection = mysql.connector.connect(
            host=host,
            database=database,
            user=username,
            password=password
        )

        if connection.is_connected():
            cursor = connection.cursor()

            # Get both temperature and humidity in single query
            cursor.execute(f"SELECT temperature, humidity FROM `{table}`")
            data = cursor.fetchall()

            avg_temp = 0.0
            avg_humidity = 0.0

            if data:
                temp_values = [row[0] for row in data if row[0] is not None]
                humidity_values = [row[1] for row in data if row[1] is not None]

                if temp_values:
                    avg_temp = sum(temp_values) / len(temp_values)
                    avg_temp = float(round(avg_temp, 2))

                if humidity_values:
                    avg_humidity = sum(humidity_values) / len(humidity_values)
                    avg_humidity = float(round(avg_humidity, 2))

                print(f"{table} - Avg: Temp {avg_temp:.2f}°C ({len(temp_values)}), Humidity {avg_humidity:.2f}% ({len(humidity_values)})")
            else:
                print("No data found in database")

            return avg_temp, avg_humidity

    except Error as e:
        print(f"Database error: {e}")
        return 0.0, 0.0
    except Exception as e:
        print(f"Error calculating averages: {e}")
        return 0.0, 0.0
    finally:
        if cursor:
            cursor.close()
        if connection and connection.is_connected():
            connection.close()


def calculate_location_statistics(host, database, username, password, table, hours=24):
    """
    Calculate comprehensive statistics for a single location over a time period.

    Args:
        host: Database host
        database: Database name
        username: Database username
        password: Database password
        table: Table name (location)
        hours: Time period in hours (default 24)

    Returns:
        JSON string with statistics
    """
    connection = None
    cursor = None

    try:
        connection = mysql.connector.connect(
            host=host,
            database=database,
            user=username,
            password=password
        )

        if not connection.is_connected():
            return json.dumps({"error": "Failed to connect to database"})

        cursor = connection.cursor()

        # Calculate time threshold
        time_threshold = datetime.now() - timedelta(hours=hours)
        time_str = time_threshold.strftime('%Y-%m-%d %H:%M:%S')

        # Query all data within time period
        # Note: Using actual database columns (windSpeed not wind_speed)
        query = f"""
            SELECT temperature, humidity, pressure, windSpeed, description, timestamp
            FROM `{table}`
            WHERE FROM_UNIXTIME(timestamp) >= %s
            ORDER BY timestamp ASC
        """
        cursor.execute(query, (time_str,))
        data = cursor.fetchall()

        if not data:
            return json.dumps({
                "location": table,
                "period_hours": hours,
                "record_count": 0,
                "error": "No data found in the specified time period"
            })

        # Convert to pandas DataFrame for easier analysis
        df = pd.DataFrame(data, columns=[
            'temperature', 'humidity', 'pressure', 'windSpeed', 'description', 'timestamp'
        ])

        # Convert Unix timestamp to datetime
        df['datetime'] = pd.to_datetime(df['timestamp'], unit='s')

        # Calculate rate of change per hour
        temp_rate = _calculate_rate_of_change(df, 'temperature')
        pressure_rate = _calculate_rate_of_change(df, 'pressure')

        # Analyze weather conditions
        weather_analysis = _analyze_weather_conditions(df)

        # Calculate statistics
        stats = {
            "location": table.upper(),
            "period_hours": hours,
            "record_count": len(df),
            "time_range": {
                "start": str(df['datetime'].min()),
                "end": str(df['datetime'].max())
            },
            "temperature": {
                "current": float(df['temperature'].iloc[-1]) if not df['temperature'].empty else None,
                "mean": float(df['temperature'].mean()),
                "min": float(df['temperature'].min()),
                "max": float(df['temperature'].max()),
                "std_dev": float(df['temperature'].std()),
                "trend": _calculate_trend(df['temperature']),
                "rate_per_hour": temp_rate  # NEW: Rate of change
            },
            "humidity": {
                "current": float(df['humidity'].iloc[-1]) if not df['humidity'].empty else None,
                "mean": float(df['humidity'].mean()),
                "min": float(df['humidity'].min()),
                "max": float(df['humidity'].max()),
                "std_dev": float(df['humidity'].std())
            },
            "pressure": {
                "current": float(df['pressure'].iloc[-1]) if not df['pressure'].empty and pd.notna(df['pressure'].iloc[-1]) else None,
                "mean": float(df['pressure'].mean()) if not df['pressure'].isna().all() else None,
                "min": float(df['pressure'].min()) if not df['pressure'].isna().all() else None,
                "max": float(df['pressure'].max()) if not df['pressure'].isna().all() else None,
                "trend": _calculate_trend(df['pressure'].dropna()) if not df['pressure'].isna().all() else "N/A",
                "rate_per_hour": pressure_rate  # NEW: Rate of change
            },
            "wind": {
                "current_speed": float(df['windSpeed'].iloc[-1]) if not df['windSpeed'].empty and pd.notna(df['windSpeed'].iloc[-1]) else None,
                "mean_speed": float(df['windSpeed'].mean()) if not df['windSpeed'].isna().all() else None,
                "max_speed": float(df['windSpeed'].max()) if not df['windSpeed'].isna().all() else None
            },
            "weather_conditions": weather_analysis  # NEW: Weather condition distribution
        }

        # Add comfort index (simple heat index calculation)
        if stats['temperature']['current'] and stats['humidity']['current']:
            stats['comfort_index'] = _calculate_comfort_index(
                stats['temperature']['current'],
                stats['humidity']['current']
            )

        return json.dumps(stats, indent=2)

    except Error as e:
        return json.dumps({"error": f"Database error: {str(e)}"})
    except Exception as e:
        return json.dumps({"error": f"Error calculating statistics: {str(e)}"})
    finally:
        if cursor:
            cursor.close()
        if connection and connection.is_connected():
            connection.close()


def compare_all_locations(host, database, username, password, hours=24):
    """
    Compare statistics across all 5 locations.

    Returns:
        JSON string with comparative statistics
    """
    connection = None
    cursor = None

    locations = ['zocca', 'rome', 'paris', 'london', 'new_york']

    try:
        connection = mysql.connector.connect(
            host=host,
            database=database,
            user=username,
            password=password
        )

        if not connection.is_connected():
            return json.dumps({"error": "Failed to connect to database"})

        cursor = connection.cursor()

        time_threshold = datetime.now() - timedelta(hours=hours)
        time_str = time_threshold.strftime('%Y-%m-%d %H:%M:%S')

        location_data = {}

        for location in locations:
            query = f"""
                SELECT temperature, humidity, pressure, timestamp
                FROM `{location}`
                WHERE timestamp >= %s
                ORDER BY timestamp DESC
                LIMIT 1
            """
            cursor.execute(query, (time_str,))
            result = cursor.fetchone()

            if result:
                location_data[location] = {
                    "temperature": float(result[0]),
                    "humidity": float(result[1]),
                    "pressure": float(result[2]),
                    "timestamp": str(result[3])
                }

        if not location_data:
            return json.dumps({"error": "No recent data found for any location"})

        # Find extremes
        temps = {loc: data['temperature'] for loc, data in location_data.items()}
        humids = {loc: data['humidity'] for loc, data in location_data.items()}
        pressures = {loc: data['pressure'] for loc, data in location_data.items()}

        warmest = max(temps, key=temps.get)
        coldest = min(temps, key=temps.get)
        most_humid = max(humids, key=humids.get)
        least_humid = min(humids, key=humids.get)
        highest_pressure = max(pressures, key=pressures.get)
        lowest_pressure = min(pressures, key=pressures.get)

        comparison = {
            "period_hours": hours,
            "locations_compared": len(location_data),
            "temperature": {
                "warmest": {
                    "location": warmest.upper(),
                    "value": temps[warmest]
                },
                "coldest": {
                    "location": coldest.upper(),
                    "value": temps[coldest]
                },
                "difference": temps[warmest] - temps[coldest],
                "mean": float(np.mean(list(temps.values())))
            },
            "humidity": {
                "most_humid": {
                    "location": most_humid.upper(),
                    "value": humids[most_humid]
                },
                "least_humid": {
                    "location": least_humid.upper(),
                    "value": humids[least_humid]
                },
                "mean": float(np.mean(list(humids.values())))
            },
            "pressure": {
                "highest": {
                    "location": highest_pressure.upper(),
                    "value": pressures[highest_pressure]
                },
                "lowest": {
                    "location": lowest_pressure.upper(),
                    "value": pressures[lowest_pressure]
                },
                "mean": float(np.mean(list(pressures.values())))
            },
            "all_locations": {
                loc.upper(): data for loc, data in location_data.items()
            }
        }

        return json.dumps(comparison, indent=2)

    except Error as e:
        return json.dumps({"error": f"Database error: {str(e)}"})
    except Exception as e:
        return json.dumps({"error": f"Error comparing locations: {str(e)}"})
    finally:
        if cursor:
            cursor.close()
        if connection and connection.is_connected():
            connection.close()


def compare_periods(host, database, username, password, table):
    """
    Compare weather statistics across different time periods.
    Compares 24 hours vs 7 days to show trends.

    Returns:
        JSON string with period comparison
    """
    connection = None
    cursor = None

    try:
        connection = mysql.connector.connect(
            host=host,
            database=database,
            user=username,
            password=password
        )

        if not connection.is_connected():
            return json.dumps({"error": "Failed to connect to database"})

        cursor = connection.cursor()

        # Get data for last 24 hours
        time_24h = (datetime.now() - timedelta(hours=24)).strftime('%Y-%m-%d %H:%M:%S')
        query_24h = f"""
            SELECT AVG(temperature) as avg_temp, AVG(humidity) as avg_hum,
                   AVG(pressure) as avg_pressure, COUNT(*) as count
            FROM `{table}`
            WHERE FROM_UNIXTIME(timestamp) >= %s
        """
        cursor.execute(query_24h, (time_24h,))
        result_24h = cursor.fetchone()

        # Get data for last 7 days
        time_7d = (datetime.now() - timedelta(days=7)).strftime('%Y-%m-%d %H:%M:%S')
        query_7d = f"""
            SELECT AVG(temperature) as avg_temp, AVG(humidity) as avg_hum,
                   AVG(pressure) as avg_pressure, COUNT(*) as count
            FROM `{table}`
            WHERE FROM_UNIXTIME(timestamp) >= %s
        """
        cursor.execute(query_7d, (time_7d,))
        result_7d = cursor.fetchone()

        if not result_24h or not result_7d:
            return json.dumps({"error": "Insufficient data for comparison"})

        # Calculate differences
        temp_diff = float(result_24h[0] - result_7d[0]) if result_24h[0] and result_7d[0] else None
        hum_diff = float(result_24h[1] - result_7d[1]) if result_24h[1] and result_7d[1] else None
        pressure_diff = float(result_24h[2] - result_7d[2]) if result_24h[2] and result_7d[2] else None

        comparison = {
            "location": table.upper(),
            "last_24_hours": {
                "avg_temperature": float(result_24h[0]) if result_24h[0] else None,
                "avg_humidity": float(result_24h[1]) if result_24h[1] else None,
                "avg_pressure": float(result_24h[2]) if result_24h[2] else None,
                "record_count": int(result_24h[3])
            },
            "last_7_days": {
                "avg_temperature": float(result_7d[0]) if result_7d[0] else None,
                "avg_humidity": float(result_7d[1]) if result_7d[1] else None,
                "avg_pressure": float(result_7d[2]) if result_7d[2] else None,
                "record_count": int(result_7d[3])
            },
            "comparison": {
                "temperature_diff": temp_diff,
                "humidity_diff": hum_diff,
                "pressure_diff": pressure_diff,
                "trend_summary": _get_trend_summary(temp_diff, pressure_diff)
            }
        }

        return json.dumps(comparison, indent=2)

    except Error as e:
        return json.dumps({"error": f"Database error: {str(e)}"})
    except Exception as e:
        return json.dumps({"error": f"Error comparing periods: {str(e)}"})
    finally:
        if cursor:
            cursor.close()
        if connection and connection.is_connected():
            connection.close()


def generate_all_statistics_json(host, database, username, password, hours=24):
    """
    Generate comprehensive statistics for all locations in JSON format.
    This is optimized for C++ UI consumption.

    Args:
        host: Database host
        database: Database name
        username: Database username
        password: Database password
        hours: Time period in hours (default 24)

    Returns:
        JSON string with statistics for all 5 locations
    """
    locations = ['zocca', 'rome', 'paris', 'london', 'new_york']
    result = {
        "generated_at": datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        "period_hours": hours,
        "locations": []
    }

    for location in locations:
        try:
            # Get statistics for this location
            stats_json = calculate_location_statistics(host, database, username, password, location, hours)
            stats = json.loads(stats_json)

            # Only add if no error
            if 'error' not in stats:
                result["locations"].append(stats)
            else:
                # Add error entry
                result["locations"].append({
                    "location": location.upper(),
                    "error": stats['error']
                })
        except Exception as e:
            result["locations"].append({
                "location": location.upper(),
                "error": str(e)
            })

    return json.dumps(result, indent=2)


# Helper functions

def _calculate_trend(series):
    """Calculate trend direction using linear regression"""
    if len(series) < 2:
        return "insufficient data"

    x = np.arange(len(series))
    y = series.values

    # Remove NaN values
    mask = ~np.isnan(y)
    if mask.sum() < 2:
        return "insufficient data"

    x = x[mask]
    y = y[mask]

    # Linear regression
    slope = np.polyfit(x, y, 1)[0]

    if abs(slope) < 0.01:
        return "stable"
    elif slope > 0:
        return f"increasing ({slope:.3f})"
    else:
        return f"decreasing ({slope:.3f})"


def _calculate_rate_of_change(df, column, time_column='timestamp'):
    """
    Calculate rate of change per hour for a given metric.

    Args:
        df: DataFrame with time series data
        column: Column name to calculate rate for
        time_column: Timestamp column name

    Returns:
        Rate of change per hour (float) or None
    """
    if len(df) < 2 or df[column].isna().all():
        return None

    # Get first and last valid readings
    valid_data = df[[time_column, column]].dropna()
    if len(valid_data) < 2:
        return None

    first_time = valid_data.iloc[0][time_column]
    last_time = valid_data.iloc[-1][time_column]
    first_value = valid_data.iloc[0][column]
    last_value = valid_data.iloc[-1][column]

    # Calculate time difference in hours
    time_diff_seconds = last_time - first_time
    time_diff_hours = time_diff_seconds / 3600.0

    if time_diff_hours == 0:
        return None

    # Calculate rate per hour
    rate = (last_value - first_value) / time_diff_hours
    return float(rate)


def _analyze_weather_conditions(df):
    """
    Analyze weather condition distribution from description field.

    Args:
        df: DataFrame with 'description' column

    Returns:
        Dictionary with weather statistics
    """
    if 'description' not in df.columns or df['description'].isna().all():
        return {
            "total_records": 0,
            "conditions": {},
            "most_common": "N/A"
        }

    # Get non-null descriptions
    descriptions = df['description'].dropna()

    if len(descriptions) == 0:
        return {
            "total_records": 0,
            "conditions": {},
            "most_common": "N/A"
        }

    # Count occurrences
    condition_counts = descriptions.value_counts()
    total = len(descriptions)

    # Calculate percentages
    conditions = {}
    for condition, count in condition_counts.items():
        conditions[condition] = {
            "count": int(count),
            "percentage": float(count / total * 100)
        }

    most_common = condition_counts.index[0] if len(condition_counts) > 0 else "N/A"

    return {
        "total_records": total,
        "conditions": conditions,
        "most_common": most_common
    }


def _get_trend_summary(temp_diff, pressure_diff):
    """
    Generate human-readable trend summary from period comparison.

    Args:
        temp_diff: Temperature difference (24h avg - 7d avg)
        pressure_diff: Pressure difference (24h avg - 7d avg)

    Returns:
        String describing the trend
    """
    if temp_diff is None or pressure_diff is None:
        return "Insufficient data"

    summary = []

    # Temperature trend
    if abs(temp_diff) < 1.0:
        summary.append("Temperature: stable")
    elif temp_diff > 0:
        summary.append(f"Temperature: Warming trend (+{temp_diff:.1f}°C vs 7-day avg)")
    else:
        summary.append(f"Temperature: Cooling trend ({temp_diff:.1f}°C vs 7-day avg)")

    # Pressure trend (important for weather prediction)
    if abs(pressure_diff) < 2.0:
        summary.append("Pressure: stable")
    elif pressure_diff > 0:
        summary.append(f"Pressure: rising (+{pressure_diff:.1f} hPa, improving weather)")
    else:
        summary.append(f"Pressure: falling ({pressure_diff:.1f} hPa, storm possible)")

    return ", ".join(summary)


def _calculate_comfort_index(temp, humidity):
    """
    Calculate comfort index based on temperature and humidity.
    Simple heat index approximation.
    """
    if temp < 10:
        return "Cold"
    elif temp < 18:
        return "Cool"
    elif temp <= 24:
        if humidity < 60:
            return "Comfortable"
        else:
            return "Slightly Humid"
    elif temp <= 28:
        if humidity < 60:
            return "Warm"
        else:
            return "Humid and Warm"
    else:
        if humidity < 60:
            return "Hot"
        else:
            return "Hot and Humid"


# Main entry point for testing
if __name__ == "__main__":
    # Test with default database configuration
    json_result = generate_all_statistics_json(
        host="localhost",
        database="weather_station_db",
        username="daopctn",
        password="dao02112003"
    )
    print(json_result)

import numpy as np
import pandas as pd
import mysql.connector
from mysql.connector import Error

def kelvin_to_celsius(kelvin_temp):
    """Convert temperature from Kelvin to Celsius"""
    celsius = kelvin_temp - 273.15
    return round(celsius, 2)


def process_api_temperature(kelvin_temperature):
    """Process temperature from weather API (convert Kelvin to Celsius)"""
    celsius_temp = kelvin_to_celsius(kelvin_temperature)
    print(f"Converted {kelvin_temperature}K to {celsius_temp}°C")
    return celsius_temp

def calculate_average_from_db(host, database, username, password, table):
    """Calculate average temperature from database"""
    connection = None
    cursor = None
    try:
        # Optionally: validate table name here!
        connection = mysql.connector.connect(
            host=host,
            database=database,
            user=username,
            password=password
        )

        if connection.is_connected():
            cursor = connection.cursor()
            cursor.execute(f"SELECT temperature FROM `{table}`")  # Use backticks for table name
            temperatures = cursor.fetchall()

            if temperatures:
                temp_values = [temp[0] for temp in temperatures]
                average = sum(temp_values) / len(temp_values)
                print(f"Calculated average temperature: {average:.2f}°C from {len(temp_values)} records")
                return float(round(average, 2))
            else:
                print("No temperature data found in database")
                return 0.0

    except Error as e:
        print(f"Database error: {e}")
        return 0.0
    except Exception as e:
        print(f"Error calculating average: {e}")
        return 0.0
    finally:
        if cursor:
            cursor.close()
        if connection and connection.is_connected():
            connection.close()

def calculate_average_humidity_from_db(host, database, username, password, table):
    """Calculate average humidity from database"""
    connection = None
    cursor = None
    try:
        # Optionally: validate table name here!
        connection = mysql.connector.connect(
            host=host,
            database=database,
            user=username,
            password=password
        )

        if connection.is_connected():
            cursor = connection.cursor()
            cursor.execute(f"SELECT humidity FROM `{table}`")  # Use backticks for table name
            humidities = cursor.fetchall()

            if humidities:
                humidity_values = [hum[0] for hum in humidities]
                average = sum(humidity_values) / len(humidity_values)
                print(f"Calculated average humidity: {average:.2f}% from {len(humidity_values)} records")
                return float(round(average, 2))
            else:
                print("No humidity data found in database")
                return 0.0

    except Error as e:
        print(f"Database error: {e}")
        return 0.0
    except Exception as e:
        print(f"Error calculating average: {e}")
        return 0.0
    finally:
        if cursor:
            cursor.close()
        if connection and connection.is_connected():
            connection.close()

def calculate_both_averages_from_db(host, database, username, password, table):
    """Calculate both temperature and humidity averages with single connection"""
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
                    print(f"Calculated average temperature: {avg_temp:.2f}°C from {len(temp_values)} records")

                if humidity_values:
                    avg_humidity = sum(humidity_values) / len(humidity_values)
                    avg_humidity = float(round(avg_humidity, 2))
                    print(f"Calculated average humidity: {avg_humidity:.2f}% from {len(humidity_values)} records")
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

def hello():
    """Simple test function"""
    print("Hello from Python processor!")
    return "Success"

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

def calculate_average_from_db(host, database, username, password):
    """Calculate average temperature from database"""
    try:
        connection = mysql.connector.connect(
            host=host,
            database=database,
            user=username,
            password=password
        )

        if connection.is_connected():
            cursor = connection.cursor()
            cursor.execute("SELECT value FROM test_location")
            temperatures = cursor.fetchall()

            if temperatures:
                temp_values = [temp[0] for temp in temperatures]
                average = sum(temp_values) / len(temp_values)
                print(f"Calculated average temperature: {average:.2f}°C from {len(temp_values)} records")
                return round(average, 2)
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
        if connection.is_connected():
            cursor.close()
            connection.close()

def hello():
    """Simple test function"""
    print("Hello from Python processor!")
    return "Success"

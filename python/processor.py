import numpy as np
import pandas as pd
import mysql.connector
from mysql.connector import Error

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

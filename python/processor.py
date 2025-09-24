import numpy as np
import pandas as pd

def kelvin_to_celsius(kelvin_temp):
    """Convert temperature from Kelvin to Celsius"""
    celsius = kelvin_temp - 273.15
    return round(celsius, 2)

def process_weather_data(temperature, humidity):
    """Process weather data and return a simple analysis"""
    return {
        'temperature_celsius': temperature,
        'humidity_percent': humidity,
        'heat_index': temperature + (humidity * 0.1),
        'status': 'normal' if temperature < 30 and humidity < 80 else 'high'
    }

def process_api_temperature(kelvin_temperature):
    """Process temperature from weather API (convert Kelvin to Celsius)"""
    celsius_temp = kelvin_to_celsius(kelvin_temperature)
    print(f"Converted {kelvin_temperature}K to {celsius_temp}°C")
    return celsius_temp

def hello():
    """Simple test function"""
    print("Hello from Python processor!")
    return "Success"
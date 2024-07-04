#!/usr/bin/env python
import bme680
import time

try:
    sensor = bme680.BME680(bme680.I2C_ADDR_PRIMARY)
except (RuntimeError, IOError):
    sensor = bme680.BME680(bme680.I2C_ADDR_SECONDARY)

sensor.set_humidity_oversample(bme680.OS_2X)
sensor.set_pressure_oversample(bme680.OS_4X)
sensor.set_temperature_oversample(bme680.OS_8X)
sensor.set_filter(bme680.FILTER_SIZE_3)

sensor.set_gas_status(bme680.ENABLE_GAS_MEAS)
sensor.set_gas_heater_temperature(320)
sensor.set_gas_heater_duration(150)
sensor.select_gas_heater_profile(0)

hPa = 0.7501

def calculate_iaq(gas_resistance, humidity):
    if gas_resistance > 0 and humidity > 0:
        humidity_score = 100 - humidity
        gas_score = (gas_resistance / 1000) * 100
        iaq = (humidity_score + gas_score) / 2
    else:
        iaq = None
    return iaq

while True:
    if sensor.get_sensor_data() and sensor.data.heat_stable:
        temperature = sensor.data.temperature
        pressure = sensor.data.pressure * hPa
        humidity = sensor.data.humidity
        gas_resistance = sensor.data.gas_resistance
        iaq = calculate_iaq(gas_resistance, humidity)

        print(f'Temperature: {temperature:.2f} C, Pressure: {pressure:.2f} mmHg, Humidity: {humidity:.2f} %, Gas Resistance: {gas_resistance:.2f} Ohms, IAQ: {iaq:.2f}')
    time.sleep(60)


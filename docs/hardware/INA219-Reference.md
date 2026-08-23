# INA219 Zero-Drift, Bi-Directional Current/Power Monitor Reference

## Overview
The INA219 is an I2C-output current shunt and power monitor. In the Plant Watering System, it is wired in series on the 12V pump rail to detect dry-run conditions (when operating current drops below threshold due to pump running without water).

## Pinout & Connections
| INA219 Pin | ESP32 / System Connection | Description |
| :--- | :--- | :--- |
| **VCC** | 3.3V Regulated | Power Supply (3.3V logic compatible) |
| **GND** | System Ground | Common Ground |
| **SDA** | GPIO 21 | I2C Serial Data |
| **SCL** | GPIO 22 | I2C Serial Clock |
| **VIN+** | 12V Supply Side | Connected to 12V wall adapter / power bus |
| **VIN-** | Pump Load Side | Connected to MOSFET / 12V Water Pump positive |

## Configuration & Specifications
- **I2C Address:** `0x40` (Default, with A0 and A1 tied to GND)
- **Bus Voltage Range:** Up to 26V
- **Dry-Run Threshold:** Current dropping below `30mA` during active pumping indicates a dry-run fault (aborts watering cycle and triggers fault lockout).

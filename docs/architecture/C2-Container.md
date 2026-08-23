# C2: Container Architecture - Automated Plant Watering System

## Overview
The C2 Container diagram breaks down the system into high-level runtime containers, illustrating the ESP32 firmware application, onboard NVS storage, and electrical hardware subsystems.

```mermaid
graph TD
    subgraph "IP65 Enclosure"
        direction TB
        buck["12V-to-5V Buck Converter (LM2596S)"]
        esp32["ESP32 Microcontroller (ESP-WROOM-32D)"]
        nvs["Preferences API (NVS Flash Memory)"]
        mosfet["IRFZ44N MOSFET Switch"]
        ina219["INA219 Current Sensor (I2C)"]
        
        esp32 -->|Stores Countdown & State| nvs
        esp32 -->|PWM / Digital GPIO 25| mosfet
        esp32 -->|I2C Telemetry| ina219
    end

    power["12V 3A Power Adapter"] -->|12V Bus| buck
    power -->|12V Direct| mosfet
    buck -->|5V Regulated| esp32

    mosfet -->|Switched 12V| pump["12V Submersible Water Pump (240L/H)"]
    pump --> valve["Anti-Siphon Check Valve"]
    valve --> manifold["4-Way Manifold & 2 GPH Drippers"]
    manifold --> pots["4 Plant Pots"]
```

## Containers
1. **ESP32 Firmware:** Runs the core non-blocking countdown timer, pump control logic, and safety monitors.
2. **NVS Flash Storage:** Persists countdown timer state every 5 minutes to survive power outages.
3. **Hardware Actuators & Sensors:** 12V pump, IRLZ44N MOSFET switch, INA219 current sensor, and buck converter.

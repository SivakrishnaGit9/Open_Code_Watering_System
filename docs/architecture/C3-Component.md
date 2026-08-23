# C3: Component Architecture - ESP32 Firmware Modules

## Overview
The C3 Component diagram details the internal software modules inside the ESP32 firmware application.

```mermaid
graph TD
    subgraph "ESP32 Firmware"
        direction TB
        main["main.cpp / Setup & Loop"]
        timer["Timer Module (millis() 48h Countdown)"]
        nvsManager["NVS Storage Manager (Preferences API)"]
        pumpController["Pump Controller (GPIO 25 + MOSFET)"]
        sensorMonitor["INA219 Sensor Monitor (Dry-Run Detection)"]
        ledFeedback["LED Indicator Module (GPIO 2)"]
        webServer["Debug Web Server (Conditional #define)"]

        main --> timer
        main --> pumpController
        main --> sensorMonitor
        main --> ledFeedback
        main --> webServer

        timer -->|Every 5 mins| nvsManager
        pumpController -->|Drives GPIO 25| sensorMonitor
        sensorMonitor -->|Fault Trigger| pumpController
        sensorMonitor -->|Fault Status| ledFeedback
    end
```

## Component Descriptions
- **Timer Module:** Non-blocking 48-hour countdown timer using `millis()`.
- **NVS Storage Manager:** Saves countdown state to ESP32 flash memory every 5 minutes.
- **Pump Controller:** Manages GPIO 25 output with hardware 10k pull-down protection.
- **Sensor Monitor:** Polls the INA219 current sensor over I2C to detect dry-running.
- **LED Feedback:** Provides visual status indication via GPIO 2 LED.
- **Debug Web Server:** Optional local web dashboard compiled only when `DEBUG_WEB_SERVER` is defined.

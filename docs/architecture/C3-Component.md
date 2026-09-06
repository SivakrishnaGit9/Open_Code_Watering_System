# C3: Component Architecture - ESP32 Firmware Modules

## Overview
The C3 Component diagram details the internal software modules inside the ESP32 firmware application.

```mermaid
graph TD
    subgraph "ESP32 Firmware"
        direction TB
        main["main.ino / Setup & Loop"]
        timer["Timer Module (millis() 48h Countdown)"]
        nvsManager["NVS Storage Manager (Preferences API)"]
        pumpController["Pump Controller (GPIO 25 + MOSFET)"]
        sensorMonitor["INA219 Sensor Monitor (Dry-Run Detection)"]
        ledFeedback["LED Indicator Module (GPIO 2)"]
        wifiAp["Wi-Fi Access Point Module (PlantWatering_AP)"]
        udpTelemetry["UDP Telemetry Broadcast Module (Port 8888)"]
        otaWebServer["HTTP OTA Web Server Module (Port 80 /update)"]

        main --> timer
        main --> pumpController
        main --> sensorMonitor
        main --> ledFeedback
        main --> wifiAp
        wifiAp --> udpTelemetry
        wifiAp --> otaWebServer

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
- **Wi-Fi Access Point Module:** Hosts `PlantWatering_AP` network for local device connection.
- **UDP Telemetry Broadcast Module:** Periodically broadcasts system state and sensor data every 4 seconds over UDP port 8888.
- **HTTP OTA Web Server Module:** Synchronous web server hosting the firmware update portal (`/update`) using dual-app OTA partitions (`ota_0`, `ota_1`).

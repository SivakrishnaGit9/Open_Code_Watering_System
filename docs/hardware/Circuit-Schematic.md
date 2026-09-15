# Plant Watering System - Detailed Electrical Circuit Schematic

This document provides a comprehensive Mermaid schematic and pinout mapping for the 12V Automated Plant Watering System (FSD v2.0).

---

## 1. System Power & Signal Architecture (Mermaid Diagram)

```mermaid
graph TD
    adapter["12V DC Wall Power Adapter (FEDUS 3A SMPS)<br/>Supply: 12V DC / Ground (GND)"] -->|12V Positive Rail| buck["LM2596S Step-Down Buck Converter<br/>Supply Input: 12V DC (IN+/IN-)<br/>Regulated Output: 5V DC (OUT+/OUT-)"]
    adapter -->|12V Positive Rail| ina219["INA219 Current Sensor Module<br/>Logic Supply: 3.3V / 5V DC (VCC/GND)<br/>High-Side Bus: VIN+ (In) & VIN- (Out)"]
    adapter -->|GND Rail| gnd1["System Ground (GND)"]

    buck -->|5V Regulated Supply| esp32["ESP32 DevKit / MCU<br/>Supply Input: 5V DC (VIN / 5V Pin) / GND"]
    
    ina219 -->|Switched 12V Line| mosfet["IRFZ44N MOSFET Module<br/>Module Supply: Logic Level Drive<br/>Terminals: Drain (D) & Source (S)"]
    
    mosfet -->|Source Pin| gnd2["System Ground (GND)"]
    
    esp32 -->|GPIO 18 Digital Output<br/>+ 10k Pulldown Resistor to GND| mosfetGate["MOSFET Gate (G)"]
    esp32 -->|GPIO 2 Digital Output| led["Status LED (with 220 Resistor) to GND"]
    esp32 -.->|I2C SDA GPIO 21 & SCL GPIO 22| ina219

    mosfet -->|Switched 12V Power| pump["12V DC Water Pump (240L/H)<br/>Supply: 12V DC via MOSFET"]
    pump --> valve["Anti-Siphon Check Valve"]
    valve --> manifold["4-Way Barbed Cross Manifold"]
    manifold --> d1["2 GPH Dripper -> Pot 1"]
    manifold --> d2["2 GPH Dripper -> Pot 2"]
    manifold --> d3["2 GPH Dripper -> Pot 3"]
    manifold --> d4["2 GPH Dripper -> Pot 4"]
```

---

## 2. Component Pinout, Power Requirements & Interfacing Reference Table

| Component | Operating Power Supply | Pin / Terminal | Connects To | Purpose / Notes |
| :--- | :--- | :--- | :--- | :--- |
| **12V Wall Adapter** | Mains AC to 12V DC (2A–3A) | Positive (+) / GND | Buck Converter, INA219, Pump | Primary system energy source. |
| **LM2596S Buck Converter** | Input: 4.5V–40V (uses 12V) <br> Output: 5V Regulated | IN+ / IN- <br> OUT+ / OUT- | 12V Adapter (+) / GND <br> ESP32 VIN / GND | Steps down 12V to stable 5V logic power for ESP32. |
| **INA219 Current Sensor** | Logic Supply: 3.0V–5.5V DC <br> Bus Voltage: 0V–26V DC | VCC <br> GND <br> VIN+ <br> VIN- <br> SDA / SCL | ESP32 3.3V/5V <br> System GND <br> 12V Adapter (+) <br> MOSFET Drain (D) <br> ESP32 GPIO 21 / 22 | Monitors high-side motor current over I2C for dry-run protection. |
| **IRFZ44N MOSFET Module** | Logic Gate: 3.3V / 5V <br> Load: 12V DC | Gate (G) <br> Drain (D) <br> Source (S) | GPIO 18 & 10k Pulldown <br> INA219 VIN- <br> Pump (-) & System GND | Fully saturates at 3.3V logic to switch 12V power to the pump. |
| **ESP32 Microcontroller** | Input: 5V DC (VIN) or 3.3V | VIN / 5V <br> GND <br> GPIO 18 <br> GPIO 2 <br> GPIO 21 / 22 | Buck 5V Output <br> System GND <br> MOSFET Gate <br> Status LED <br> INA219 SDA/SCL | Central controller managing timing, NVS, Wi-Fi telemetry, and OTA. |
| **12V Water Pump (240L/H)**| 12V DC (Submersible) | Positive (+) <br> Negative (-) | MOSFET Switched 12V <br> System GND | Pumps water reservoir through manifold. |
| **Anti-Siphon Check Valve** | Passive Hydraulic | Inlet / Outlet | Pump Outlet & Manifold | Prevents back-siphoning when idle. |
| **4-Way Barbed Cross & Drippers** | Passive Hydraulic | Barbed Ports | Check Valve & 4x Dripper Tubing | Balances distribution (~125ml per pot). |

# Comprehensive Engineering Design Document & Functional Specification (v2.0)
* **Project Name:** Automated Plant Watering System (v2.0)
* **Target Scale:** 4 Plant Pots (Equal Distribution via 2 GPH Drippers)
* **Core Microcontroller:** ESP32 NodeMCU / DevKit
* **Power Supply:** 12V DC Wall Power Adapter (Mains-Powered)
* **Status:** Finalized Architecture & Design Specification

---

## 1. Executive Summary & Project Scope

### 1.1 Purpose
This document outlines the complete architectural, electrical, mechanical, and software design for an automated plant watering system powered by a 12V DC wall adapter. Designed for reliable operation while traveling, it operates on a strict time-based schedule (watering every alternate day) using a water bucket reservoir.

To eliminate real-world field failure points (such as motor dry-burns, accidental siphoning, and power outage disruptions), this architecture integrates robust hardware safety buffers, non-volatile memory (NVS) state persistence, precise hydraulic balancing, and local feedback mechanisms without relying on complex, fragile soil moisture sensors.

### 1.2 Scope
* Time-based automated irrigation running on a fixed 48-hour alternate-day cycle.
* Mains-powered via 12V DC wall power adapter for 100% uptime reliability.
* NVS memory persistence to survive household power flickers/outages without missing or duplicating watering cycles.
* Hardware-level failure mitigations including dry-run protection, anti-siphon flow control, and GPIO pulldown resistors.
* Local feedback via status LEDs, USB serial logging, and optional development-only Wi-Fi web dashboard (compiled out for production).

---

## 2. Complete Bill of Materials (BOM) & Component Justification

| Category | Component Item | Specification / Model | Engineering Justification |
| :--- | :--- | :--- | :--- |
| **Control** | Microcontroller | ESP32 DevKit / NodeMCU | Low-power capabilities, NVS flash memory for state persistence, robust hardware timers, built-in Wi-Fi for development debugging. |
| **Power Supply** | Wall Power Adapter | 12V DC, 2A–3A | Primary power input; directly energizes 12V pump bus and step-down buck converter. |
| **Regulation** | DC-DC Step-Down | 12V-to-5V Buck Converter (LM2596) | Steps down 12V input to regulated 5V to safely power the ESP32 logic via VIN pin. |
| **Actuation** | Water Pump | 12V Mini DC Submersible/Diaphragm | Provides sufficient head pressure to lift water from the bucket to the pots. |
| **Switching** | Logic-Level MOSFET | IRLZ44N (or 3.3V Optoisolated Relay) | Fully saturates at 3.3V VGS (RDS(on) ≤ 0.025Ω) for reliable pump switching. |
| **Safety** | Gate Pulldown | 10kΩ Resistor | Ensures GPIO 25 remains strictly LOW during ESP32 bootloader/strapping phase. |
| **Safety** | Dry-Run Sensor | INA219 Current Sensor Module | Monitors pump motor electrical load via I2C; detects dry-running when current drops below threshold. |
| **Safety** | Anti-Siphon Valve | Inline Check Valve | Prevents gravity-fed siphoning/draining of the bucket when the pump is idle. |
| **Enclosure** | Weatherproof Box | IP65 Plastic Electrical Junction Box | Protects electronics, wiring, and ESP32 from outdoor rain, humidity, and sun. |
| **Hydraulics** | Tubing & Manifold | 1/4-inch PVC Tubing + 4-Way Cross | Standard flexible routing paired with a symmetric 1-to-4 splitter. |
| **Hydraulics** | Emitters | 4x Pressure-Compensating Drippers (2 GPH) | Guarantees exact, equal water distribution (~125ml per pot) regardless of path length. |

---

## 3. End-to-End System Architecture & Wiring Schematic

```text
[ 12V DC Wall Power Adapter (2A-3A) ]
                 |
                 +-----------------------------------+-----------------------------------+
                 |                                   |                                   |
                 v                                   v                                   v
   [ 12V-to-5V Buck Converter ]         [ IRLZ44N MOSFET Module ]              [ INA219 Current Sensor ]
                 |                         (Gate pulled down via 10k)          (Connected via I2C SDA/SCL)
                 v                                   |                                   |
     [ ESP32 DevKit (5V VIN) ]                       v                                   v
                 |                          [ 12V DC Water Pump ]               [ Status LED (GPIO 2) ]
                 |                                   |
                 +--(GPIO 25 Control)----------------+
                                                     v
                                        [ Anti-Siphon Check Valve ]
                                                     |
                                       [ 4-Way Barbed Cross Manifold ]
                                                     |
                                  [ 4x 2 GPH Pressure-Compensating Drippers ]
                                                     |
                                          [ 4 Individual Plant Pots ]
```

### GPIO & Interface Assignment & Hardware Safety Rules
* **Pump Control:** GPIO 25 (Digital Output). An external 10kΩ pulldown resistor to GND ensures GPIO 25 stays strictly LOW during ESP32 power-on / bootloader execution.
* **Current Monitoring (INA219):** I2C Bus (SDA / SCL, default GPIO 21 / 22). Monitors pump motor electrical current for dry-run detection.
* **Status LED:** GPIO 2 (Digital Output). Visual system state feedback.
* Strapping pins (GPIO 0, 2, 12, 15) are avoided for pump control to prevent accidental triggers during boot.

---

## 4. Functional Requirements

### 4.1 Power Subsystem Requirements
* **FR-P1:** The system shall draw primary energy from a 12V DC wall power adapter (2A–3A rating).
* **FR-P2:** A 12V-to-5V step-down buck converter (LM2596) shall supply regulated power to the ESP32 logic board.
* **FR-P3:** The 12V power bus shall directly supply the DC water pump via the MOSFET switching module.

### 4.2 Control & Computation Requirements
* **FR-C1:** The ESP32 shall maintain an internal non-blocking countdown timer corresponding to a 48-hour watering interval.
* **FR-C2 (NVS State Persistence):** Every 5 minutes during countdown, the ESP32 shall write the remaining countdown time and total watering cycle count to Non-Volatile Storage (NVS / Preferences API). Upon reboot or power restoration, the countdown shall resume from the last saved state.
* **FR-C3:** The system shall provide visual feedback via status LEDs (slow blink for idle countdown, solid ON during active pumping, rapid flash for error/fault conditions).
* **FR-C4 (Debug Web Server):** A local HTTP web server for system telemetry and manual pump overrides shall be available only during development, controlled by the `#define DEBUG_WEB_SERVER` compile flag. In production builds, the Wi-Fi stack is disabled for maximum stability.

### 4.3 Actuation & Hydraulics Requirements
* **FR-H1:** The system shall utilize a 12V mini DC water pump switched via an IRLZ44N logic-level MOSFET module (or 3.3V optoisolated relay) controlled by digital GPIO 25 from the ESP32. A 10kΩ external pulldown resistor shall be installed on the MOSFET gate line.
* **FR-H2 (Hydraulic Calibration):** A single watering cycle shall run for a calibrated duration of **60 seconds**, delivering approximately **500ml** total water volume (~125ml per pot). Based on 4x 2 GPH pressure-compensating drippers: total flow = 8 GPH ≈ 504 ml/minute ≈ 8.4 ml/second; 60s × 8.4 ml/s ≈ 504 ml.
* **FR-H3:** Water shall pass through an inline anti-siphon check valve into a 4-way barbed cross manifold, terminating in 4 individual **2 GPH pressure-compensating drippers** to ensure balanced fluid distribution.

### 4.4 Safety & Fault Mitigation Requirements
* **FR-S1 (Dry-Run Protection):** An INA219 current sensor module wired in series with the 12V pump circuit shall monitor motor operating current via I2C. If current drops below the dry-run threshold during active pumping, the ESP32 shall abort the watering cycle, lock out the pump, force GPIO 25 LOW, and signal a fault via rapid LED flash.
* **FR-S2 (Back-Siphon Prevention):** An anti-siphon check valve shall prevent passive gravity drainage of the bucket when the pump is idle.
* **FR-S3 (Watchdog / Boot Safety):** Upon any unexpected system reset or boot cycle, the ESP32 initialization sequence shall immediately force GPIO 25 to LOW (OFF) before any other operations. The external 10kΩ pulldown resistor provides redundant hardware-level protection.

---

## 5. Operational State Machine

```text
                +----------------------------------+
                |            BOOT_INIT             |
                | - Force GPIO 25 LOW              |
                | - Read NVS state (time/count)    |
                | - Init I2C & INA219 / LED        |
                +----------------+-----------------+
                                 |
                                 v
                +----------------------------------+
                |      STATE_IDLE_COUNTDOWN        |
                | - Non-blocking 48h countdown     |
                | - Save state to NVS every 5min   |
                | - LED slow blink                 |
                +----------------+-----------------+
                                 |
                    (48-Hour Timer Reached)
                                 |
                                 v
                +----------------------------------+
                |  STATE_WATERING_ACTIVE           |
                | - Pump GPIO 25 HIGH              |
                | - Run 60s timer                  |
                | - Monitor pump current via INA219|
                | - LED solid ON                   |
                +--------+----------------+--------+
                         |                |
             (Current OK)|                | (Current Low / Dry-Run)
                         v                v
 +-----------------------+--+          +--+------------------------+
 |    STATE_CYCLE_RESET     |          |      FAULT_LOCKOUT        |
 | - GPIO 25 LOW            |          | - Force GPIO 25 LOW       |
 | - Increment cycle count  |          | - Rapid LED flash         |
 | - Write NVS (reset timer)|          | - Block pump              |
 | - Log telemetry          |          | - Wait for manual reset   |
 +-----------+--------------+          +---------------------------+
             |
             +------------------> (Return to STATE_IDLE_COUNTDOWN)
```

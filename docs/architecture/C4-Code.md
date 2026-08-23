# C4: Code Architecture & Class/Module Design

## Overview
The C4 Code level specifies the core software routines, state machines, and hardware mapping corresponding to `FSD_Base/FSD_V_1_0.md`.

## Core Pin & Hardware Mapping
- **Pump Output:** `PIN_PUMP = 25` (Digital Output, Default LOW)
- **Status LED:** `PIN_LED = 2` (Digital Output)
- **I2C Bus (INA219):** `SDA = 21`, `SCL = 22`
- **Watering Duration:** `60,000 ms` (60 seconds)
- **Watering Interval:** `172,800,000 ms` (48 hours)
- **NVS Save Interval:** `300,000 ms` (5 minutes)

## State Machine
1. **IDLE / COUNTDOWN:** ESP32 counts down 48 hours; saves state to NVS every 5 minutes; LED slowly pulses.
2. **WATERING:** Pump GPIO 25 goes HIGH for 60 seconds; INA219 monitors motor current; LED is solid ON.
3. **FAULT / DRY-RUN:** If INA219 current drops below threshold, pump is forced LOW, state is locked, and LED flashes rapidly.

# Plant Watering System - Improvement & Calibration Roadmap

This document tracks identified enhancement points, calibration tasks, and robustness improvements for the Plant Watering System (FSD v2.0).

---

## 1. High Priority: INA219 Current Sensor Calibration & Inrush Blanking

### Background & Objective
The system uses an INA219 current sensor on the 12V bus to detect dry-run conditions (when the pump runs without water, drawing significantly less current). The current hardcoded threshold is `< 30.0mA`.

### Tasks & Implementation Plan
- [x] **Empirical Current Measurement**: Tested under real-world conditions:
  1. *Normal Pumping (Submerged in water)*: Steady-state operating current ~300mA (with minor transitional dips).
  2. *Dry-Run (Pump running dry / air)*: Running current ~70mA (varying down to lower values).
- [x] **Dynamic Threshold Tuning**: Updated `DRY_RUN_THRESHOLD_MA` in `src/main.ino` to `100.0f` to reliably separate normal load from dry-run states.
- [x] **Digital Filtering**: Implemented an Exponential Moving Average (EMA) filter ($\alpha = 0.2$) on INA219 current measurements to eliminate high-frequency noise and smooth telemetry data.
- [ ] **Startup Inrush Blanking Window**: Add a short grace period (e.g., 500ms to 1000ms) after pump activation (`STATE_WATERING` entry) before enforcing the dry-run current check. This accommodates motor startup inrush current transients.

---

## 2. Medium Priority: State Persistence & Fault Management

### Background & Objective
Improve resilience against unexpected power interruptions and fault lockouts.

### Tasks & Implementation Plan
- [ ] **Fault State Persistence in NVS**: Save fault lockout states (`STATE_FAULT`) to NVS so that a power cycle does not automatically clear a dry-run fault without user intervention or manual reset.
- [ ] **Manual Reset Endpoint / Button**: Implement a web handler or physical button trigger to clear fault lockouts and reset countdown timers.
- [ ] **Timestamp-Based Countdown**: Consider storing the target absolute epoch timestamp (or millis offset anchor) instead of raw remaining milliseconds to eliminate drift and save frequency overhead.

---

## 3. Security & Access Control

### Background & Objective
Secure the onboard AP and HTTP OTA web portal.

### Tasks & Implementation Plan
- [ ] **HTTP Basic Auth for OTA Portal**: Add password protection or basic authentication to the `/update` HTTP endpoint on the AP web server (`192.168.4.1`) to prevent unauthorized firmware uploads.

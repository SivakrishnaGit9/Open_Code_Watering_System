# Plant Watering System - Improvement & Calibration Roadmap

This document tracks identified enhancement points, calibration tasks, and robustness improvements for the Plant Watering System (FSD v2.0).

---

## 1. High Priority: INA219 Current Sensor Calibration & Inrush Blanking

### Background & Objective
The system uses an INA219 current sensor on the 12V bus to detect dry-run conditions (when the pump runs without water, drawing significantly less current). The current hardcoded threshold is `< 30.0mA`.

### Tasks & Implementation Plan
- [ ] **Empirical Current Measurement**: Connect the 12V pump under two real-world test conditions with the INA219 logging telemetry to Serial:
  1. *Normal Pumping (Submerged in water)*: Record steady-state operating current (e.g., typically ~100mA–300mA depending on the 240L/H pump model).
  2. *Dry-Run (Pump running dry / air)*: Record running current without water resistance.
- [ ] **Dynamic Threshold Tuning**: Update `DRY_RUN_THRESHOLD_MA` in `src/main.ino` based on empirical measurements to reliably separate normal load from dry-run states without false positives.
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

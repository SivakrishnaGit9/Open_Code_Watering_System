# ina219-current-calibration Specification

## Purpose
Provides calibrated dry-run current protection threshold and digital noise filtering for the INA219 current sensor to ensure reliable pump protection and stable telemetry.
## Requirements
### Requirement: Calibrated Dry-Run Current Threshold
The system SHALL use a dry-run current threshold of 100.0mA during active pumping to reliably detect when the pump is running without water resistance.

#### Scenario: Dry-run detection at 100mA threshold
- **WHEN** the pump is active and the filtered current drops below 100.0mA
- **THEN** the system aborts the watering cycle, forces pump output LOW, transitions to fault state, and logs the fault

### Requirement: Exponential Moving Average Digital Filtering
The system SHALL apply an Exponential Moving Average (EMA) digital filter to INA219 current measurements to eliminate instantaneous motor commutation and fluid noise in telemetry and safety checks.

#### Scenario: Smoothed current telemetry
- **WHEN** telemetry or dry-run checks sample the INA219 current sensor
- **THEN** the system returns the filtered moving average current value rather than raw instantaneous spikes


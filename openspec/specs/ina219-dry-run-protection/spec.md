# ina219-dry-run-protection Specification

## Purpose
Provides real-time electrical load monitoring via the INA219 current sensor to detect dry-run conditions and safely abort watering.
## Requirements
### Requirement: INA219 Current Monitoring
The system SHALL monitor pump motor operating current via the INA219 current sensor over I2C during active pumping.

#### Scenario: Active pump current check
- **WHEN** the water pump is running
- **THEN** the system reads current draw from the INA219 sensor

### Requirement: Dry-Run Fault Abort
The system SHALL abort the watering cycle, lock out the pump, force GPIO 25 LOW, and signal a fault if current drops below 30mA during active pumping.

#### Scenario: Dry-run condition detected
- **WHEN** pump current drops below the 30mA threshold while pumping
- **THEN** the watering cycle is aborted, GPIO 25 is forced LOW, and an error state is signaled


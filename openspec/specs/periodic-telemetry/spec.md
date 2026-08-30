# periodic-telemetry Specification

## Purpose
Provides scheduled, throttled system status and sensor telemetry over the serial interface for continuous operational monitoring.
## Requirements
### Requirement: Periodic Telemetry Logging
The system SHALL log telemetry data containing system uptime, current state, bus voltage, and shunt current at a configurable periodic interval.

#### Scenario: Telemetry logging during idle state
- **WHEN** the system is in STATE_IDLE and the telemetry interval (4 seconds) elapses
- **THEN** the system prints uptime, current state, bus voltage, and current draw over the serial interface with proper line termination

#### Scenario: Telemetry logging during watering state
- **WHEN** the system is in STATE_WATERING and the telemetry interval elapses
- **THEN** the system prints active pump current and bus voltage telemetry over the serial interface


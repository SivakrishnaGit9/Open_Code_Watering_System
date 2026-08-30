# plant-watering-controller Specification

## Purpose
Manages the core time-based watering schedule, non-volatile state persistence, and boot-time safety controls for the plant watering system.
## Requirements
### Requirement: Time-Based Watering Schedule
The system SHALL maintain an internal non-blocking countdown timer corresponding to a 48-hour watering interval and execute a 60-second watering cycle delivering ~500ml total water volume.

#### Scenario: Scheduled watering execution
- **WHEN** the 48-hour countdown timer expires
- **THEN** the system activates the water pump for exactly 60 seconds

### Requirement: NVS State Persistence
The system SHALL write remaining countdown time and total watering cycle count to Non-Volatile Storage (NVS) every 5 minutes during countdown.

#### Scenario: Periodic state saving
- **WHEN** 5 minutes elapse during the countdown
- **THEN** the remaining time and cycle count are persisted to NVS

### Requirement: Boot Safety and Initialization
The system SHALL immediately force GPIO 25 LOW upon boot before any other operations to prevent accidental pump actuation.

#### Scenario: System startup or reset
- **WHEN** the ESP32 powers on or resets
- **THEN** GPIO 25 is set to LOW output immediately


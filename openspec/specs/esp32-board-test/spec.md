# esp32-board-test Specification

## Purpose
Provides a baseline hardware verification and health check routine for the ESP32 development board, confirming boot status, serial communication, and basic GPIO outputs.
## Requirements
### Requirement: Microcontroller Health Reporting
The system SHALL output chip diagnostics (model, core count, CPU frequency, and heap memory) over the serial interface upon boot.

#### Scenario: Successful boot diagnostics
- **WHEN** the ESP32 powers up and initializes Serial at 115200 baud
- **THEN** it prints chip model, core count, frequency, and free heap size to the serial monitor

### Requirement: On-board LED Status Toggle
The system SHALL toggle the on-board LED (GPIO 2) at a fixed interval to indicate active operation and board health.

#### Scenario: LED pulsing
- **WHEN** the board is running the test loop
- **THEN** it switches the LED state between ON and OFF every second with a corresponding serial log


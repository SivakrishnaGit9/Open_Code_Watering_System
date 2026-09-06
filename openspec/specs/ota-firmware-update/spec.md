# ota-firmware-update Specification

## Purpose
Provides a secure, local over-the-air (OTA) firmware update mechanism hosted via synchronous HTTP on the ESP32 Wi-Fi Access Point, allowing field maintenance without physical USB connections.
## Requirements
### Requirement: OTA Firmware Update Portal
The system SHALL host a lightweight synchronous HTTP web server on the Wi-Fi Access Point at `192.168.4.1` providing an endpoint for firmware binary uploads.

#### Scenario: Accessing update portal
- **WHEN** user connects to `PlantWatering_AP` and navigates to `http://192.168.4.1`
- **THEN** system serves an HTML firmware upload interface

#### Scenario: Successful firmware upload
- **WHEN** user uploads a valid compiled firmware binary file via HTTP POST
- **THEN** system verifies partition space, writes the binary to the inactive OTA slot, updates boot flags, and restarts the device

#### Scenario: Invalid binary rejection
- **WHEN** user uploads an invalid or corrupted binary file
- **THEN** system aborts the flash process, reports an error, and retains the current stable firmware


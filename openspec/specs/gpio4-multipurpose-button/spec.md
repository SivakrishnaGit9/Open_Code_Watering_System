# gpio4-multipurpose-button Specification

## Purpose
Provides local physical control via a multi-purpose tactile pushbutton on GPIO 4 for immediate watering, watering cancellation, and Wi-Fi AP toggling with NVS persistence.
## Requirements
### Requirement: GPIO 4 Pushbutton Input and Debouncing
The system SHALL configure GPIO 4 as an input with an internal pull-up resistor (`INPUT_PULLUP`), reading LOW when pressed to GND, and perform non-blocking software debouncing.

#### Scenario: Button press detection
- **WHEN** the pushbutton connected to GPIO 4 is pressed
- **THEN** the system detects the falling edge with software debouncing without blocking execution or timers

### Requirement: Single-Click Immediate Watering
The system SHALL trigger an immediate 60-second watering cycle and reset the 48-hour countdown timer when a single click is registered within the detection window.

#### Scenario: Single click triggers watering
- **WHEN** the user presses the button once and no second click or long press occurs within the detection window
- **THEN** the system immediately enters watering state for 60 seconds and resets the 48-hour interval timer

### Requirement: Long-Press Watering Cancellation
The system SHALL cancel an active watering cycle if the pushbutton is held down for more than 2 seconds.

#### Scenario: Long press cancels active watering
- **WHEN** the user holds the pushbutton down continuously for greater than 2000ms while the system is watering
- **THEN** the system immediately stops the pump, transitions back to idle state, and resumes normal countdown tracking

### Requirement: Double-Click Wi-Fi AP Toggle and NVS Persistence
The system SHALL toggle the Wi-Fi Access Point mode (ON/OFF) upon a double click within the multi-click window and persist the new Wi-Fi state in NVS preferences.

#### Scenario: Double click toggles Wi-Fi AP and saves state
- **WHEN** the user clicks the pushbutton twice within an 800ms gap inside the 4-second total detection window
- **THEN** the system toggles the Wi-Fi AP state, saves the updated preference to NVS, and remembers the state across device reboots

### Requirement: Visual Feedback via Built-in LED
The system SHALL provide distinct visual feedback via the built-in LED (GPIO 2) in response to button actions.

#### Scenario: LED flashes on button actions
- **WHEN** a single click, double click, or long press is recognized
- **THEN** the LED emits corresponding flash patterns (1 flash for single click, 2 flashes for double click, sustained signal for cancel)


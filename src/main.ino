#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

// Debug macro switch (define to enable continuous telemetry printing for testing)
#define DEBUG_SENSOR_TELEMETRY 1

// Pin definitions per FSD v2.0
#define PUMP_PIN 25
#define LED_PIN  2

// Timing and thresholds per FSD v2.0
const unsigned long WATERING_INTERVAL_MS = 48UL * 3600UL * 1000UL; // 48 hours
const unsigned long WATERING_DURATION_MS = 60UL * 1000UL;           // 60 seconds
const unsigned long NVS_SAVE_INTERVAL_MS = 5UL * 60UL * 1000UL;     // 5 minutes
const unsigned long TELEMETRY_INTERVAL_MS = 4000UL;                 // 4 seconds periodic telemetry
const float DRY_RUN_THRESHOLD_MA = 30.0f;                           // < 30mA dry-run threshold

Preferences preferences;
Adafruit_INA219 ina219;

enum SystemState {
  STATE_IDLE,
  STATE_WATERING,
  STATE_FAULT
};

SystemState currentState = STATE_IDLE;

unsigned long lastMillis = 0;
unsigned long remainingCountdownMs = WATERING_INTERVAL_MS;
unsigned long lastNvsSaveMs = 0;
unsigned long lastTelemetryMs = 0;
unsigned long wateringStartMs = 0;
unsigned long cycleCount = 0;
bool ina219Available = false;

void setup() {
  // CRITICAL: Force pump pin LOW immediately upon boot before any other operations
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  delay(500);

  Serial.println("=========================================");
  Serial.println("Plant Watering System (FSD v2.0) Starting");
  Serial.println("=========================================");

  // Initialize NVS Preferences
  preferences.begin("plant-water", false);
  remainingCountdownMs = preferences.getULong("remaining", WATERING_INTERVAL_MS);
  cycleCount = preferences.getULong("cycles", 0);
  preferences.end();

  Serial.printf("Restored State -> Remaining Countdown: %lu s, Total Cycles: %lu\r\n", 
                remainingCountdownMs / 1000UL, cycleCount);

  // Initialize I2C and INA219 current sensor
  Wire.begin(21, 22);
  if (ina219.begin()) {
    ina219Available = true;
    Serial.println("INA219 Current Sensor initialized successfully at 0x40.");
  } else {
    ina219Available = false;
    Serial.println("WARNING: INA219 sensor not found! Dry-run protection will operate in fallback mode.");
  }

  lastMillis = millis();
  lastNvsSaveMs = millis();
  lastTelemetryMs = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - lastMillis;
  lastMillis = currentMillis;

  // Periodic throttled telemetry handler (every 4 seconds)
  if (currentMillis - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = currentMillis;
    float busvoltage = ina219Available ? ina219.getBusVoltage_V() : 0.0f;
    float current_mA = ina219Available ? ina219.getCurrent_mA() : 0.0f;
    unsigned long uptimeSec = currentMillis / 1000UL;
    Serial.printf("[TELEMETRY] Uptime: %lus | State: %d | Bus: %.2fV | Current: %.2fmA | Remaining: %lus\r\n",
                  uptimeSec, (int)currentState, busvoltage, current_mA, remainingCountdownMs / 1000UL);
  }

  switch (currentState) {
    case STATE_IDLE: {
      // Slow blink LED for idle countdown
      digitalWrite(LED_PIN, (currentMillis / 1000) % 2 == 0 ? HIGH : LOW);

#if DEBUG_SENSOR_TELEMETRY
      if (ina219Available) {
        float busvoltage = ina219.getBusVoltage_V();
        float current_mA = ina219.getCurrent_mA();
        Serial.printf("[DEBUG IDLE] Bus: %.2fV | Current: %.2fmA\r\n", busvoltage, current_mA);
      }
#endif

      if (remainingCountdownMs > elapsed) {
        remainingCountdownMs -= elapsed;
      } else {
        remainingCountdownMs = 0;
      }

      // Periodically save state to NVS every 5 minutes
      if (currentMillis - lastNvsSaveMs >= NVS_SAVE_INTERVAL_MS) {
        lastNvsSaveMs = currentMillis;
        preferences.begin("plant-water", false);
        preferences.putULong("remaining", remainingCountdownMs);
        preferences.putULong("cycles", cycleCount);
        preferences.end();
        Serial.println("State persisted to NVS (5 min checkpoint).");
      }

      // Check if countdown expired
      if (remainingCountdownMs == 0) {
        Serial.println("Countdown expired! Starting 60s watering cycle.");
        currentState = STATE_WATERING;
        wateringStartMs = currentMillis;
        digitalWrite(PUMP_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH); // Solid ON during pumping
      }
      break;
    }

    case STATE_WATERING: {
      // Solid ON LED during active pumping
      digitalWrite(LED_PIN, HIGH);

      // Check current draw via INA219 if available
      if (ina219Available) {
        float current_mA = ina219.getCurrent_mA();
        Serial.printf("Pumping Active -> Current: %.2f mA\r\n", current_mA);

        if (current_mA < DRY_RUN_THRESHOLD_MA) {
          Serial.println("ERROR: Dry-run detected! Current below 30mA threshold. Aborting watering cycle!");
          digitalWrite(PUMP_PIN, LOW);
          currentState = STATE_FAULT;
          break;
        }
      }

      // Check if 60s watering duration is complete
      if (currentMillis - wateringStartMs >= WATERING_DURATION_MS) {
        Serial.println("Watering cycle complete (60s). Shutting off pump.");
        digitalWrite(PUMP_PIN, LOW);
        cycleCount++;
        remainingCountdownMs = WATERING_INTERVAL_MS;

        // Save new cycle count and reset countdown to NVS
        preferences.begin("plant-water", false);
        preferences.putULong("remaining", remainingCountdownMs);
        preferences.putULong("cycles", cycleCount);
        preferences.end();

        currentState = STATE_IDLE;
      }
      break;
    }

    case STATE_FAULT: {
      // Rapid flash for error/fault conditions
      digitalWrite(LED_PIN, (currentMillis / 200) % 2 == 0 ? HIGH : LOW);
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("SYSTEM FAULT LOCKOUT: Check water reservoir and INA219 sensor.");
      delay(2000);
      break;
    }
  }

  delay(100);
}

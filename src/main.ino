#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Update.h>

// Debug macro switch (define to enable continuous telemetry printing for testing)
#define DEBUG_SENSOR_TELEMETRY 0

// Wi-Fi Access Point (AP) and UDP Telemetry configuration
#define AP_SSID "PlantWatering_AP"
#define AP_PASSWORD "plant_password_123"
#define UDP_PORT 8888

WiFiUDP udp;
WebServer server(80);
IPAddress broadcastIp(192, 168, 4, 255); // AP subnet broadcast
bool wifiApActive = false;
#define PUMP_PIN 18
#define LED_PIN  2
#define BUTTON_PIN 4

const char* serverIndex = 
  "<!DOCTYPE html><html>"
  "<head><title>Plant Watering OTA Update</title>"
  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
  "<style>body{font-family:sans-serif;text-align:center;padding:20px;background:#f4f4f9;color:#333;}"
  "h2{color:#2e7d32;}.card{background:#fff;padding:20px;border-radius:8px;box-shadow:0 4px 8px rgba(0,0,0,0.1);max-width:400px;margin:0 auto;}"
  "input[type='file']{margin:15px 0;}.btn{background:#2e7d32;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer;font-size:16px;}"
  ".btn:hover{background:#1b5e20;}</style></head>"
  "<body><div class='card'>"
  "<h2>Plant Watering System</h2>"
  "<p>Over-the-Air Firmware Update Portal</p>"
  "<form method='POST' action='/update' enctype='multipart/form-data'>"
  "<input type='file' name='update'><br>"
  "<input type='submit' class='btn' value='Update Firmware'>"
  "</form></div></body></html>";

// Timing and thresholds per FSD v2.0
const unsigned long WATERING_INTERVAL_MS = 48UL * 3600UL * 1000UL; // 48 hours
const unsigned long WATERING_DURATION_MS = 60UL * 1000UL;           // 60 seconds
const unsigned long NVS_SAVE_INTERVAL_MS = 5UL * 60UL * 1000UL;     // 5 minutes
const unsigned long TELEMETRY_INTERVAL_MS = 5000UL;                 // 5 seconds periodic telemetry
const float DRY_RUN_THRESHOLD_MA = 40.0f;                           // < 40mA dry-run threshold per refined spec

Preferences preferences;
Adafruit_INA219 ina219;

// Button and Wi-Fi toggle variables
bool wifiEnabled = true;
int lastButtonReading = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50UL;

unsigned long pressStartTime = 0;
bool isPressed = false;
bool longPressTriggered = false;

unsigned long firstPressTime = 0;
int clickCount = 0;
bool waitingForClicks = false;
const unsigned long TOTAL_WINDOW_MS = 4000UL;
const unsigned long LONG_PRESS_MS = 2000UL;

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

// EMA Filter variables for INA219 current
float filteredCurrent_mA = 0.0f;
bool firstCurrentSample = true;
const float EMA_ALPHA = 0.05f;
const unsigned long INRUSH_BLANKING_MS = 4000UL; // 2-second inrush blanking window for motor spin-up

const char* getStateName(SystemState state) {
  switch (state) {
    case STATE_IDLE: return "IDLE";
    case STATE_WATERING: return "WATERING";
    case STATE_FAULT: return "FAULT";
    default: return "UNKNOWN";
  }
}

void systemLog(const char* format, ...) {
  char logBuffer[200];
  va_list args;
  va_start(args, format);
  vsnprintf(logBuffer, sizeof(logBuffer), format, args);
  va_end(args);

  Serial.print(logBuffer);

  if (wifiApActive) {
    udp.beginPacket(broadcastIp, UDP_PORT);
    udp.write((uint8_t*)logBuffer, strlen(logBuffer));
    udp.endPacket();
  }
}

void setup() {
  // CRITICAL: Force pump pin LOW immediately upon boot before any other operations
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  delay(500);

  Serial.println("=========================================");
  Serial.println("Plant Watering System (FSD v2.0) Starting");
  Serial.println("=========================================");

  // Initialize NVS Preferences
  preferences.begin("plant-water", false);
  remainingCountdownMs = preferences.getULong("remaining", WATERING_INTERVAL_MS);
  cycleCount = preferences.getULong("cycles", 0);
  wifiEnabled = preferences.getBool("wifi_enabled", true);
  preferences.end();

  Serial.printf("Restored State -> Remaining Countdown: %lu s, Total Cycles: %lu | Wi-Fi Enabled: %s\r\n", 
                remainingCountdownMs / 1000UL, cycleCount, wifiEnabled ? "YES" : "NO");

  // Initialize I2C and INA219 current sensor
  Wire.begin(21, 22);
  if (ina219.begin()) {
    ina219Available = true;
    Serial.println("INA219 Current Sensor initialized successfully at 0x40.");
  } else {
    ina219Available = false;
    Serial.println("WARNING: INA219 sensor not found! Dry-run protection will operate in fallback mode.");
  }

  // Initialize Wi-Fi Access Point (AP) mode if enabled
  if (wifiEnabled) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    IPAddress apIP = WiFi.softAPIP();
    wifiApActive = true;
    Serial.printf("Wi-Fi Access Point Started! SSID: %s | AP IP: %s\r\n", AP_SSID, apIP.toString().c_str());
    udp.begin(UDP_PORT);
  } else {
    WiFi.mode(WIFI_OFF);
    wifiApActive = false;
    Serial.println("Wi-Fi AP is disabled via NVS preference.");
  }

  // Initialize HTTP OTA Web Server
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", Update.hasError() ? "FAIL" : "SUCCESS. Rebooting...");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("OTA Update Start: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("OTA Update Success: %u bytes\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  Serial.println("HTTP OTA Server started on port 80.");

  lastMillis = millis();
  lastNvsSaveMs = millis();
  lastTelemetryMs = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - lastMillis;
  lastMillis = currentMillis;

  // Button polling & non-blocking multi-click / long-press handler
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonReading) {
    lastDebounceTime = currentMillis;
  }
  lastButtonReading = reading;

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        // Press down (falling edge)
        pressStartTime = currentMillis;
        longPressTriggered = false;
        isPressed = true;
      } else {
        // Release (rising edge)
        if (isPressed && !longPressTriggered) {
          unsigned long pressDuration = currentMillis - pressStartTime;
          if (pressDuration < LONG_PRESS_MS) {
            // Valid click detected
            if (!waitingForClicks) {
              firstPressTime = currentMillis;
              waitingForClicks = true;
              clickCount = 1;
            } else {
              // Within the 4-second slot, increment click count
              clickCount++;
            }
          }
        }
        isPressed = false;
      }
    }
  }

  // Check long press while held down
  if (isPressed && !longPressTriggered) {
    if (currentMillis - pressStartTime >= LONG_PRESS_MS) {
      longPressTriggered = true;
      isPressed = false;
      // Long press action: cancel active watering if watering
      if (currentState == STATE_WATERING) {
        systemLog("[BUTTON] Long press detected (>2s) during watering! Cancelling watering cycle.\r\n");
        digitalWrite(PUMP_PIN, LOW);
        currentState = STATE_IDLE;
        remainingCountdownMs = WATERING_INTERVAL_MS;
        // LED visual feedback: 3 fast flashes
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          delay(100);
          digitalWrite(LED_PIN, LOW);
          delay(100);
        }
      }
    }
  }

  // Check multi-click window expiration (4 seconds total window)
  if (waitingForClicks && (currentMillis - firstPressTime >= TOTAL_WINDOW_MS)) {
    waitingForClicks = false;
    if (clickCount == 1) {
      systemLog("[BUTTON] Single click detected! Triggering immediate watering cycle.\r\n");
      currentState = STATE_WATERING;
      wateringStartMs = currentMillis;
      remainingCountdownMs = WATERING_INTERVAL_MS;
      digitalWrite(PUMP_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      // LED visual feedback: 1 flash
      delay(200);
      digitalWrite(LED_PIN, LOW);
    } else if (clickCount == 2) {
      wifiEnabled = !wifiEnabled;
      systemLog("[BUTTON] Double click detected! Toggling Wi-Fi AP state to: %s\r\n", wifiEnabled ? "ON" : "OFF");
      
      preferences.begin("plant-water", false);
      preferences.putBool("wifi_enabled", wifiEnabled);
      preferences.end();

      if (wifiEnabled) {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_SSID, AP_PASSWORD);
        wifiApActive = true;
        udp.begin(UDP_PORT);
        server.begin();
        systemLog("[WIFI] Wi-Fi Access Point enabled.\r\n");
      } else {
        server.stop();
        udp.stop();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        wifiApActive = false;
        systemLog("[WIFI] Wi-Fi Access Point disabled.\r\n");
      }

      // LED visual feedback: 2 flashes
      for (int i = 0; i < 2; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(150);
        digitalWrite(LED_PIN, LOW);
        delay(150);
      }
    } else {
      systemLog("[BUTTON] Too many presses (%d)! Invalid action sequence ignored.\r\n", clickCount);
      // LED visual feedback: 3 rapid error flashes
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(80);
        digitalWrite(LED_PIN, LOW);
        delay(80);
      }
    }
    clickCount = 0;
  }

  // Periodic throttled telemetry handler (every 5 seconds)
  if (currentMillis - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = currentMillis;
    float busvoltage = ina219Available ? ina219.getBusVoltage_V() : 0.0f;
    float rawCurrent = ina219Available ? ina219.getCurrent_mA() : 0.0f;
    if (ina219Available) {
      if (firstCurrentSample) {
        filteredCurrent_mA = rawCurrent;
        firstCurrentSample = false;
      } else {
        filteredCurrent_mA = (EMA_ALPHA * rawCurrent) + ((1.0f - EMA_ALPHA) * filteredCurrent_mA);
      }
    }
    unsigned long uptimeSec = currentMillis / 1000UL;
    
    char logBuffer[160];
    snprintf(logBuffer, sizeof(logBuffer),
             "[TELEMETRY] Uptime: %lus | State: %s | Bus: %.2fV | Current: %.2fmA | Remaining: %lus\r\n",
             uptimeSec, getStateName(currentState), busvoltage, filteredCurrent_mA, remainingCountdownMs / 1000UL);
    
    systemLog("%s", logBuffer);
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
        systemLog("[SYSTEM] State persisted to NVS (5 min checkpoint).\r\n");
      }

      // Check if countdown expired
      if (remainingCountdownMs == 0) {
        systemLog("[SYSTEM] Countdown expired! Starting 60s watering cycle.\r\n");
        currentState = STATE_WATERING;
        wateringStartMs = currentMillis;
        firstCurrentSample = true;
        digitalWrite(PUMP_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH); // Solid ON during pumping
      }
      break;
    }

    case STATE_WATERING: {
      // Solid ON LED during active pumping
      digitalWrite(LED_PIN, HIGH);

      unsigned long elapsedPumping = currentMillis - wateringStartMs;

      // Monitor motor operating current via INA219 current sensor
      if (ina219Available) {
        float rawCurrent = ina219.getCurrent_mA();
        
        // Clamp negative zero-drift quantization noise to 0.0mA
        if (rawCurrent < 0.0f) {
          rawCurrent = 0.0f;
        }

        // Continuously apply EMA filter from t=0 to smooth startup ramp and filter out transients
        if (firstCurrentSample) {
          filteredCurrent_mA = rawCurrent;
          firstCurrentSample = false;
        } else {
          filteredCurrent_mA = (EMA_ALPHA * rawCurrent) + ((1.0f - EMA_ALPHA) * filteredCurrent_mA);
        }
        
        // After the 2000ms inrush blanking window expires, evaluate direct dry-run threshold check
        if (elapsedPumping >= INRUSH_BLANKING_MS) {
          if (filteredCurrent_mA < DRY_RUN_THRESHOLD_MA) {
            systemLog("[ERROR] Dry-run detected! Filtered current below 50mA threshold (%.2fmA). Aborting watering cycle!\r\n", filteredCurrent_mA);
            digitalWrite(PUMP_PIN, LOW);
            currentState = STATE_FAULT;
            break;
          }
        }
      }

      // Check if 60s watering duration is complete
      if (currentMillis - wateringStartMs >= WATERING_DURATION_MS) {
        systemLog("[SYSTEM] Watering cycle complete (60s). Shutting off pump.\r\n");
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
      systemLog("[FAULT] SYSTEM FAULT LOCKOUT: Check water reservoir and INA219 sensor.\r\n");
      delay(2000);
      break;
    }
  }

  server.handleClient();
  delay(100);
}

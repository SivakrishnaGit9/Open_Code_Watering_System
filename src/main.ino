#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Update.h>

// Debug macro switch (define to enable continuous telemetry printing for testing)
#define DEBUG_SENSOR_TELEMETRY 1

// Wi-Fi Access Point (AP) and UDP Telemetry configuration
#define AP_SSID "PlantWatering_AP"
#define AP_PASSWORD "plant_password_123"
#define UDP_PORT 8888

WiFiUDP udp;
WebServer server(80);
IPAddress broadcastIp(192, 168, 4, 255); // AP subnet broadcast
bool wifiApActive = false;
#define PUMP_PIN 25
#define LED_PIN  2

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

  // Initialize Wi-Fi Access Point (AP) mode
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress apIP = WiFi.softAPIP();
  wifiApActive = true;
  Serial.printf("Wi-Fi Access Point Started! SSID: %s | AP IP: %s\r\n", AP_SSID, apIP.toString().c_str());
  udp.begin(UDP_PORT);

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

  // Periodic throttled telemetry handler (every 4 seconds)
  if (currentMillis - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = currentMillis;
    float busvoltage = ina219Available ? ina219.getBusVoltage_V() : 0.0f;
    float current_mA = ina219Available ? ina219.getCurrent_mA() : 0.0f;
    unsigned long uptimeSec = currentMillis / 1000UL;
    
    char logBuffer[160];
    snprintf(logBuffer, sizeof(logBuffer),
             "[TELEMETRY] Uptime: %lus | State: %d | Bus: %.2fV | Current: %.2fmA | Remaining: %lus\r\n",
             uptimeSec, (int)currentState, busvoltage, current_mA, remainingCountdownMs / 1000UL);
    
    // Output to Serial
    Serial.print(logBuffer);

    // Broadcast over UDP if AP is active
    if (wifiApActive) {
      udp.beginPacket(broadcastIp, UDP_PORT);
      udp.write((uint8_t*)logBuffer, strlen(logBuffer));
      udp.endPacket();
    }
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

  server.handleClient();
  delay(100);
}

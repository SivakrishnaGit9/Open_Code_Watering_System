/*
 * ESP32 Minimal Board Test Sketch
 * Target: ESP32 NodeMCU / DevKit
 * Purpose: Verify basic board health, serial communication, and GPIO toggling.
 */

#define LED_PIN 2  // On-board LED on standard ESP32 dev boards

void setup() {
  // Initialize Serial Monitor at 115200 baud
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("ESP32 Board Test Initialized");
  Serial.println("=================================");
  
  // Print ESP32 chip diagnostics
  Serial.printf("Model: %s\n", ESP.getChipModel());
  Serial.printf("Cores: %d\n", ESP.getChipCores());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());

  // Configure on-board LED pin as output
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Turn LED ON and log status
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ON - Board is healthy");
  delay(1000);

  // Turn LED OFF and log status
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF");
  delay(1000);
}

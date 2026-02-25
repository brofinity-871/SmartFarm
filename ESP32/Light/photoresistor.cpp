#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
// CONFIGURATION
const char* ssid = "*****";
const char* password = "*****";
const char* serverURL = "****/api/lux_snapshot"; // 

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8 * 3600;   // Sacramento, CA
const int daylightOffset_sec = 3600;

// PIN DEFINITIONS
const int pinLED = 26;
const int pinLDR = 34;

// STATE MACHINE
enum State {BOOT, SYNC, ROUTINE, ON_DEMAND, ERROR};
State currentState = BOOT;

// TIMING
const unsigned long sampleInterval = 500;
const unsigned long reportInterval = 120000;

unsigned long lastSampleTime = 0;
unsigned long lastReportTime = 0;

// ACCUMULATORS
long sumLight = 0;
int sampleCount = 0;
int minLight = 4095;
int maxLight = 0;

// LED CONTROL
bool led_on = false;
int blinkCount = 0;
int blinkStep = 0;
unsigned long lastLEDTime = 0;
struct LEDPattern {
  int steps;  
  int stepInterval;
  int rest;
};
LEDPattern patterns[] = {
  {1, 500, 1000}, // BOOT
  {2, 250, 500},  // SYNC
  {1, 500, 500},  // ROUTINE
  {1, 100, 100},  // ON_DEMAND
  {4, 150, 500}   // ERROR
};
void updateLEDs(unsigned long now) {
  LEDPattern p = patterns[currentState];

  if (now - lastLEDTime >= p.stepInterval){
    lastLEDTime = now;
    if (p.stepInterval > 0) {
      led_on = !led_on;
      digitalWrite(pinLED, led_on);
      blinkStep++;
    }
  
    // If finished group of blinks, pause before repeating
    if (blinkStep >= p.steps * 2) { 
      blinkStep = 0;
      lastSampleTime += p.rest;
      digitalWrite(pinLED, LOW);
    }
  }
}

// TIME FUNCTION
String getTimeStamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "TIME_NOT_SYNCED";

  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// SENSOR SAMPLING
void sampleLight(unsigned long now) {
  if (now - lastSampleTime >= sampleInterval) {
    lastSampleTime = now;
    
    int lightValue = analogRead(pinLDR);
    Serial.print("LDR: ");
    Serial.println(lightValue);

    sumLight += lightValue;
    sampleCount++;

    if (lightValue < minLight) minLight = lightValue;
    if (lightValue > maxLight) maxLight = lightValue;
  }
}

// HTTP 
void sendSnapshot(float meanLux, int minLux, int maxLux, int count) {
  if (WiFi.status() != WL_CONNECTED) {
    currentState = ERROR;
    Serial.println("Failed to send report");
    return;
  }

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  char payload[256];
  snprintf(payload, sizeof(payload), 
           "{\"esp_id\":\"ESP32_ROOM1\","
           "\"last_snap\":\"%s\","
           "\"mean_lux\":%.2f,"
           "\"min_lux\":%d,"
           "\"max_lux\":%d,"
           "\"sample_count\":%d}",
           getTimeStamp().c_str(),
           meanLux, 
           minLux,
           maxLux,
           count);

  int responseCode = http.POST(payload);

  if (responseCode <= 0) {
    currentState = ERROR;
    Serial.println("HTTP Post failed");
  }
  else{
    Serial.println("Report Sent");
  }
  http.end();
}

// SNAPSHOT REPORT
void reportLuxSnapshot(unsigned long now) {
  if (now - lastReportTime >= reportInterval && sampleCount > 0) {
    lastReportTime = now;
    float meanLight = (float)sumLight / sampleCount;

    sendSnapshot(meanLight, minLight, maxLight, sampleCount);
    // reset accumulators
    sumLight = 0;
    sampleCount = 0;
    minLight = 4095;
    maxLight = 0;
  }
}

// WIFI + NTP
void connectWiFi() {
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  pinMode(pinLDR, INPUT);

  currentState = BOOT;
  connectWiFi();
  currentState = SYNC;
  setupTime();

  currentState = ROUTINE;  
}

void loop() {
  unsigned long now = millis();
  if (currentState == ROUTINE) {
    sampleLight(now);
    reportLuxSnapshot(now);
  }
  updateLEDs(now);
}

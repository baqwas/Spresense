/**
 * @file        02-gnss-acquisition.ino
 * @author      Matha Goram
 * @copyright   ParkCircus Productions
 * @brief       GNSS Satellite Acquisition & Signal Health Evaluation
 * * @section DESCRIPTION
 * This evaluation project validates the high-performance GNSS (Global Navigation 
 * Satellite System) capabilities of the Sony Spresence. It attempts to 
 * initialize the satellite receiver and scan for visible satellites.
 * * @section ASSUMPTIONS
 * 1. The device has a clear view of the sky (or is near a window).
 * 2. The Spresence GNSS firmware was updated during the "Burn Bootloader" phase.
 * * @section PREREQUISITES
 * - Hardware: Sony Spresence Main Board.
 * - Library: GNSS library (Standard Sony Spresence SDK).
 * * @section LICENSE
 * MIT License
 * Copyright (c) 2026 ParkCircus Productions
 */

// Disable PROGMEM to avoid the pgm_read_ptr compilation error on Spresense
#define ARDUINOJSON_ENABLE_PROGMEM 0 

#include <ArduinoJson.h>
#include <GNSS.h>

SpGnss Gnss;

// LED Pins for visual feedback
const int LED_HEARTBEAT = LED0;
const int LED_SIGNAL    = LED1;
const int LED_FIX       = LED3;

void setup() {
  // Use a fast baud rate to ensure JSON strings don't clog the buffer
  Serial.begin(115200);
  
  // Allow time for Serial to stabilize
  delay(3000); 

  pinMode(LED_HEARTBEAT, OUTPUT);
  pinMode(LED_SIGNAL,    OUTPUT);
  pinMode(LED_FIX,       OUTPUT);

  if (Gnss.begin() != 0) {
    // Rapid flash LED0 if hardware fails
    while(1) {
      digitalWrite(LED_HEARTBEAT, HIGH); delay(100);
      digitalWrite(LED_HEARTBEAT, LOW);  delay(100);
    }
  }

  Gnss.select(GPS);
  Gnss.select(GLONASS);
  Gnss.start(COLD_START);
}

void loop() {
  static bool heart = false;
  digitalWrite(LED_HEARTBEAT, heart = !heart);

  if (Gnss.waitUpdate(1000)) {
    SpNavData NavData;
    Gnss.getNavData(&NavData);

    // Update Status LEDs
    digitalWrite(LED_SIGNAL, (NavData.numSatellites > 0) ? HIGH : LOW);
    digitalWrite(LED_FIX,    (NavData.posDataExist) ? HIGH : LOW);

    // Create the JSON Document
    StaticJsonDocument<256> doc;

    // Node-RED will overwrite this timestamp with its own system clock
    doc["Waqt"] = "0"; 
    doc["Device"] = "Spresense-Alpha";

    // Location Hierarchy
    JsonObject location = doc.createNestedObject("Location");
    location["lat"] = NavData.latitude;
    location["lon"] = NavData.longitude;
    location["alt"] = NavData.altitude;

    // Signal Metadata
    JsonObject signal = doc.createNestedObject("Signal");
    signal["satellites"] = NavData.numSatellites;
    signal["fix_type"]   = NavData.posDataExist ? "3D" : "No Fix";
    
    // Send JSON to Serial Port
    serializeJson(doc, Serial);
    Serial.println(); // Crucial for Node-RED to detect end-of-line
  }
}
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
 * {"Waqt":"0","Device":"02-gnss-acquisition",
    "Location":{"lat":2.572432626e-11,"lon":-2.577521514e-11,"alt":-18},
    "Signal":{"satellites":3,"fix_type":"No Fix","used":0,"pdop":-1,"hdop":-1,"vdop":-1}}
 * {"Waqt":798072,"Device":"02-gnss-acquisition","SatelliteId":31,"SatelliteType":1,"CN0":26.47,"Used":false}}
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

// Define a simple structure to pair the ID with a human-readable name
struct GnssConstellation {
  uint32_t id;
  const char* name;
};

void setupGnss() {
  // 1. Define the list of satellites to iterate through
  /**
   * SpGnss E: Failed to set satellite
   * Error selecting GALILEO satellites
   * SpGnss E: Failed to set satellite
   * Error selecting BEIDOU satellites
   */

  GnssConstellation systems[] = {
    {GPS,     "GPS"},
    {GLONASS, "GLONASS"},
    {GALILEO, "GALILEO"},
    {BEIDOU,  "BEIDOU"}
  };

  // 2. Iterate through the list using a range-based for loop
  for (const auto& system : systems) {
    int return_code = Gnss.select(system.id);
    /*
    if (return_code != 0) {
      Serial.print("Error selecting ");
      Serial.print(system.name);
      Serial.println(" satellites");
    } else {
      Serial.print(system.name);
      Serial.println(" selected successfully.");
    }
     */
    delay(500); // trying to understand why GPS gets skipped in Serial Monitor window
  }
}

void sendJsonOutput(char* device_name) {

    SpNavData NavData;
    Gnss.getNavData(&NavData);

    // Update Status LEDs
    digitalWrite(LED_SIGNAL, (NavData.numSatellites > 0) ? HIGH : LOW);
    digitalWrite(LED_FIX,    (NavData.posDataExist) ? HIGH : LOW);

    // Create the JSON Document
    StaticJsonDocument<512> parent;

    // Node-RED will overwrite this timestamp with its own system clock
    parent["Waqt"] = 0; 
    parent["Device"] = device_name;

    // Location Hierarchy
    JsonObject location = parent.createNestedObject("Location");
    location["lat"] = NavData.latitude;
    location["lon"] = NavData.longitude;
    location["alt"] = NavData.altitude;

    // Signal Metadata
    JsonObject signal = parent.createNestedObject("Signal");
    signal["satellites"] = NavData.numSatellites;
    signal["fix_type"]   = NavData.posDataExist ? "3D" : "No Fix";
    signal["used"]       = NavData.numSatellitesCalcPos; // How many actually used
    // --- Added DOP Values Here ---
    signal["pdop"] = NavData.pdop; // Total 3D accuracy
    signal["hdop"] = NavData.hdop; // Horizontal accuracy (Lat/Lon)
    signal["vdop"] = NavData.vdop; // Vertical accuracy (Altitude)
    // Send JSON to Serial Port
    serializeJson(parent, Serial);
    // Serial.println(); // Crucial for Node-RED to detect end-of-line

    for (int i = 0; i < NavData.numSatellites; i++) {
      StaticJsonDocument<128> snr_doc;
      // Collect Data
      snr_doc["Waqt"] = millis();
      snr_doc["Device"] = device_name;
      
      // Accessing ID and Type
      snr_doc["SatelliteId"] = NavData.getSatelliteId(i);
      snr_doc["SatelliteType"] = NavData.getSatelliteType(i);
      snr_doc["CN0"]  = NavData.getSatelliteSignalLevel(i);
      
      // Bit 0 of the status indicates if the satellite is used for positioning
      // (status & 0x01) will be 1 if used, 0 if not.
      snr_doc["Used"] = (i < NavData.numSatellitesCalcPos);

      // Print as JSON Payload
      serializeJson(snr_doc, Serial);
      // Serial.println(); // Crucial for Node-RED to detect end-of-line
    }

}

void setup() {
  // Use a fast baud rate to ensure JSON strings don't clog the buffer
  // Serial.begin(115200);
  
  // Allow time for Serial to stabilize
  delay(3000); 

  pinMode(LED_HEARTBEAT, OUTPUT);
  pinMode(LED_SIGNAL,    OUTPUT);
  pinMode(LED_FIX,       OUTPUT);

// 1. Initialize the GNSS hardware
  int ret = Gnss.begin();
  if (ret != 0) {
    // Serial.print(ret);
    // Serial.println(" <- Gnss.begin() failed!");
    digitalWrite(LED_HEARTBEAT, HIGH); 
    return;
  }

  setupGnss();
  // 4. Start positioning
  ret = Gnss.start(COLD_START);
  if (ret != 0) {
    // Serial.print(ret);
    // Serial.println(" <- Gnss.start() failed!");
  } else {
    // Serial.println("GNSS Acquisition started (Cold Start)...");
  }
}

void loop() {
  char* DEVICE_NAME = "02-gnss-acquisition";
  static bool heart = false;
  digitalWrite(LED_HEARTBEAT, heart = !heart);

  if (Gnss.isUpdate()) {
    sendJsonOutput(DEVICE_NAME);
  }
}
/**
 * @file      02_gnss_expansion.ino
 * @author    Matha Goram
 * @brief     Phase 1: High-Precision Timing & Satellite Metadata Expansion.
 * @version   2.0.5 (Direct Member Mapping Fix)
 * @date      2026-01-15
 * * ============================================================================
 * COPYRIGHT NOTICE:
 * Copyright (c) 2026. All rights reserved.
 * Distributed under the MIT License.
 * ============================================================================
 * * PROCESSING WORKFLOW:
 * 1. INITIALIZATION: Configures Serial at 115200 baud, verifies the MicroSD 
 * filesystem, and prepares the telemetry CSV file header.
 * 2. GNSS CONFIGURATION: Initializes the CXD5602 engine with GPS and GLONASS.
 * 3. REMOTE INTERRUPT: Listens for Serial 'R' to initiate an SD card data dump.
 * 4. DATA ACQUISITION: 
 * a. Synchronizes with satellite atomic clocks for nanosecond timestamps.
 * b. Iterates through the navigation data satellite inventory.
 * c. Maps raw signal levels and orbital positions to the JSON stream.
 * 5. MULTI-CHANNEL OUTPUT: Simultaneous JSON (Serial) and CSV (SD Card) logging.
 * * OUTPUT FORMATS:
 * - JSON (Stream): {"UTC": "...", "Location": {...}, "Satellites": [...], "Count": X}
 * - CSV (Archive): Timestamp_UTC, Satellites, Latitude, Longitude, Avg_SNR
 * * HARDWARE: Sony Spresense (Main + Extension), Active Antenna, 32GB MicroSD.
 */

#include <GNSS.h>
#include <Storage.h>

/* --- Pin Definitions --- */
#define LED_SYSTEM_READY LED0   // Solid when GNSS is active
#define LED_SD_ACTIVITY  LED3   // Flashes during SD write operations

/* --- Configuration Constants --- */
const char* LOG_FILENAME = "telemetry_v2.csv";
const int SERIAL_BAUD    = 115200;

/* --- Global Objects --- */
SpGnss Gnss;

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial); 

  pinMode(LED_SYSTEM_READY, OUTPUT);
  pinMode(LED_SD_ACTIVITY, OUTPUT);

  // 1. Storage Verification
  File root = Storage.open("/");
  if (!root) {
    Serial.println("{\"level\":\"error\",\"msg\":\"SD_NOT_MOUNTED\"}");
    for(int i=0; i<5; i++) {
      digitalWrite(LED_SD_ACTIVITY, HIGH); delay(80);
      digitalWrite(LED_SD_ACTIVITY, LOW);  delay(80);
    }
  } else {
    root.close();
    File myFile = Storage.open(LOG_FILENAME, FILE_WRITE);
    if (myFile) {
      if (myFile.size() == 0) {
        myFile.println("Timestamp_UTC,Satellites,Latitude,Longitude,Avg_SNR");
      }
      myFile.close();
    }
  }

  // 2. GNSS Initialization
  if (Gnss.begin() != 0) {
    Serial.println("{\"level\":\"error\",\"msg\":\"GNSS_INIT_FAIL\"}");
  } else {
    Gnss.select(GPS);
    Gnss.select(GLONASS);
    Gnss.start(COLD_START);
    digitalWrite(LED_SYSTEM_READY, HIGH);
    Serial.println("{\"level\":\"info\",\"msg\":\"PHASE_1_LIVE\"}");
  }
}

void loop() {
  // --- 1. REMOTE LOG RETRIEVAL TRIGGER ---
  if (Serial.available() > 0) {
    if (Serial.read() == 'R') {
      digitalWrite(LED_SYSTEM_READY, LOW); 
      File dataFile = Storage.open(LOG_FILENAME, FILE_READ);
      if (dataFile) {
        Serial.println("\n---BEGIN_DUMP---");
        while (dataFile.available()) { Serial.write(dataFile.read()); }
        dataFile.close();
        Serial.println("---END_DUMP---");
      }
      digitalWrite(LED_SYSTEM_READY, HIGH);
    }
  }

  // --- 2. ENHANCED TELEMETRY ACQUISITION ---
  if (Gnss.waitUpdate(-1)) {
    SpNavData NavData;
    Gnss.getNavData(&NavData);

    // High-Precision ISO-8601 Timestamp
    char utcTs[35]; 
    sprintf(utcTs, "%04d-%02d-%02dT%02d:%02d:%02d.%09ldZ",
            NavData.time.year, NavData.time.month, NavData.time.day,
            NavData.time.hour, NavData.time.minute, NavData.time.sec,
            (long)NavData.time.usec * 1000);

    // --- 3. CONSTRUCT JSON PAYLOAD ---
    Serial.print("{");
    Serial.print("\"UTC\":\"");      Serial.print(utcTs);                Serial.print("\",");
    Serial.print("\"Location\":{");  Serial.print("\"lat\":");           Serial.print(NavData.latitude, 6);
    Serial.print(",\"lon\":");       Serial.print(NavData.longitude, 6); Serial.print("},");
    
    Serial.print("\"Satellites\":[");
    float totalSnr = 0;
    int satCount = NavData.numSatellites;

    for (int i = 0; i < satCount; i++) {
      // NOTE: Sony Spresense library uses 'sigLevel' for SNR and 'svid' for PRN
      float currentSnr = NavData.satellite[i].sigLevel;
      totalSnr += currentSnr;

      Serial.print("{");
      Serial.print("\"id\":");    Serial.print(NavData.satellite[i].svid);
      Serial.print(",\"snr\":");  Serial.print(currentSnr, 1);
      Serial.print(",\"el\":");   Serial.print(NavData.satellite[i].elevation);
      Serial.print(",\"az\":");   Serial.print(NavData.satellite[i].azimuth);
      Serial.print("}");
      
      if (i < satCount - 1) Serial.print(",");
    }
    Serial.print("],");
    Serial.print("\"Count\":"); Serial.print(satCount);
    Serial.println("}");

    // --- 4. DATA PERSISTENCE (SD Storage) ---
    File csvFile = Storage.open(LOG_FILENAME, FILE_WRITE);
    if (csvFile) {
      digitalWrite(LED_SD_ACTIVITY, HIGH);
      csvFile.print(String(utcTs) + ",");
      csvFile.print(String(satCount) + ",");
      csvFile.print(String(NavData.latitude, 6) + ",");
      csvFile.print(String(NavData.longitude, 6) + ",");
      
      float avgSnr = (satCount > 0) ? (totalSnr / satCount) : 0;
      csvFile.println(String(avgSnr, 2)); 
      
      csvFile.close();
      digitalWrite(LED_SD_ACTIVITY, LOW);
    }
  }
}
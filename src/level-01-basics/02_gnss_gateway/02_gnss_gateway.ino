/**
 * @file      Spresense_GNSS_Gateway.ino
 * @author    Matha Goram
 * @brief     Dual-mode GNSS Telemetry Gateway & Blackbox Recorder.
 * @version   1.0.3 (Evaluation 03)
 * @date      2026-01-14
 * * @copyright Copyright (c) 2026
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * * HARDWARE SETUP:
 * - Sony Spresense Main Board + Extension Board.
 * - Active GNSS Antenna connected to Main Board.
 * - MicroSD Card (FAT32) in Extension Board slot.
 * * DATA SCHEMA:
 * - Serial Out: JSON string for Ubuntu/MQTT Gateway.
 * - SD Card:    CSV rows for offline data analysis.
 */

#include <GNSS.h>
#include <Storage.h>

/* --- Pin Definitions --- */
#define LED_SYSTEM_READY LED0   // Green/Blue: App initialized
#define LED_SD_ACTIVITY  LED3   // Flash: Physical disk write

/* --- Global Objects & Constants --- */
SpGnss Gnss;
const char* LOG_FILENAME = "telemetry.csv";
const int SERIAL_BAUD    = 115200;

/**
 * @brief Initializes serial, storage, and GNSS hardware.
 */
void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial); 

  pinMode(LED_SYSTEM_READY, OUTPUT);
  pinMode(LED_SD_ACTIVITY, OUTPUT);

  // Initialize SD Storage
  // Spresense Storage is auto-initialized by the OS. 
  // We check if it is ready by attempting to open the root directory.
  File root = Storage.open("/");
  if (!root) {
    Serial.println("{\"level\":\"error\",\"msg\":\"SD_NOT_MOUNTED\"}");
    for(int i=0; i<5; i++) {
      digitalWrite(LED_SD_ACTIVITY, HIGH); delay(80);
      digitalWrite(LED_SD_ACTIVITY, LOW);  delay(80);
    }
  } else {
    root.close();
    
    // Check if header is required
    File myFile = Storage.open(LOG_FILENAME, FILE_WRITE);
    if (myFile) {
      if (myFile.size() == 0) {
        myFile.println("Timestamp_UTC,Satellites,Latitude,Longitude");
      }
      myFile.close();
    }
  }

  // Initialize GNSS Engine
  if (Gnss.begin() != 0) {
    Serial.println("{\"level\":\"error\",\"msg\":\"GNSS_INIT_FAIL\"}");
  } else {
    Gnss.select(GPS);
    Gnss.select(GLONASS);
    Gnss.start(COLD_START);
    digitalWrite(LED_SYSTEM_READY, HIGH);
    Serial.println("{\"level\":\"info\",\"msg\":\"SYSTEM_LIVE\"}");
  }
}

/**
 * @brief Main execution loop handling data acquisition and Serial triggers.
 */
void loop() {
  // --- 1. REMOTE LOG RETRIEVAL TRIGGER ---
  if (Serial.available() > 0) {
    if (Serial.read() == 'R') {
      digitalWrite(LED_SYSTEM_READY, LOW); 
      File dataFile = Storage.open(LOG_FILENAME, FILE_READ);
      if (dataFile) {
        Serial.println("\n---BEGIN_DUMP---");
        while (dataFile.available()) {
          Serial.write(dataFile.read());
        }
        dataFile.close();
        Serial.println("---END_DUMP---");
      }
      digitalWrite(LED_SYSTEM_READY, HIGH);
    }
  }

  // --- 2. TELEMETRY ACQUISITION ---
  if (Gnss.waitUpdate(-1)) {
    SpNavData NavData;
    Gnss.getNavData(&NavData);

    // Format ISO-8601 Timestamp from Satellite Atomic Clock
    char utcTs[25];
    sprintf(utcTs, "%04d-%02d-%02dT%02d:%02d:%02dZ",
            NavData.time.year, NavData.time.month, NavData.time.day,
            NavData.time.hour, NavData.time.minute, NavData.time.sec);

    // Build JSON Message for MQTT Gateway
    Serial.print("{");
    Serial.print("\"UTC\":\"");      Serial.print(utcTs);                Serial.print("\",");
    Serial.print("\"Signal\":{");    Serial.print("\"satellites\":");    Serial.print(NavData.numSatellites); Serial.print("},");
    Serial.print("\"Location\":{");  Serial.print("\"lat\":");           Serial.print(NavData.latitude, 6); 
    Serial.print(",\"lon\":");       Serial.print(NavData.longitude, 6); Serial.print("}");
    Serial.println("}");

    // Log to SD Blackbox (CSV)
    File csvFile = Storage.open(LOG_FILENAME, FILE_WRITE);
    if (csvFile) {
      digitalWrite(LED_SD_ACTIVITY, HIGH);
      csvFile.print(String(utcTs) + ",");
      csvFile.print(String(NavData.numSatellites) + ",");
      csvFile.print(String(NavData.latitude, 6) + ",");
      csvFile.println(String(NavData.longitude, 6));
      csvFile.close();
      digitalWrite(LED_SD_ACTIVITY, LOW);
    }
  }
}
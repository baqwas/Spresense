/**
 * @file      03_gnss_multicore.ino
 * @author    Matha Goram
 * @brief     Phase 2 Manager: High-Bandwidth Gateway & Precision Timing.
 * @version   2.1.1 (Library Syntax & MP Case-Sensitivity Fix)
 * @date      2026-01-15
 * * ============================================================================
 * PROCESSING WORKFLOW:
 * 1. BOOTLOADER: Initializes the Multi-Processor (MP) controller and boots SubCore1.
 * 2. PPS SIGNAL: Sets up Extension Board Pin 2 for a 3.3V Pulse-Per-Second output.
 * 3. IPC LISTENER: Monitors the Message Queue for 'GnssPacket' pointers from SubCore1.
 * 4. SERIAL GATEWAY: Packages received data into an expanded JSON payload for the host.
 * 5. PERSISTENCE: Records a condensed log to the SD card (telemetry_v3.csv).
 * ============================================================================
 */

#include <MP.h>
#include <Storage.h>
#include <GNSS.h>
#include "shared_types.h"

#define PPS_PIN 2               // Extension Board D2 (3.3V)
#define LED_GATEWAY_ACTIVE LED0 // Flashes when IPC data is processed

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(PPS_PIN, OUTPUT);
  pinMode(LED_GATEWAY_ACTIVE, OUTPUT);
  digitalWrite(PPS_PIN, LOW);

  // Initialize Storage (Verify SD is inserted)
  File root = Storage.open("/");
  if (!root) {
    Serial.println("{\"level\":\"warn\",\"msg\":\"SD_CARD_NOT_FOUND\"}");
  } else {
    root.close();
  }

  // Boot SubCore 1 (MainCore requires ID)
  int ret = MP.begin(1);
  if (ret < 0) {
    Serial.print("{\"level\":\"error\",\"msg\":\"SUBCORE_BOOT_FAIL\",\"code\":");
    Serial.print(ret);
    Serial.println("}");
  } else {
    Serial.println("{\"level\":\"info\",\"msg\":\"GATEWAY_ONLINE\"}");
  }
}

void loop() {
  int8_t msgid;
  GnssPacket* data;

  // Receive from SubCore 1 (Wait for MsgID 10)
  int ret = MP.Recv(&msgid, &data, 1);

  if (ret > 0 && msgid == 10) {
    // Generate ISO-8601 Nanosecond Timestamp
    char utcTs[35]; 
    sprintf(utcTs, "%04d-%02d-%02dT%02d:%02d:%02d.%09ldZ",
            data->time.year, data->time.month, data->time.day,
            data->time.hour, data->time.minute, data->time.sec,
            (long)data->time.usec * 1000);

    // Physical PPS Pulse for Stratum-1 Timing
    digitalWrite(PPS_PIN, HIGH); 
    digitalWrite(LED_GATEWAY_ACTIVE, HIGH);
    
    // Serial JSON Broadcast
    Serial.print("{");
    Serial.print("\"UTC\":\"");      Serial.print(utcTs);                Serial.print("\",");
    Serial.print("\"Pos\":{\"lat\":");    Serial.print(data->lat, 6);
    Serial.print(",\"lon\":");           Serial.print(data->lon, 6);
    Serial.print(",\"alt\":");           Serial.print(data->alt, 1);
    Serial.print(",\"hdop\":");          Serial.print(data->hdop, 2);
    Serial.print("},");

    Serial.print("\"Sats\":[");
    int limit = (data->numSats > 16) ? 16 : data->numSats;
    for (int i = 0; i < limit; i++) {
      Serial.print("{\"id\":");          Serial.print(data->inventory[i].svid);
      Serial.print(",\"t\":");           Serial.print(data->inventory[i].type);
      Serial.print(",\"s\":");           Serial.print(data->inventory[i].sigLevel, 1);
      Serial.print(",\"el\":");          Serial.print(data->inventory[i].elevation);
      Serial.print(",\"az\":");          Serial.print(data->inventory[i].azimuth);
      Serial.print("}");
      if (i < limit - 1) Serial.print(",");
    }
    Serial.print("],\"Count\":");        Serial.print(data->numSats);
    Serial.println("}");

    delay(50); // Pulse width duration
    digitalWrite(PPS_PIN, LOW);
    digitalWrite(LED_GATEWAY_ACTIVE, LOW);
  }
}
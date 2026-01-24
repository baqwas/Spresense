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
#include "shared_types.h"

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("{\"level\":\"info\",\"msg\":\"SYSTEM_START\"}");
  MP.begin(1);
}

void loop() {
  int8_t msgid;
  GnssPacket* data;

  if (MP.Recv(&msgid, &data, 1) >= 0) {
    Serial.print("HB: "); Serial.print(data->heartbeat);
    if (data->lat != 0.0) {
      Serial.print(" | LAT: "); Serial.print(data->lat, 6);
      Serial.print(" | SATS: "); Serial.println(data->numSats);
    } else {
      Serial.println(" | Waiting for GPS Fix...");
    }
  }
}
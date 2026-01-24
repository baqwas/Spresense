/**
 * @file      subcore1.ino
 * @author    Matha Goram
 * @brief     Phase 2 Worker: GNSS Hardware Polling & IPC Data Transmission.
 * @version   2.1.2 (SubCore API & GNSS Structure Fix)
 * @date      2026-01-15
 * * ============================================================================
 * WORKFLOW:
 * 1. INITIALIZATION: Registers as SubCore 1 and initializes the GNSS engine.
 * 2. POLLING: Monitors the CXD5602 hardware for position updates.
 * 3. PACKAGING: Copies raw hardware registers into the shared 'GnssPacket'.
 * 4. IPC SEND: Forwards the packet address to MainCore (ID 0) with MsgID 10.
 * ============================================================================
 */

#include <GNSS.h>
#include <MP.h>
#include "shared_types.h"

SpGnss Gnss;
static GnssPacket sharedPacket __attribute__((section(".shared_memory")));
uint32_t counter = 0;
bool gnss_active = false;

void setup() {
  MP.begin();
  pinMode(LED1, OUTPUT);

  // Reset GNSS hardware
  Gnss.end();
  delay(1000);

  if (Gnss.begin() == 0) {
    Gnss.select(GPS);
    Gnss.select(GLONASS);
    if (Gnss.start(COLD_START) == 0) {
      gnss_active = true;
    }
  }
}

void loop() {
  sharedPacket.heartbeat = counter++;
  
  if (gnss_active && Gnss.waitUpdate(100)) {
    SpNavData NavData;
    Gnss.getNavData(&NavData);
    sharedPacket.lat = NavData.latitude;
    sharedPacket.lon = NavData.longitude;
    sharedPacket.hdop = NavData.hdop;
    sharedPacket.numSats = NavData.numSatellites;
    sharedPacket.sec = NavData.time.sec;
    
    digitalWrite(LED1, HIGH); // Solid flash = GPS Fix/Update
  } else {
    // No GPS data yet, but keep the heartbeat moving
    sharedPacket.lat = 0.0; 
    digitalWrite(LED1, LOW);
  }

  GnssPacket* ptr = &sharedPacket;
  MP.Send(10, ptr, 0); 
  
  delay(900); 
}
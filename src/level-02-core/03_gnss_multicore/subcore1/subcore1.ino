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
static GnssPacket sharedPacket;

void setup() {
  MP.begin();
  
  // Milestone LEDs
  pinMode(LED1, OUTPUT); digitalWrite(LED1, HIGH); 

  int result = Gnss.begin(); 
  if (result != 0) {
    // If this fails, the hardware isn't responding
    return;
  }
/* * ANTENNA SELECTION
   * 0: Automatic (Attempts to detect)
   * 1: Internal (The white chip antenna on the board) <-- TRY THIS
   * 2: External (The U.FL connector)
   */
  // Gnss.setAntennaSelection(1);
  // --- Constellation Diversity ---
  // Enable a wider variety of constellations to overcome the wall blockage
  Gnss.select(GPS); // (0): GPS + QZSS (Michibiki)
  Gnss.select(GLONASS); // (1): GLONASS (Russian)
  Gnss.select(SBAS);   // Improves accuracy

  // These constants are often missing in the header, but the chip hears them:
  Gnss.select(6); // BeiDou (Chinese - massive constellation)
  Gnss.select(7); // Galileo (European - very high precision)

  // WARM_START: Uses saved almanac but re-acquires signals.
  // This is the most resilient mode for your 15ft wall environment.
  // int result = Gnss.start(WARM_START);
  result = Gnss.start(COLD_START);
  
  if (result != 0) {
    // If Warm Start fails (memory corrupted), fallback to Cold Start
    Gnss.start(COLD_START);
  }
}

void loop() {
  // Non-blocking check (10ms timeout)
  bool hasUpdate = Gnss.waitUpdate(10); 

  if (hasUpdate) {
    SpNavData NavData;
    Gnss.getNavData(&NavData);
    
    sharedPacket.lat = NavData.latitude;
    sharedPacket.lon = NavData.longitude;
    sharedPacket.alt = NavData.altitude;
    sharedPacket.hdop = NavData.hdop;
    sharedPacket.numSats = NavData.numSatellites;

    sharedPacket.time.year = NavData.time.year;
    sharedPacket.time.month = NavData.time.month;
    sharedPacket.time.day = NavData.time.day;
    sharedPacket.time.hour = NavData.time.hour;
    sharedPacket.time.minute = NavData.time.minute;
    sharedPacket.time.sec = NavData.time.sec;
    sharedPacket.time.usec = NavData.time.usec;
  } else {
    // If no hardware data, keep the heartbeat flowing with a marker value
    sharedPacket.lat = 0.000002; 
    sharedPacket.hdop = 99.8;
  }

  // IPC Send is now OUTSIDE the 'if' block.
  // This prevents the Main Core from hanging at MP.Recv.
  MP.Send(10, &sharedPacket, 0);

  // Toggle LED1 to show the loop is actually running
  digitalWrite(LED1, !digitalRead(LED1));
  delay(1000); 
}
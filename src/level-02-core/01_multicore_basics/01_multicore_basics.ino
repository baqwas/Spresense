/**
 * @file SubCore1.ino
 * @brief Worker core responsible for GNSS data processing.
 */

#include <MP.h>
#include <GNSS.h>

// This structure must match exactly in the Main Core sketch
struct GnssData {
  float lat;
  float lon;
  int sats;
};

void setup() {
  // Initialize Multi-Processor communication
  int ret = MP.begin();
  if (ret < 0) {
    // We can't use Serial.print here easily; 
    // Sub-core errors are usually handled by the Main Core.
  }
}

void loop() {
  // 1. Imagine GNSS logic happens here...
  GnssData myData;
  myData.lat = 35.123456; 
  myData.lon = 139.654321;
  myData.sats = 12;

  // 2. Send the data structure to the Main Core
  // '1' is the message ID we choose for GNSS data
  MP.send(1, &myData); 

  delay(1000); // Send data once per second
}
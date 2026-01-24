#include <MP.h>

#define SUBCORE_ID 5
#define MAIN_CORE  0
#define MSG_ID_HEALTH_REQUEST 105
#define CHECK_INTERVAL 10000 

void setup() {
  MP.begin();
}

void loop() {
  // Just send a signal to MainCore
  uint32_t dummy = 1; 
  MP.Send(MSG_ID_HEALTH_REQUEST, dummy, MAIN_CORE);
  
  delay(CHECK_INTERVAL);
}
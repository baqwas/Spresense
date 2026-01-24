/*
 *  Name  Pin           Signal
 *  unavailable
 */

#include <MP.h>

#define BLINK_PERIOD 1000
#define SUBCORE_ID 4
#define SUBCORE_LED 3

void setup()
{
  MP.begin();
}

void loop()
{
  MPLog("loop %d\n", SUBCORE_ID);
  delay(BLINK_PERIOD);
}
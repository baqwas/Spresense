/*
 *  Name  Pin           Signal
 *  LED3  I2SO_LRCK     GPO
 */

#include <MP.h>

#define BLINK_PERIOD 1000
#define SUBCORE_ID 3
#define SUBCORE_LED 2

void blink(int core_id, int blink_period)
{
  ledOn(core_id);
  delay(blink_period);
  ledOff(core_id);
  delay(blink_period);
}

void setup()
{
  MP.begin();
}

void loop()
{
  MPLog("loop %d\n", SUBCORE_ID);
  blink(SUBCORE_LED, BLINK_PERIOD);
}
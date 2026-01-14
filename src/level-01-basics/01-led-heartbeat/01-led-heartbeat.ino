/**
 * @file        01-led-heartbeat.ino
 * @author      Matha Goram
 * @copyright   ParkCircus Productions
 * @brief       Hardware Logic Validation (Universal Arduino Method)
 * * @section DESCRIPTION
 * This evaluation project validates the physical logic of the Sony Spresence 
 * Main Board using the universal digitalWrite() method. This approach was 
 * adopted to ensure robust cross-platform compatibility on Ubuntu systems 
 * where native Sony <led.h> linking may encounter path conflicts.
 * * @section WORKFLOW
 * 1. Initialize High-Speed UART (115200 bps).
 * 2. Set LED0-LED3 as OUTPUT.
 * 3. Execute sequential "Heartbeat" pulse across 4 onboard blue LEDs.
 * * @license     MIT License
 */

// Define the 4 onboard blue LEDs using standard Arduino pin numbers
const int BOARD_LED0 = LED0; 
const int BOARD_LED1 = LED1;
const int BOARD_LED2 = LED2;
const int BOARD_LED3 = LED3;

const uint32_t PULSE_INTERVAL = 150;
const uint32_t REST_INTERVAL  = 1000;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("EVALUATION 01: LED LOGIC VALIDATION (SAFE MODE)"));

  // Standard Arduino initialization for the 4 onboard LEDs
  pinMode(BOARD_LED0, OUTPUT);
  pinMode(BOARD_LED1, OUTPUT);
  pinMode(BOARD_LED2, OUTPUT);
  pinMode(BOARD_LED3, OUTPUT);
  
  Serial.println(F("STATUS: GPIO Pins Configured for Output."));
}

void loop() {
  Serial.print(F("[HEARTBEAT] Sequence Start... "));

  // Phase 1: Sequential Activation
  int leds[] = {BOARD_LED0, BOARD_LED1, BOARD_LED2, BOARD_LED3};
  
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    delay(PULSE_INTERVAL);
  }

  // Phase 2: Sequential Deactivation
  for (int i = 3; i >= 0; i--) {
    digitalWrite(leds[i], LOW);
    delay(PULSE_INTERVAL);
  }

  Serial.println(F("Verified."));
  delay(REST_INTERVAL);
}


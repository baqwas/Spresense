
/*
 *  Name  Pin           Signal
 *  LED0  PWM3          GPO
 *  LED1  I2SO_DATA_OUT GPO
 *  LED2  I2SO_DATA_IN  GPO
 *  LED3  I2SO_LRCK     GPO
 *  Return Code   Error Constant  Meaning
 *  0             SUCCESS         Operation completed successfully
 *  -2            -ENOENT         No such file or directory
 *                                The SubCore program binary could not be found
 *  -12           -ENOMEM         Out of memory
 *                                Failed to allocate memory for the task or message queue
 *  -16           -EBUSY          Device or resource busy
 *                                The core is already running or the resource is locked
 *  -19           -ENODEV         No such device
 *                                The specified SubCore ID is invalid or does not exist
 *  -22           -EINVAL         Invalid argument
 *                                Often triggered by passing an out-of-range subid (valid range is 1–5)
 *  -116          -ETIMEDOUT      Connection timed out
 *                                The SubCore was loaded but failed to respond to the boot handshake
 * * Handles Camera, 5-SubCore Boot Management, and IPC Routing.
 * * CAM_ERR constants:
 * 0   SUCCESS, -1  NOT_INITIALIZED, -2  NOT_STREEMING, 
 * -4  INVALID_PARAM, -5  OUT_OF_MEMORY, -6  TIMEOUT
 */

#include <MP.h>
#include <Camera.h>

// Define the SubCore we are talking to
const int subid = 1;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // 1. Initialize MP for SubCore 1
  // The error log shows your library needs the ID: MP.begin(subid)
  int ret = MP.begin(subid); 
  if (ret < 0) {
    Serial.print("MP.begin error: ");
    Serial.println(ret);
  }

  // 2. Initialize Camera
  Serial.println("MainCore: Initializing Camera...");
  ret = theCamera.begin();
  if (ret < 0) {
    Serial.print("Camera begin error: ");
    Serial.println(ret);
  }

  // Start streaming so the camera hardware stays active
  // The SubCore will eventually process these frames
  ret = theCamera.startStreaming(true, [](CamImage img) {
    // Keep this empty for simplicity
  });

  Serial.println("MainCore: System Ready. Listening for SubCore 1...");
}

void loop() {
  int8_t   msgid;
  uint32_t msgdata;

  // 3. Receive signal from SubCore
  // Using 'Recv' (Capitalized) as suggested by your previous error log
  int ret = MP.Recv(&msgid, &msgdata, subid);

  if (ret >= 0) {
    // If msgdata is 1, it means the SubCore detected something
    if (msgdata == 1) {
      Serial.println(">>> EVENT: Human Detected by SubCore!");
    }
  }

  delay(10); 
}
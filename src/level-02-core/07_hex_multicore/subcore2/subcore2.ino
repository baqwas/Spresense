#include <MP.h>

void setup() {
  // Initialize MP on SubCore 2
  MP.begin(); 
}

void loop() {
  int8_t msgid;
  uint32_t addr;

  // Receive the memory address from MainCore
  int ret = MP.Recv(&msgid, &addr);

  if (ret >= 0 && msgid == 10) {
    // 'addr' is the pointer to the image in the shared pool
    uint8_t* imagePixels = (uint8_t*)addr;

    // PERFORM PROCESSING HERE
    // Example: Check the brightness of the first pixel
    uint8_t firstPixel = imagePixels[0];
    
    // Send a result back to MainCore if needed
    MP.Send(1, (uint32_t)firstPixel);
  }
}
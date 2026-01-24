/**
 * @file        subcore2.ino
 * @author      Matha Goram
 * @brief       Level 03: Multicore Capture Orchestrator (SubCore 2).
 * @version     1.1.0
 * @date        2026-01-15
 * * @copyright   Copyright (c) 2026 ParkCircus Productions; All Rights Reserved
 * * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * * ---
 * * @section Prerequisites
 * - Sony Spresense Board Package (v3.4.5+)
 * - Shared Inter-Processor Communication header: "common/vision_ipc.h"
 * - Main Core (Core 0) must be flashed with "07_camera_multicore.ino"
 * * @section Description
 * This module serves as the "Decision Maker" of the distributed vision system.
 * It maintains the logic state (Start/Stop) and issues timed capture requests.
 * * @section Interface_Options
 * - INPUTS:  Receives CMD_CAM_READY, CMD_STOP_LOOP from Main Core (ID 0).
 * - OUTPUTS: Dispatches CMD_REQ_CAPTURE to Main Core (ID 0).
 * - PROTOCOL: Asynchronous Message Passing via Spresense MP Mailbox.
 * * @attention 
 * Ensure "SubCore 2" is selected in the Tools > Board menu before compilation.
 */

#include <MP.h>
#include "vision_ipc.h"

/* --- Global State Variables --- */
bool isAutoMode = false;         // Controls if the capture loop is running
uint32_t captureCount = 0;       // Counter for unique filenames
uint32_t lastCaptureTime = 0;    // Timing marker
const uint32_t CAPTURE_INTERVAL = 5000; // Trigger every 5 seconds (5000ms)

/* --- Function Prototypes --- */
void requestCapture();

void setup() {
  // 1. Initialize MP for SubCore 2
  // Even if the compiler allows 0 arguments, specifying '2' ensures
  // the core attaches to the correct mailbox.
  int ret = MP.begin();
  if (ret < 0) {
    while(1); // Halt on sync error
  }
}

void loop() {
  int8_t msgid;
  VisionMessage *inMsg;

  // 2. Check for Control Messages from Main Core (ID 0)
  // (void**)&inMsg allows the library to update our pointer address.
  int ret = MP.Recv(&msgid, (void**)&inMsg, 0);
  
  if (ret >= 0) {
    switch (inMsg->commandID) {
      
      case CMD_CAM_READY:   // Triggered by Serial '1'
        isAutoMode = true;
        break;

      case CMD_STOP_LOOP:   // Triggered by Serial '0'
        isAutoMode = false;
        break;

      case CMD_SHUTDOWN:    // Triggered by Serial 'q'
        isAutoMode = false;
        // Logic to finalize state could go here
        break;
    }
  }

  // 3. Orchestration Logic: Trigger capture based on timer
  if (isAutoMode) {
    uint32_t currentTime = millis();
    
    // Check if 5 seconds have passed since the last capture
    if (currentTime - lastCaptureTime > CAPTURE_INTERVAL) {
      requestCapture();
      lastCaptureTime = currentTime;
    }
  }
}

/**
 * @brief  Constructs a Capture Request and sends it to the Main Core.
 * @details Uses a static buffer to ensure memory stability during IPC.
 */
void requestCapture() {
  // Static ensures this memory isn't wiped when the function returns
  static VisionMessage outMsg; 
  
  outMsg.commandID = CMD_REQ_CAPTURE; // Sub2 -> Main: Take a picture
  
  // Increment counter and format the filename string
  captureCount++;
  snprintf(outMsg.fileName, sizeof(outMsg.fileName), "IMG_%03d.JPG", captureCount);

  // Send the command to the Main Core (ID 0)
  MP.Send(CMD_REQ_CAPTURE, &outMsg, 0);
}
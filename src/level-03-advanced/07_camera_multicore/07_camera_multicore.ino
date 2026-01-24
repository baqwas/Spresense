/**
 * @file        07_camera_multicore.ino
 * @author      Matha Goram
 * @brief       Level 03: Multicore Hardware I/O & IPC Server.
 * @version     1.1.0
 * @date        2026-01-15
 * * @copyright   Copyright (c) 2026 ParkCircus Productions; All Rights Reserved
 * * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * * @details     
 * This sketch serves as the central hub (Core 0) for a distributed vision 
 * pipeline on the Sony Spresense. It manages hardware peripherals (Camera, SDHCI) 
 * and routes Inter-Processor Communication (IPC) between SubCore 2 (Orchestrator) 
 * and SubCore 3 (Analyst).
 * * Architecture:
 * - Core 0 (Main): Hardware I/O Server & Serial Interface.
 * - Core 2 (Sub): Logical Timer & Capture Request Orchestrator.
 * - Core 3 (Sub): Zero-copy Image Brightness Analyst (Heartbeat on LED3).
 * * Prerequisites:
 * - Sony Spresense Board Package v3.4.5 or higher.
 * - FAT32 formatted microSD card inserted into the Spresense extension board.
 * - Shared header file "common/vision_ipc.h" present in the project tree.
 * * User Interface (Serial Monitor @ 115200 baud):
 * - Send '1': Resume/Start automatic 5-second capture loop.
 * - Send '0': Pause capture loop (Hardware remains initialized).
 * - Send 'q': Initiate safe shutdown sequence (Closes files, halts system).
 * * LED Indicators:
 * - LED 3: Pulses on SubCore 3 during image analysis.
 * - ALL LEDs ON: System safely halted; okay to remove SD card.
 */
#include <MP.h>
#include <SDHCI.h>
#include <Camera.h>
#include "vision_ipc.h"

SDClass theSD;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("🌐 MAIN CORE: Starting I/O Server...");

  // 1. Initialize Hardware
  if (!theSD.begin()) Serial.println("❌ SD Init Failed.");
  
  CamErr camErr = theCamera.begin();
  if (camErr != CAM_ERR_SUCCESS) Serial.println("❌ Camera Init Failed.");

  theCamera.setStillPictureImageFormat(
    CAM_IMGSIZE_QVGA_H,
    CAM_IMGSIZE_QVGA_V,
    CAM_IMAGE_PIX_FMT_JPG);

  // 2. Wake up the Crew
  MP.begin(2); 
  MP.begin(3);

  Serial.println("⌨️  Commands: [1] Start, [0] Pause, [q] Shutdown");
}

void loop() {
  // --- USER INPUT HANDLING ---
  if (Serial.available() > 0) {
    char c = Serial.read();
    VisionMessage ctrlMsg;

    if (c == '1') {
      Serial.println("▶️ Command: START");
      ctrlMsg.commandID = CMD_CAM_READY;
      MP.Send(CMD_CAM_READY, &ctrlMsg, 2); 
    } 
    else if (c == '0') {
      Serial.println("⏸️ Command: PAUSE");
      ctrlMsg.commandID = CMD_STOP_LOOP;
      MP.Send(CMD_STOP_LOOP, &ctrlMsg, 2); 
    }
    else if (c == 'q') {
      performSafeShutdown();
    }
  }

  // --- CORE COMMUNICATION ---
  int8_t msgid;
  VisionMessage *msg;

  // Listen to Sub2 (Capture Requests)
  if (MP.Recv(&msgid, (void**)&msg, 2) >= 0) {
    if (msg->commandID == CMD_REQ_CAPTURE) handleCaptureRequest(msg);
  }

  // Listen to Sub3 (Save Requests)
  if (MP.Recv(&msgid, (void**)&msg, 3) >= 0) {
    if (msg->commandID == CMD_SAVE_FILE) handleSaveRequest(msg);
  }
}

void handleCaptureRequest(VisionMessage *msg) {
  CamImage img = theCamera.takePicture();
  if (img.isAvailable()) {
    VisionMessage outMsg;
    outMsg.commandID  = CMD_PROCESS_IMG;
    outMsg.bufferAddr = (uint32_t)img.getImgBuff();
    outMsg.dataSize   = img.getImgSize();
    strcpy(outMsg.fileName, msg->fileName);
    
    MP.Send(CMD_PROCESS_IMG, &outMsg, 3); // Forward to the Analyst
  }
}

void handleSaveRequest(VisionMessage *msg) {
  File myFile = theSD.open(msg->fileName, FILE_WRITE);
  if (myFile) {
    myFile.write((uint8_t*)msg->bufferAddr, msg->dataSize);
    myFile.close();
    Serial.print("💾 Saved: "); Serial.println(msg->fileName);
  }
}

void performSafeShutdown() {
  Serial.println("\n🛑 SHUTDOWN SEQUENCE INITIATED...");
  
  // 1. Tell SubCores to stop (Stop the message flow)
  VisionMessage stopMsg;
  stopMsg.commandID = CMD_SHUTDOWN;
  MP.Send(CMD_SHUTDOWN, &stopMsg, 2);
  MP.Send(CMD_SHUTDOWN, &stopMsg, 3);

  // 2. Stop Camera hardware
  theCamera.end(); 
  Serial.println("📷 Camera: End.");

  // 3. Status Update
  // In Spresense SDHCI, closing the File flushes the data. 
  // No .end() is required for the SDClass.
  Serial.println("💾 SD Card: Write-buffer flushed. Safe to unplug.");

  // 4. Visual Confirmation: Light up all LEDs (Visual Halt)
  digitalWrite(LED0, HIGH); 
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH); 
  digitalWrite(LED3, HIGH);

  while(1); // Final Halt
}
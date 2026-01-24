/**
 * @file        subcore3.ino
 * @author      Matha Goram
 * @brief       Level 03: High-Performance Image Analysis Sub-Module.
 * @version     1.0.0
 * @date        2026-01-15
 * * @copyright   Copyright (c) 2026 ParkCircus Productions; All Rights Reserved
 * This software is released under the MIT License.
 * See the LICENSE file in the project root for full license text.
 * * @license     MIT License:
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * * --- MODULE OVERVIEW ---
 * This module operates as a dedicated "Analyst" within the Spresense Multicore 
 * framework. It is designed for zero-copy pixel manipulation, performing luminance 
 * calculations on image buffers provided by the Main Core (Core 0).
 * * --- PREREQUISITES ---
 * 1. Hardware: Sony Spresense (Main Board + Extension Board + Camera).
 * 2. Board Package: Sony Spresense SDK / Arduino Board Package v3.4.5+.
 * 3. Configuration: Must be flashed specifically as "SubCore 3".
 * 4. Dependencies: Requires <MP.h> and shared "vision_ipc.h" header.
 * * --- INTER-MODULE INTERFACE (IPC) ---
 * - INBOUND (Core 0 -> Core 3): 
 * - CMD_PROCESS_IMG: Receives VisionMessage containing shared RAM address.
 * - CMD_SHUTDOWN: Receives signal to halt processing and clear GPIO states.
 * - OUTBOUND (Core 3 -> Core 0):
 * - CMD_SAVE_FILE: Signals completion of analysis; requests SD persistence.
 * * --- HARDWARE RESOURCES ---
 * - LED3: Used as a "Heartbeat" indicator for active pixel processing.
 * - RAM: Operates on Shared Memory segments (0x000A0000 range typical).
 */

#include <MP.h>
#include "vision_ipc.h"

/* --- Function Prototypes --- */
float calculateBrightness(uint8_t* buffer, uint32_t size);

void setup() {
  // 1. Initialize MP for SubCore 3
  // Explicitly identifying as Core 3 for the 3.4.5 library
  int ret = MP.begin();
  if (ret < 0) {
    while(1); // Halt on hardware sync failure
  }

  // 2. Initialize Hardware Feedback (Heartbeat)
  pinMode(LED3, OUTPUT);
  digitalWrite(LED3, LOW);
}

void loop() {
  int8_t msgid;
  VisionMessage *msg;

  // 3. Listen for Image Analysis Requests from Main Core (ID 0)
  // We pass the address of our pointer (&msg) cast as (void**)
  int ret = MP.Recv(&msgid, (void**)&msg, 0);

  if (ret >= 0) {
    if (msg->commandID == CMD_PROCESS_IMG) {
      
      // VISUAL FEEDBACK: Heartbeat ON
      digitalWrite(LED3, HIGH);

      // 4. Zero-Copy Analysis
      // We are looking at the exact same RAM where the Camera driver
      // placed the data. No data was moved between cores.
      float avgBrightness = calculateBrightness((uint8_t*)msg->bufferAddr, msg->dataSize);

      // 5. Prepare Return Message
      // We copy the metadata to a local struct to send back the save command
      VisionMessage backMsg;
      backMsg.commandID = CMD_SAVE_FILE;
      backMsg.bufferAddr = msg->bufferAddr;
      backMsg.dataSize = msg->dataSize;
      strncpy(backMsg.fileName, msg->fileName, sizeof(backMsg.fileName));

      // 6. Signal Main Core to write to SD
      MP.Send(CMD_SAVE_FILE, &backMsg, 0);

      // VISUAL FEEDBACK: Heartbeat OFF (pulse length ~40ms)
      delay(40); 
      digitalWrite(LED3, LOW);
    }
    else if (msg->commandID == CMD_SHUTDOWN) {
      // Handle system-wide shutdown if necessary
      digitalWrite(LED3, LOW);
    }
  }
}

/**
 * @brief  Estimates image brightness using a sampling algorithm.
 * @param  buffer Pointer to the raw image data in shared RAM.
 * @param  size   Total size of the image buffer in bytes.
 * @return Average luminance value (0.0 to 255.0).
 */
float calculateBrightness(uint8_t* buffer, uint32_t size) {
  if (size == 0 || buffer == NULL) return 0.0f;

  uint32_t sum = 0;
  const uint32_t samplePoints = 100; // Efficient sampling
  uint32_t step = size / samplePoints;
  
  if (step == 0) step = 1;

  for (uint32_t i = 0; i < size; i += step) {
    sum += buffer[i];
  }

  return (float)sum / (float)samplePoints;
}
/**
 * @file        04_sd_multicore.ino
 * @author      Matha Goram
 * @brief       Main Core I/O Server for Multi-core SD Operations.
 * @details     Acts as the centralized hardware controller for the SDHCI 
 * interface. Listens for Inter-Processor Communication (IPC) 
 * requests from SubCores to perform file system operations, 
 * ensuring thread-safe access to the physical SD media.
 * * @target      Sony Spresense (CXD5602) - Main Core
 * @dependency  SDHCI, File, MP (Spresense Arduino Board Package)
 * @version     1.0.0
 * @date        2026-01-15
 * * @copyright   Copyright (c) 2026 ParkCircus Productions
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
 */
#include <SDHCI.h>
#include <File.h>
#include <MP.h>
#include "commands.h"

// FIX: Declare the SDHCI class instance
SDClass theSD; 

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("🚀 [Main] Starting Multi-core I/O Server...");

  // Initialize the SD card hardware (Main Core only) 
  if (theSD.begin()) {
    Serial.println("✅ [Main] SD Card initialized.");
  } else {
    Serial.println("❌ [Main] SD Card initialization failed!");
  }

  // Start SubCore 2
  int ret = MP.begin(2); 
  if (ret < 0) {
    Serial.print("❌ [Main] Failed to start SubCore 2: ");
    Serial.println(ret);
  }
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    
    // We make this 'static' or global to ensure the memory stays valid 
    // while SubCore 2 is reading it.
    static SDMessage myMsg; 

    if (c == 'w') {
      Serial.println("📤 [Main] Sending Write Command to SubCore 2...");
      myMsg.command = CMD_WRITE_LOG;
      strcpy(myMsg.payload, "remote.txt");
      
      // Argument 1: msgid (user defined)
      // Argument 2: pointer to the data (cast to void*)
      // Argument 3: Target SubCore ID (2)
      MP.Send(1, (void*)&myMsg, 2); 
    } 
    else if (c == 'd') {
      Serial.println("📤 [Main] Sending Delete Command to SubCore 2...");
      myMsg.command = CMD_DELETE_ALL;
      strcpy(myMsg.payload, "remote.txt");
      MP.Send(2, (void*)&myMsg, 2);
    }

    // --- Acknowledgment Handling ---
    int8_t responseId;
    uint32_t responseData; // Matches the uint32_t candidate in MP.h
    
    // Non-blocking check for a reply from SubCore 2
    // We specify '2' to listen only to the SD-Worker
    int ret = MP.Recv(&responseId, &responseData, 2); 
    
    if (ret >= 0) {
      Serial.print("✅ [Main] SubCore 2 Acknowledged. Status Code: ");
      Serial.println(responseData);
    }
  }
}
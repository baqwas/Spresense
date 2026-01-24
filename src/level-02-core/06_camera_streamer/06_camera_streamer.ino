/**
 * @file        06_camera_streamer.ino
 * @author      Matha Goram
 * @brief       Level 02: Robust Continuous Streamer with Real-time Metrics.
 * @details     Implements a high-performance streaming pipeline for the CXD5602.
 * The sketch utilizes a hardware-triggered frame callback to 
 * decouple image acquisition from the main execution loop. 
 * Includes real-time performance monitoring (FPS/Delta) and 
 * asynchronous command handling.
 * * @section workflow Processing Workflow
 * 1. Initialization: The Main Core establishes communication with the Camera ISP 
 * and validates the driver state[cite: 5, 6].
 * 2. Configuration: The sensor is set to QVGA resolution (320x240) in JPG format 
 * to optimize memory throughput for streaming[cite: 7].
 * 3. Streaming: Upon activation, the hardware pushes frames to a shared memory 
 * buffer, triggering 'CamCB' for every valid frame.
 * 4. Metrics: The system calculates the 'Estimated FPS' by measuring the 
 * temporal delta between 5-second reporting windows.
 * * @section ui User Interface & Control
 * - [1] START: Commands the hardware engine to begin streaming and resets counters.
 * - [0] STOP: Halts the streaming engine and releases the hardware locks.
 * - Monitor: Serial output provides periodic status updates on stream health.
 * * @target      Sony Spresense (CXD5602) with Camera Board
 * @dependency  Camera.h
 * @version     1.0.4
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

#include <Camera.h>

/* Global State and Metrics */
volatile uint32_t totalFrameCount = 0;
bool isStreaming = false;
uint32_t lastReportTime = 0;

/**
 * @brief  Camera Frame Callback
 */
void CamCB(CamImage img) {
  if (img.isAvailable()) {
    totalFrameCount++;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("--- Level 02: Professional Streamer & Stats ---");

  CamErr err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS) {
    Serial.print("❌ CRITICAL ERROR: Hardware Start Failed: "); Serial.println(err);
    while(1);
  }

  // Configure high-speed compatible format
  theCamera.setStillPictureImageFormat(CAM_IMGSIZE_QVGA_H, CAM_IMGSIZE_QVGA_V, CAM_IMAGE_PIX_FMT_JPG);

  Serial.println("✅ Camera Initialized.");
  Serial.println("⌨️ Commands: [1] START | [0] STOP");
}

void loop() {
  // 1. Listen for Toggle Commands
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1' && !isStreaming) {
      Serial.println("🚀 Starting Stream...");
      totalFrameCount = 0;
      lastReportTime = millis();
      theCamera.startStreaming(true, CamCB);
      isStreaming = true;
    } 
    else if (cmd == '0' && isStreaming) {
      Serial.println("🛑 Stopping Stream...");
      theCamera.startStreaming(false, NULL);
      isStreaming = false;
    }
  }

  // 2. The Return of STREAM STATS
  if (isStreaming) {
    uint32_t currentTime = millis();
    // Report every 5 seconds
    if (currentTime - lastReportTime > 5000) {
      float elapsedSeconds = (currentTime - lastReportTime) / 1000.0;
      
      Serial.print("📊 STREAM STATS: Total Frames: ");
      Serial.print(totalFrameCount);
      Serial.print(" | Seconds: ");
      Serial.print(elapsedSeconds);
      
      if (elapsedSeconds > 0) {
        Serial.print(" | Est. FPS: ");
        Serial.println((float)totalFrameCount / elapsedSeconds);
      } else {
        Serial.println();
      }

      // Reset window for next 5-second report
      totalFrameCount = 0; 
      lastReportTime = currentTime;
    }
  }
}
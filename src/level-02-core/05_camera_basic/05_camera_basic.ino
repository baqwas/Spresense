/**
 * @file        05_camera_basic.ino
 * @author      Matha Goram
 * @brief       Level 01: Hello Camera Implementation for Sony Spresense. [cite: 9]
 * @details     Initializes the CXD5602 camera module via the parallel interface 
 * and performs a single-frame capture. The resulting JPEG image 
 * buffer is written to the SDHCI-connected media. [cite: 10]
 * This sketch serves as a hardware validation test for the 
 * integrated camera-to-storage pipeline.
 * * @target      Sony Spresense (CXD5602) with Camera Board and Extension Board [cite: 11]
 * @dependency  Camera.h, SDHCI.h, File.h
 * @version     1.0.1
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
 * @note
 * The SD Card Serial Bridge Solution
 *  If you don't want to keep pulling the card out, you can flash a specific utility sketch to the Spresense 
 *  that turns it into a USB Card Reader.
 *  In the Arduino IDE, navigate to File > Examples > SDHCI > Tool > SdCardReaderWriter.
 *  Upload this to your Main Core.
 *  Once uploaded, your Ubuntu system should suddenly see a new "Mass Storage Device" pop up in the sidebar, 
 *  just like a thumb drive.
 *  Note: You must re-flash your 05_camera_basic.ino sketch when you want to go back to taking pictures.
 *  @note
 *  🔍 Key Implementation Logic Reference
 *  Hardware Initialization: The script first validates the SD card presence before initiating the camera driver 
 *  using theCamera.begin().
 *  Capture Parameters: The capture is standardized to QUADVGA resolution in JPG format.
 *  Buffer Validation: Before writing to storage, the script verifies image availability in the system memory buffer.
 *  Storage Integrity: To prevent conflicts, the script explicitly removes any existing version of the file before 
 *  opening a new write stream.
 */

#include <SDHCI.h>
#include <Camera.h>

SDClass theSD;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // 1. Initialize SD Card (Required to save the image)
  if (!theSD.begin()) {
    Serial.println("❌ SD Card Error. Please check insertion.");
    return;
  }

  // 2. Initialize Camera
  Serial.println("📸 Initializing Camera...");
  CamErr err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS) {
    Serial.print("❌ Camera Begin Error: "); Serial.println(err);
    return;
  }

  // 3. Set Parameters
  // Parameters: Still image format, Still image resolution, Video format
  theCamera.setStillPictureImageFormat(
    CAM_IMGSIZE_QUADVGA_H,
    CAM_IMGSIZE_QUADVGA_V,
    CAM_IMAGE_PIX_FMT_JPG);

  Serial.println("✅ Camera Ready. Taking picture in 2 seconds...");
  delay(2000);

  // 4. Capture
  CamImage img = theCamera.takePicture();

  if (img.isAvailable()) {
    char filename[16];
    sprintf(filename, "05_camera_basic.jpg");
    Serial.print("💾 Saving: "); Serial.println(filename);

    theSD.remove(filename); // Delete if exists
    File myFile = theSD.open(filename, FILE_WRITE);
    myFile.write(img.getImgBuff(), img.getImgSize());
    myFile.close();
    
    Serial.println("🏁 Done! Check your SD card.");
  } else {
    Serial.println("❌ Failed to capture image.");
  }
}

void loop() {
  // Stay idle
}
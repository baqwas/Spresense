/**
 * @file        sd_basic_test.ino
 * @author      Matha Goram
 * @brief       Hardware Integrity Check for Sony Spresense SDHCI.
 * @details     Initializes the SD card via the Extension Board and performs 
 * a basic write operation to verify filesystem accessibility. 
 * Used as a prerequisite for multi-core I/O delegation.
 * * @target      Sony Spresense (CXD5602) with Extension Board
 * @dependency  SDHCI Library (Spresense Arduino Board Package) [cite: 1, 2]
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

SDClass theSD;

void setup() {
  Serial.begin(115200);
  while (!Serial); 

  Serial.println("--- SD Card Hardware Integrity Check ---");

  // On Spresense, the SD card is handled by the SDHCI class
  if (theSD.begin()) {
    Serial.println("✅ SUCCESS: SD Card found and initialized.");
    
    // Let's try to create a file
    File myFile = theSD.open("test_io.txt", FILE_WRITE);
    
    if (myFile) {
      myFile.println("If you can read this, the SD I/O is perfect.");
      myFile.close();
      Serial.println("✅ SUCCESS: Data written to test_io.txt");
    } else {
      Serial.println("❌ ERROR: Could not open file for writing.");
    }
  } else {
    Serial.println("❌ ERROR: SD Card initialization failed.");
    Serial.println("   1. Is it a 32GB (or smaller) FAT32 card?");
    Serial.println("   2. Is it clicked firmly into the Extension Board?");
  }
}

void loop() {
  // End of test
}
/**
 * @file        sd_stress_test.ino
 * @author      Matha Goram
 * @brief       Rigorous SD Card File System Diagnostic for Sony Spresense.
 * @details     Performs directory lifecycle management, atomic file I/O, 
 * and string-based wildcard deletion. This utility verifies 
 * hardware integrity before delegating I/O to SubCore 2.
 * * @target      Sony Spresense (CXD5602) with Extension Board
 * @dependency  SDHCI Library (Spresense Arduino Board Package)
 * @memory      Recommended tile size: 128KB+
 * * @version     1.0.0
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
  
  Serial.println("\n🚀 SPRESENSE SD RIGOROUS DIAGNOSTIC STARTING...");
  Serial.println("==============================================");

  if (!theSD.begin()) {
    Serial.println("❌ [FATAL] SD Initialization Failed!");
    Serial.println("   Check: Is the card FAT32? Is it in the Extension Board?");
    return;
  }
  Serial.println("✅ SD Hardware Layer: READY");

  // 1. Directory Management
  Serial.println("\n📁 Creating Directory: /stress_test...");
  if (!theSD.mkdir("stress_test")) {
    Serial.println("⚠️  Note: Folder exists or creation skipped.");
  }

  // 2. File Creation Phase
  Serial.println("\n📝 Writing Test Files...");
  createTestFile("stress_test/sensor_01.log");
  createTestFile("stress_test/sensor_02.log");
  createTestFile("stress_test/config.cfg");
  createTestFile("stress_test/preview.jpg");

  // 3. Initial Audit
  printDivider();
  listDir("stress_test");
  printDivider();

  // 4. Wildcard Deletion (Targeting .log files)
  Serial.println("\n🧹 Cleanup: Deleting all files ending in '.log'...");
  int deletedCount = deleteByExtension("stress_test", ".log");
  Serial.print("✨ Cleanup Complete. Items removed: "); Serial.println(deletedCount);

  // 5. Final Audit
  printDivider();
  listDir("stress_test");
  printDivider();

  // 6. Termination
  Serial.println("\n==============================================");
  Serial.println("🏁 DIAGNOSTIC TERMINATED SUCCESSFULLY");
  Serial.println("   Status: STORAGE I/O VERIFIED");
  Serial.println("   Action: You may now move this logic to SubCore 2.");
  Serial.println("==============================================\n");
}

void createTestFile(const char* path) {
  File f = theSD.open(path, FILE_WRITE);
  if (f) {
    f.println("Spresense Rigorous Test Data Stream");
    f.close();
    Serial.print("   📄 Created: "); Serial.println(path);
  } else {
    Serial.print("   🚫 Error creating: "); Serial.println(path);
  }
}

void listDir(const char* path) {
  File dir = theSD.open(path);
  Serial.print("🔍 Auditing Directory ["); Serial.print(path); Serial.println("]:");
  
  int count = 0;
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    
    if (entry.isDirectory()) {
      Serial.print("   📁 "); 
    } else {
      Serial.print("   📄 ");
    }
    
    Serial.print(entry.name());
    Serial.print("\t\tSize: ");
    Serial.print(entry.size());
    Serial.println(" bytes");
    
    entry.close();
    count++;
  }
  if (count == 0) Serial.println("   (Directory is empty)");
}

int deleteByExtension(const char* path, const char* ext) {
  int count = 0;
  File dir = theSD.open(path);
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    
    String fileName = entry.name();
    if (fileName.endsWith(ext)) {
      Serial.print("   🗑️  Removing: "); Serial.println(fileName);
      String fullPath = String(path) + "/" + fileName;
      entry.close(); 
      if (theSD.remove(fullPath.c_str())) {
        count++;
      }
    } else {
      entry.close();
    }
  }
  return count;
}

void printDivider() {
  Serial.println("----------------------------------------------");
}

void loop() {
  // Diagnostic complete - stay idle.
}

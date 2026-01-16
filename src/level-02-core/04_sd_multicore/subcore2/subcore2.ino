/**
 * @file        subcore2.ino
 * @author      Matha Goram
 * @brief       SubCore 2 Requester - Background Data Processing.
 * @details     Handles background computational tasks or sensor polling. 
 * Delegates all File I/O operations to the Main Core via the 
 * MP Mailbox system. This core does NOT have direct access 
 * to the SDHCI hardware.
 * * @target      Sony Spresense (CXD5602) - SubCore 2
 * @dependency  MP Library (Spresense Arduino Board Package)
 * @version     1.0.0
 * @date        2026-01-15
 * * @copyright   Copyright (c) 2026 ParkCircus Productions
 * * @license MIT
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
#include <MP.h>
#include "commands.h"

void setup() {
  MP.begin(); 
}

void loop() {
  // Example: Every 5 seconds, ask Main Core to log something
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 5000) {
    lastLog = millis();
    
    static SDMessage msg;
    msg.command = CMD_WRITE_LOG;
    strcpy(msg.payload, "sub2_data.txt");
    
    // Send request to Main Core (ID 0)
    MP.Send(1, (void*)&msg, 0); 
  }
}
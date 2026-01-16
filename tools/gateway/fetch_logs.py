#!/usr/bin/env python3
"""
@file      fetch_logs.py
@author    Reza
@brief     Remote SD Card Data Retrieval Utility for Sony Spresense GNSS Gateway.
@version   1.0.1
@date      2026-01-14
@license   MIT

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

USAGE:
    1. Ensure the systemd gateway service is stopped: 
       'sudo systemctl stop spresense-gateway.service'
    2. Run this script: 'python3 fetch_logs.py'
"""

import serial
import time
import sys
import os

# --- Configuration ---
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE   = 115200
OUTPUT_FILE = "downloaded_telemetry.csv"
CMD_TRIGGER = b'R'  # Command sent to Spresense to trigger dump
EOF_MARKER  = "---END_DUMP---"
TIMEOUT_SEC = 5

def fetch_logs():
    """
    Connects to the Spresense via Serial, triggers a data dump from the 
    SD card, and saves the stream to a local CSV file.
    """
    ser = None
    
    print("="*50)
    print(" SPRESENSE SD LOG RETRIEVAL UTILITY ")
    print("="*50)

    try:
        # 1. Resource Validation
        if not os.path.exists(SERIAL_PORT):
            raise FileNotFoundError(f"Device not found at {SERIAL_PORT}. Is it plugged in?")

        # 2. Establish Serial Connection
        print(f"[*] Opening {SERIAL_PORT} at {BAUD_RATE} baud...")
        ser = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            timeout=TIMEOUT_SEC,
            xonxoff=False,
            rtscts=False,
            dsrdtr=False
        )
        
        # Spresense often resets on serial connection; wait for boot.
        print("[*] Synchronizing with hardware (2s delay)...")
        time.sleep(2)
        ser.reset_input_buffer()

        # 3. Trigger Remote Read
        print(f"[*] Sending trigger command: '{CMD_TRIGGER.decode()}'")
        ser.write(CMD_TRIGGER)
        ser.flush()

        # 4. Stream Data to File
        line_count = 0
        print(f"[*] Receiving data into '{OUTPUT_FILE}'", end="", flush=True)
        
        with open(OUTPUT_FILE, "w", encoding='utf-8') as f:
            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                # Check for end of stream markers
                if EOF_MARKER in line or "EOF" in line:
                    print("\n[*] End of File marker detected.")
                    break
                
                # Filter out the JSON live telemetry if it's still printing
                if line.startswith("{") or "START_OF_SD" in line:
                    continue
                
                if line:
                    f.write(line + "\n")
                    line_count += 1
                    if line_count % 10 == 0:
                        print(".", end="", flush=True)

        if line_count > 0:
            print(f"\n[SUCCESS] Retrieval complete. {line_count} rows saved.")
        else:
            print("\n[WARNING] Connection successful but no data rows were captured.")

    except serial.SerialException as e:
        print(f"\n[SERIAL ERROR] Could not access port: {e}")
        print("Tip: Check if another process (like your Gateway Service) is using the port.")
    
    except PermissionError:
        print(f"\n[PERMISSION ERROR] Access denied to {SERIAL_PORT}.")
        print("Tip: Run 'sudo usermod -a -G dialout $USER' and restart your session.")
        
    except KeyboardInterrupt:
        print("\n[ABORTED] User interrupted the process.")
        
    except Exception as e:
        print(f"\n[UNEXPECTED ERROR] {e}")

    finally:
        if ser and ser.is_open:
            ser.close()
            print("[*] Serial port closed.")
        print("="*50)

if __name__ == "__main__":
    fetch_logs()

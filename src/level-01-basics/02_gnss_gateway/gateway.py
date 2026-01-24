#!/usr/bin/env python3
"""
================================================================================
Project:        Spresense-to-LAN Telemetry Gateway
File:           gateway.py
Description:    Bridges Sony Spresense Serial JSON data to an MQTT Broker.
                Injects ISO-8601 'Waqt' timestamps and handles data routing.
Author:         Matha Goram
Copyright:      (c) 2026 ParkCircus Productions
License:        MIT License

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
================================================================================
"""

import serial
import json
import logging
import argparse
import sys
import time
from datetime import datetime
import paho.mqtt.client as mqtt

# Try to use built-in tomllib (Python 3.11+) or fallback to tomli
try:
    import tomllib as toml
except ImportError:
    import tomli as toml

def setup_logging(level, log_file):
    """Configures the logging system with a custom format."""
    log_format = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    logging.basicConfig(
        level=getattr(logging, level.upper(), logging.INFO),
        format=log_format,
        handlers=[
            logging.FileHandler(log_file),
            logging.StreamHandler(sys.stdout)
        ]
    )
    return logging.getLogger("Gateway")

def parse_arguments():
    """Handles command line flags."""
    parser = argparse.ArgumentParser(description="Spresense to MQTT Gateway")
    parser.add_argument("--config", default="config.toml", help="Path to config.toml")
    parser.add_argument("--debug", action="store_true", help="Force debug logging")
    return parser.parse_args()

def main():
    args = parse_arguments()
    
    # 1. Load Configuration
    try:
        with open(args.config, "rb") as f:
            config = toml.load(f)
    except Exception as e:
        print(f"CRITICAL: Could not load config file: {e}")
        sys.exit(1)

    # 2. Setup Logging
    log_level = "DEBUG" if args.debug else config['logging']['level']
    logger = setup_logging(log_level, config['logging']['file'])
    logger.info("Starting Spresense Gateway Service...")

    # 3. Setup MQTT Client
    client = mqtt.Client(config['mqtt']['client_id'])
    
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            logger.info("Connected to MQTT Broker successfully.")
        else:
            logger.error(f"MQTT Connection failed with code {rc}")

    client.on_connect = on_connect

    try:
        client.connect(config['mqtt']['broker'], int(config['mqtt']['port']), 60)
        client.loop_start()
    except Exception as e:
        logger.error(f"Could not connect to MQTT Broker: {e}")
        sys.exit(1)

    # 4. Main Serial Loop with Error Handling
    ser = None
    try:
        logger.info(f"Opening Serial Port: {config['serial']['port']}...")
        ser = serial.Serial(
            port=config['serial']['port'],
            baudrate=config['serial']['baud'],
            timeout=config['serial']['timeout']
        )
        
        while True:
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8').strip()
                    
                    # Ensure we are receiving valid JSON
                    if line.startswith('{') and line.endswith('}'):
                        data = json.loads(line)
                        
                        # Inject your custom timestamp (Waqt)
                        data["Waqt"] = datetime.now().isoformat() + "Z"
                        
                        # Publish to LAN
                        payload = json.dumps(data)
                        client.publish(config['mqtt']['topic'], payload)
                        logger.debug(f"Published: {payload}")
                    
                except json.JSONDecodeError:
                    logger.warning(f"Malformed JSON received: {line}")
                except Exception as e:
                    logger.error(f"Loop Error: {e}")
            
            time.sleep(0.1) # Prevent CPU hogging

    except serial.SerialException as e:
        logger.critical(f"Serial Hardware Error: {e}")
    except KeyboardInterrupt:
        logger.info("Gateway stopped by user.")
    finally:
        if ser:
            ser.close()
        client.loop_stop()
        client.disconnect()
        logger.info("Cleanup complete. Goodbye.")

if __name__ == "__main__":
    main()

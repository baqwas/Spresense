/**
 * @file        00-hello-spresense.ino
 * @author      baqwas (https://github.com/baqwas)
 * @brief       The definitive "Hello World" for Sony Spresense.
 * @details     Verifies Serial communication, prints system clock speed, 
 * and identifies the 6-core architecture status.
 * * @version     1.0.0
 * @date        2026-01-13
 * @target      Sony Spresense (CXD5602)
 * @license     MIT License
 * @reference   https://developer.sony.com/spresense/development-guides/?page=home&lang=en
 */

void setup() {
  
  Serial.begin(115200); // Spresense supports high-speed Serial. 115200 is the standard.
  while (!Serial) { // Wait for Serial to be ready (Native USB requirement)
    ; 
  }
  delay(1000);

  Serial.println("========================================");
  Serial.println("   HELLO SPRESENCE - SYSTEM CHECK       ");
  Serial.println("========================================");

  // Print CPU Frequency
  Serial.print("CPU Clock Frequency: ");
  Serial.print(F_CPU / 1000000);
  Serial.println(" MHz");

  // Multicore Info
  Serial.println("Architecture: 6-Core ARM Cortex-M4F");
  Serial.println("Status: MainCore is ACTIVE.");
  Serial.println("----------------------------------------");
  Serial.println("Action: Type something and press Enter...");
}

void loop() {
  // Echo back any user input to verify two-way communication
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    Serial.print("Spresence received: ");
    Serial.println(input);
    
    Serial.println("Result: Serial I/O Verified. System Healthy.");
  }
}
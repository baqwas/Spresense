/**
 * @file        vision_ipc.h
 * @brief       Inter-Processor Communication protocol for Level 03.
 * @details     Defines Command IDs for Camera and SD operations across 
 * Main Core, Sub2, and Sub3.
 */

#ifndef VISION_IPC_H
#define VISION_IPC_H

/* Command IDs */
enum VisionCmd {
  CMD_CAM_READY   = 10,  // Main -> Sub: Camera is initialized / Start Loop
  CMD_STOP_LOOP   = 11,  // Main -> Sub2: Stop the automatic capture timer
  CMD_REQ_CAPTURE = 12,  // Sub2 -> Main: Take a picture now
  CMD_PROCESS_IMG = 13,  // Main -> Sub3: Analyze this memory address
  CMD_SAVE_FILE   = 14,  // Sub3 -> Main: Analysis done, write buffer to SD
  CMD_SHUTDOWN    = 15   // Main -> All: Unmount SD and halt system
};

/* Shared Message Structure */
struct VisionMessage {
  uint8_t  commandID;    // From VisionCmd
  uint32_t bufferAddr;   // Physical address of the CamImage buffer
  uint32_t dataSize;     // Size of the data in bytes
  char     fileName[16]; // Filename for SD operations
};

#endif
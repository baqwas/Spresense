#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <stdint.h>

/**
 * @struct GnssPacket
 * @brief  Synchronized data structure for Inter-Processor Communication (IPC).
 * * NOTE: This structure must be identical on all cores. The Spresense 
 * uses an ARM Cortex-M4F for both Main and Sub cores, ensuring 
 * consistent data type sizes (e.g., 8-byte doubles, 4-byte floats).
 */
struct GnssPacket {
  // --- Positional Data ---
  double lat;        // Latitude in decimal degrees
  double lon;        // Longitude in decimal degrees
  float alt;         // Altitude above sea level (meters)
  float hdop;        // Horizontal Dilution of Precision (lower is better)
  float velocity;    // Current ground speed (m/s)
  int numSats;       // Total satellites used in navigation solution
  
  // --- High-Precision Atomic Timing ---
  struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  sec;
    uint32_t usec;   // Microseconds for sub-second precision
  } time;

  // --- Satellite Inventory (Top 16 signals) ---
  struct SatData {
    int svid;        // Space Vehicle ID (PRN)
    int type;        // Constellation Type (0:GPS, 1:GLONASS, 6:BEIDOU, 7:GALILEO)
    float sigLevel;  // Signal-to-Noise Ratio (SNR/CNo)
    int elevation;   // Degrees above horizon
    int azimuth;     // Direction in degrees (0-359)
  } inventory[16];
};

#endif // SHARED_TYPES_H
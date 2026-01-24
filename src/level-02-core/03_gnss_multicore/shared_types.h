#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <stdint.h>

struct GnssPacket {
  float lat;
  float lon;
  float hdop;
  uint32_t sec;
  uint32_t numSats;
  uint32_t heartbeat; // Keep this!
};

#endif
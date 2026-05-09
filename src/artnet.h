#pragma once
#include <stdint.h>

#define ARTNET_OPCODE_DMX   0x5000
#define ARTNET_HEADER       "Art-Net"
#define ARTNET_HEADER_LEN   8       // including null terminator

typedef struct {
    uint16_t universe;              // full 15-bit Port-Address
    uint16_t dmx_len;               // number of DMX bytes (up to 512)
    const uint8_t *dmx;             // pointer into the original buffer
} artnet_packet_t;

// Parse a raw UDP payload.  Fills *pkt and returns true if valid ArtDMX packet.
bool artnet_parse(const uint8_t *buf, int buf_len, artnet_packet_t *pkt);

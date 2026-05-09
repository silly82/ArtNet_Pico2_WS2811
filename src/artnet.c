#include "artnet.h"
#include <string.h>
#include <stdbool.h>

// ArtDMX packet layout (Art-Net 4 spec):
//   [0..7]   "Art-Net\0"
//   [8..9]   OpCode   = 0x0050 little-endian  (= 0x5000 as uint16)
//   [10..11] ProtVer  = 0x000E big-endian
//   [12]     Sequence
//   [13]     Physical
//   [14]     SubUni   (universe low byte)
//   [15]     Net      (universe high 7 bits)
//   [16..17] Length   big-endian, must be even, ≤ 512
//   [18..]   DMX data

#define MIN_PKT_LEN  18

bool artnet_parse(const uint8_t *buf, int buf_len, artnet_packet_t *pkt)
{
    if (buf_len < MIN_PKT_LEN) return false;
    if (memcmp(buf, ARTNET_HEADER, ARTNET_HEADER_LEN) != 0) return false;

    uint16_t opcode = (uint16_t)buf[8] | ((uint16_t)buf[9] << 8);
    if (opcode != ARTNET_OPCODE_DMX) return false;

    uint16_t dmx_len = ((uint16_t)buf[16] << 8) | buf[17];
    if (dmx_len == 0 || dmx_len > 512) return false;
    if (buf_len < MIN_PKT_LEN + (int)dmx_len) return false;

    pkt->universe = (uint16_t)buf[14] | ((uint16_t)(buf[15] & 0x7F) << 8);
    pkt->dmx_len  = dmx_len;
    pkt->dmx      = buf + MIN_PKT_LEN;
    return true;
}

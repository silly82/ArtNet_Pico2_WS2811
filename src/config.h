#pragma once
#include <stdint.h>

// ── W6300 QSPI (fixed on W6300-EVB-Pico2) ───────────────────────────────────
// The W6300 uses Quad-SPI (4-bit) implemented via RP2350 PIO.
// Hardware SPI is NOT used. Pins are hardwired on the board:
#define W6300_PIN_INT   15
#define W6300_PIN_CS    16
#define W6300_PIN_SCK   17
#define W6300_PIN_IO0   18   // MOSI / data lane 0
#define W6300_PIN_IO1   19   // MISO / data lane 1
#define W6300_PIN_IO2   20   // data lane 2
#define W6300_PIN_IO3   21   // data lane 3
#define W6300_PIN_RST   22

// ── WS2811 outputs (GP0–GP7 → SN74LVC4245A → LEDs) ─────────────────────────
#define WS2811_BASE_PIN     0       // first output pin; outputs on GP0, GP1, … GP(N-1)
#define WS2811_NUM_OUTPUTS  6       // number of parallel universes

// ── LED parameters ───────────────────────────────────────────────────────────
#define LEDS_PER_UNIVERSE   170     // 512 DMX channels / 3  = 170 full RGB pixels
#define WS2811_COLOR_RGB    1       // 1 = RGB order (WS2811), 0 = GRB order (WS2812)

// ── ArtNet ──────────────────────────────────────────────────────────────────
#define ARTNET_PORT             6454
#define ARTNET_UNIVERSE_START   0   // first ArtNet universe mapped to output 0

// ── Network (adjust to your setup) ──────────────────────────────────────────
#define NET_MAC     { 0x00, 0x08, 0xDC, 0xAB, 0xCD, 0xEF }
#define NET_IP      { 192, 168, 1, 200 }
#define NET_SUBNET  { 255, 255, 255, 0 }
#define NET_GW      { 192, 168, 1, 1 }
#define NET_DNS     { 8, 8, 8, 8 }

// ── Internal ─────────────────────────────────────────────────────────────────
#define UDP_BUF_SIZE    (18 + 512)  // ArtNet header + max DMX payload
#define SOCKET_ARTNET   0           // WIZnet socket number for ArtNet UDP

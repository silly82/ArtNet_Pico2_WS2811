#pragma once
#include <stdint.h>

// ── W6300 SPI (fixed on W6300-EVB-Pico2) ────────────────────────────────────
#define W6300_SPI_PORT  spi1
#define W6300_SPI_BAUD  33000000u   // 33 MHz — safe for W6300 (max 60 MHz)
#define W6300_PIN_SCK   10
#define W6300_PIN_MOSI  11
#define W6300_PIN_MISO  12
#define W6300_PIN_CS    13
#define W6300_PIN_RST   20
#define W6300_PIN_INT   21

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

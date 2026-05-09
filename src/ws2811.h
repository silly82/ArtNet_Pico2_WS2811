#pragma once
#include <stdint.h>
#include <stdbool.h>

// Initialise PIO state machines and DMA channels.
// base_pin: first GPIO (outputs on base_pin … base_pin+num_outputs-1)
void ws2811_init(uint num_outputs, uint base_pin);

// Update one universe from raw DMX data (3 bytes per pixel, RGB).
// dmx_len: number of valid DMX bytes (≤ 512).
// Safe to call from the ArtNet callback; double-buffered internally.
void ws2811_update(uint universe, const uint8_t *dmx, uint dmx_len);

// Push all universe buffers to the LEDs via DMA.
// Returns immediately; DMA runs in background.
// Call once per frame (≤ 44 Hz to stay within ArtNet spec).
void ws2811_send_all(void);

// Block until the previous DMA transfers have completed.
void ws2811_wait(void);

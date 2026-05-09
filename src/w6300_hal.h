#pragma once

// Initialise SPI bus and W6300, apply static IP from config.h.
// Call once at startup before any socket operations.
void wiznet_init(void);

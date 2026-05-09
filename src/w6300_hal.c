#include "w6300_hal.h"
#include "config.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

// WIZnet ioLibrary headers
#include "Ethernet/wizchip_conf.h"
#include "Ethernet/socket.h"

// The W6300-EVB-Pico2 uses QSPI (Quad-SPI) via RP2350 PIO.
// The WIZnet library provides the QSPI PIO driver; include its header here:
#include "W6300/w6300_qspi.h"   // adjust path if your checkout differs

// ── Hardware reset ────────────────────────────────────────────────────────────

static void _chip_reset(void)
{
    gpio_init(W6300_PIN_RST);
    gpio_set_dir(W6300_PIN_RST, GPIO_OUT);
    gpio_put(W6300_PIN_RST, 0);
    sleep_ms(10);
    gpio_put(W6300_PIN_RST, 1);
    sleep_ms(100);
}

// ── Public init ───────────────────────────────────────────────────────────────

void wiznet_init(void)
{
    // Initialise the WIZnet QSPI PIO driver.
    // This sets up the PIO state machine on the pins defined in config.h
    // and registers the low-level read/write callbacks into wizchip_conf.
    //
    // The exact function name depends on the WIZnet library version:
    //   • ioLibrary_Driver W6300 branch: wizchip_qspi_initialize()
    //   • W6300-EVB-Pico2 example repo:  w6300_qspi_initialize()
    // Check lib/ioLibrary_Driver/Ethernet/W6300/ for the correct name.
    wizchip_qspi_initialize(
        W6300_PIN_SCK,
        W6300_PIN_CS,
        W6300_PIN_IO0,  // IO0–IO3 must be consecutive GPIOs
        W6300_PIN_RST
    );

    _chip_reset();

    // RX/TX buffer sizes: 2 KB per socket × 8 sockets
    uint8_t rx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    uint8_t tx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    if (wizchip_init(tx_size, rx_size) != 0) {
        // Init failed — blink LED as error indicator
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        while (true) {
            gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
            sleep_ms(100);
        }
    }

    // Static IP
    wiz_NetInfo net = {
        .mac  = NET_MAC,
        .ip   = NET_IP,
        .sn   = NET_SUBNET,
        .gw   = NET_GW,
        .dns  = NET_DNS,
        .dhcp = NETINFO_STATIC,
    };
    wizchip_setnetinfo(&net);
}

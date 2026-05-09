#include "w6300_hal.h"
#include "config.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// WIZnet ioLibrary headers (cloned into lib/ioLibrary_Driver)
#include "Ethernet/wizchip_conf.h"
#include "Ethernet/socket.h"

// ── SPI callbacks required by the WIZnet ioLibrary ───────────────────────────

static void _cs_select(void)   { gpio_put(W6300_PIN_CS, 0); }
static void _cs_deselect(void) { gpio_put(W6300_PIN_CS, 1); }

static uint8_t _spi_read(void)
{
    uint8_t byte;
    spi_read_blocking(W6300_SPI_PORT, 0xFF, &byte, 1);
    return byte;
}

static void _spi_write(uint8_t byte)
{
    spi_write_blocking(W6300_SPI_PORT, &byte, 1);
}

static void _spi_read_burst(uint8_t *buf, uint16_t len)
{
    spi_read_blocking(W6300_SPI_PORT, 0xFF, buf, len);
}

static void _spi_write_burst(uint8_t *buf, uint16_t len)
{
    spi_write_blocking(W6300_SPI_PORT, buf, len);
}

// ── Hardware reset ────────────────────────────────────────────────────────────

static void _chip_reset(void)
{
    gpio_put(W6300_PIN_RST, 0);
    sleep_ms(10);
    gpio_put(W6300_PIN_RST, 1);
    sleep_ms(100);
}

// ── Public init ───────────────────────────────────────────────────────────────

void wiznet_init(void)
{
    // SPI bus
    spi_init(W6300_SPI_PORT, W6300_SPI_BAUD);
    gpio_set_function(W6300_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(W6300_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(W6300_PIN_MISO, GPIO_FUNC_SPI);

    // CS and RST as plain GPIO
    gpio_init(W6300_PIN_CS);  gpio_set_dir(W6300_PIN_CS,  GPIO_OUT);
    gpio_init(W6300_PIN_RST); gpio_set_dir(W6300_PIN_RST, GPIO_OUT);
    gpio_put(W6300_PIN_CS, 1);

    _chip_reset();

    // Register callbacks into the WIZnet ioLibrary
    reg_wizchip_cs_cbfunc(_cs_select, _cs_deselect);
    reg_wizchip_spi_cbfunc(_spi_read, _spi_write);
    reg_wizchip_spiburst_cbfunc(_spi_read_burst, _spi_write_burst);

    // RX/TX buffer sizes: 2 KB per socket × 8 sockets (W6300 default)
    uint8_t rx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    uint8_t tx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    if (wizchip_init(tx_size, rx_size) != 0) {
        // Init failed — blink built-in LED as error indicator
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        while (true) {
            gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
            sleep_ms(100);
        }
    }

    // Static IP configuration
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

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "config.h"
#include "wiznet_hal.h"
#include "ws2811.h"
#include "artnet.h"

#include "Ethernet/socket.h"

// ── Timing ───────────────────────────────────────────────────────────────────
// ArtNet max = 44 fps; WS2811 frame takes ~5.2 ms per 170 LEDs.
// We output a new frame whenever there is new data, throttled to 40 fps.
#define FRAME_INTERVAL_US   25000u  // 40 fps = 25 ms

// ── State ─────────────────────────────────────────────────────────────────────
static uint8_t  _udp_buf[UDP_BUF_SIZE];
static bool     _dirty[WS2811_NUM_OUTPUTS];  // true = new data received

// ── ArtNet reception ──────────────────────────────────────────────────────────
static void poll_artnet(void)
{
    if (getSn_RX_RSR(SOCKET_ARTNET) == 0) return;

    uint8_t  remote_ip[4];
    uint16_t remote_port;
    int32_t  len = recvfrom(SOCKET_ARTNET, _udp_buf, sizeof(_udp_buf),
                            remote_ip, &remote_port);
    if (len <= 0) return;

    artnet_packet_t pkt;
    if (!artnet_parse(_udp_buf, len, &pkt)) return;

    int output = (int)pkt.universe - ARTNET_UNIVERSE_START;
    if (output < 0 || output >= WS2811_NUM_OUTPUTS) return;

    ws2811_update((uint)output, pkt.dmx, pkt.dmx_len);
    _dirty[output] = true;
}

// ── LED output ────────────────────────────────────────────────────────────────
static void output_frame(void)
{
    bool any = false;
    for (int i = 0; i < WS2811_NUM_OUTPUTS; i++) {
        if (_dirty[i]) { any = true; break; }
    }
    if (!any) return;

    ws2811_send_all();

    for (int i = 0; i < WS2811_NUM_OUTPUTS; i++) _dirty[i] = false;
}

// ── Entry point ───────────────────────────────────────────────────────────────
int main(void)
{
    stdio_init_all();

    // Ethernet + W6300
    wiznet_init();
    socket(SOCKET_ARTNET, Sn_MR_UDP, ARTNET_PORT, 0);

    // WS2811 PIO + DMA
    ws2811_init(WS2811_NUM_OUTPUTS, WS2811_BASE_PIN);

    // Status LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    uint32_t last_frame_us = time_us_32();

    while (true) {
        poll_artnet();

        uint32_t now = time_us_32();
        if ((now - last_frame_us) >= FRAME_INTERVAL_US) {
            output_frame();
            last_frame_us = now;
        }
    }
}

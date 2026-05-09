cmake_minimum_required(VERSION 3.20)

# Pull in Pico SDK (PICO_SDK_PATH must be set in environment or passed via -D)
include(pico_sdk_import.cmake)

project(ArtNetToWS2811 C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

# ── WIZnet ioLibrary ──────────────────────────────────────────────────────────
# Clone into lib/ioLibrary_Driver:
#   git clone https://github.com/WIZnet-ioLibrary/ioLibrary_Driver lib/ioLibrary_Driver
#
# The W6300 needs the W6300 branch:
#   cd lib/ioLibrary_Driver && git checkout W6300
set(WIZNET_DIR ${CMAKE_SOURCE_DIR}/lib/ioLibrary_Driver)

add_library(ioLibrary STATIC
    ${WIZNET_DIR}/Ethernet/wizchip_conf.c
    ${WIZNET_DIR}/Ethernet/socket.c
    # W6300 register driver + QSPI PIO driver:
    ${WIZNET_DIR}/Ethernet/W6300/w6300.c
    ${WIZNET_DIR}/Ethernet/W6300/w6300_qspi.c   # PIO-based QSPI — adjust if path differs
)
target_include_directories(ioLibrary PUBLIC
    ${WIZNET_DIR}
    ${WIZNET_DIR}/Ethernet
    ${WIZNET_DIR}/Ethernet/W6300
    ${WIZNET_DIR}/Internet/DHCP
)
# QSPI is implemented via PIO — no hardware_spi needed
target_link_libraries(ioLibrary pico_stdlib hardware_pio hardware_gpio)

# ── PIO compilation ───────────────────────────────────────────────────────────
# pioasm generates src/ws2811.pio.h automatically via pico_generate_pio_header
add_executable(ArtNetToWS2811
    src/main.c
    src/ws2811.c
    src/artnet.c
    src/w6300_hal.c
)

pico_generate_pio_header(ArtNetToWS2811 ${CMAKE_SOURCE_DIR}/src/ws2811.pio)

target_include_directories(ArtNetToWS2811 PRIVATE src)

target_link_libraries(ArtNetToWS2811
    pico_stdlib
    hardware_pio
    hardware_dma
    hardware_spi
    hardware_clocks
    hardware_gpio
    ioLibrary
)

# USB serial for printf debugging (switch to uart if preferred)
pico_enable_stdio_usb(ArtNetToWS2811 1)
pico_enable_stdio_uart(ArtNetToWS2811 0)

# Generate .uf2 and .bin alongside the ELF
pico_add_extra_outputs(ArtNetToWS2811)

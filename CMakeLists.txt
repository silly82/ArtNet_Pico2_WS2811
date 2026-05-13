cmake_minimum_required(VERSION 3.20)

# Pull in Pico SDK (PICO_SDK_PATH must be set in environment or passed via -D)
include(pico_sdk_import.cmake)

project(ArtNetToWS2811 C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

# ── Board → WIZnet chip selection ────────────────────────────────────────────
#   cmake .. -DPICO_BOARD=pico2  →  W6300-EVB-Pico2  (RP2350, QSPI)
#   cmake .. -DPICO_BOARD=pico   →  W5500-EVB-Pico   (RP2040, SPI)
if(PICO_BOARD STREQUAL "pico2")
    set(WIZNET_CHIP "W6300")
else()
    set(WIZNET_CHIP "W5500")
endif()

message(STATUS "Target board: ${PICO_BOARD}  →  WIZnet ${WIZNET_CHIP}")

# ── WIZnet ioLibrary ──────────────────────────────────────────────────────────
# Clone into lib/ioLibrary_Driver and check out the matching branch:
#   W6300:  cd lib/ioLibrary_Driver && git checkout W6300
#   W5500:  cd lib/ioLibrary_Driver && git checkout master
set(WIZNET_DIR ${CMAKE_SOURCE_DIR}/lib/ioLibrary_Driver)

if(WIZNET_CHIP STREQUAL "W6300")
    set(WIZNET_CHIP_SRCS
        ${WIZNET_DIR}/Ethernet/W6300/w6300.c
        ${WIZNET_DIR}/Ethernet/W6300/w6300_qspi.c
    )
    set(WIZNET_CHIP_INC  ${WIZNET_DIR}/Ethernet/W6300)
    set(WIZNET_HAL_SRC   src/w6300_hal.c)
    set(WIZNET_HAL_LIBS  hardware_pio)
else()
    set(WIZNET_CHIP_SRCS
        ${WIZNET_DIR}/Ethernet/W5500/w5500.c
    )
    set(WIZNET_CHIP_INC  ${WIZNET_DIR}/Ethernet/W5500)
    set(WIZNET_HAL_SRC   src/w5500_hal.c)
    set(WIZNET_HAL_LIBS  hardware_spi)
endif()

add_library(ioLibrary STATIC
    ${WIZNET_DIR}/Ethernet/wizchip_conf.c
    ${WIZNET_DIR}/Ethernet/socket.c
    ${WIZNET_CHIP_SRCS}
)
target_include_directories(ioLibrary PUBLIC
    ${WIZNET_DIR}
    ${WIZNET_DIR}/Ethernet
    ${WIZNET_CHIP_INC}
    ${WIZNET_DIR}/Internet/DHCP
)
target_link_libraries(ioLibrary pico_stdlib ${WIZNET_HAL_LIBS} hardware_gpio)

# ── Application ───────────────────────────────────────────────────────────────
# pioasm generates src/ws2811.pio.h automatically via pico_generate_pio_header
add_executable(ArtNetToWS2811
    src/main.c
    src/ws2811.c
    src/artnet.c
    ${WIZNET_HAL_SRC}
)

# Expose which chip is active to C code
target_compile_definitions(ArtNetToWS2811 PRIVATE WIZNET_${WIZNET_CHIP}=1)

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

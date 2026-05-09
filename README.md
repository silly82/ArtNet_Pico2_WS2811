# ArtNet → WS2811 Bridge

Firmware for the **WIZnet W6300-EVB-Pico2** that receives ArtNet DMX over Ethernet and drives up to **8 parallel WS2811 LED universes** in real time.

Built for LED walls — each output carries one ArtNet universe (170 RGB pixels / 512 DMX channels) at up to 40 fps.

---

## Platform

| | |
|---|---|
| **MCU** | RP2350 (Raspberry Pi Pico 2 core) |
| **Ethernet** | WIZnet W6300 — hardware TCP/IP, IPv4/IPv6, 10/100 Mbps |
| **SDK** | [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) ≥ 2.0 |
| **Build** | CMake + ARM GCC (`arm-none-eabi-gcc`) |
| **Board** | W6300-EVB-Pico2 |

> This project uses the **Pico SDK directly** (not Arduino). Full PIO and DMA access is required for real-time parallel LED output.

---

## Hardware

| Component | Purpose |
|---|---|
| [W6300-EVB-Pico2](https://docs.wiznet.io/Product/Chip/Ethernet/W6300/w6300-evb-pico2) | RP2350 MCU + WIZnet W6300 hardware TCP/IP |
| SN74LVC4245A (LJ245A) | 3.3 V → 5 V level shift for WS2811 data lines |
| 5 V PSU | Power for LEDs + level shifter VCCB |

### Pinout — WS2811 Outputs (8 channels)

| RP2350 GPIO | SN74LVC4245A | ArtNet Universe | PIO Block | State Machine | Default use |
|:-----------:|:------------:|:---------------:|:---------:|:-------------:|-------------|
| **GP0** | A1 → B1 | 0 | PIO0 | SM0 | Universe 0 |
| **GP1** | A2 → B2 | 1 | PIO0 | SM1 | Universe 1 |
| **GP2** | A3 → B3 | 2 | PIO0 | SM2 | Universe 2 |
| **GP3** | A4 → B4 | 3 | PIO0 | SM3 | Universe 3 |
| **GP4** | A5 → B5 | 4 | PIO1 | SM0 | Universe 4 |
| **GP5** | A6 → B6 | 5 | PIO1 | SM1 | Universe 5 |
| **GP6** | A7 → B7 | 6 | PIO1 | SM2 | Reserve |
| **GP7** | A8 → B8 | 7 | PIO1 | SM3 | Reserve |

> `WS2811_BASE_PIN` and `WS2811_NUM_OUTPUTS` in `config.h` control which pins and how many are active.  
> The universe offset is set with `ARTNET_UNIVERSE_START` (default 0).

### Wiring Diagram

```
W6300-EVB-Pico2          SN74LVC4245A          WS2811 strips
─────────────────        ────────────          ─────────────
GP0  ──────────── A1 ──► B1 ───────────────► Universe 0  (170 RGB px)
GP1  ──────────── A2 ──► B2 ───────────────► Universe 1
GP2  ──────────── A3 ──► B3 ───────────────► Universe 2
GP3  ──────────── A4 ──► B4 ───────────────► Universe 3
GP4  ──────────── A5 ──► B5 ───────────────► Universe 4
GP5  ──────────── A6 ──► B6 ───────────────► Universe 5
GP6  ──────────── A7 ──► B7 ───────────────► Universe 6  (optional)
GP7  ──────────── A8 ──► B8 ───────────────► Universe 7  (optional)

3.3V ───────────── VCCA
5V (ext PSU) ───── VCCB
GND ────────────── GND (common with PSU!)
HIGH ───────────── DIR   (A→B direction)
GND ────────────── /OE   (always enabled)

W6300 QSPI via PIO (fixed on board — do not use):
  GP15=INT  GP16=CS  GP17=SCK  GP18=IO0  GP19=IO1  GP20=IO2  GP21=IO3  GP22=RST
```

> **Important:** Connect GND of the external 5 V PSU to the board GND. Missing common ground is the most common cause of signal errors.

---

## Features

- **Hardware ArtNet reception** via W6300 (hardware TCP/IP, no lwIP overhead)
- **Parallel WS2811 output** using RP2350 PIO — all universes sent simultaneously
- **DMA-driven** — zero CPU load during LED output
- **Double-buffered** — new ArtNet data can arrive while the previous frame is being sent
- **40 fps** frame rate (configurable)
- Up to **8 independent universes** (limited by RP2350 PIO state machines)

---

## Configuration

Edit `src/config.h` before building:

```c
#define WS2811_BASE_PIN     0       // first output GPIO
#define WS2811_NUM_OUTPUTS  6       // 1–8

#define ARTNET_UNIVERSE_START  0    // first ArtNet universe → output 0

#define NET_IP     { 192, 168, 1, 200 }
#define NET_SUBNET { 255, 255, 255, 0 }
#define NET_GW     { 192, 168, 1, 1   }
```

---

## Build

### Requirements

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) (RP2350 branch or ≥ 2.0)
- CMake ≥ 3.20
- ARM GCC toolchain (`arm-none-eabi-gcc`)

### Steps

```bash
# 1. Clone WIZnet ioLibrary (W6300 branch)
git clone https://github.com/WIZnet-ioLibrary/ioLibrary_Driver lib/ioLibrary_Driver
cd lib/ioLibrary_Driver && git checkout W6300 && cd ../..

# 2. Copy pico_sdk_import.cmake
cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .

# 3. Configure & build
mkdir build && cd build
cmake .. -DPICO_BOARD=pico2
make -j4
```

Flash `build/ArtNetToWS2811.uf2` via USB (hold BOOTSEL while plugging in).

---

## ArtNet Setup

| Parameter | Value |
|---|---|
| Protocol | Art-Net 4 (UDP) |
| Port | 6454 |
| Universe 0 | → Output GP0 |
| Universe 1 | → Output GP1 |
| … | … |

Compatible with **MadMapper**, **QLC+**, **Resolume**, **sACN-to-ArtNet bridges**, and any Art-Net 4 source.

---

## Architecture

```
Ethernet → W6300 (HW TCP/IP) → UDP socket
                                    │
                              ArtNet parser
                                    │
                         universe buffer (double-buffered)
                                    │
                          DMA → PIO state machine
                                    │
                         SN74LVC4245A (3.3V→5V)
                                    │
                            WS2811 LED strip
```

The RP2350 has 8 PIO state machines across 2 PIO blocks. Each state machine drives one universe independently. DMA transfers run in the background — the CPU only handles Ethernet polling and ArtNet parsing.

---

## License

MIT

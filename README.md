# PRC152-N Firmware

![Platform](https://img.shields.io/badge/platform-ESP32--S2-blue)
![Framework](https://img.shields.io/badge/framework-Arduino%20%2F%20PlatformIO-orange)
![Board](https://img.shields.io/badge/board-Saola--1-lightgrey)

Firmware for the **FCS PRC152-N** — a functional replica of the Harris AN/PRC-152 handheld radio. The radio is a real, working VHF/UHF FM transceiver: channel memories, CTCSS/CDCSS sub-audio, dual watch, a broadcast FM receiver, a detachable KDU (keypad/display unit), and WiFi-based programming — all driven by an ESP32-S2 running this code.

> This project is not affiliated with or endorsed by L3Harris. It is community firmware for a replica product.

## Features

- **VHF/UHF FM transceiver** (A20/SA818-class RF module): VFO and channel modes, 99 memory channels with nicknames, split TX/RX frequencies
- **Sub-audio**: CTCSS/CDCSS encode and decode per channel, adjustable squelch (0–8), wide/narrow bandwidth, high/low power
- **Dual watch**: monitors two channels, locking onto whichever receives a signal
- **Broadcast FM receiver** (RDA5807) with station scan
- **KDU support**: the detachable keypad/display unit controls the radio over its serial link, as on the original
- **WiFi programming**: the radio hosts its own access point with a web UI for channel/parameter programming, real-time remote control, and over-the-air firmware updates
- **Radio ergonomics**: backlight timeout, keypad lock, TX time-out timer, roger/PTT tones, low-battery protection, auto power-off, zeroize

## Hardware at a glance

| Subsystem | Part |
|---|---|
| MCU | ESP32-S2 (Saola-1 module) |
| RF transceiver | A20 (SA818-class), controlled over UART at 9600 baud |
| Broadcast FM | RDA5807 |
| Display | 128×32 LCD + PWM backlight |
| I/O expander | CH423 (power rails, audio routing) |
| Audio gain matrix | M62364 DAC |
| Battery | 4× 18650 (2S2P, ~7.4 V), 12 V input also supported |

## Building

### Prerequisites

The [PlatformIO Core CLI](https://docs.platformio.org/en/latest/core/installation/index.html) (`pio`):

```sh
# macOS
brew install platformio

# any OS, via pip
pip install platformio
```

All other dependencies (ESP32 toolchain, Arduino core, ArduinoJson) are fetched automatically on first build.

### Build

```sh
pio run
```

The binary lands in `.pio/build/esp32-s2-saola-1/` as `PRC152-N<version>.bin` (the version comes from `custom_prog_version` in `platformio.ini`).

## Testing

- **Host unit tests**: `pio test -e native` (Unity framework; pure logic in `lib/pure_logic/` and `include/kdu_protocol.h`)
- **On-device tests**: `pytest tools/hil --serial <port>` against a dev build (see [`tools/hil/README.md`](tools/hil/README.md); hardware required)
- **Dev console**: `pio run -e esp32-s2-saola-1-dev` embeds an interactive console over serial/HTTP for testing

See `CLAUDE.md` (architecture) and [`docs/superpowers/`](docs/superpowers/) (design and plan) for details.

## Flashing your radio

There are two ways to get firmware onto the radio: over WiFi (easiest, no wiring) or over the serial programming connector.

### Option 1 — over WiFi (recommended)

1. With the radio **off**, hold the **squelch key** and power on. Keep holding for ~3 seconds until the maintenance menu appears.
2. Select **`1. WIFI To CONTROL`**. The radio starts its access point and shows the address on screen.
3. Connect your computer or phone to the WiFi network **`FCS_Configure`** (default password **`123456789`**).
4. Open **http://192.168.152.1** in a browser (any address works — the radio runs a captive portal).
5. Use the firmware-update section of the page to upload the `PRC152-N<version>.bin` you built.
6. The radio shuts down when the update completes; power it back on and check the version under `4. DEVICE ABOUT` or in the OPTION menu.

The same web page also programs channels and settings, so this is the everyday way to configure the radio too. Double-click the encoder knob to leave WiFi mode.

### Option 2 — over the serial connector

The radio's programming header exposes VCC, RX, TX, GND, DTR and RTS — connect it to a USB-serial adapter (DTR/RTS are used for auto-reset into the bootloader), then:

```sh
# find your port first
pio device list

# macOS
pio run -t upload --upload-port /dev/cu.usbserial-XXXX

# Linux
pio run -t upload --upload-port /dev/ttyUSB0

# Windows
pio run -t upload --upload-port COM17
```

Note: `platformio.ini` hardcodes `COM17` (Windows); on macOS/Linux always pass `--upload-port` explicitly.

### Serial monitor

```sh
pio device monitor    # 115200 baud
```

## The boot maintenance menu

Holding the squelch key at power-on for ~3 s opens a small recovery/service menu, independent of the main firmware UI:

1. **WIFI To CONTROL** — WiFi AP for programming and OTA updates (see above)
2. **USB To UPGRADE** — firmware update over the serial connector using the vendor's PC tool protocol
3. **USB To CHANNEL PGM** — channel programming over serial
4. **DEVICE ABOUT** — hardware/firmware version info

## Repository layout

```
include/FCS152_KDU.h    Chip selection, KDU protocol field layout, version strings
src/main.cpp            setup() + the main super-loop
src/main_fun.cpp        Main VFO page, menus, PTT/squelch/channel logic
src/bsp_*.cpp           Board support: UART, I2C, timers, storage, WiFi, power
src/lcd.cpp, font.cpp   Display drawing
src/rda5807.cpp         FM broadcast receiver
src/html_*.cpp          Embedded web pages (programming + remote control)
```

The firmware is a single-threaded super-loop (no RTOS tasks) ported from an earlier STM32 target — see `CLAUDE.md` for the architecture notes and `.github/copilot-instructions.md` for the code review checklist.

## Contributing

- `pio run` must build cleanly.
- Say in your PR what was hardware-tested (the `tools/hil` suite counts), or list what still needs a radio to verify.
- The review bar and firmware-specific pitfalls (loop timing, ISR safety, KDU protocol layout, power regressions) are documented in [`.github/copilot-instructions.md`](.github/copilot-instructions.md).
- Write new comments in English; existing Chinese comments are being translated over time.

## Transmit responsibly

This firmware will transmit on whatever frequency it is told to. Operating a transmitter requires an appropriate amateur radio license (or other authorization) in virtually every country, and the legal bands, power limits and tone requirements vary by region. Know your local regulations before keying up.

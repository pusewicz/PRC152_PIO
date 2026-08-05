# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Firmware for the "FCS PRC152-N", a replica of the Harris PRC-152 radio, targeting the ESP32-S2 (Saola-1 board) on the Arduino framework via PlatformIO. The code was ported from STM32F103RET6/CM32M101A targets: `THISCHIP` in `include/FCS152_KDU.h` selects the chip (fixed to `THISCHIP_ESP32S2` here), and dead `#if` branches for the other chips remain throughout. Comments are a mix of English and Chinese (translation to English is in progress).

## Commands

Requires the PlatformIO Core CLI (`pio`); on macOS install via `brew install platformio`.

- Build: `pio run`
- Flash: `pio run -t upload --upload-port <port>` — `platformio.ini` hardcodes `COM17` (Windows); override on macOS (e.g. `/dev/cu.usbserial-*`)
- Serial monitor: `pio device monitor` (115200 baud)

There is no test suite and no linter.

## Versioning

The firmware version lives in two places that must be kept in sync:

- `custom_prog_version` in `platformio.ini` — becomes the binary name (`PRC152-N<version>.bin`) via the `name_firmware.py` pre-script.
- `VERSION_152` / `VERSION_KDU` strings in `include/FCS152_KDU.h`, shown in the UI. A changelog of past revisions is kept in comments directly above them.

## Architecture

Single-threaded Arduino super-loop; no RTOS tasks. `loop()` in `src/main.cpp` polls in order: main-page LCD refresh (`VFO_Refresh`), global state processing (`MY_GLOBAL_FUN`: PTT, squelch, battery, sleep), encoder, matrix-keypad events, and KDU serial processing (`KDU_Processor`). Menus are blocking functions that run their own poll loops until exit.

`setup()` initializes hardware in a deliberate order (startup-noise suppression, power-rail sequencing) — preserve the order when modifying it.

### Layers

- `bsp_*` files are the board-support layer: UART, I2C, CH423 I2C GPIO expander, M62364 DAC, ADC/DAC/PWM, timers, LCD transport, pin definitions (`bsp_conio`), persistent storage, WiFi/web server, and power/sleep/boot menu (`bsp_device`).
- Mid-level drivers: `lcd.cpp` + `font.cpp` (drawing), `key.cpp`/`encoder.cpp` (input), `tim_int.cpp` (software timers), `controller.cpp` (power rails, MIC/SPK routing), `rda5807.cpp` (FM broadcast receiver, enabled via `FM_EN` in `FCS152_KDU.h`).
- Application: `src/main_fun.cpp` (~4,200 lines) holds the main VFO page, all menus (RT/TX-RX settings, PGM, OPTION, Zeroize, shortcuts), channel switching, and PTT/squelch logic. `handleData.cpp` + `bsp_json.cpp` marshal parameters to/from the `parameterValue[]` string table used for JSON/WiFi exchange.

### External units and protocols

- **A20 RF module** (UART2): the actual transceiver. Configured via `Set_A20*` calls; `A002_CALLBACK()` must run for its responses to be consumed.
- **KDU** — detachable keypad/display unit (UART1): exchanges one framed ASCII buffer containing all parameters. Field offsets are the `Length_*` / `*_RANK` / `*_rank` constant chains computed in `FCS152_KDU.h`; payload bytes are offset by `'0'` (`kdu_send_data`/`kdu_recv_data` in `userinclude.h`). Changing any field's length means updating the whole `Length_*`/`*_RANK` chain on both radio and KDU sides.
- **WiFi**: SoftAP + DNS + WebServer serving two HTML pages embedded as C strings — `html_PGM.cpp` (parameter programming) and `html_RCU.cpp` (real-time remote control) — plus OTA firmware update (`ConfigureToUpdate` in `bsp_wifi.cpp`).

### Data model and persistence

`CHAN_ARV` (in `FCS152_KDU.h`) is the channel record: RX/TX frequency, sub-audio tones, power, bandwidth, nickname, scan flag. Four in-RAM slots (`chan_arv[]`): `NOW` (active), `TMP` (edit buffer), `CHANA`/`CHANB` (dual-watch mode). Settings persist through `bsp_storage` `save_*`/`load_*` pairs; `Init_Storage`/`DATA_Init` handle first-boot and zeroize defaults.

### Boot maintenance menu

`enterSecondSystem()` (`bsp_device.cpp`) polls the squelch key for ~3 s at boot; holding it enters the maintenance menu (WiFi firmware update, USB update, channel settings, about).

## Code review

When reviewing changes or PRs, apply `.github/copilot-instructions.md` — it contains the firmware-specific review checklist (loop-path timing, the empty `DISABLE_INT` critical-section trap, buffer/protocol/version sync, power regressions) and the merge bar. Since there is no test suite, every behavioral change must state what was hardware-tested or list what still needs hardware testing.

## Subagents

Project agents in `.claude/agents/`: `firmware-engineer` (implementation), `firmware-reviewer` (read-only diff review), `hardware-radio-expert` (pins/rails/RF/protocol consulting), `power-perf-optimizer` (idle-power work, carries the measured power analysis and ranked backlog). Prefer delegating matching work to them.

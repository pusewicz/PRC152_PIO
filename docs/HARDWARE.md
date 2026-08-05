# Target Hardware

This document describes the physical hardware this firmware runs on and drives: the MCU
board, the peripheral chips wired to it, and the pin/bus map connecting them. Facts pulled
directly from this repo's source are cited as `file:line`; facts from external sources are
cited with a link. Anything inferred rather than stated outright is flagged as such.

## What this firmware targets

The "FCS PRC152-N" is a hobbyist/prop replica of the real Harris (now L3Harris) AN/PRC-152
Falcon III combat-net radio — a genuine multiband military handheld fielded from the
mid-2000s for encrypted VHF/UHF voice and data comms ([Wikipedia][wiki-prc152]). The
replica has no RF transceiver capability of its own comparable to the original; it's a
display/UI/control shell around a narrowband FM transceiver module, built primarily for
airsoft/collector use.

The manufacturer's product page for the closely related **FCS152A** ([fcs.fun/FCS152A][fcs-page])
states that the current ("official") hardware revision uses **an ESP32-S2 module running
Arduino**, replacing an earlier "trial version" that used an STM32F103RET6/GD32F103RET6 MCU
under Keil MDK — this matches exactly the `THISCHIP` dead-branch history in
`include/FCS152_KDU.h` (see [Firmware variants](#firmware-variants--chip-selection) below).
The same manufacturer page directly hosts downloadable source archives named
`PRC152_PIO.zip` and `FCS_152-KDU.zip`, strongly suggesting this repository (and its KDU
counterpart) descend from the manufacturer's own released source.

Manufacturer/reseller-stated physical characteristics of the enclosure (not verifiable from
source code, listed here for context only):

- Full-metal, CNC-machined aluminum body; "three anti" (waterproof/shockproof/dustproof)
  design ([fcs.fun][fcs-page], [emperionstore.com][emperion-listing])
- Stainless-steel 6-pin connector, MIL-pattern, compatible with common third-party
  PTT/mic accessories ([emperionstore.com][emperion-listing])
- Runs on 4×18650 cells via an external battery case, or AC adapter, or USB data-line
  power when tethered to a PC for programming ([fcs.fun][fcs-page], [emperionstore.com][emperion-listing])
- Dot-matrix LCD on both the main body and the detachable KDU ([emperionstore.com][emperion-listing])

No teardown or forum writeup of this specific product's internal PCB was found; the GPIO
map below is reconstructed entirely from this repo's source.

## MCU: ESP32-S2-Saola-1

`platformio.ini:12-14`: `platform = espressif32`, `board = esp32-s2-saola-1`,
`framework = arduino`.

Espressif's ESP32-S2-Saola-1 is a WROVER-module dev board built around the ESP32-S2 SoC
([Espressif datasheet][esp32s2-datasheet], [Saola-1 user guide][saola1-guide]):

- Single-core Xtensa LX7 @ up to 240 MHz, 128 KB ROM, 320 KB SRAM, 16 KB RTC SRAM; flash
  (and PSRAM, on -R/-N4R2-class modules) external via SPI/QSPI
- **WiFi-only** (802.11 b/g/n) — no Bluetooth/BLE, which avoids radio-coexistence handling
- Native full-speed USB OTG (separate from the USB-UART bridge used for flashing/`Serial`)
- 43 GPIOs, 2× UART, 2× I2C (hardware peripheral — unused here, see [I2C bus](#bit-banged-i2c-bus--ch423-gpio-expander)),
  4× SPI, I2S, 2× 12-bit SAR ADC, 2× 8-bit native DAC, LEDC (PWM) up to 8 channels

This board was very likely chosen for its low cost, high GPIO count, and native USB —
there's no use of Bluetooth or the hardware I2C/SPI peripherals in this firmware; nearly
every bus is bit-banged (see below).

## Firmware variants / chip selection

`include/userinclude.h:14-16` defines the three historical targets:

```c
#define THISCHIP_STM32F103RET6   0
#define THISCHIP_CM32M101A       1
#define THISCHIP_ESP32S2         2
```

`include/FCS152_KDU.h:10` hardcodes `#define THISCHIP THISCHIP_ESP32S2` — the
STM32F103RET6 and CM32M101A `#elif` branches (`FCS152_KDU.h:14-24,51-63,66-96`) are dead
code for this target, kept around from the STM32/CM32 port. One artifact of this:
`ResetSystem()` on ESP32S2 is just `ESP.restart()` (`FCS152_KDU.h:54-56`) — there's no real
watchdog timer kick on this target (`FeedDog()` is a bare `delay_ms(5)`), unlike the STM32
branches which touch real `IWDG` registers.

Per the manufacturer's version-comparison table, the detachable KDU head unit's own MCU
went through the same STM32→CM32M101A transition (official "KDU-N" = CM32M101A;
[fcs.fun][fcs-page]) — this firmware only targets the radio body, not the KDU's own MCU.

`VERSION_152 = "Rev 2.2.5507"`, `VERSION_KDU = "Rev 2.1.1226"` (`FCS152_KDU.h:108-120`,
inside the `__NEW__` branch shared by CM32M101A and ESP32S2). The manufacturer's public
changelog (`fcs.fun`) additionally warns that trial and official firmware images are not
cross-flashable and mixing them "will cause the machine to change bricks" — i.e. there's no
safe path between the STM32-based trial hardware and the ESP32-S2 official hardware this
repo targets.

## Pinout (GPIO map)

Reconstructed from `include/bsp_conio.h`, `include/bsp_iic.h`, `include/bsp_lcd.h`,
`include/bsp_m62364.h`, `include/bsp_MatrixKeyBoard.h`, and `src/bsp_uart.cpp`.

| Function | GPIO | Source |
|---|---|---|
| Battery-voltage ADC | 10 | `bsp_conio.h:8` |
| Native DAC (sidetone) / UART2 RX (shared name `DAC_RX_PIN`) | 18 | `bsp_conio.h:9`, `bsp_uart.cpp:48`, `bsp_timer.h:20` |
| LCD backlight PWM | 35 | `bsp_conio.h:10` |
| Power/encoder-click ("wake") button | 21 | `bsp_conio.h:13,36` |
| Volume + | 6 | `bsp_conio.h:15` |
| Volume − | 7 | `bsp_conio.h:18` |
| PTT | 4 | `bsp_conio.h:21` |
| Squelch button | 5 | `bsp_conio.h:24` |
| Encoder spin (A) | 20 | `bsp_conio.h:30` |
| Encoder spin (B) | 19 | `bsp_conio.h:33` |
| Main power-rail enable | 33 | `bsp_conio.h:40` |
| A20 module: squelch in | 15 | `bsp_conio.h:65` |
| A20 module: power-down | 16 | `bsp_conio.h:66` |
| A20 module: PTT out | 14 | `bsp_conio.h:67` |
| Bit-banged I2C SCL | 9 | `bsp_iic.h:5` |
| Bit-banged I2C SDA | 8 | `bsp_iic.h:6` |
| LCD CS | 36 | `bsp_lcd.h:11` |
| LCD RST | 37 | `bsp_lcd.h:15` |
| LCD RS | 38 | `bsp_lcd.h:19` |
| LCD SDA (bit-banged) | 39 | `bsp_lcd.h:23` |
| LCD SCL (bit-banged) | 40 | `bsp_lcd.h:27` |
| M62364 LD (latch) | 13 | `bsp_m62364.h:24` |
| M62364 CLK | 12 | `bsp_m62364.h:28` |
| M62364 DATA | 11 | `bsp_m62364.h:32` |
| UART2 TX (to A20 module) | 17 | `bsp_uart.cpp:48` |
| Matrix keypad rows | 2, 3, 0, 1 | `bsp_MatrixKeyBoard.h:12-15` |
| Matrix keypad columns | 41, 42, 45, 46 | `bsp_MatrixKeyBoard.h:18-21` |

Notes:

- `DAC_RX_PIN` (GPIO18) is reused as both the label for the native-DAC pin and UART2's RX
  pin — almost certainly a leftover name from the STM32 port rather than two physical
  pins; worth cleaning up if touching that code.
- Power-rail selects (8V/12V), audio routing (MIC/SPK in/out), FM amp/supply enable, and
  the 6-pin accessory power (VDO) are **not** direct ESP32-S2 GPIOs — they're channels on
  the CH423 I2C expander, driven through `SetIOChannel`/`ClrIOChannel`/`SetOCChannel`/
  `ClrOCChannel` macros (`bsp_conio.h:41-92`).
- UART1 (to the KDU) uses the Arduino core's default `Serial` object with no explicit pin
  assignment in this codebase (`bsp_uart.cpp:12`) — on the ESP32-S2-Saola-1 that maps to
  the board's onboard USB-UART bridge pins by default; the exact GPIOs aren't
  determinable from this repo's source alone.

## Serial peripherals

Two UARTs, both ASCII/framed-text protocols, no hardware handshaking:

- **UART1 — KDU link / firmware upgrade** (`bsp_uart.h:9`, comment: "串口1 KDU/固件升级"):
  the Arduino default `Serial`, 115200 baud (`bsp_uart.cpp:12,71`). Carries the single
  framed ASCII buffer exchanging all radio parameters with the detachable KDU: each field's
  byte offset is computed by a `Length_*`/`*_RANK` constant chain (e.g. `Length_CHAN`,
  `Length_RX`, ... at `FCS152_KDU.h:303-324`, summed into `CHAN_RANK`, `RX_RANK`, ... at
  `FCS152_KDU.h:350-360`). Buffer size `USART1_BUF_SIZE` = 1024+8+1+50 bytes
  (`FCS152_KDU.h:127`).
- **UART2 — A20 RF module** (`bsp_uart.h:15`, "串口2 A20模块"): `Serial1`, TX=GPIO17,
  RX=GPIO18, 9600 baud (`bsp_uart.cpp:48`, called from `bsp_A002_Init()` at
  `bsp_conio.cpp:12`). Buffer size `USART2_BUF_SIZE` = 255 bytes.

The A20 module speaks an AT-command protocol, e.g. `AT+DMOSETGROUP=1,436.025,436.025,000,1,001,1`,
`AT+DMOSETMIC`, `AT+DMOAUTOPOWCONTR`, `AT+DMOREADRSSI`, `AT+DMOSETVOLUME`
(`bsp_uart.cpp:108` and surrounding). The `DMO*` command family is characteristic of the
common DRA818/SA818-class narrowband FM transceiver modules used widely in hobbyist radio
projects — this repo's "A20" is very likely one of that module family, or a compatible
clone, though no explicit part-number string appears in source. `A002_CALLBACK()`
(`bsp_uart.cpp:209`) parses its `+DMOCONNECT:`/`+DMOSETGROUP:`-style responses; its own
header comment states it "must be processed for A20 data to be returned"
(`include/bsp_uart.h:32`), and callers poll it explicitly rather than relying on an
interrupt (`src/controller.cpp:69,75`, `src/main_fun.cpp:696,3180`).

## Bit-banged I2C bus & CH423 GPIO expander

I2C is entirely software bit-banged on GPIO8 (SDA) / GPIO9 (SCL) — `src/bsp_iic.cpp`
manually clocks start/stop/ack/byte transfers with `digitalWrite`/`digitalRead` and
`delay_us(1)` bit timing. The ESP32-S2's two hardware I2C peripherals are not used.

Two devices share this bus:

**CH423** — a WCH (Nanjing Qinheng Microelectronics) I2C GPIO expander, address `0x40`
(`bsp_ch423.h:5`). Provides 8 bidirectional GPIO + 16 output-only GPO over I2C
([WCH product page][ch423-page], [DFRobot wiki][ch423-dfrobot]). In this firmware it fans
out: FM amp enable, 8V/12V power-rail enable, MIC in/out enable, SPK in/out enable, and the
6-pin accessory-power (VDO) line (`bsp_ch423.h:69-101`). Init sequence
(`bsp_ch423.cpp:9-21`) clears all outputs, sets IO direction, then brings IO0/IO1/IO7 high
and OC8 high as a known startup state.

**RDA5807** — an RDA Microelectronics single-chip FM broadcast receiver, I2C addresses
`0x22`/`0x23` (8-bit write/read, i.e. 7-bit `0x11`) (`include/rda5807.h:5-6`,
[datasheet][rda5807-datasheet]). Tunes ~50–115 MHz with RDS/RBDS decode, auto seek/tune,
and two programmable GPIOs; output is analog line-level, no onboard amp
([ElectronicWings][rda5807-ew]). Compiled in via `FM_EN` (`FCS152_KDU.h:139`) as the
broadcast-FM listen feature (`src/rda5807.cpp`, 1015 lines — communicates over the same
bit-banged bus, `rda5807.cpp:9,31-39`).

## M62364 — audio-level control DAC (not on the I2C bus)

Despite sharing a shelf with the CH423 and RDA5807 in casual descriptions of this
firmware's peripherals, the M62364 is wired to its **own dedicated 3-wire bit-banged
bus**, separate from the I2C bus above — LD=GPIO13, CLK=GPIO12, DATA=GPIO11
(`bsp_m62364.h:24-32`), driven by `M62364_sendData()` (`bsp_m62364.cpp:38-59`, 12-bit
shift-out: 8 data bits + 4-bit channel address).

The M62364 is an 8-bit, 8-channel multiplying D/A converter with buffered outputs —
originally a Mitsubishi Electric part, now second-sourced by Unisonic Technologies (UTC)
([UTC datasheet][m62364-datasheet]). This firmware drives 6 of its channels
(`bsp_m62364.h:5-11`): `TONE_OUT`, `FM_S_EN` (FM receiver power), `MIC_OUT`, `MIC_IN`,
`WFM_LINE`, `A20_LINE` — i.e. it's the analog volume/level-control point for every audio
path in the radio, plus the FM receiver's power gate.

## ADC / native DAC / PWM

- **ADC**: `bsp_ADC_Init()` (`bsp_analog.cpp:16-21`) reads GPIO10 at 13-bit resolution
  (`analogReadResolution(13)`, the ESP32-S2's native ADC width). `Use_ADC()` averages 10
  samples with min/max trimming, then scales by `v0 * 2563 / 8191 * 321 / 51`
  (`bsp_analog.cpp:69`) — the constants imply a resistor-divider calibration for battery
  voltage, but the divider ratio/reference aren't documented in code.
- **DAC**: uses the ESP32-S2's native hardware DAC peripheral (`driver/dac.h`),
  `DAC_CHANNEL_2` — which maps to GPIO18, the same pin reused as UART2 RX (see pinout
  notes above). Used only to generate sidetone/ring sine waves (2kHz/1.5kHz lookup
  tables, `bsp_dac.cpp:38,79-89`) from a hardware-timer ISR.
- **PWM**: `bsp_PWM_Init()` (`bsp_analog.cpp:10-15`) uses the ESP32 LEDC peripheral,
  channel 1, 24kHz, 8-bit resolution, on GPIO35 — drives LCD backlight brightness.

## LCD

Fully bit-banged over 5 GPIOs (CS=36, RST=37, RS=38, SDA=39, SCL=40) — `LCD_Write()`
(`src/lcd.cpp:100-124`) manually clocks bits on SDA/SCL with CS/RS framing; the ESP32's
hardware SPI peripheral is not used.

`LCD_Init()` (`src/lcd.cpp:8-97`) contains two controller init sequences selected by the
`_LCD` macro (`include/bsp_lcd.h:9`): an `LCD12864` (128×64) branch and the active
`LCD12832` (128×32) branch — this firmware is built for the **128×32** display; the 128×64
path is dead code here, same pattern as the STM32/CM32 dead branches elsewhere. The init
command bytes (`0xE2` soft-reset, `0x2C/0x2E/0x2F` booster steps, `0x81` contrast-set) are
characteristic of an ST7565/UC1701-family mono LCD controller — inferred from the command
sequence, not stated by name in source.

## Storage

`src/bsp_storage.cpp` (893 lines) uses **ESP-IDF NVS** (Non-Volatile Storage), not raw
flash or emulated EEPROM: `nvs_flash_init()`, then `nvs_open("PRC152MARK2", NVS_READWRITE, …)`
(`bsp_storage.cpp:46-59`). Every setting (channel params, mic level, squelch, step,
backlight, WiFi SSID/password, FM frequency, etc.) is an individual NVS key/value pair.
The dead STM32/CM32 branches in `FCS152_KDU.h:65-96` reference raw flash-address constants
for a bootloader/app partition scheme (e.g. `KDU_RUN_ADDR 0x08010000`) that's simply unused
on this target (`FCS152_KDU.h:66`).

## WiFi / web server

`src/bsp_wifi.cpp` (915 lines) uses the Arduino-ESP32 `WiFi` library in **SoftAP mode
only** — no station/client WiFi code exists. Default SSID `"FCS_Configure"`, default
password `"123456789"` (`bsp_wifi.cpp:5-6`, both runtime-mutable and NVS-persisted).
Static AP address `192.168.152.1/24` (`bsp_wifi.cpp:7,9,317`), with a `DNSServer` doing
wildcard captive-portal-style redirection (`bsp_wifi.cpp:8,325-333`).

Two route sets on `WebServer server(80)`: `initWebServer_PGM()` (parameter programming —
`/`, `/finish`, `/update`, `/set`, `/getAll`, `/getChan`) and `initWebServer_RCU()`
(real-time remote control), serving `html_PGM.cpp`/`html_RCU.cpp` as embedded C-string
literals. OTA firmware update goes through the standard ESP32 `Update` library over HTTP
(`bsp_wifi.cpp:70`, `/update` handler) — not a custom bootloader, which is why the
STM32-style flash-address bootloader constants mentioned above go unused here.

Per the manufacturer's public changelog, more recent official firmware moved WiFi
programming from SmartConfig to a direct phone-to-radio connection, and added an "RCU
control mode" for routine WiFi remote control alongside the USB/CPS-software and Android
APP programming paths ([fcs.fun][fcs-page]) — this repo's `initWebServer_RCU()` maps to
that RCU feature.

## Power-up sequencing (`src/main.cpp` `setup()`)

Order matters here — noise suppression (step 3) and power-rail gating (steps 5, 12) happen
before any RF/audio hardware is enabled (step 13 onward):

1. `UART1_Init()` — KDU/debug `Serial` at 115200 baud.
2. `CH423_Init()` — bit-bang I2C bring-up, expander outputs reset to a known (rails-off) state.
3. `SPK_SWITCH(IN, OFF)` — mute the speaker path before anything else powers up (code
   comment: "限制开机的杂音", suppress startup noise).
4. `Key_Init()` — encoder, function keys, matrix keypad.
5. `Standby_Init()` — polls the power/encoder-click pin; only asserts the main power-rail
   enable (GPIO33) after a sustained hold, otherwise drops into a standby/sleep loop.
6. `ADC_Init()` — battery-voltage sensing.
7. `Timer_Init()` — hardware timers (1ms/100ms/7µs) plus software auto-timers for
   key-scan, blink, RSSI-poll, voltage-refresh.
8. `M62364_Init()` — zero all audio/FM-power channels (explicitly mutes output and cuts FM
   power before anything is enabled).
9. `LCD_Init()`, `PWM_Init()` — display + backlight.
10. `Init_Storage(true)` — NVS bring-up.
11. `enterSecondSystem()` — squelch-hold check for the boot maintenance menu.
12. `EN_GPIO_Init()` — reads battery voltage, selects the 8V or 12V power rail via CH423,
    shuts down on out-of-range voltage.
13. `SineWave_Data()`, `VFO_Load_Data()`, `A002_Init()` (opens UART2 to the A20 module,
    sets an initial volume, calls `Set_A20()`), `VFO_Clear()`.

## Sources

- [fcs.fun/FCS152A][fcs-page] — manufacturer product/changelog page (hardware revision
  table, firmware changelogs, download links)
- [emperionstore.com — AN/PRC-152(A) listing][emperion-listing] — reseller-stated physical specs
- [Wikipedia — AN/PRC-152][wiki-prc152] — real-radio background
- [Espressif ESP32-S2 datasheet][esp32s2-datasheet]
- [ESP32-S2-Saola-1 user guide][saola1-guide]
- [RDA5807 datasheet][rda5807-datasheet], [ElectronicWings RDA5807 reference][rda5807-ew]
- [WCH CH423 product page][ch423-page], [DFRobot CH423 module wiki][ch423-dfrobot]
- [Unisonic Technologies M62364 datasheet][m62364-datasheet]

[fcs-page]: https://www.fcs.fun/FCS152A
[emperion-listing]: https://emperionstore.com/an-prc-152-a-inter-intra-multiband-radio-cnc-no-battery-fcs
[wiki-prc152]: https://en.wikipedia.org/wiki/AN/PRC-152
[esp32s2-datasheet]: https://documentation.espressif.com/esp32-s2_datasheet_en.pdf
[saola1-guide]: https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32s2/hw-reference/esp32s2/user-guide-saola-1-v1.2.html
[rda5807-datasheet]: https://cdn-shop.adafruit.com/product-files/5651/5651_tuner84_RDA5807M_datasheet_v1.pdf
[rda5807-ew]: https://www.electronicwings.com/components/rda5807-i2c-fm-receiver/1/datasheet
[ch423-page]: https://www.wch-ic.com/products/CH423.html
[ch423-dfrobot]: https://wiki.dfrobot.com/SKU_DFR0979_Gravity_CH423_I2C_24_Digital_IO_Expansion_Module
[m62364-datasheet]: https://www.unisonic.com.tw/uploadfiles/836/part_no_pdf/M62364.pdf

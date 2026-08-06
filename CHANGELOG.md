# Changelog

All notable changes to the PRC152-N firmware are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); version numbers continue the vendor's `Rev X.Y.ZZZZ` numbering carried over from the STM32/CM32 firmware this project ports (see `include/FCS152_KDU.h` for pre-port revision history).

## [Unreleased]

### Added

- Compile-time guards pinning the KDU frame and storage offsets (`include/kdu_protocol.h`): changing any field length now fails the build instead of silently desyncing the KDU protocol or EEPROM layout. The stale offset comments in the header were corrected to the real computed values.
- A debug-build test console (`pio run -e esp32-s2-saola-1-dev`): serial and WiFi (`POST /dev`) transports for reading/writing radio state, injecting key/encoder events, and dumping a shadow copy of the LCD — plus an on-device pytest suite in `tools/hil/`. Release firmware is unchanged (verified code-identical).
- Host-side unit tests (`pio test -e native`) covering the frequency step-correction math and channel parameter marshalling, now run in CI alongside a build of the devconsole env.

## [2.2.5507] - 2026-08-06

### Added

- Initial release of the ESP32-S2 (Saola-1) port.

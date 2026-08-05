# Changelog

All notable changes to the PRC152-N firmware are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); version numbers continue the vendor's `Rev X.Y.ZZZZ` numbering carried over from the STM32/CM32 firmware this project ports (see `include/FCS152_KDU.h` for pre-port revision history).

## [Unreleased]

### Fixed

- 6.25 kHz-step channels were mistuned by up to 500 Hz: the A20 tuning command sent frequencies rounded to 3 decimal places; it now sends the full 5-decimal value (pending hardware verification of the module's parser). ([#41])
- The A20 tuning command is now built with a bounds-checked `snprintf` and band-validated frequencies, removing a stack-buffer overflow reachable from unvalidated KDU frequency input. ([#4])

[#41]: https://github.com/pusewicz/PRC152_PIO/issues/41
[#4]: https://github.com/pusewicz/PRC152_PIO/issues/4

## [2.2.5507] - 2026-08-06

### Added

- Initial release of the ESP32-S2 (Saola-1) port.

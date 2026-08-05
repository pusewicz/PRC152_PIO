# Testing Strategy for PRC152 Firmware

**Date:** 2026-08-05
**Status:** Approved

## Goals

Protect three things, in priority order:

1. **Protocol invariants** — the KDU frame offset chains (`Length_*` / `*_RANK`) and
   parameter marshalling must not silently desync.
2. **Refactor safety** — the ongoing comment translation and cleanup must not silently
   change behavior of pure logic.
3. **Hardware confidence** — pre-merge checks on the real radio should be a repeatable
   automated script, not a manual memory exercise.

The codebase as-is is largely untestable on a host (global state, hardware macros inside
logic, blocking menu loops, `FCS152_KDU.h` includes `Arduino.h`). The strategy below works
with that reality instead of fighting it: compile-time guards, small carved-out pure-logic
units, and an on-device debug console for automated hardware-in-the-loop testing.

## Phase 1 — Compile-time protocol guards

A `static_assert` block in `include/FCS152_KDU.h` pinning:

- every computed `*_RANK` offset to its golden value (the values already documented in
  the trailing comments: `//3`, `//11`, `//19`, …), and
- the total KDU frame length.

Assert messages state which offset moved and remind that the KDU-side firmware must
change in lockstep. Checked on every build; no test infrastructure.

## Phase 2 — Host-native unit tests

- Add `[env:native]` (`platform = native`) to `platformio.ini`; tests run with
  `pio test -e native` using Unity (PlatformIO's built-in framework).
- Do **not** attempt to make the whole firmware host-compilable. Pure-logic units are
  extracted into small self-contained files with no Arduino dependency, compiled by both
  the firmware and the tests.
- First extraction candidates, in value order:
  1. `checkFreqFloat` (`src/main_fun.cpp`) — frequency validation/step-correction math.
     The global `STEP` becomes a parameter; the firmware keeps a thin wrapper so call
     sites don't change. A characterization test pins current behavior, including the
     past-bug value `430.13751` noted in the code comment.
  2. KDU framing — the `±'0'` encode/decode (`kdu_send_data`/`kdu_recv_data`) plus
     offset arithmetic, tested against hand-built golden frames.
  3. `parameterValue[]` ↔ `CHAN_ARV` marshalling round-trips — the pure sprintf/atoi
     mapping separated from the hardware reads (`PTT_READ`, `Get_A20_RSSI()`) currently
     mixed into `writeOtherValue2buf`.
- Working rule going forward: before translating or refactoring a pure-logic function,
  write a characterization test that pins its current behavior. Coverage grows where the
  risk is; no blanket campaign.
- Optional: a CI job running `pio test -e native` plus a firmware build check.

## Phase 3 — DevConsole + hardware-in-the-loop suite

### Build gating

New `[env:esp32-s2-saola-1-dev]` in `platformio.ini` extending the base env with
`-DDEVCONSOLE`. The ship env produces firmware identical in behavior to today.

### Console core

New `devconsole.cpp`, transport-agnostic command processor. Command families:

- `get <name>` / `set <name> <value>` — backed by the existing `parameterValue[]`
  registry plus extra debug-only names (flags, `chan_arv` slots, RSSI, battery).
- `dump chan|flags|params|screen` — structured JSON out.
- `inject key <code>` / `inject enc cw|ccw|click|long` — synthetic events queued into
  the input scan primitives.
- Responses framed as JSON lines with a distinguishing prefix so they are
  machine-parseable amid `D_printf` output on serial.

### Poll placement (load-bearing decision)

`DevConsole_Poll()` is called from inside the input scan primitives
(`Matrix_KEY_Scan` / `Encoder_Switch_Scan` path), which every blocking menu loop already
polls, plus `loop()`. This keeps the console alive in every UI state without
restructuring the blocking-menu architecture. Work per poll is strictly bounded
(non-blocking reads, at most one command per call) to respect loop-path timing.

### Transports (both from day one)

- **Serial:** line-based commands over UART0/USB-CDC, read non-blockingly by
  `DevConsole_Poll()`.
- **WiFi:** a `/dev` endpoint on the existing WebServer routed to the same command core.
  In `-DDEVCONSOLE` builds the SoftAP is brought up at boot and
  `server.handleClient()` is polled from `DevConsole_Poll()`. (Today the WebServer only
  runs inside the blocking WiFi mode at `bsp_wifi.cpp:574`; debug builds add
  polling during normal operation.)

### Screen dump

In debug builds, `LCD_Write`/`LCD_Setxy` mirror all writes into a ~1 KB shadow buffer
that tracks the LCD controller's address auto-increment. `dump screen` returns the
buffer encoded; the host side can diff regions, assert golden screens, or render to
ASCII/PNG for debugging. This is the fiddliest piece: the shadow must replicate the
address-counter behavior exactly and gets validated once against the real display early.

### HIL suite

`tools/hil/` pytest with a transport fixture (pyserial or HTTP — parametrized, the same
tests run over both), covering flows such as:

- parameter set → get round-trip
- inject keys to enter a frequency → `dump chan` → assert
- channel switch
- VFO screen assertion via `dump screen`

Requires the radio on the desk; documented as the companion to the review rule that
every behavioral change states what was hardware-tested.

### Accepted divergences

Debug builds run with WiFi on (power/timing differ from ship). The serial transport
exists partly so timing-sensitive checks can run with WiFi off.

## Non-goals

- On-target Unity tests (the devconsole covers hardware confidence with less machinery).
- Mocking frameworks.
- Host-testing menus, LCD drawing, or blocking poll loops (the devconsole covers UI
  flows on real hardware instead).
- Refactoring for testability beyond what a specific test needs.

## Rollout order

Phase 1 (immediate protection) → Phase 3 (the big build, enables automated on-device
testing) → Phase 2 (grows opportunistically as characterization tests before each
refactor). Each phase is independently shippable.

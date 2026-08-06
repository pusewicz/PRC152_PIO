# Hardware-in-the-Loop pytest Suite

A pure-Python pytest suite for testing the PRC152 dev firmware's devconsole (serial or WiFi).

## Setup

Install dependencies:
```bash
pip install -r requirements.txt
```

Flash the dev firmware:
```bash
pio run -e esp32-s2-saola-1-dev -t upload --upload-port <port>
```

## Running Tests

### Serial Transport

Close the serial monitor, then:
```bash
pytest --serial /dev/cu.usbserial-* -v
```

### WiFi Transport

1. Join the radio's WiFi network:
   - SSID: `FCS_Configure`
   - Password: `123456789`

2. Run tests:
```bash
pytest --wifi 192.168.152.1 -v
```

## Caveats

- These tests require physical hardware (radio) and are never run in CI.
- Serial monitor must be closed while testing over serial.
- Do NOT attach a physical KDU during serial HIL (unarbitrated interleaving).
- `>enc long` powers the radio off.
- Leaving the on-radio WiFi menus kills the WiFi transport until reboot.
- `>set` performs no range validation (it feeds the radio's apply path with KDU-level trust) — always use in-range values.
- The WiFi console is NOT available while the boot maintenance menu is active (SoftAP comes up at the end of `setup()`); serial console does work in the maintenance menu since it polls the scan primitives.
- `>key` requires a numeric code 0–15; malformed/empty arguments are silently treated as code 0 (a valid key) — always pass explicit codes.
- Issue `>set`/`>kdu` only while the radio is on the main VFO page — dispatching them inside a blocking menu mutates channel state under the menu's stale local copies (last-write-wins confusion, dev-only but flaky).
- `get voltage` and `get rssi` return stale values by design: the console's registry refresh deliberately skips the ADC/A20 hardware reads, so those fields only update via a real KDU/WiFi exchange.
- Responses to `>set`/`>kdu` are preceded by a raw KDU JSON line the radio emits on the same port (`compriseSendJson`) — clients must skip non-`##` lines (the provided client already does).
- Expect roughly a 0.2 s UI stall per large response (`dump screen`, `dump params`) over serial at 115200 baud — one command per poll blocks the super-loop while transmitting.

## Test Structure

- `test_smoke.py`: Basic ping and schema validation
- `test_params.py`: Parameter get/set roundtrips
- `test_ui_flow.py`: User interaction sequences (key/encoder events, menu navigation)
- `test_screen.py`: LCD framebuffer capture and rendering

## Client API

`DevConsole` class methods:
- `.ping()` — health check
- `.get(name)` — read parameter
- `.set(name, value)` — write parameter
- `.dump(what)` — fetch structured state (chan, params, flags, screen)
- `.key(code)` — inject keypad event (0–15 or name like "UP", "ENT")
- `.enc(event)` — inject encoder event (click | double | long | cw | ccw)
- `.screen()` — fetch 1024-byte LCD framebuffer
- `render_screen(data)` — convert framebuffer to 64×128 ASCII art

# Code review instructions — PRC152_PIO firmware

This repo is firmware for a battery-powered VHF/UHF radio (FCS PRC-152 replica): ESP32-S2 (single-core), Arduino framework via PlatformIO, single-threaded super-loop, no RTOS tasks, no test suite, no OTA rollback. A bad merge bricks radios in the field. Review accordingly: correctness and power first, style last.

Build check: `pio run` (PlatformIO CLI). If a PR plausibly affects compilation, it must build cleanly.

## Review priorities, in order

### 1. Loop-path timing (radio responsiveness)

- The idle super-loop (`loop()` in `src/main.cpp`) paces at ~5 ms; `FeedDog()` is actually `delay_ms(5)` (`include/FCS152_KDU.h`), not a watchdog. Flag any new blocking call, long UART wait, or unconditional LCD redraw added to `loop()`, `MY_GLOBAL_FUN`, or any menu's poll loop — anything over ~20 ms per pass degrades PTT/squelch latency.
- Every `while` loop must call `FeedDog()` or `MY_GLOBAL_FUN()` each iteration, or the radio goes deaf inside it. Menus are blocking by design and must pump `MY_GLOBAL_FUN()`.

### 2. Concurrency (this port's biggest trap)

- `DISABLE_INT()` / `ENABLE_INT()` in `include/tim_int.h` are **empty macros** — code that looks like a critical section is not one. `timer0_cb` (1 ms esp_timer) and `timer1_cb` (100 ms Ticker) preempt the loop. Flag any new shared state between loop and timer callbacks that is multi-word or read-modify-write.
- `timer2_cb` (`src/bsp_timer.cpp`) is an IRAM ISR: nothing may be added there beyond the DAC write — no Serial, no allocation, no float, no non-IRAM calls.

### 3. Buffers and parsing

- `strstr`/`atoi` on `rx1_buf`/`rx2_buf` require guaranteed NUL termination after every receive path.
- Check worst-case `sprintf` lengths into fixed buffers (e.g. `BIG_MODE_buf` is 12 bytes and receives channel nicknames).
- Flag new stack buffers over ~256 bytes (existing code already carries 1-2 KB locals; don't let it grow).

### 4. Protocol and version sync (silent field breakage)

- Any change to the `Length_*` / `*_RANK` / `*_rank` chains in `include/FCS152_KDU.h` changes the KDU wire format: the whole chain must be updated consistently and the PR must state KDU-side compatibility impact.
- Version must move in both places together: `custom_prog_version` in `platformio.ini` and `VERSION_152`/`VERSION_KDU` in `include/FCS152_KDU.h`.
- New persisted settings need the `save_*`/`load_*` pair in `src/bsp_storage.cpp` **and** a first-boot/zeroize default in `Init_Storage`/`DATA_Init` — a missing default reads garbage on fresh flash.
- A20 module commands (`Set_A20*`) require `A002_CALLBACK()` to run afterward on the calling path, or the response is never consumed.

### 5. Power (idle current is a feature)

- No busy-wait loops; no polling without a yield; no per-iteration LCD redraws of unchanged content.
- Anything turned on must be turned off: WiFi paths must end in `WiFi.mode(WIFI_OFF)`, FM in `FM_S_EN_CLR`, audio amps in `SPK_SWITCH(x, OFF)`.
- Changes to `setup()` init order, `SHUT()`, `Sys_Enter_Standby`, or rail-enable macros are high-risk (power sequencing and startup-noise suppression are order-dependent) and need explicit justification.

### 6. Hardware correctness

- Pin usage must match `include/bsp_conio.h`. Rail/amp macro polarity is mixed active-high/active-low — verify against the macro definition, not the name.
- GPIO18 is shared between UART2 (A20) and the tone DAC; no A20 traffic during `Start_Tone`/`Start_ToneSql0`.

### 7. Hygiene (advisory, don't block on these)

- New comments in English (existing Chinese comments are being translated over time — leave them unless that's the PR's purpose).
- Don't touch dead `#if THISCHIP` branches for STM32/CM32 in unrelated PRs; don't add new per-chip branches.
- Avoid Arduino `String` and heap allocation in loop paths.

## Merge bar

Block on: categories 1-4 violations, power-rail/init-order changes without justification, build failure. Since there is no test suite, every behavioral PR should state what was tested on hardware (or explicitly "build-only, needs hardware test: X, Y") — flag PRs that claim radio behavior works with no hardware evidence.

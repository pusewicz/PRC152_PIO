# Testing Strategy Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add compile-time KDU-protocol guards, a debug-build devconsole (serial + WiFi) with input injection and screen dump for automated on-device testing, a host-side HIL pytest suite, and a native unit-test environment with the first characterization tests.

**Architecture:** Three independent phases. Phase 1 pins the `*_RANK` offset chain with `static_assert`. Phase 3 adds a `-DDEVCONSOLE`-gated console core polled from the input scan primitives (alive in every blocking menu), speaking a line protocol over `Serial` (shared with the KDU via first-byte arbitration) and over a `/dev` WebServer endpoint. Phase 2 adds `[env:native]` + Unity and extracts pure logic (`checkFreqFloat`, channel marshalling) into Arduino-free units.

**Tech Stack:** PlatformIO (espressif32 + native envs), Unity test framework, ArduinoJson 6, Python 3 + pytest + pyserial + requests for HIL.

## Global Constraints

- Spec: `docs/superpowers/specs/2026-08-05-testing-strategy-design.md`.
- The ship env `esp32-s2-saola-1` must keep today's behavior. All devconsole code is `#ifdef DEVCONSOLE` (or lives in files whose content is entirely inside that guard). Tasks 2, 4, 5, 6 must leave the ship `firmware.bin` byte-identical (verified against a baseline hash). Tasks 3 and 8 restructure shared code and may change the binary layout; they must be semantically inert, verified by build + review.
- There is no test suite on hardware paths: every task lists what it verified by build/native-test and what remains for hardware testing. Collect these in the final task.
- Single-threaded super-loop: no blocking calls, no unbounded loops in any poll path. `DevConsole_Poll()` does bounded work: at most one command executed per call.
- Comments in new code: English. Match existing style (banner-free, brief).
- Do not renumber or change any `Length_*` value — this plan only pins them.
- All commits end with:
  ```
  Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
  Claude-Session: https://claude.ai/code/session_01FykANu4TyjkNsSWKAcEXJa
  ```

## Shared reference: devconsole protocol (used by Tasks 2–7)

Serial request: one line, `>` + command + `\n`. Serial response: one line, `##` + JSON + `\n` (everything else on the port — `D_printf`, KDU JSON replies — is noise the client skips). HTTP: `POST /dev` with form field `c` = the command (leading `>` optional); response body = the JSON (no `##` prefix), content-type `application/json`.

| Command | Response (success) |
|---|---|
| `>ping` | `{"ok":1,"fw":"Rev 2.2.5507","dc":1}` (fw from `VERSION_152`) |
| `>get <name>` | `{"ok":1,"name":"sql","val":"4"}` — name is a `parameterValue[].item` string; registry refreshed first (no hardware reads) |
| `>set <name> <value>` | `{"ok":1}` — applies via the existing `readWriteValueToKDU` dispatch (real side effects); supported names listed in Task 3 |
| `>kdu <json>` | `{"ok":1,"cmd":<n>}` — raw KDU command passthrough into the same dispatch |
| `>dump params` | `{"ok":1,"params":{"current":"008",...}}` |
| `>dump chan` | `{"ok":1,"chan":[{"slot":0,"chan":8,"rx":"435.55000","tx":"435.55000","rs":0,"ts":0,"pw":1,"bw":1,"nn":"...","scan":0}, ...4 slots]}` |
| `>dump flags` | `{"ok":1,"cf":0,"vu":0,"kdu":0,"home":1,"wfm":0,"step":0,"sql":4,"vol":5}` |
| `>dump screen` | `{"ok":1,"pages":8,"cols":128,"hex":"<2048 hex chars>"}` |
| `>key <0-15>` | `{"ok":1}` — MATRIX_RESULT_* code, queued (depth 16; `{"ok":0,"err":"full"}` when full) |
| `>enc click\|double\|long\|cw\|ccw` | `{"ok":1}` — click/double/long queued for `Encoder_Switch_Scan`; cw/ccw adjust `TIMES` immediately |
| any error | `{"ok":0,"err":"<short reason>"}` |

Public API (`include/devconsole.h`), stubs compiled when `DEVCONSOLE` is undefined:

```c
void DevConsole_Init(void);                 // no-op stub in ship builds
void DevConsole_WifiInit(void);             // no-op until Task 6; no-op stub in ship builds
void DevConsole_Poll(void);                 // no-op stub in ship builds
int  DevConsole_Execute(const char *line, char *out, int outsz); // returns strlen(out)
unsigned char DevConsole_TakeInjectedKey(void); // 17 (MATRIX_RESULT_ERROR) if none
unsigned char DevConsole_TakeInjectedEnc(void); // 0 (key_idle) if none
```

Serial arbitration (the load-bearing detail): `Serial` (UART0/USB) is BOTH the flashing/debug port and the KDU port — `bsp_UART1_Init` at `src/bsp_uart.cpp:10` runs `Serial.begin`, and `PRC152receiveProcess` (`src/handleData.cpp:319`) drains it whenever bytes are available. KDU traffic is JSON starting with `{`. Console traffic starts with `>`. Rules:

- `DevConsole_Poll()` consumes from `Serial` only when `Serial.peek() == '>'`, then consumes to `\n` across polls (line state machine — once a line is started, consume available bytes regardless of content until newline).
- `PRC152receiveProcess` gets a guard at the top of its `UART1_getRcvFlag()` branch: in DEVCONSOLE builds, if `Serial.peek() == '>'`, return `NO_OPERATE` (leave the bytes for the console).
- A physical KDU and an HIL client must not be attached simultaneously (interleaving mid-frame is unarbitrated); document in `tools/hil/README.md`.

---

### Task 1: Phase 1 — static_assert protocol guards + baseline capture

**Files:**
- Modify: `include/FCS152_KDU.h` (after line 438, `BUF_SIZE`)
- Modify: stale offset comments on lines 351–395 of the same file

**Interfaces:**
- Produces: nothing callable — compile-time guards only. Later tasks rely on the captured baseline hash in `docs/superpowers/plans/baseline-ship-firmware.sha256`.

- [ ] **Step 1: Capture the ship-firmware baseline BEFORE any edits**

```bash
pio run -e esp32-s2-saola-1
shasum -a 256 .pio/build/esp32-s2-saola-1/firmware.bin | tee docs/superpowers/plans/baseline-ship-firmware.sha256
```

(If the build dir name differs, find the bin with `ls .pio/build/*/firmware.bin`.)

- [ ] **Step 2: Add the static_assert block**

In `include/FCS152_KDU.h`, immediately after the `#define BUF_SIZE …` line (line 438), insert:

```c
// Compile-time pin of the KDU frame layout. These offsets are the de facto
// protocol shared with the KDU-side firmware and the EEPROM storage layout:
// if an assert fires, a Length_* changed — the KDU firmware and any stored
// settings must change in lockstep. Do not just update the numbers here.
static_assert(RX_RANK         == 3,  "KDU frame offset moved: RX_RANK");
static_assert(TX_RANK         == 12, "KDU frame offset moved: TX_RANK");
static_assert(RS_RANK         == 21, "KDU frame offset moved: RS_RANK");
static_assert(TS_RANK         == 24, "KDU frame offset moved: TS_RANK");
static_assert(POWER_RANK      == 27, "KDU frame offset moved: POWER_RANK");
static_assert(BW_RANK         == 28, "KDU frame offset moved: BW_RANK");
static_assert(NN_RANK         == 29, "KDU frame offset moved: NN_RANK");
static_assert(SCAN_RANK       == 37, "KDU frame offset moved: SCAN_RANK");
static_assert(CF_RANK         == 38, "KDU frame offset moved: CF_RANK");
static_assert(VU_RANK         == 39, "KDU frame offset moved: VU_RANK");
static_assert(CHANA_RANK      == 40, "KDU frame offset moved: CHANA_RANK");
static_assert(CHANB_RANK      == 43, "KDU frame offset moved: CHANB_RANK");
static_assert(VOLUME_RANK     == 46, "KDU frame offset moved: VOLUME_RANK");
static_assert(STEP_RANK       == 47, "KDU frame offset moved: STEP_RANK");
static_assert(SQL_RANK        == 48, "KDU frame offset moved: SQL_RANK");
static_assert(AUDIO_RANK      == 49, "KDU frame offset moved: AUDIO_RANK");
static_assert(MIC_RANK        == 50, "KDU frame offset moved: MIC_RANK");
static_assert(ENCRYPTION_RANK == 51, "KDU frame offset moved: ENCRYPTION_RANK");
static_assert(TOT_RANK        == 52, "KDU frame offset moved: TOT_RANK");
static_assert(VDO_RANK        == 53, "KDU frame offset moved: VDO_RANK");
static_assert(PRETONE_RANK    == 54, "KDU frame offset moved: PRETONE_RANK");
static_assert(ENDTONE_RANK    == 55, "KDU frame offset moved: ENDTONE_RANK");
static_assert(FMFREQ_RANK     == 56, "KDU frame offset moved: FMFREQ_RANK");
static_assert(WFM_RANK        == 60, "KDU frame offset moved: WFM_RANK");
static_assert(FMCHAN_RANK     == 61, "KDU frame offset moved: FMCHAN_RANK");
static_assert(VOLTAGE_RANK    == 62, "KDU frame offset moved: VOLTAGE_RANK");
static_assert(RSSI_RANK       == 65, "KDU frame offset moved: RSSI_RANK");
static_assert(KEY_SQ_RANK     == 68, "KDU frame offset moved: KEY_SQ_RANK");
static_assert(KEY_SQU_RANK    == 69, "KDU frame offset moved: KEY_SQU_RANK");
static_assert(KEY_PTT_RANK    == 70, "KDU frame offset moved: KEY_PTT_RANK");
static_assert(HOMEMODE_RANK   == 71, "KDU frame offset moved: HOMEMODE_RANK");
static_assert(NOWRCVCHAN_RANK == 72, "KDU frame offset moved: NOWRCVCHAN_RANK");
static_assert(NOWSELCHAN_RANK == 75, "KDU frame offset moved: NOWSELCHAN_RANK");
static_assert(BUF_SIZE        == 93, "KDU frame total length changed: BUF_SIZE");
static_assert(BACKLIGHTNESS_RANK   == 60, "storage offset moved: BACKLIGHTNESS_RANK");
static_assert(FLAG_BACKLIGHT_RANK  == 63, "storage offset moved: FLAG_BACKLIGHT_RANK");
static_assert(LAMPTIME_RANK        == 64, "storage offset moved: LAMPTIME_RANK");
static_assert(SCREEN_CONTRAST_RANK == 65, "storage offset moved: SCREEN_CONTRAST_RANK");
```

- [ ] **Step 3: Fix the stale trailing comments**

The `//3 //11 //19 …` comments on the `*_RANK` lines (351–395) predate the widening of `Length_RX`/`Length_TX` to 9 and are wrong. Update each to the asserted value from Step 2 (e.g. `TX_RANK` comment `//11` → `//12`, `RS_RANK` `//19` → `//21`, … `NOWSELCHAN_RANK` → `//75`). The two `//////58` comments (`WFM_RANK`, `BACKLIGHTNESS_RANK`) both become `//////60`.

- [ ] **Step 4: Verify the guard actually guards**

Temporarily change `#define Length_RS 3` to `4`, run `pio run -e esp32-s2-saola-1`, confirm the build FAILS with `KDU frame offset moved: TS_RANK` (and successors). Revert to `3`.

- [ ] **Step 5: Verify clean build + unchanged binary**

```bash
pio run -e esp32-s2-saola-1
shasum -a 256 .pio/build/esp32-s2-saola-1/firmware.bin
```

Expected: PASS, hash equal to the baseline (asserts and comments generate no code).

- [ ] **Step 6: Commit**

```bash
git add include/FCS152_KDU.h docs/superpowers/plans/baseline-ship-firmware.sha256
git commit -m "Pin KDU frame offsets with static_asserts"
```

---

### Task 2: Devconsole skeleton — dev env, serial line loop, poll hooks, ping

**Files:**
- Modify: `platformio.ini`
- Create: `include/devconsole.h`
- Create: `src/devconsole.cpp`
- Modify: `src/bsp_MatrixKeyBoard.cpp:94` (`Matrix_KEY_Scan`)
- Modify: `src/encoder.cpp:99` (`Encoder_Switch_Scan`)
- Modify: `src/handleData.cpp:340` (`PRC152receiveProcess` read branch)
- Modify: `src/main.cpp` (`setup()` tail)

**Interfaces:**
- Produces: the full public API from the shared reference above (all functions; `DevConsole_Execute` handles only `ping` for now, everything else returns `{"ok":0,"err":"unknown"}`). Tasks 3–6 extend `DevConsole_Execute`'s dispatch; Task 4 fills the `TakeInjected*` queues (this task implements them as always-empty).

- [ ] **Step 1: Add the dev env and default_envs to `platformio.ini`**

Add to the `[env:esp32-s2-saola-1]`-topped file:

```ini
[platformio]
default_envs = esp32-s2-saola-1

[env:esp32-s2-saola-1-dev]
extends = env:esp32-s2-saola-1
build_flags =
    -DVERSION=${env:esp32-s2-saola-1.custom_prog_version}
    -DDEVCONSOLE
```

(`extends` copies board/framework/lib_deps; `build_flags` must repeat the VERSION define because it is overridden, not merged. Keep `custom_prog_version` only in the base env.)

- [ ] **Step 2: Create `include/devconsole.h`**

```c
/*
    #include "devconsole.h"
    Debug-build test console (see docs/superpowers/specs/2026-08-05-testing-strategy-design.md).
    Whole feature is gated on -DDEVCONSOLE (dev env); ship builds compile the no-op stubs.
*/
#ifndef __DEVCONSOLE_H__
#define __DEVCONSOLE_H__

#ifdef DEVCONSOLE

void DevConsole_Init(void);
void DevConsole_WifiInit(void);
void DevConsole_Poll(void);
int  DevConsole_Execute(const char *line, char *out, int outsz);
unsigned char DevConsole_TakeInjectedKey(void);
unsigned char DevConsole_TakeInjectedEnc(void);

#else

static inline void DevConsole_Init(void) {}
static inline void DevConsole_WifiInit(void) {}
static inline void DevConsole_Poll(void) {}
static inline unsigned char DevConsole_TakeInjectedKey(void) { return 17; } /* MATRIX_RESULT_ERROR */
static inline unsigned char DevConsole_TakeInjectedEnc(void) { return 0; }  /* key_idle */

#endif
#endif
```

- [ ] **Step 3: Create `src/devconsole.cpp`**

```cpp
#include "devconsole.h"
#ifdef DEVCONSOLE

#include "main.h"

#define DC_LINE_SIZE 1024
#define DC_OUT_SIZE  4096

static char dc_line[DC_LINE_SIZE];
static int  dc_line_len = 0;
static u8   dc_in_line = 0;       // saw '>' and consuming until '\n'
static char dc_out[DC_OUT_SIZE];

void DevConsole_Init(void)
{
    dc_line_len = 0;
    dc_in_line = 0;
}

void DevConsole_WifiInit(void) {} // Task 6

unsigned char DevConsole_TakeInjectedKey(void) { return MATRIX_RESULT_ERROR; } // Task 4
unsigned char DevConsole_TakeInjectedEnc(void) { return key_idle; }            // Task 4

int DevConsole_Execute(const char *line, char *out, int outsz)
{
    if (!strncmp(line, "ping", 4))
        return snprintf(out, outsz, "{\"ok\":1,\"fw\":\"%s\",\"dc\":1}", VERSION_152);
    return snprintf(out, outsz, "{\"ok\":0,\"err\":\"unknown\"}");
}

// Bounded serial poll: consume console bytes only ('>' first-byte arbitration
// against KDU JSON frames), execute at most one command per call.
void DevConsole_Poll(void)
{
    static uint32_t last_ms = 0;
    uint32_t now = millis();
    if (now - last_ms < 5)
        return;
    last_ms = now;

    while (Serial.available())
    {
        if (!dc_in_line)
        {
            if (Serial.peek() != '>')
                return; // not ours: leave for the KDU processor
            Serial.read(); // consume '>'
            dc_in_line = 1;
            dc_line_len = 0;
            continue;
        }
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r')
        {
            dc_line[dc_line_len] = '\0';
            dc_in_line = 0;
            if (dc_line_len)
            {
                DevConsole_Execute(dc_line, dc_out, DC_OUT_SIZE);
                Serial.printf("##%s\n", dc_out);
            }
            return; // one command per poll
        }
        if (dc_line_len < DC_LINE_SIZE - 1)
            dc_line[dc_line_len++] = c;
    }
}
#endif
```

(`main.h` pulls in `FCS152_KDU.h` (Arduino, `VERSION_152`), `bsp_MatrixKeyBoard.h` (`MATRIX_RESULT_ERROR`), `userinclude.h` (`key_idle`, `u8`).)

- [ ] **Step 4: Hook the scan primitives**

`src/bsp_MatrixKeyBoard.cpp` — add `#include "devconsole.h"` after the existing includes, then at the very top of `Matrix_KEY_Scan` (before the `bsp_CheckTimer(TMR_KEY_SCAN)` early-return):

```c
    DevConsole_Poll();
    unsigned char inj = DevConsole_TakeInjectedKey();
    if (inj != MATRIX_RESULT_ERROR)
        return inj;
```

`src/encoder.cpp` — add `#include "devconsole.h"`, then at the top of `Encoder_Switch_Scan`:

```c
    DevConsole_Poll();
    unsigned char inj_enc = DevConsole_TakeInjectedEnc();
    if (inj_enc != key_idle)
        return inj_enc;
```

No `#ifdef` needed at call sites — ship builds compile the inline no-op stubs (zero code).

- [ ] **Step 5: Guard the KDU reader**

`src/handleData.cpp`, in `PRC152receiveProcess` directly before `if (UART1_getRcvFlag())` (line 340):

```c
#ifdef DEVCONSOLE
    if (Serial.available() && Serial.peek() == '>')
        return NO_OPERATE; // console traffic; DevConsole_Poll will consume it
#endif
```

- [ ] **Step 6: Init from `setup()`**

`src/main.cpp` — add `#include "devconsole.h"` under `#include "main.h"`, and at the end of `setup()` (after `VFO_Clear();`):

```c
    DevConsole_Init();
    DevConsole_WifiInit();
```

- [ ] **Step 7: Verify both builds**

```bash
pio run                      # ship env only (default_envs)
shasum -a 256 .pio/build/esp32-s2-saola-1/firmware.bin   # must equal baseline
pio run -e esp32-s2-saola-1-dev                           # must compile clean
```

If the ship hash drifted, the stubs are not truly no-op — fix before committing.

- [ ] **Step 8: Commit**

```bash
git add platformio.ini include/devconsole.h src/devconsole.cpp src/bsp_MatrixKeyBoard.cpp src/encoder.cpp src/handleData.cpp src/main.cpp
git commit -m "Add DEVCONSOLE dev env with serial console skeleton"
```

**Hardware-test note:** `>ping` over USB serial on a dev build; KDU still connects on a dev build (arbitration guard); ship build unchanged (hash-verified, no HW test needed).

---

### Task 3: Console get/set/kdu/dump (params, chan, flags)

**Files:**
- Modify: `src/handleData.cpp` (split `writeOtherValue2buf`, lines 18–54)
- Modify: `include/main.h` (declare the new core function next to the existing `PRC152receiveProcess` declaration at line 45)
- Modify: `src/devconsole.cpp` (extend `DevConsole_Execute`)

**Interfaces:**
- Consumes: Task 2's skeleton; existing `parseRcvJson`/`compriseSendJson` (`bsp_json.h`), `readWriteValueToKDU(int)` (`src/handleData.cpp:95` — add a declaration to `main.h` if missing), `writeChanToArray`/`readChanFromArray`, `prefix_buf[][16]` (`font.h`), `get_Flag` (`bsp_storage.h`), globals as externed at `src/handleData.cpp:4-14`.
- Produces: `void writeOtherValue2buf_core(void)` — the sprintf-only body of `writeOtherValue2buf` (no `PTT_READ`/`Get_A20_RSSI` hardware reads; uses current `RSSI` global). `writeOtherValue2buf` becomes: hardware-read block (lines 37–45 as today) + call `writeOtherValue2buf_core()`. Console commands per the shared reference. Task 7's HIL suite relies on the exact response shapes.

- [ ] **Step 1: Split `writeOtherValue2buf`**

In `src/handleData.cpp` (current function at lines 18–54), split into:

- `void writeOtherValue2buf_core(void)` — every `sprintf(parameterValue[...]...)` line **except** the `Jvoltage` one, in the same order. The `Jrssi` sprintf stays in the core but reads the current `RSSI` global as-is (no hardware query).
- `void writeOtherValue2buf(void)` — the hardware block exactly as today (the `PTT_READ` / `A002_SQ_READ` / `Get_A20_RSSI()` logic that refreshes `RSSI`, lines 37–45), then a call to `writeOtherValue2buf_core()`, then the `Jvoltage` sprintf (`Get_Battery_Vol()` is an ADC read — hardware side).

Net effect: existing callers see identical behavior and ordering of side effects; the console gets a hardware-free refresh. Declare `void writeOtherValue2buf_core(void);` in `include/main.h` next to the existing `PRC152receiveProcess` declaration.

- [ ] **Step 2: Add a registry-serialize helper to `src/devconsole.cpp`**

```cpp
#include <ArduinoJson.h>
extern ParameterValue_t parameterValue[ITEMSUM];
extern unsigned char rx1_buf[]; // volatile in bsp_uart.cpp; cast on use

static void dc_refresh_registry(void)
{
    writeOtherValue2buf_core();
    writeChanToArray(&chan_arv[NOW]);
}

static int dc_serialize_registry(char *dst, int dstsz, const char *cmd)
{
    DynamicJsonDocument doc(2048);
    for (int i = 0; i < ITEMSUM; i++)
        doc[parameterValue[i].item] = parameterValue[i].valStr;
    if (cmd) doc["cmd"] = cmd;
    return (int)serializeJson(doc, dst, dstsz);
}
```

(`rx1_buf` is declared `volatile unsigned char rx1_buf[]` in `bsp_uart.cpp:7` — extern it to match exactly, `extern volatile unsigned char rx1_buf[];`, and cast `(char*)rx1_buf` at use, same as `handleData.cpp` does.)

- [ ] **Step 3: Implement `get`, `dump params`, `dump chan`, `dump flags`**

Extend `DevConsole_Execute` dispatch (after the `ping` branch):

```cpp
    if (!strncmp(line, "get ", 4))
    {
        dc_refresh_registry();
        const char *name = line + 4;
        for (int i = 0; i < ITEMSUM; i++)
            if (!strcmp(parameterValue[i].item, name))
                return snprintf(out, outsz, "{\"ok\":1,\"name\":\"%s\",\"val\":\"%s\"}",
                                name, parameterValue[i].valStr);
        return snprintf(out, outsz, "{\"ok\":0,\"err\":\"name\"}");
    }
    if (!strcmp(line, "dump params"))
    {
        dc_refresh_registry();
        int n = snprintf(out, outsz, "{\"ok\":1,\"params\":");
        n += dc_serialize_registry(out + n, outsz - n, NULL);
        n += snprintf(out + n, outsz - n, "}");
        return n;
    }
    if (!strcmp(line, "dump chan"))
    {
        int n = snprintf(out, outsz, "{\"ok\":1,\"chan\":[");
        for (int s = 0; s < ARV_MEM_COUNT; s++)
        {
            CHAN_ARV_P b = &chan_arv[s];
            n += snprintf(out + n, outsz - n,
                "%s{\"slot\":%d,\"chan\":%d,\"rx\":\"%3.5f\",\"tx\":\"%3.5f\","
                "\"rs\":%d,\"ts\":%d,\"pw\":%d,\"bw\":%d,\"nn\":\"%s\",\"scan\":%d}",
                s ? "," : "", s, b->CHAN, b->RX_FREQ, b->TX_FREQ,
                b->RS, b->TS, b->POWER, b->GBW, (const char *)b->NN, b->SCAN);
        }
        n += snprintf(out + n, outsz - n, "]}");
        return n;
    }
    if (!strcmp(line, "dump flags"))
        return snprintf(out, outsz,
            "{\"ok\":1,\"cf\":%d,\"vu\":%d,\"kdu\":%d,\"home\":%d,\"wfm\":%d,"
            "\"step\":%d,\"sql\":%d,\"vol\":%d}",
            get_Flag(FLAG_CF_SWITCH_ADDR), get_Flag(FLAG_VU_SWITCH_ADDR),
            KDU_INSERT, Home_Mode, WFM, STEP, SQL, VOLUME);
```

Mirror the extern declarations exactly as they appear at `src/handleData.cpp:4-14` (`extern u8 ... STEP, SQL, ... VOLUME ...; extern volatile char Home_Mode; extern volatile u8 KDU_INSERT;`); `WFM` — find its declaration with `grep -n "extern.*WFM\b" src/*.cpp` and mirror it.

- [ ] **Step 4: Implement `set` and `kdu` via the KDU dispatch**

Indexing convention (verified against `src/font.cpp:17` and `src/handleData.cpp:319-395`): the `recv_mess` enum alternates bare/`FCS+` names and `prefix_buf` mirrors it — `prefix_buf[ASKALL=0]="ASKALL"` (reply cmd string), `prefix_buf[_ASKALL=1]="FCS+ASKALL"` (incoming cmd string). `PRC152receiveProcess` matches `prefix_buf[i]` for **odd** `i` (`for (i = _ASKALL; i < _SETDUALPOS + 1; i += 2)`) and dispatches `readWriteValueToKDU(i)` with the matched odd value; the reply uses `prefix_buf[Cmd-1]`. The console mirrors this exactly:

```cpp
// name -> FCS command (odd recv_mess values) whose apply-path exists in
// readWriteValueToKDU. Incoming cmd string for value C is prefix_buf[C];
// dispatch is readWriteValueToKDU(C) — same convention as PRC152receiveProcess.
static const struct { const char *name; int cmd; } dc_set_map[] = {
    {"step", _SETSTEP}, {"sql", _SETSQL}, {"audio", _SETAUD}, {"mic", _SETAUD},
    {"tot", _SETTOT}, {"outPower", _SETVDO}, {"volume", _SETVOLU},
    {"preTone", _SETTONE}, {"endTone", _SETTONE}, {"wfm", _SETFM},
    {"fmFreq", _SETFM}, {"homemode", _SETHOMEMODE}, {"selChan", _SETDUALPOS},
};

    if (!strncmp(line, "set ", 4))
    {
        char name[16] = {0}, val[16] = {0};
        if (sscanf(line + 4, "%15s %15s", name, val) != 2)
            return snprintf(out, outsz, "{\"ok\":0,\"err\":\"args\"}");
        for (unsigned i = 0; i < ARR_SIZE(dc_set_map); i++)
            if (!strcmp(dc_set_map[i].name, name))
            {
                dc_refresh_registry();
                for (int j = 0; j < ITEMSUM; j++)
                    if (!strcmp(parameterValue[j].item, name))
                        snprintf(parameterValue[j].valStr, 16, "%s", val);
                // route through the real KDU apply path for true side effects
                dc_serialize_registry((char *)rx1_buf, USART1_BUF_SIZE,
                                      prefix_buf[dc_set_map[i].cmd]);
                readWriteValueToKDU(dc_set_map[i].cmd);
                return snprintf(out, outsz, "{\"ok\":1}");
            }
        return snprintf(out, outsz, "{\"ok\":0,\"err\":\"unsupported\"}");
    }
    if (!strncmp(line, "kdu ", 4))
    {
        snprintf((char *)rx1_buf, USART1_BUF_SIZE, "%s", line + 4);
        for (int i = _ASKALL; i < _SETDUALPOS + 1; i += 2)
            if (strstr((char *)rx1_buf, prefix_buf[i]))
            {
                readWriteValueToKDU(i);
                return snprintf(out, outsz, "{\"ok\":1,\"cmd\":%d}", i);
            }
        return snprintf(out, outsz, "{\"ok\":0,\"err\":\"cmd\"}");
    }
```

Note on `readWriteValueToKDU`: it ends with `compriseSendJson`, which serializes the registry to `Serial` — harmless protocol noise the HIL client skips (non-`##` line).

- [ ] **Step 5: Build + verify**

```bash
pio run -e esp32-s2-saola-1-dev   # clean
pio run                            # ship still builds; binary MAY differ (writeOtherValue2buf split) — confirm the split is semantically identical by re-reading the diff
```

- [ ] **Step 6: Commit**

```bash
git add src/handleData.cpp include/main.h src/devconsole.cpp
git commit -m "Add devconsole get/set/kdu/dump commands"
```

**Hardware-test note:** on a dev build: `>get sql`, `>set sql 3` then `>get sql` (and audible squelch change), `>dump params/chan/flags` shapes; ship build: one KDU connect/disconnect cycle (writeOtherValue2buf split touches the KDU ask path).

---

### Task 4: Synthetic key/encoder injection

**Files:**
- Modify: `src/devconsole.cpp` (queues + `key`/`enc` commands, replace the stub `TakeInjected*`)

**Interfaces:**
- Consumes: Task 2's hooks (already call `TakeInjected*` in both scan primitives).
- Produces: working `>key <code>` / `>enc <event>` commands; `DevConsole_TakeInjectedKey/Enc` returning queued events FIFO. `extern volatile int TIMES;` (defined `src/encoder.cpp:5`) for cw/ccw.

- [ ] **Step 1: Implement the queues**

```cpp
#define DC_INJ_DEPTH 16
static u8 dc_key_q[DC_INJ_DEPTH]; static int dc_key_head = 0, dc_key_tail = 0;
static u8 dc_enc_q[DC_INJ_DEPTH]; static int dc_enc_head = 0, dc_enc_tail = 0;
extern volatile int TIMES;

static int dc_q_push(u8 *q, int *tail, int head, u8 v)
{
    int next = (*tail + 1) % DC_INJ_DEPTH;
    if (next == head) return 0;
    q[*tail] = v; *tail = next; return 1;
}

unsigned char DevConsole_TakeInjectedKey(void)
{
    if (dc_key_head == dc_key_tail) return MATRIX_RESULT_ERROR;
    u8 v = dc_key_q[dc_key_head];
    dc_key_head = (dc_key_head + 1) % DC_INJ_DEPTH;
    return v;
}

unsigned char DevConsole_TakeInjectedEnc(void)
{
    if (dc_enc_head == dc_enc_tail) return key_idle;
    u8 v = dc_enc_q[dc_enc_head];
    dc_enc_head = (dc_enc_head + 1) % DC_INJ_DEPTH;
    return v;
}
```

(Delete the Task-2 stub implementations of both `TakeInjected*`.)

- [ ] **Step 2: Add the commands to `DevConsole_Execute`**

```cpp
    if (!strncmp(line, "key ", 4))
    {
        int code = atoi(line + 4);
        if (code < 0 || code > 15)
            return snprintf(out, outsz, "{\"ok\":0,\"err\":\"code\"}");
        if (!dc_q_push(dc_key_q, &dc_key_tail, dc_key_head, (u8)code))
            return snprintf(out, outsz, "{\"ok\":0,\"err\":\"full\"}");
        return snprintf(out, outsz, "{\"ok\":1}");
    }
    if (!strncmp(line, "enc ", 4))
    {
        const char *ev = line + 4;
        if (!strcmp(ev, "cw"))  { TIMES += 1; return snprintf(out, outsz, "{\"ok\":1}"); }
        if (!strcmp(ev, "ccw")) { TIMES -= 1; return snprintf(out, outsz, "{\"ok\":1}"); }
        u8 code = !strcmp(ev, "click") ? key_click :
                  !strcmp(ev, "double") ? key_double :
                  !strcmp(ev, "long") ? key_long : key_idle;
        if (code == key_idle)
            return snprintf(out, outsz, "{\"ok\":0,\"err\":\"event\"}");
        if (!dc_q_push(dc_enc_q, &dc_enc_tail, dc_enc_head, code))
            return snprintf(out, outsz, "{\"ok\":0,\"err\":\"full\"}");
        return snprintf(out, outsz, "{\"ok\":1}");
    }
```

Safety: refuse to inject `key_long` on the encoder? `key_long` triggers `SHUT()` (power-off) from the main loop and WiFi loop. Keep it injectable (tests may need it) but document in `tools/hil/README.md` that `>enc long` powers the radio down.

- [ ] **Step 3: Build both envs, ship hash equal to baseline**

```bash
pio run -e esp32-s2-saola-1-dev && pio run
shasum -a 256 .pio/build/esp32-s2-saola-1/firmware.bin   # equals baseline
```

- [ ] **Step 4: Commit**

```bash
git add src/devconsole.cpp
git commit -m "Add key/encoder injection to devconsole"
```

**Hardware-test note:** `>key 12` (P/up) and `>key 13` (N/down) navigate the VFO channel; `>enc cw` changes the selected value; a menu opened by injected keys responds to further injected keys (proves poll placement inside blocking loops).

---

### Task 5: LCD shadow buffer + `dump screen`

**Files:**
- Modify: `src/lcd.cpp:100-127` (`LCD_Write`)
- Modify: `include/lcd.h` (shadow extern, `DEVCONSOLE` only)
- Modify: `src/devconsole.cpp` (`dump screen`)

**Interfaces:**
- Consumes: `LCD_Write(dat, rs)` — the single funnel for all controller traffic; `LCD_Setxy` (`src/lcd.cpp:156`) which emits `0xB0+p` / `0x10|hi` / `lo&0x0F` command bytes.
- Produces: `extern volatile unsigned char lcd_shadow[8][128];` (in `lcd.h` under `#ifdef DEVCONSOLE`), maintained by `LCD_Write`; `>dump screen` response per the shared reference.

- [ ] **Step 1: Add the shadow to `src/lcd.cpp`**

Below the includes:

```c
#ifdef DEVCONSOLE
// Shadow of the LCD controller GRAM for `dump screen`: mirrors the
// address-counter state machine (page 0xB0+p, column 0x10|hi / 0x0F&lo,
// auto-increment on data). 0x81 (contrast) takes a parameter byte that
// must not be misread as a column command.
volatile unsigned char lcd_shadow[8][128];
static unsigned char shadow_page = 0, shadow_col = 0, shadow_skip_param = 0;
#endif
```

At the top of `LCD_Write(unsigned char dat, unsigned char rs)`:

```c
#ifdef DEVCONSOLE
    if (rs)
    {
        lcd_shadow[shadow_page & 7][shadow_col & 127] = dat;
        shadow_col = (shadow_col + 1) & 127;
    }
    else if (shadow_skip_param)
        shadow_skip_param = 0;
    else if ((dat & 0xF0) == 0xB0)
        shadow_page = dat & 0x0F;
    else if ((dat & 0xF0) == 0x10)
        shadow_col = (shadow_col & 0x0F) | ((dat & 0x0F) << 4);
    else if (dat < 0x10)
        shadow_col = (shadow_col & 0xF0) | dat;
    else if (dat == 0x81)
        shadow_skip_param = 1;
#endif
```

- [ ] **Step 2: Extern in `include/lcd.h`**

After the includes:

```c
#ifdef DEVCONSOLE
extern volatile unsigned char lcd_shadow[8][128];
#endif
```

- [ ] **Step 3: `dump screen` in `DevConsole_Execute`**

```cpp
    if (!strcmp(line, "dump screen"))
    {
        int n = snprintf(out, outsz, "{\"ok\":1,\"pages\":8,\"cols\":128,\"hex\":\"");
        for (int p = 0; p < 8; p++)
            for (int c = 0; c < 128; c++)
                n += snprintf(out + n, outsz - n, "%02x", lcd_shadow[p][c]);
        n += snprintf(out + n, outsz - n, "\"}");
        return n;
    }
```

(2048 hex chars + wrapper < `DC_OUT_SIZE` 4096. `devconsole.cpp` already includes `lcd.h` via `main.h`.)

- [ ] **Step 4: Build both envs; ship hash equal to baseline; commit**

```bash
pio run -e esp32-s2-saola-1-dev && pio run
shasum -a 256 .pio/build/esp32-s2-saola-1/firmware.bin   # equals baseline
git add src/lcd.cpp include/lcd.h src/devconsole.cpp
git commit -m "Add LCD shadow buffer and dump screen command"
```

**Hardware-test note (critical for this task):** `>dump screen` on the VFO page and compare a rendered image of the hex against the physical display (the HIL suite's `render_screen` helper from Task 7 renders it) — this validates the address-counter mirroring once, as the spec requires. Check again after opening a menu and after `LCD_Clear`.

---

### Task 6: WiFi transport (`/dev` endpoint, SoftAP at boot in dev builds)

**Files:**
- Modify: `src/bsp_wifi.cpp` (add `/dev` handler + `DevConsole_WifiSetup` helper)
- Modify: `include/bsp_wifi.h` (declare the helper)
- Modify: `src/devconsole.cpp` (real `DevConsole_WifiInit`, `handleClient` in poll)

**Interfaces:**
- Consumes: `initSoftAP()` (`src/bsp_wifi.cpp:314`), `WebServer server` (`:11`), `DevConsole_Execute` (Task 2).
- Produces: dev builds boot with SoftAP `FCS_Configure` / `123456789` at `192.168.152.1`, serving `POST /dev` (form field `c`) during normal operation, all UI states included.

- [ ] **Step 1: Add the endpoint in `src/bsp_wifi.cpp`**

```cpp
#ifdef DEVCONSOLE
#include "devconsole.h"
static char dev_out[4096];

static void myHandleDev(void)
{
    String cmd = server.arg("c");
    const char *c = cmd.c_str();
    if (*c == '>') c++;
    DevConsole_Execute(c, dev_out, sizeof(dev_out));
    server.send(200, "application/json", dev_out);
}

void DevConsole_WifiSetup(void)
{
    initSoftAP();
    server.on("/dev", HTTP_POST, myHandleDev);
    server.begin();
}
#endif
```

Declare in `include/bsp_wifi.h` under `#ifdef DEVCONSOLE`: `void DevConsole_WifiSetup(void);`

- [ ] **Step 2: Wire into the console**

`src/devconsole.cpp`:

```cpp
#include "bsp_wifi.h"
void DevConsole_WifiInit(void) { DevConsole_WifiSetup(); }
```

and at the end of `DevConsole_Poll` (after the serial block, before returning — i.e. run every rate-limited poll):

```cpp
    server.handleClient();
```

`server` is the global `WebServer` from `bsp_wifi.cpp` — add `extern WebServer server;` to `bsp_wifi.h` under the `DEVCONSOLE` guard (include `<WebServer.h>` there; check `bsp_wifi.h`'s existing includes first and reuse them).

Note: `DevConsole_Poll`'s early-return paths (`Serial.peek() != '>'`) must NOT skip `handleClient` — restructure the serial block so the function always falls through to `server.handleClient()` before returning. One command per poll still holds per transport.

- [ ] **Step 3: Interaction with the existing WiFi menus**

When the user enters the WiFi PGM/RCU menu on a dev build, `handleWIFIServer` (`src/bsp_wifi.cpp:525`) re-runs `initSoftAP` and registers more routes on the same `server`; `stopWIFIServer` (`:358`) calls `WiFi.mode(WIFI_OFF)`, killing the console transport until reboot. Accept this; add a comment in `DevConsole_WifiSetup` and a note in `tools/hil/README.md` ("dev builds: WiFi console dies after leaving the on-radio WiFi menus; reboot to restore").

- [ ] **Step 4: Build both envs; ship hash equal to baseline; commit**

```bash
pio run -e esp32-s2-saola-1-dev && pio run
shasum -a 256 .pio/build/esp32-s2-saola-1/firmware.bin   # equals baseline
git add src/bsp_wifi.cpp include/bsp_wifi.h src/devconsole.cpp
git commit -m "Serve devconsole over WiFi /dev endpoint in dev builds"
```

**Hardware-test note:** dev build boots with SSID visible; `curl -X POST -d 'c=ping' http://192.168.152.1/dev` returns the ping JSON; serial console still works simultaneously; boot current draw of the dev build is expected to rise (WiFi on) — ship build unaffected.

---

### Task 7: HIL pytest suite

**Files:**
- Create: `tools/hil/README.md`
- Create: `tools/hil/requirements.txt`
- Create: `tools/hil/conftest.py`
- Create: `tools/hil/dc_client.py`
- Create: `tools/hil/test_smoke.py`
- Create: `tools/hil/test_params.py`
- Create: `tools/hil/test_ui_flow.py`
- Create: `tools/hil/test_screen.py`

**Interfaces:**
- Consumes: the devconsole protocol exactly as specified in the shared reference (Tasks 2–6).
- Produces: `DevConsole` client class (`dc_client.py`) with `.cmd(line) -> dict`, `.get(name)`, `.set(name, value)`, `.dump(what)`, `.key(code)`, `.enc(event)`, `.screen() -> bytes (1024)`, `render_screen(data) -> str` (ASCII art, 128x64, `#`/`.`); pytest fixtures `dc` (transport-selected client).

- [ ] **Step 1: Write `dc_client.py`**

```python
"""Devconsole client for the PRC152 dev firmware (serial or WiFi)."""
import json
import time

MATRIX_KEYS = {"0": 0, "1": 1, "2": 2, "3": 3, "4": 4, "5": 5, "6": 6,
               "7": 7, "8": 8, "9": 9, "CLR": 10, "ENT": 11, "UP": 12,
               "DOWN": 13, "LEFT": 14, "RIGHT": 15}


class SerialTransport:
    def __init__(self, port, baud=115200, timeout=3.0):
        import serial
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.timeout = timeout

    def cmd(self, line):
        self.ser.reset_input_buffer()
        self.ser.write(b">" + line.encode() + b"\n")
        deadline = time.monotonic() + self.timeout
        while time.monotonic() < deadline:
            raw = self.ser.readline()
            if raw.startswith(b"##"):
                return json.loads(raw[2:].decode(errors="replace"))
        raise TimeoutError(f"no ## response to {line!r}")


class HttpTransport:
    def __init__(self, host, timeout=5.0):
        import requests
        self.requests = requests
        self.url = f"http://{host}/dev"
        self.timeout = timeout

    def cmd(self, line):
        r = self.requests.post(self.url, data={"c": line}, timeout=self.timeout)
        r.raise_for_status()
        return r.json()


class DevConsole:
    def __init__(self, transport):
        self.t = transport

    def cmd(self, line):
        resp = self.t.cmd(line)
        assert resp.get("ok") == 1, f"{line!r} failed: {resp}"
        return resp

    def ping(self):
        return self.cmd("ping")

    def get(self, name):
        return self.cmd(f"get {name}")["val"]

    def set(self, name, value):
        return self.cmd(f"set {name} {value}")

    def dump(self, what):
        return self.cmd(f"dump {what}")

    def key(self, code):
        code = MATRIX_KEYS.get(code, code)
        r = self.cmd(f"key {code}")
        time.sleep(0.15)  # let the super-loop consume the event
        return r

    def enc(self, event):
        r = self.cmd(f"enc {event}")
        time.sleep(0.15)
        return r

    def screen(self):
        return bytes.fromhex(self.cmd("dump screen")["hex"])


def render_screen(data, pages=8, cols=128):
    """Render page-major GRAM bytes as ASCII art (rows = pages*8 pixels)."""
    lines = []
    for page in range(pages):
        for bit in range(8):
            row = data[page * cols:(page + 1) * cols]
            lines.append("".join("#" if b & (1 << bit) else "." for b in row))
    return "\n".join(lines)
```

- [ ] **Step 2: Write `conftest.py`**

```python
import pytest
from dc_client import DevConsole, SerialTransport, HttpTransport


def pytest_addoption(parser):
    parser.addoption("--serial", help="serial port, e.g. /dev/cu.usbserial-0001")
    parser.addoption("--wifi", help="radio IP, e.g. 192.168.152.1")


@pytest.fixture(scope="session")
def dc(request):
    port = request.config.getoption("--serial")
    host = request.config.getoption("--wifi")
    if port:
        return DevConsole(SerialTransport(port))
    if host:
        return DevConsole(HttpTransport(host))
    pytest.skip("no --serial or --wifi transport given (hardware required)")
```

- [ ] **Step 3: Write the tests**

`test_smoke.py`:

```python
def test_ping(dc):
    r = dc.ping()
    assert r["dc"] == 1
    assert r["fw"].startswith("Rev ")


def test_dump_shapes(dc):
    assert "params" in dc.dump("params")
    chan = dc.dump("chan")["chan"]
    assert len(chan) == 4
    flags = dc.dump("flags")
    for k in ("cf", "vu", "kdu", "home", "wfm", "step", "sql", "vol"):
        assert k in flags
```

`test_params.py`:

```python
def test_sql_roundtrip(dc):
    orig = dc.get("sql")
    try:
        new = "3" if orig != "3" else "4"
        dc.set("sql", new)
        assert dc.get("sql") == new
    finally:
        dc.set("sql", orig)


def test_volume_roundtrip(dc):
    orig = dc.get("volume")
    try:
        new = "2" if orig != "2" else "3"
        dc.set("volume", new)
        assert dc.get("volume") == new
    finally:
        dc.set("volume", orig)
```

`test_ui_flow.py`:

```python
import time


def test_channel_step_updates_state(dc):
    """UP then DOWN on the VFO page: channel changes and comes back."""
    before = dc.dump("chan")["chan"][0]["chan"]
    dc.key("UP")
    time.sleep(0.4)
    after = dc.dump("chan")["chan"][0]["chan"]
    dc.key("DOWN")
    time.sleep(0.4)
    restored = dc.dump("chan")["chan"][0]["chan"]
    assert after != before
    assert restored == before


def test_menu_open_close_via_injection(dc):
    """ENT opens a blocking menu; CLR exits — console must stay alive inside."""
    dc.key("ENT")
    time.sleep(0.4)
    assert dc.ping()["ok"] == 1   # poll placement keeps console alive in menus
    dc.key("CLR")
    time.sleep(0.4)
    assert dc.ping()["ok"] == 1
```

`test_screen.py`:

```python
from dc_client import render_screen


def test_screen_not_blank(dc):
    data = dc.screen()
    assert len(data) == 1024
    assert any(data), "shadow buffer is all zeros - LCD mirroring broken?"


def test_screen_render_smoke(dc):
    print(render_screen(dc.screen()))  # visual aid: pytest -s shows the display
```

- [ ] **Step 4: Write `requirements.txt` and `README.md`**

`requirements.txt`: `pyserial`, `requests`, `pytest` (one per line). README covers: flashing the dev env (`pio run -e esp32-s2-saola-1-dev -t upload --upload-port <port>`); running over serial (`pytest --serial /dev/cu.usbserial-*`) vs WiFi (join SSID `FCS_Configure`, password `123456789`, then `pytest --wifi 192.168.152.1`); serial monitor must be closed while testing over serial; do NOT attach a physical KDU during serial HIL (unarbitrated interleaving); `>enc long` powers the radio off; leaving the on-radio WiFi menus kills the WiFi transport until reboot; these tests require the radio and are never run in CI.

- [ ] **Step 5: Syntax-verify without hardware, then commit**

```bash
python3 -m py_compile tools/hil/*.py
python3 -m pytest tools/hil -q --collect-only   # collects, then skips without transport args
git add tools/hil
git commit -m "Add hardware-in-the-loop pytest suite for the devconsole"
```

**Hardware-test note:** the entire suite IS the hardware test — run `pytest --serial …` and `pytest --wifi …` against a dev build and record results in the final task.

---

### Task 8: Phase 2 — native test env + pure-logic extraction + unit tests

**Files:**
- Modify: `platformio.ini` (add `[env:native]`)
- Create: `include/kdu_protocol.h` (moved chain — see Step 2)
- Modify: `include/FCS152_KDU.h` (replace moved block with `#include "kdu_protocol.h"`)
- Create: `lib/pure_logic/freq_math.h`
- Create: `lib/pure_logic/freq_math.cpp`
- Create: `lib/pure_logic/param_marshal.h`
- Create: `lib/pure_logic/param_marshal.cpp`
- Modify: `src/main_fun.cpp:1622-1651` (`checkFreqFloat` → wrapper)
- Modify: `src/handleData.cpp` (remove moved marshalling functions)
- Modify: `src/bsp_json.cpp:7-46` (remove `parameterValue` definition, now in `param_marshal.cpp`)
- Create: `test/test_native/test_main.cpp`

**Interfaces:**
- Consumes: nothing from Tasks 2–7 (independent of the devconsole).
- Produces:
  - `include/kdu_protocol.h`: Arduino-free header holding `Length_*`, `*_RANK`, `BUF_SIZE`, lowercase `*_rank`, the `static_assert` block (moved from Task 1), and `CHAN_ARV`/`CHAN_ARV_P`/`ARV_MEM_COUNT`/the `NOW..CHANB` enum (moved from `FCS152_KDU.h:194-217`). Includes `"userinclude.h"` only.
  - `lib/pure_logic/freq_math.h`: `double checkFreqFloatStep(double freq_buf, unsigned char step);`
  - `lib/pure_logic/param_marshal.h`: `extern ParameterValue_t parameterValue[ITEMSUM];` plus `void readChanFromArray(CHAN_ARV_P B); void writeChanToArray(CHAN_ARV_P B);` (includes `"bsp_json.h"` and `"kdu_protocol.h"`).
  - `src/main_fun.cpp` keeps `double checkFreqFloat(double freq_buf) { return checkFreqFloatStep(freq_buf, STEP); }` so all call sites are untouched.

- [ ] **Step 1: Add the native env**

```ini
[env:native]
platform = native
test_framework = unity
build_src_filter = -<*>
build_flags = -DUNITY_INCLUDE_DOUBLE
```

(`UNITY_INCLUDE_DOUBLE` is required for the `TEST_ASSERT_*_DOUBLE*` assertions.)

(`default_envs` from Task 2 keeps `pio run` on the ship env. If Task 2 has not run yet, add `[platformio] default_envs = esp32-s2-saola-1` here.)

- [ ] **Step 2: Extract `kdu_protocol.h`**

Move from `FCS152_KDU.h` into new `include/kdu_protocol.h` (guarded `__KDU_PROTOCOL_H__`, includes `"userinclude.h"`): lines 194–217 (`CHAN_ARV` struct through the `NOW/TMP/CHANA/CHANB` enum, plus `extern CHAN_ARV chan_arv[ARV_MEM_COUNT];`), lines 302–438 (`Length_*` through `BUF_SIZE`), and the Task-1 `static_assert` block. In `FCS152_KDU.h`, replace the moved text with `#include "kdu_protocol.h"` at the same position. The `#ifdef EN_EEROOM` address block (`FCS152_KDU.h:445+`) references `*_RANK` — it stays in `FCS152_KDU.h` **below** the new include, so it still compiles.

- [ ] **Step 3: Write the failing tests**

`test/test_native/test_main.cpp`:

```cpp
#include <unity.h>
#include "kdu_protocol.h"
#include "freq_math.h"
#include "param_marshal.h"
#include <string.h>

CHAN_ARV chan_arv[ARV_MEM_COUNT]; // firmware defines this in main_fun.cpp; tests define their own

void setUp(void) {}
void tearDown(void) {}

// ---- kdu_protocol: the compile-time pins re-checked at runtime (belt and braces)
void test_frame_layout(void)
{
    TEST_ASSERT_EQUAL_INT(12, TX_RANK);
    TEST_ASSERT_EQUAL_INT(75, NOWSELCHAN_RANK);
    TEST_ASSERT_EQUAL_INT(93, BUF_SIZE);
}

void test_kdu_byte_encoding(void)
{
    TEST_ASSERT_EQUAL_INT('7', kdu_send_data(7));
    TEST_ASSERT_EQUAL_INT(7, kdu_recv_data('7'));
}

// ---- freq_math: characterization of checkFreqFloat behavior
void test_freq_on_5k_grid_passes_through(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(435.55, checkFreqFloatStep(435.55, 0));
}

void test_freq_on_625_grid_passes_through(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(430.10625, checkFreqFloatStep(430.10625, 1));
}

void test_freq_past_bug_430_13751_corrected(void)
{
    // The comment at src/main_fun.cpp:1632 records 430.13751 as a value that
    // once wrongly passed. Pin today's corrected result on the 5 kHz step.
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 430.14, checkFreqFloatStep(430.13751, 0));
}

void test_freq_corrects_to_625_grid(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 430.1375, checkFreqFloatStep(430.139, 1));
}

// ---- param_marshal: registry <-> CHAN_ARV round-trip
void test_chan_roundtrip(void)
{
    CHAN_ARV a, b;
    memset((void *)&a, 0, sizeof a);
    memset((void *)&b, 0, sizeof b);
    a.CHAN = 42; a.RX_FREQ = 435.55; a.TX_FREQ = 431.125;
    a.RS = 12; a.TS = 38; a.POWER = 1; a.GBW = 0;
    snprintf((char *)a.NN, sizeof a.NN, "%s", "TESTNN7");

    writeChanToArray(&a);
    readChanFromArray(&b);

    TEST_ASSERT_EQUAL_INT(42, b.CHAN);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 435.55, b.RX_FREQ);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 431.125, b.TX_FREQ);
    TEST_ASSERT_EQUAL_INT(12, b.RS);
    TEST_ASSERT_EQUAL_INT(38, b.TS);
    TEST_ASSERT_EQUAL_INT(1, b.POWER);
    TEST_ASSERT_EQUAL_INT(0, b.GBW);
    TEST_ASSERT_EQUAL_STRING("TESTNN7", (const char *)b.NN);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_frame_layout);
    RUN_TEST(test_kdu_byte_encoding);
    RUN_TEST(test_freq_on_5k_grid_passes_through);
    RUN_TEST(test_freq_on_625_grid_passes_through);
    RUN_TEST(test_freq_past_bug_430_13751_corrected);
    RUN_TEST(test_freq_corrects_to_625_grid);
    RUN_TEST(test_chan_roundtrip);
    return UNITY_END();
}
```

Before relying on the expected values above, hand-verify each against the verbatim algorithm (Step 5) — e.g. 430.13751: `freq_int = 4301`, `freq_tail = 3751`, not divisible by 500/625, `mul = (3751*10/500 + 5)/10 = 8`, result `8*500/100000.0 + 430.1 = 430.14`. If a hand-check disagrees with an expectation, the expectation is wrong — characterization tests record actual behavior; fix the test value, never the algorithm.

- [ ] **Step 4: Run tests → expect compile failure**

```bash
pio test -e native
```

Expected: FAIL — `freq_math.h` / `param_marshal.h` do not exist yet.

- [ ] **Step 5: Extract `freq_math`**

`lib/pure_logic/freq_math.h`:

```c
#ifndef __FREQ_MATH_H__
#define __FREQ_MATH_H__
// Frequency grid validation/correction, extracted from main_fun.cpp for
// host-side testing. step: 0 = 5 kHz grid (500), 1 = 6.25 kHz grid (625).
double checkFreqFloatStep(double freq_buf, unsigned char step);
#endif
```

`lib/pure_logic/freq_math.cpp`: the verbatim body of `src/main_fun.cpp:1622-1651` with the signature `double checkFreqFloatStep(double freq_buf, unsigned char step)` and `step_temp[STEP]` → `step_temp[step]` (twice, line 1646). Keep the Chinese comments and commented-out lines as-is (translation is a separate effort). Known quirk, do NOT fix: `step >= 2` indexes past `step_temp[2]` — pre-existing behavior, out of scope.

In `src/main_fun.cpp`, replace the function body with:

```c
double checkFreqFloat(double freq_buf)
{
    return checkFreqFloatStep(freq_buf, STEP);
}
```

and add `#include "freq_math.h"` next to the other includes.

- [ ] **Step 6: Extract `param_marshal`**

`lib/pure_logic/param_marshal.h`:

```c
#ifndef __PARAM_MARSHAL_H__
#define __PARAM_MARSHAL_H__
// parameterValue[] registry and its CHAN_ARV marshalling, extracted from
// bsp_json.cpp/handleData.cpp for host-side testing. Arduino-free.
#include "bsp_json.h"
#include "kdu_protocol.h"

extern ParameterValue_t parameterValue[ITEMSUM];

void readChanFromArray(CHAN_ARV_P B);
void writeChanToArray(CHAN_ARV_P B);
#endif
```

`lib/pure_logic/param_marshal.cpp`: `#include "param_marshal.h"` + `#include <stdio.h>` + `#include <stdlib.h>`; move verbatim: the `parameterValue[ITEMSUM] = {…}` initializer from `src/bsp_json.cpp:7-46`, and `readChanFromArray`/`writeChanToArray` from `src/handleData.cpp:58-92`. Delete the moved definitions from their old files and replace their local `extern ParameterValue_t parameterValue[ITEMSUM];` declarations with `#include "param_marshal.h"` in **both** `src/handleData.cpp` and `src/bsp_json.cpp` — this include is also what makes PlatformIO's library dependency finder link `lib/pure_logic` into the firmware envs; without it the firmware fails to link `parameterValue`. Check `include/main.h` for existing `readChanFromArray`/`writeChanToArray` declarations — if declared there, leave them (signatures unchanged).

- [ ] **Step 7: Run the tests to green**

```bash
pio test -e native
```

Expected: PASS (7 tests). If `TEST_ASSERT_EQUAL_DOUBLE` is unavailable in the Unity build, switch those to `TEST_ASSERT_DOUBLE_WITHIN(1e-9, …)`.

- [ ] **Step 8: Verify the firmware still builds (both envs)**

```bash
pio run && pio run -e esp32-s2-saola-1-dev
```

(Binary layout may change — moved translation units. Semantics must not: the diff for this task is only moves, the `checkFreqFloat` wrapper, and includes.)

- [ ] **Step 9: Commit**

```bash
git add platformio.ini include/kdu_protocol.h include/FCS152_KDU.h lib/pure_logic test/test_native src/main_fun.cpp src/handleData.cpp src/bsp_json.cpp
git commit -m "Add native test env with freq/marshalling characterization tests"
```

**Hardware-test note:** frequency entry via RT menu (checkFreqFloat wrapper path) on either build; one KDU exchange cycle (marshalling moved).

---

### Task 9: Documentation — CLAUDE.md, README, spec cross-links

**Files:**
- Modify: `CLAUDE.md`
- Modify: `README.md`

**Interfaces:** none — text only.

- [ ] **Step 1: Update `CLAUDE.md`**

- In **Commands**: `pio run` now builds only the ship env (`default_envs`); add `pio run -e esp32-s2-saola-1-dev` (devconsole test build), `pio test -e native` (host unit tests), and `pytest tools/hil --serial <port>` (on-device suite, hardware required).
- Replace "There is no test suite and no linter." with: "Host unit tests: `pio test -e native` (Unity; pure logic in `lib/pure_logic/`). On-device tests: `tools/hil/` pytest against a `-DDEVCONSOLE` dev build (see `tools/hil/README.md`). No linter."
- In **Architecture**, add one short paragraph: dev builds (`esp32-s2-saola-1-dev`, `-DDEVCONSOLE`) embed a test console (`src/devconsole.cpp`) polled from the input scan primitives, speaking `>`-prefixed lines over the shared KDU/USB serial port and `POST /dev` on the boot-time SoftAP; ship builds compile no-op stubs.
- In **Code review**, adjust "Since there is no test suite…" to "...state what was hardware-tested (the `tools/hil` suite counts) or list what still needs hardware testing"; add: changes to `Length_*` now fail the build via `include/kdu_protocol.h` static_asserts — never bump the pinned values without the KDU-side change.

- [ ] **Step 2: Update `README.md`**

Add a "Testing" section mirroring the same three commands, one line each, linking `tools/hil/README.md` and the spec/plan docs.

- [ ] **Step 3: Commit**

```bash
git add CLAUDE.md README.md
git commit -m "Document test environments and devconsole"
```

---

### Task 10: Final verification, review, hardware-test checklist

**Files:** none created (fixes only if review finds defects).

- [ ] **Step 1: Full verification sweep**

```bash
pio run                                    # ship env, clean
pio run -e esp32-s2-saola-1-dev            # dev env, clean
pio test -e native                         # all green
python3 -m pytest tools/hil -q --collect-only
git log --oneline dev..HEAD                # one commit per task
```

- [ ] **Step 2: Dispatch the `firmware-reviewer` agent** on `git diff dev...HEAD` with the plan+spec paths for context. Fix CONFIRMED findings; re-run the sweep after any fix.

- [ ] **Step 3: Produce the hardware-test checklist** — collect every "Hardware-test note" from Tasks 2–8 into the final summary for the user (nothing in this plan can hardware-verify itself; the radio is required).

- [ ] **Step 4: Commit any review fixes** (one commit per concern, not a bundle).

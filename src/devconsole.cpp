#include "devconsole.h"
#ifdef DEVCONSOLE

#include "main.h"
#include "bsp_json.h"
#include "bsp_wifi.h"
#include <ArduinoJson.h>

#define DC_LINE_SIZE 1024
#define DC_OUT_SIZE  4096

static char dc_line[DC_LINE_SIZE];
static int  dc_line_len = 0;
static u8   dc_in_line = 0;       // saw '>' and consuming until '\n'
static uint32_t dc_line_last_ms = 0; // last time a byte was consumed while in-line
static char dc_out[DC_OUT_SIZE];

#define DC_INJ_DEPTH 16
static u8 dc_key_q[DC_INJ_DEPTH]; static int dc_key_head = 0, dc_key_tail = 0;
static u8 dc_enc_q[DC_INJ_DEPTH]; static int dc_enc_head = 0, dc_enc_tail = 0;

static int dc_q_push(u8 *q, int *tail, int head, u8 v)
{
    int next = (*tail + 1) % DC_INJ_DEPTH;
    if (next == head) return 0;
    q[*tail] = v; *tail = next; return 1;
}

// Mirrors the local externs at src/handleData.cpp:4-14 (same globals, defined
// in src/main_fun.cpp) — not declared in any header. WFM is excluded: main.h
// already externs it (guarded by #if FM_EN, which is 1 in this build).
extern u8 STEP, SQL, VOLUME;
extern volatile char Home_Mode;
extern volatile u8 KDU_INSERT;

extern ParameterValue_t parameterValue[ITEMSUM];
// rx1_buf is already declared (with its real bound) by bsp_uart.h, pulled in
// via main.h -> bsp_uart.h; no separate extern needed here. Cast to (char*)
// at each use site, same convention as src/handleData.cpp.

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

// JSON-escape a string into dst (quotes, backslashes, control chars).
// Truncates safely (never overflows dst) rather than emitting a partial
// escape sequence. Needed for any field built with raw %s instead of
// ArduinoJson (which escapes automatically): nicknames come from the WiFi
// PGM page unfiltered (maxlength 7, no character filtering), so a nickname
// containing '"' or '\' would otherwise emit invalid JSON.
static const char *dc_json_escape(const char *src, char *dst, int dstsz)
{
    static const char hex[] = "0123456789abcdef";
    int n = 0;
    if (dstsz <= 0)
        return dst;
    for (; *src; src++)
    {
        unsigned char c = (unsigned char)*src;
        int need = (c == '"' || c == '\\') ? 2 : (c < 0x20 ? 6 : 1);
        if (n + need > dstsz - 1)
            break; // stop before overflowing; leaves room for the '\0'
        if (c == '"' || c == '\\')
        {
            dst[n++] = '\\';
            dst[n++] = (char)c;
        }
        else if (c < 0x20)
        {
            dst[n++] = '\\';
            dst[n++] = 'u';
            dst[n++] = '0';
            dst[n++] = '0';
            dst[n++] = hex[(c >> 4) & 0xF];
            dst[n++] = hex[c & 0xF];
        }
        else
        {
            dst[n++] = (char)c;
        }
    }
    dst[n] = '\0';
    return dst;
}

// Accumulates a snprintf-style write, clamping so `n` never exceeds `outsz` —
// keeps a following `outsz - n` non-negative for every step in a chain (a
// negative int passed as serializeJson/snprintf's size_t size parameter would
// wrap to a huge value and defeat the bound). Ignores a negative `written`
// (snprintf encoding error) rather than corrupting the running position.
static inline int dc_appended(int n, int written, int outsz)
{
    if (written < 0)
        return n;
    n += written;
    return n > outsz ? outsz : n;
}

// name -> FCS command (odd recv_mess values) whose apply-path exists in
// readWriteValueToKDU. Incoming cmd string for value C is prefix_buf[C];
// dispatch is readWriteValueToKDU(C) — same convention as PRC152receiveProcess.
static const struct { const char *name; int cmd; } dc_set_map[] = {
    {"step", _SETSTEP}, {"sql", _SETSQL}, {"audio", _SETAUD}, {"mic", _SETAUD},
    {"tot", _SETTOT}, {"outPower", _SETVDO}, {"volume", _SETVOLU},
    {"preTone", _SETTONE}, {"endTone", _SETTONE}, {"wfm", _SETFM},
    {"fmFreq", _SETFM}, {"homemode", _SETHOMEMODE}, {"selChan", _SETDUALPOS},
};

void DevConsole_Init(void)
{
    dc_line_len = 0;
    dc_in_line = 0;
}

void DevConsole_WifiInit(void) { DevConsole_WifiSetup(); }

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

// True while a '>'-prefixed line is mid-flight (arbitration hint for
// PRC152receiveProcess: don't let the KDU reader drain our continuation bytes).
int DevConsole_LineInProgress(void) { return dc_in_line; }

int DevConsole_Execute(const char *line, char *out, int outsz)
{
    if (!strcmp(line, "ping"))
        return snprintf(out, outsz, "{\"ok\":1,\"fw\":\"%s\",\"dc\":1}", VERSION_152);

    if (!strncmp(line, "get ", 4))
    {
        dc_refresh_registry();
        const char *name = line + 4;
        for (int i = 0; i < ITEMSUM; i++)
            if (!strcmp(parameterValue[i].item, name))
            {
                char esc[64];
                dc_json_escape(parameterValue[i].valStr, esc, sizeof(esc));
                return snprintf(out, outsz, "{\"ok\":1,\"name\":\"%s\",\"val\":\"%s\"}",
                                name, esc);
            }
        return snprintf(out, outsz, "{\"ok\":0,\"err\":\"name\"}");
    }
    if (!strcmp(line, "dump params"))
    {
        dc_refresh_registry();
        int n = dc_appended(0, snprintf(out, outsz, "{\"ok\":1,\"params\":"), outsz);
        n = dc_appended(n, dc_serialize_registry(out + n, outsz - n, NULL), outsz);
        n = dc_appended(n, snprintf(out + n, outsz - n, "}"), outsz);
        return n;
    }
    if (!strcmp(line, "dump chan"))
    {
        int n = dc_appended(0, snprintf(out, outsz, "{\"ok\":1,\"chan\":["), outsz);
        for (int s = 0; s < ARV_MEM_COUNT; s++)
        {
            CHAN_ARV_P b = &chan_arv[s];
            char nn_esc[64];
            dc_json_escape((const char *)b->NN, nn_esc, sizeof(nn_esc));
            n = dc_appended(n, snprintf(out + n, outsz - n,
                "%s{\"slot\":%d,\"chan\":%d,\"rx\":\"%3.5f\",\"tx\":\"%3.5f\","
                "\"rs\":%d,\"ts\":%d,\"pw\":%d,\"bw\":%d,\"nn\":\"%s\",\"scan\":%d}",
                s ? "," : "", s, b->CHAN, b->RX_FREQ, b->TX_FREQ,
                b->RS, b->TS, b->POWER, b->GBW, nn_esc, b->SCAN), outsz);
        }
        n = dc_appended(n, snprintf(out + n, outsz - n, "]}"), outsz);
        return n;
    }
    if (!strcmp(line, "dump flags"))
        return snprintf(out, outsz,
            "{\"ok\":1,\"cf\":%d,\"vu\":%d,\"kdu\":%d,\"home\":%d,\"wfm\":%d,"
            "\"step\":%d,\"sql\":%d,\"vol\":%d}",
            get_Flag(FLAG_CF_SWITCH_ADDR), get_Flag(FLAG_VU_SWITCH_ADDR),
            KDU_INSERT, Home_Mode, WFM, STEP, SQL, VOLUME);

    if (!strcmp(line, "dump screen"))
    {
        int n = snprintf(out, outsz, "{\"ok\":1,\"pages\":8,\"cols\":128,\"hex\":\"");
        for (int p = 0; p < 8; p++)
            for (int c = 0; c < 128; c++)
                n = dc_appended(n, snprintf(out + n, outsz - n, "%02x", lcd_shadow[p][c]), outsz);
        n = dc_appended(n, snprintf(out + n, outsz - n, "\"}"), outsz);
        return n;
    }

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

    return snprintf(out, outsz, "{\"ok\":0,\"err\":\"unknown\"}");
}

// Bounded serial poll: consume console bytes only ('>' first-byte arbitration
// against KDU JSON frames), execute at most one command per call. Also pumps
// the WiFi transport (server.handleClient(), which dispatches at most one
// pending HTTP request per call) -- the serial block below uses `break`
// rather than `return` on every exit path so this always runs once the 5ms
// rate gate above has opened, regardless of what the serial side did.
void DevConsole_Poll(void)
{
    static uint32_t last_ms = 0;
    uint32_t now = millis();
    if (now - last_ms < 5)
        return;
    last_ms = now;

    // A dead/slow client can leave us stuck mid-line, which would keep
    // DevConsole_LineInProgress() true forever and wedge the KDU reader.
    if (dc_in_line && (now - dc_line_last_ms > 500))
    {
        dc_in_line = 0;
        dc_line_len = 0;
    }

    while (Serial.available())
    {
        if (!dc_in_line)
        {
            if (Serial.peek() != '>')
                break; // not ours: leave for the KDU processor
            Serial.read(); // consume '>'
            dc_in_line = 1;
            dc_line_len = 0;
            dc_line_last_ms = now;
            continue;
        }
        char c = (char)Serial.read();
        dc_line_last_ms = now;
        if (c == '\n' || c == '\r')
        {
            dc_line[dc_line_len] = '\0';
            dc_in_line = 0;
            if (dc_line_len)
            {
                DevConsole_Execute(dc_line, dc_out, DC_OUT_SIZE);
                // Leading '\n': some commands (set/kdu) route through
                // readWriteValueToKDU -> compriseSendJson, which writes raw
                // JSON straight to Serial with no trailing newline (protocol
                // noise for the KDU wire format). Without this, "##..." would
                // land mid-line and a client matching lines by a "##" prefix
                // would miss the response entirely.
                Serial.printf("\n##%s\n", dc_out);
            }
            break; // one command per poll
        }
        if (dc_line_len < DC_LINE_SIZE - 1)
            dc_line[dc_line_len++] = c;
    }

    server.handleClient();
}
#endif

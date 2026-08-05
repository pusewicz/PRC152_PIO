#include "devconsole.h"
#ifdef DEVCONSOLE

#include "main.h"

#define DC_LINE_SIZE 1024
#define DC_OUT_SIZE  4096

static char dc_line[DC_LINE_SIZE];
static int  dc_line_len = 0;
static u8   dc_in_line = 0;       // saw '>' and consuming until '\n'
static uint32_t dc_line_last_ms = 0; // last time a byte was consumed while in-line
static char dc_out[DC_OUT_SIZE];

void DevConsole_Init(void)
{
    dc_line_len = 0;
    dc_in_line = 0;
}

void DevConsole_WifiInit(void) {} // Task 6

unsigned char DevConsole_TakeInjectedKey(void) { return MATRIX_RESULT_ERROR; } // Task 4
unsigned char DevConsole_TakeInjectedEnc(void) { return key_idle; }            // Task 4

// True while a '>'-prefixed line is mid-flight (arbitration hint for
// PRC152receiveProcess: don't let the KDU reader drain our continuation bytes).
int DevConsole_LineInProgress(void) { return dc_in_line; }

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
                return; // not ours: leave for the KDU processor
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
                Serial.printf("##%s\n", dc_out);
            }
            return; // one command per poll
        }
        if (dc_line_len < DC_LINE_SIZE - 1)
            dc_line[dc_line_len++] = c;
    }
}
#endif

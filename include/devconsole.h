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

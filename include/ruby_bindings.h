/**
 * @file ruby_bindings.h
 * @brief MRuby/C bindings for PRC-152 radio firmware
 *
 * This file provides the interface between mruby/c Ruby code and
 * the C/C++ hardware drivers (LCD, Radio, Storage, etc.)
 */

#ifndef __RUBY_BINDINGS_H__
#define __RUBY_BINDINGS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Memory pool for mruby/c VM (40KB should be sufficient)
#define MRBC_MEMORY_SIZE (1024 * 40)

/**
 * @brief Initialize the mruby/c VM and register all bindings
 * @return 0 on success, -1 on failure
 */
int ruby_init(void);

/**
 * @brief Run a compiled Ruby bytecode task
 * @param bytecode Pointer to compiled .mrb bytecode
 * @return 0 on success, -1 on failure
 */
int ruby_run_bytecode(const uint8_t *bytecode);

/**
 * @brief Execute one iteration of the mruby/c VM
 * Call this in the main loop for cooperative multitasking
 */
void ruby_tick(void);

/**
 * @brief Check if Ruby VM has active tasks
 * @return 1 if running, 0 if idle
 */
int ruby_is_running(void);

/**
 * @brief Send an event to Ruby (encoder, keypad, etc.)
 * @param event_type Type of event (KEY_CLICK, KEY_LONG, ENCODER_CW, etc.)
 * @param value Event-specific value
 */
void ruby_send_event(int event_type, int value);

// Event types for ruby_send_event
#define RUBY_EVENT_KEY_CLICK     1
#define RUBY_EVENT_KEY_LONG      2
#define RUBY_EVENT_KEY_DOUBLE    3
#define RUBY_EVENT_ENCODER_CW    4
#define RUBY_EVENT_ENCODER_CCW   5
#define RUBY_EVENT_MATRIX_KEY    6
#define RUBY_EVENT_PTT_DOWN      7
#define RUBY_EVENT_PTT_UP        8
#define RUBY_EVENT_SQUELCH       9

#ifdef __cplusplus
}
#endif

#endif // __RUBY_BINDINGS_H__

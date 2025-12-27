/**
 * @file ruby_bindings.cpp
 * @brief MRuby/C bindings implementation for PRC-152 radio firmware
 */

#include <Arduino.h>
#include "mrubyc.h"
#include "ruby_bindings.h"
#include "lcd.h"
#include "bsp_storage.h"
#include "FCS152_KDU.h"

// External references to radio state
extern CHAN_ARV chan_arv[];
extern volatile u8 VOLUME;
extern u8 RSSI;
extern u8 SQL;

// Memory pool for mruby/c
static uint8_t memory_pool[MRBC_MEMORY_SIZE];

// Event queue for Ruby
#define EVENT_QUEUE_SIZE 16
static struct {
    int type;
    int value;
} event_queue[EVENT_QUEUE_SIZE];
static volatile int event_head = 0;
static volatile int event_tail = 0;

// ============================================================================
// LCD Class Bindings
// ============================================================================

// LCD.clear(scope = :all)
// scope: :all, :edit_zone
static void c_lcd_clear(mrbc_vm *vm, mrbc_value v[], int argc) {
    _ClearScope scope = GLOBAL32;
    if (argc >= 1 && v[1].tt == MRBC_TT_SYMBOL) {
        const char *sym = mrbc_symbol_cstr(&v[1]);
        if (strcmp(sym, "edit_zone") == 0) {
            scope = EDITZONE32;
        }
    }
    LCD_Clear(scope);
    SET_NIL_RETURN();
}

// LCD.text(x, y, string, size = :medium, invert: false)
// size: :small (4x8), :medium (6x8), :large (10x16)
static void c_lcd_text(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc < 3) {
        SET_NIL_RETURN();
        return;
    }

    int x = mrbc_integer(v[1]);
    int y = mrbc_integer(v[2]);
    const char *str = (const char *)GET_STRING_ARG(3);

    // Default to medium (6x8)
    int size = 1;
    int flag = 1;  // normal (not inverted)

    if (argc >= 4 && v[4].tt == MRBC_TT_SYMBOL) {
        const char *sym = mrbc_symbol_cstr(&v[4]);
        if (strcmp(sym, "small") == 0) size = 0;
        else if (strcmp(sym, "large") == 0) size = 2;
    }

    // Check for invert keyword (simplified - check 5th arg)
    if (argc >= 5 && v[5].tt == MRBC_TT_TRUE) {
        flag = 0;  // inverted
    }

    switch (size) {
        case 0: LCD_ShowString0408(x, y, str, flag); break;
        case 1: LCD_ShowString0608(x, y, str, flag, 128); break;
        case 2: LCD_ShowString1016(x, y, str, flag, 128); break;
    }

    SET_NIL_RETURN();
}

// LCD.char(x, y, char_code, size = :medium, invert: false)
static void c_lcd_char(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc < 3) {
        SET_NIL_RETURN();
        return;
    }

    int x = mrbc_integer(v[1]);
    int y = mrbc_integer(v[2]);
    int ch = mrbc_integer(v[3]);
    int size = 1;
    int flag = 1;

    if (argc >= 4 && v[4].tt == MRBC_TT_SYMBOL) {
        const char *sym = mrbc_symbol_cstr(&v[4]);
        if (strcmp(sym, "small") == 0) size = 0;
        else if (strcmp(sym, "large") == 0) size = 2;
    }

    switch (size) {
        case 0: LCD_ShowAscii0408(x, y, ch); break;
        case 1: LCD_ShowAscii0608(x, y, ch, flag); break;
        case 2: LCD_ShowAscii1016(x, y, ch, flag); break;
    }

    SET_NIL_RETURN();
}

// LCD.battery(percent)
static void c_lcd_battery(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        int percent = mrbc_integer(v[1]);
        LCD_ShowBattery(percent);
    }
    SET_NIL_RETURN();
}

// LCD.signal(level)
static void c_lcd_signal(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        int level = mrbc_integer(v[1]);
        LCD_ShowSignal(level);
    }
    SET_NIL_RETURN();
}

// LCD.volume(level)
static void c_lcd_volume(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        int vol = mrbc_integer(v[1]);
        LCD_ShowVolume(vol);
    }
    SET_NIL_RETURN();
}

// LCD.frequency(x, y, freq, invert = false)
static void c_lcd_frequency(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc < 3) {
        SET_NIL_RETURN();
        return;
    }
    int x = mrbc_integer(v[1]);
    int y = mrbc_integer(v[2]);
    double freq = mrbc_float(v[3]);
    int flag = (argc >= 4 && v[4].tt == MRBC_TT_TRUE) ? 0 : 1;
    LCD_ShowFreq(x, y, freq, flag);
    SET_NIL_RETURN();
}

// LCD.channel(x, y, number, invert = false)
static void c_lcd_channel(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc < 3) {
        SET_NIL_RETURN();
        return;
    }
    int x = mrbc_integer(v[1]);
    int y = mrbc_integer(v[2]);
    int chan = mrbc_integer(v[3]);
    int flag = (argc >= 4 && v[4].tt == MRBC_TT_TRUE) ? 0 : 1;
    LCD_ShowChan(x, y, chan, flag);
    SET_NIL_RETURN();
}

// LCD.icon(x, y, icon_id)
static void c_lcd_icon(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc < 3) {
        SET_NIL_RETURN();
        return;
    }
    int x = mrbc_integer(v[1]);
    int y = mrbc_integer(v[2]);
    int icon = mrbc_integer(v[3]);
    LCD_ShowPIC0808(x, y, icon);
    SET_NIL_RETURN();
}

// LCD.progress_bar(x, y, percent)
static void c_lcd_progress(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 3) {
        int x = mrbc_integer(v[1]);
        int y = mrbc_integer(v[2]);
        int pct = mrbc_integer(v[3]);
        LCD_ShowProcessBar(x, y, pct);
    }
    SET_NIL_RETURN();
}

// LCD.scrollbar(total_items, selected, items_per_page = 3)
static void c_lcd_scrollbar(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 2) {
        int total = mrbc_integer(v[1]);
        int sel = mrbc_integer(v[2]);
        int per_page = (argc >= 3) ? mrbc_integer(v[3]) : 3;
        LCD_ShowPageBar(total, sel, per_page);
    }
    SET_NIL_RETURN();
}

// LCD.contrast(level)
static void c_lcd_contrast(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        int level = mrbc_integer(v[1]);
        LCD_LightRatio(level * 5);
    }
    SET_NIL_RETURN();
}

// ============================================================================
// Radio Class Bindings
// ============================================================================

// Radio.channel -> Hash with channel info
static void c_radio_channel(mrbc_vm *vm, mrbc_value v[], int argc) {
    mrbc_value hash = mrbc_hash_new(vm, 8);

    mrbc_hash_set(&hash, &mrbc_symbol_value(mrbc_str_to_symid("number")),
                  &mrbc_integer_value(chan_arv[NOW].CHAN));
    mrbc_hash_set(&hash, &mrbc_symbol_value(mrbc_str_to_symid("rx_freq")),
                  &mrbc_float_value(vm, chan_arv[NOW].RX_FREQ));
    mrbc_hash_set(&hash, &mrbc_symbol_value(mrbc_str_to_symid("tx_freq")),
                  &mrbc_float_value(vm, chan_arv[NOW].TX_FREQ));
    mrbc_hash_set(&hash, &mrbc_symbol_value(mrbc_str_to_symid("power")),
                  &mrbc_integer_value(chan_arv[NOW].POWER));

    // Nickname as string
    mrbc_value nn = mrbc_string_new_cstr(vm, (const char*)chan_arv[NOW].NN);
    mrbc_hash_set(&hash, &mrbc_symbol_value(mrbc_str_to_symid("name")), &nn);

    SET_RETURN(hash);
}

// Radio.volume -> Integer
static void c_radio_volume(mrbc_vm *vm, mrbc_value v[], int argc) {
    SET_INT_RETURN(VOLUME);
}

// Radio.volume=(level)
static void c_radio_set_volume(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        VOLUME = mrbc_integer(v[1]);
        // TODO: Actually set volume on hardware
    }
    SET_NIL_RETURN();
}

// Radio.rssi -> Integer (signal strength 0-100)
static void c_radio_rssi(mrbc_vm *vm, mrbc_value v[], int argc) {
    SET_INT_RETURN(RSSI);
}

// Radio.squelch -> Integer
static void c_radio_squelch(mrbc_vm *vm, mrbc_value v[], int argc) {
    SET_INT_RETURN(SQL);
}

// Radio.ptt? -> true if receiving (PTT not pressed)
static void c_radio_ptt(mrbc_vm *vm, mrbc_value v[], int argc) {
    // PTT_READ is high when not transmitting
    extern int PTT_READ;
    if (PTT_READ) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

// Radio.battery -> Integer (battery percentage)
static void c_radio_battery(mrbc_vm *vm, mrbc_value v[], int argc) {
    extern int Get_Battery_Vol(void);
    SET_INT_RETURN(Get_Battery_Vol());
}

// Radio.send_at(command) - send AT command to radio module
static void c_radio_send_at(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        const char *cmd = (const char *)GET_STRING_ARG(1);
        Serial2.print(cmd);
        Serial2.print("\r\n");
    }
    SET_NIL_RETURN();
}

// ============================================================================
// Event Class Bindings
// ============================================================================

// Event.poll -> Hash or nil
static void c_event_poll(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (event_head != event_tail) {
        int type = event_queue[event_tail].type;
        int value = event_queue[event_tail].value;
        event_tail = (event_tail + 1) % EVENT_QUEUE_SIZE;

        mrbc_value hash = mrbc_hash_new(vm, 2);
        mrbc_hash_set(&hash, &mrbc_symbol_value(mrbc_str_to_symid("type")),
                      &mrbc_integer_value(type));
        mrbc_hash_set(&hash, &mrbc_symbol_value(mrbc_str_to_symid("value")),
                      &mrbc_integer_value(value));
        SET_RETURN(hash);
    } else {
        SET_NIL_RETURN();
    }
}

// ============================================================================
// Timer Class Bindings
// ============================================================================

// Timer.millis -> Integer
static void c_timer_millis(mrbc_vm *vm, mrbc_value v[], int argc) {
    SET_INT_RETURN((int)millis());
}

// Timer.delay(ms)
static void c_timer_delay(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        int ms = mrbc_integer(v[1]);
        delay(ms);
    }
    SET_NIL_RETURN();
}

// ============================================================================
// Debug/System Bindings
// ============================================================================

// System.log(message)
static void c_system_log(mrbc_vm *vm, mrbc_value v[], int argc) {
    if (argc >= 1) {
        const char *msg = (const char *)GET_STRING_ARG(1);
        Serial.print("[Ruby] ");
        Serial.println(msg);
    }
    SET_NIL_RETURN();
}

// System.free_memory -> Integer
static void c_system_free_mem(mrbc_vm *vm, mrbc_value v[], int argc) {
    SET_INT_RETURN(ESP.getFreeHeap());
}

// ============================================================================
// Registration
// ============================================================================

static void register_lcd_class(void) {
    mrbc_class *cls = mrbc_define_class(0, "LCD", mrbc_class_object);

    mrbc_define_method(0, cls, "clear", c_lcd_clear);
    mrbc_define_method(0, cls, "text", c_lcd_text);
    mrbc_define_method(0, cls, "char", c_lcd_char);
    mrbc_define_method(0, cls, "battery", c_lcd_battery);
    mrbc_define_method(0, cls, "signal", c_lcd_signal);
    mrbc_define_method(0, cls, "volume", c_lcd_volume);
    mrbc_define_method(0, cls, "frequency", c_lcd_frequency);
    mrbc_define_method(0, cls, "channel", c_lcd_channel);
    mrbc_define_method(0, cls, "icon", c_lcd_icon);
    mrbc_define_method(0, cls, "progress_bar", c_lcd_progress);
    mrbc_define_method(0, cls, "scrollbar", c_lcd_scrollbar);
    mrbc_define_method(0, cls, "contrast", c_lcd_contrast);
}

static void register_radio_class(void) {
    mrbc_class *cls = mrbc_define_class(0, "Radio", mrbc_class_object);

    mrbc_define_method(0, cls, "channel", c_radio_channel);
    mrbc_define_method(0, cls, "volume", c_radio_volume);
    mrbc_define_method(0, cls, "volume=", c_radio_set_volume);
    mrbc_define_method(0, cls, "rssi", c_radio_rssi);
    mrbc_define_method(0, cls, "squelch", c_radio_squelch);
    mrbc_define_method(0, cls, "ptt?", c_radio_ptt);
    mrbc_define_method(0, cls, "battery", c_radio_battery);
    mrbc_define_method(0, cls, "send_at", c_radio_send_at);
}

static void register_event_class(void) {
    mrbc_class *cls = mrbc_define_class(0, "Event", mrbc_class_object);
    mrbc_define_method(0, cls, "poll", c_event_poll);
}

static void register_timer_class(void) {
    mrbc_class *cls = mrbc_define_class(0, "Timer", mrbc_class_object);
    mrbc_define_method(0, cls, "millis", c_timer_millis);
    mrbc_define_method(0, cls, "delay", c_timer_delay);
}

static void register_system_class(void) {
    mrbc_class *cls = mrbc_define_class(0, "System", mrbc_class_object);
    mrbc_define_method(0, cls, "log", c_system_log);
    mrbc_define_method(0, cls, "free_memory", c_system_free_mem);
}

// ============================================================================
// Public API
// ============================================================================

int ruby_init(void) {
    mrbc_init(memory_pool, MRBC_MEMORY_SIZE);

    // Register all classes
    register_lcd_class();
    register_radio_class();
    register_event_class();
    register_timer_class();
    register_system_class();

    Serial.println("[Ruby] mruby/c VM initialized");
    Serial.printf("[Ruby] Memory pool: %d bytes\n", MRBC_MEMORY_SIZE);

    return 0;
}

int ruby_run_bytecode(const uint8_t *bytecode) {
    if (mrbc_create_task(bytecode, 0) == NULL) {
        Serial.println("[Ruby] Failed to create task");
        return -1;
    }
    return 0;
}

void ruby_tick(void) {
    mrbc_run();
}

int ruby_is_running(void) {
    // Check if any tasks are still running
    return 1;  // Simplified - always return running
}

void ruby_send_event(int event_type, int value) {
    int next_head = (event_head + 1) % EVENT_QUEUE_SIZE;
    if (next_head != event_tail) {  // Not full
        event_queue[event_head].type = event_type;
        event_queue[event_head].value = value;
        event_head = next_head;
    }
}

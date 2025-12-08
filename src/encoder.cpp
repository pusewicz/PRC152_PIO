#include "encoder.h"
#include "bsp_delay.h"
#include "bsp_conio.h"

volatile int TIMES = 0;	// Encoder operation value, positive for add, negative for subtract
volatile u8 key_timer_cnt1 = 0;
volatile u8 key_timer_cnt2 = 0;
volatile int spin_cal = 0;
static volatile int spin_old = 0;

extern void ClearShut(void);
extern int L_LAST, R_LAST;	


void Encoder_Init()
{
    Encoder_Click_Init();
    Encoder_Spin_init();
    
    L_LAST = ENCODER_SPIN_L_READ;
    R_LAST = ENCODER_SPIN_R_READ;
}

void Encoder_DeInit()
{
    // Encoder_Click_DeInit();
    Encoder_Spin_DeInit();
}



int EncoderClickValidate(void)
{
    return (ENCODER_CLICK_READ==0);
}
static unsigned char key_driver(void)
{
    static unsigned char key_state_buffer1 = key_state_0;

    unsigned char key_return = key_idle;
    unsigned char key = 0;
    
    if(ENCODER_CLICK_READ==0)
    {
        delay_ms(5);
        if(ENCODER_CLICK_READ==0)
            key=1;
    }
        
    switch(key_state_buffer1)
    {
        case key_state_0:
            if(key)
                key_state_buffer1 = key_state_1;
                // Key pressed, state transitions to debounce and confirm state //
            break;
            
        case key_state_1:
            if(key)
            {
                key_timer_cnt1 = 0;
                key_state_buffer1 = key_state_2;
                // Key still pressed
                // Debounce complete, key_timer starts counting
                // State transitions to press duration timing state
            }
            else
                key_state_buffer1 = key_state_0;
                // Key released, return to initial state
            break;  // Software debounce complete
            
        case key_state_2:
            if(!key)
            {
                key_return = key_click;  // Key released, generate click event
                key_state_buffer1 = key_state_0;  // Transition to initial state
            }
            else if(key && key_timer_cnt1 >= 15)  // Key still pressed, timing exceeds 1000ms
            {
                ClearShut();
                key_return = key_long;  // Return long press event
                key_state_buffer1 = key_state_3;  // Transition to wait for key release state
            }
            break;

        case key_state_3:  // Wait for key release
            if(!key)  // Key released
                key_state_buffer1 = key_state_0;  // Return to initial state
            break;
    }
    return key_return;
}
 
/***************************************************************************
Function: Middle-layer key processing function, calls lower-layer function once,
          handles double-click event detection,
          returns correct states: idle, click, double-click, long-press
This function is called by upper layer in loop, interval 10ms
***************************************************************************/
u8 Encoder_Switch_Scan(u8 mode)
{
    static unsigned char key_state_buffer2 = key_state_0;

    unsigned char key_return = key_idle;
    unsigned char key;
    
    key = key_driver();
    
    switch(key_state_buffer2)
    {
        case key_state_0:
            if(key == key_click)
            {
                key_timer_cnt2 = 0;  // First click, don't return, go to next state to check for double-click
                key_state_buffer2 = key_state_1;
				ClearShut();
            }
            else
                key_return = key;  // For idle or long-press, return original event
            break;

        case key_state_1:
            if(key == key_click)  // Another click, time interval less than 500ms
            {
                key_return = key_double;  // Return double-click event, back to initial state
                key_state_buffer2 = key_state_0;
            }
            else if(key_timer_cnt2 > 3)//4
            {
                // Within 500ms, reads are always idle events because long-press is >1000ms
                // Before 1s, lower layer returns idle

                key_return = key_click;  // No second click within 500ms, return click event
                key_state_buffer2 = key_state_0;  // Return to initial state

            }
            break;
    }
    
    return key_return;
}
//

void disposeEncoderSpined(void)
{
    // D_printf("A:%d, B:%d\n", ENCODER_SPIN_L_READ, ENCODER_SPIN_R_READ);
    if (L_LAST != ENCODER_SPIN_L_READ && spin_cal - spin_old > 100)
    {
        ClearShut();
        L_LAST = ENCODER_SPIN_L_READ;
        TIMES = (ENCODER_SPIN_L_READ != ENCODER_SPIN_R_READ ? 1 : -1);
        spin_old = spin_cal;
    }
}

#include "bsp_conio.h"
#include "bsp_uart.h"  

void ControlGPIO_Init(void)
{
    pinMode(POWER_EN_PIN, OUTPUT);
    POWER_EN_CLR;
}

void bsp_A002_Init(void)
{
    bsp_UART2_Init(9600);

    pinMode(A002_SQ_PIN,  INPUT_PULLUP);
    pinMode(A002_PD_PIN,  OUTPUT);
    pinMode(A002_PTT_PIN, OUTPUT);

    A002_PTT_SET;
    A002_PD_CLR;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int L_LAST=0, R_LAST=0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  // Declare portMUX_TYPE variable for synchronization between main code and interrupts
void Encoder_Click_Init(void)
{
    pinMode(ENCODER_CLICK_PIN, INPUT_PULLUP);
}
void Encoder_Click_DeInit(void)
{
    // pinMode(ENCODER_CLICK_PIN, INPUT_PULLDOWN);// After enabling, device can't power off, always draws 21mA
}
void EncoderLPinInterrupt();
void Encoder_Spin_init(void)
{
    pinMode(ENCODER_SPIN_R_PIN, INPUT_PULLUP);
    pinMode(ENCODER_SPIN_L_PIN, INPUT_PULLUP);  // We set this to pulldown INPUT_PULLDOWN
    // We attach interrupt to pin by calling attachInterrupt function
    // handleInterrupt is the callback function after interrupt trigger

    attachInterrupt(digitalPinToInterrupt(ENCODER_SPIN_L_PIN), EncoderLPinInterrupt, CHANGE);

}
void Encoder_Spin_DeInit(void)
{
    detachInterrupt(digitalPinToInterrupt(ENCODER_SPIN_L_PIN));
    pinMode(ENCODER_SPIN_R_PIN, INPUT_PULLDOWN);   
    pinMode(ENCODER_SPIN_L_PIN, INPUT_PULLDOWN);
}

extern void disposeEncoderSpined(void);
void EncoderLPinInterrupt()
{
    // portENTER_CRITICAL_ISR(&mux);
    
    disposeEncoderSpined();
    
    // portEXIT_CRITICAL_ISR(&mux);
}

void Function_Key_Init(void)
{
    pinMode(VOL_ADD_PIN, INPUT_PULLUP);
    pinMode(VOL_SUB_PIN, INPUT_PULLUP);
    pinMode(PTT_PIN,     INPUT_PULLUP);
    pinMode(SQUELCH_PIN, INPUT_PULLUP);
}
void Function_Key_DeInit(void)
{
    pinMode(VOL_ADD_PIN, INPUT_PULLDOWN);
    pinMode(VOL_SUB_PIN, INPUT_PULLDOWN);
    pinMode(PTT_PIN,     INPUT_PULLDOWN);
    pinMode(SQUELCH_PIN, INPUT_PULLDOWN);
}

/// @brief PTT button state
/// @param  void
/// @return true: pressed; false: idle
bool PTTPress(void)
{
    return !PTT_READ;
}
/// @brief A20 receive state
/// @param  void
/// @return true: signal received; false: idle
bool RcvSignal(void)
{
    return !A002_SQ_READ;
}

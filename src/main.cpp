//VCC RX TX GND DTR_RST RTS_tck

#define FREERTOS_CONFIG_XTENSA_H
#include "main.h"
extern void menuUpdate(void);
void setup()
{
    UART1_Init(); // Initialize serial port

    CH423_Init(); // I2C init --> CH423 init --> CH423 controlled pins init

    SPK_SWITCH(IN, OFF); // Suppress startup noise

    Key_Init(); // Initialize keys: encoder button; check if BOOT mode is needed

    Standby_Init(); // Verify encoder long press; enable 3.3V control pin  ////POWER_EN_SET;//

    // UART1_Init(); // Initialize serial port
    ADC_Init();   // For voltage detection
    Timer_Init(); // Start timer for ADC voltage detection and DAC output interrupt

    // Initialize control pins and set states to avoid incorrect IO states
    M62364_Init(); // M62364 init --> disable audio output / turn off FM power

    LCD_Init();
    PWM_Init();
    LCD_ShowPICALL(pic_HARRIS);

    Init_Storage(true);
    enterSecondSystem(); // menuUpdate();    // Channel settings require memory initialization

    EN_GPIO_Init(); // POWER_EN_8_SET;//

    SineWave_Data();
    VFO_Load_Data();
    A002_Init();
    VFO_Clear();
    
    // while(1)FeedDog();
    // test_A20Deinit();
    // test_DAC_always();
    // test_FM();
    // while(1)PGM_Menu();
    //     PGM_RTControl();
}
void loop()
{
    VFO_Refresh();
    MY_GLOBAL_FUN();
    Encoder_process(Encoder_Switch_Scan(0));
    Argument_process(Event_Matrix(Matrix_KEY_Scan(0))); // Matrix key event processing
    KDU_Processor();                                    // KDU processing
}

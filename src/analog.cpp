#include "analog.h"
#include "tim_int.h"
#include "controller.h"

#include "bsp_m62364.h"
#include "bsp_delay.h"
#include "bsp_dac.h"
#include "bsp_uart.h"
#include "bsp_device.h"

extern int LAMP_TIME;
extern u8 BL;
extern u8 A20_LEVEL[8], VOLUME, AUD;
extern volatile u8 KDU_INSERT;
u8 POWER_SELECT_FLAG = 1;

void PWM_Init(void)
{
    bsp_PWM_Init();
    BackLight_SetVal(50);
}

void ADC_Init(void)
{
    bsp_ADC_Init();
}

void DAC_Init(void)
{
    SineWave_Data(); // Generate data
    bsp_DAC_Init();
}

void LightBacklight(void)
{
    // Serial.printf("LAMP_TIME :%d && KDU_INSERT = %d\n", LAMP_TIME, KDU_INSERT);
    // Serial.printf(" \n@@@@@%04d  KDU_INSERT = %d@@@@@%\n", __LINE__, KDU_INSERT);
    if (LAMP_TIME > 0 && KDU_INSERT == OFF)
    {
        // D_printf("*****%s\n", __FUNCTION__);
        bsp_StartAutoTimer(TMR_FLOW, LAMP_TIME);
        BackLight_SetVal(BL);
    }
}

int Get_Battery_Vol(void)
{
    int voltage = Use_ADC();
    static u8 lowBatteryCal = 0; // Low battery voltage detection count
    if (POWER_SELECT_FLAG)       // 12V
    {
        if (voltage < VOLTAGE_OFF_12_FLOOR || voltage > VOLTAGE_OFF_12_UPPER)
            lowBatteryCal++;
        voltage = (voltage - VOLTAGE_OFF_12_FLOOR) / 30;
    }
    else // 8V
    {
        if (voltage < VOLTAGE_OFF_8_FLOOR || voltage > VOLTAGE_OFF_8_UPPER)
            lowBatteryCal++;
        voltage = (voltage - VOLTAGE_OFF_8_FLOOR) / 20;
    }
    if (lowBatteryCal >= 5)
    {
        D_printf("Low Power to Shut...\n");
        SHUT();
    }

    if (voltage < 0)
        voltage = 0;
    else if (voltage > 100)
        voltage = 100;

    return voltage;
}

void test_DAC_always()
{
    SPK_SWITCH(AUD, ON);
    M62364_SetSingleChannel(A20_LINE_CHAN, Di_Gain);
    M62364_SetSingleChannel(8, Di_Gain_Local);
    pinMode(DAC_RX_PIN, OUTPUT);
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHAN));
    RingTone(TONE2K, ON);
    while (1)
    {
        FeedDog();
    }
}

// PTT press TX start and end tones
void Start_Tone(unsigned char STOP_START)
{
    SPK_SWITCH(AUD, ON); // ==> Sound TX notification

    if (VOLUME > 0)
    {
        M62364_SetSingleChannel(A20_LINE_CHAN, Di_Gain);
        M62364_SetSingleChannel(8, Di_Gain_Local);
    }
    else
    {
        M62364_SetSingleChannel(A20_LINE_CHAN, 0);
        M62364_SetSingleChannel(8, 0);
    }
    M62364_SetSingleChannel(TONE_OUT_CHAN, 100); // Output to A20 transmit

    delay_ms(200); // Required delay, otherwise first tone is missing

    // pinMode(DAC_RX_PIN, OUTPUT);
    bsp_UART2_DeInit();
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHAN));
    if (STOP_START == 1) // Start
    {
        // Pre-transmission tone
        RingTone(TONE2K, ON);
        delay_ms(100); // delay_ms(80);
        RingTone(TONE2K, OFF);
        delay_ms(110);
        RingTone(TONE2K, ON);
        delay_ms(160);
        RingTone(TONE2K, OFF);
    }
    else
    {
        // End transmission tone
        RingTone(TONE1_5K, ON);
        delay_ms(100);
        RingTone(TONE1_5K, OFF);
    }
    //////////////////////////////////////////////////////////////////
    dac_output_voltage(DAC_CHAN, 0);
    ESP_ERROR_CHECK(dac_output_disable(DAC_CHAN));
    bsp_UART2_Init(9600);
    // A002_Init();

    SPK_SWITCH(AUD, OFF); // ==> TX notification
    M62364_SetSingleChannel(A20_LINE_CHAN, 0);
    M62364_SetSingleChannel(TONE_OUT_CHAN, 0);
    M62364_SetSingleChannel(8, 0);
    // delay_ms(5);
}

// Long-press squelch button to enter constant squelch mode notification tone
void Start_ToneSql0(void)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    M62364_SetSingleChannel(A20_LINE_CHAN, Di_Gain);            // Modify gain output "Di"
    M62364_SetSingleChannel(8, 50);                             // Tone out enable
    //////////////////////////////////////////////////////////////////
    // Configure DAC
    pinMode(DAC_RX_PIN, OUTPUT);                                // Set pin to output mode to start DAC
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHAN));               // Set serial port pin
    //////////////////////////////////////////////////
    RingTone(TONE1_5K, ON);
    delay_ms(60);
    RingTone(TONE1_5K, OFF);
    //////////////////////////////////////////////////
    // End DAC, reconfigure A002
    dac_output_voltage(DAC_CHAN, 0);
    ESP_ERROR_CHECK(dac_output_disable(DAC_CHAN));
    bsp_UART2_Init(9600);                                       // Pull high, restore UART2 RX, set serial port pin
    //////////////////////////////////////////////////////////////////
    M62364_SetSingleChannel(8, 0);                              // Tone out disable
    M62364_SetSingleChannel(A20_LINE_CHAN, A20_LEVEL[VOLUME]);  // Restore current gain level for audio output
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // delay_ms(5);
}
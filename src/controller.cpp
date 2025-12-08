#include "controller.h"
#include "bsp_uart.h"
#include "bsp_analog.h"
#include "analog.h"
#include "bsp_m62364.h"

extern void SHUT(void);
extern uint8_t SQL;
void EN_GPIO_Init(void) // Enable pin initialization
{
    switch (Select_Power())
    {
    case VOLTAGE_NORMAL:
        D_printf("Normal voltage startup\n");
        return;

    case VOLTAGE_LOW:
        LCD_Clear(GLOBAL32);
        LCD_ShowString0608(25, 2, "Power LOW", 1, 128);
        break;

    case VOLTAGE_HIGH:
        LCD_Clear(GLOBAL32);
        LCD_ShowString0608(25, 2, "Power HIGH", 1, 128);
        break;

    case VOLTAGE_ERROR:
        D_printf("Voltage error\n");
        LCD_Clear(GLOBAL32);
        LCD_ShowString0608(25, 2, "Power ERROR", 1, 128);
        break;

    default:
        break;
    }
    delay_ms(2000);
    SHUT();
}
void A002_Init(void) // A20 control pin initialization
{
    bsp_A002_Init();
    A002_PTT_SET;
    A002_PD_SET;
    delay_ms(2);

    unsigned char a002_send_buff[20] = "AT+DMOSETVOLUME=5\r\n";
    for (int i = 0; i < 19; i++)
        UART2_Put_Char(a002_send_buff[i]);
    Set_A20(chan_arv[NOW], SQL);
    // Serial.printf("A002 Transmition Moudle Initial Successfully!\n");
}

void A002_Deinit(void) // A20 control pin de-initialization
{
    pinMode(A002_SQ_PIN, INPUT_PULLUP);
    pinMode(A002_PD_PIN, INPUT_PULLDOWN);
    pinMode(A002_PTT_PIN, INPUT_PULLUP);
    pinMode(DAC_RX_PIN, INPUT_PULLDOWN);
    pinMode(GPIO_NUM_17, INPUT_PULLDOWN);
}

void test_A20Deinit(void)
{
    while (1)
    {
        Serial.printf("save power:1****\n");
        // Set_A20_SavePower(ON);
        A002_Init();
        A002_CALLBACK();
        delay_ms(15000);

        Serial.printf("save power:0****\n");
        // Set_A20_SavePower(OFF);
        A002_Deinit();
        A002_CALLBACK();
        delay_ms(10000);
    }
}

u8 Select_Power(void)
{
    // for(u8 i=0; i<5; i++)
    //  refreshADCVal();
    uint32_t adc_val = Use_ADC();
    // Serial.printf("\nCurrent voltage:%d\n", adc_val);

    if (adc_val > VOLT8_ON_FLOOR && adc_val < VOLT8_ON_UPPER)
    {
        POWER_EN_12_SET;
        delay_ms(100);
        POWER_EN_12_CLR; // Hardware bug: 8V power current pulled low instantly by 12V power circuit causing shutdown
        POWER_EN_8_SET;
        POWER_SELECT_FLAG = 0;
        return VOLTAGE_NORMAL;
    }
    else if (adc_val > VOLTAGE_ON_12_FLOOR) //&& adc_val<VOLTAGE_ON_12_UPPER
    {
        POWER_EN_12_SET;
        POWER_SELECT_FLAG = 1;
        return VOLTAGE_NORMAL;
    }
    else
    {
        if (adc_val >= VOLT8_ON_UPPER && adc_val <= VOLTAGE_ON_12_FLOOR)
            return VOLTAGE_ERROR;
        if (adc_val <= VOLT8_ON_FLOOR)
            return VOLTAGE_LOW;
        return VOLTAGE_ERROR;
    }
}

extern u8 MIC_LEVEL[3], MIC;

void MIC_SWITCH(char mic_temp, char on_off)
{
    D_printf("\nMIC:%d, sta:%d\n", mic_temp, on_off);
    // Enable op-amp
    // Set gain
    if (on_off)
    {
        switch (mic_temp)
        {
        case IN: // Internal mic
            MIC_IN_SET;
            M62364_SetSingleChannel(MIC_IN_CHAN, MIC_LEVEL[1]);
            break;

        case TOP:  // Top mic
        case SIDE: // Side mic
            MIC_OUT_SET;
            M62364_SetSingleChannel(MIC_OUT_CHAN, MIC_LEVEL[MIC]);
            break;
        }
    }
    else
    {
        // Disable op-amp
        MIC_IN_CLR;
        MIC_OUT_CLR;
        // Disable matrix gain
        M62364_SetSingleChannel(MIC_IN_CHAN, 0);
        M62364_SetSingleChannel(MIC_OUT_CHAN, 0);
    }
}
//
void SPK_SWITCH(char spk_temp, char on_off)
{
    D_printf("\nSPK:%d, sta:%d\n", spk_temp, on_off);
    if (on_off)
    {
        // Enable power amp
        switch (spk_temp)
        {
        case IN: // Internal spk
            SPK_IN_SET;
            break;

        case TOP:  // Top spk
        case SIDE: // Side spk
            SPK_OUT_SET;
            break;
        }
    }
    else
    {
        // Disable power amp
        SPK_IN_CLR;
        SPK_OUT_CLR;
    }
    delay_ms(1);
}
void VDO_SWITCH(unsigned char on_off) // 6-pin header power output
{
    if (on_off)
    {
        VDO_SET;
        return;
    }
    VDO_CLR;
}
//

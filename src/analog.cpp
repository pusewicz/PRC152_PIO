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
    SineWave_Data(); //生成数据
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
    static u8 lowBatteryCal = 0; //检测电池电压低次数
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

//按下PTT的发射和结束提示音
void Start_Tone(unsigned char STOP_START)
{
    SPK_SWITCH(AUD, ON); //==>响 发射提示

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
    M62364_SetSingleChannel(TONE_OUT_CHAN, 100); //输出到A20发射

    delay_ms(200); //必需的延时,否则缺失第一声

    // pinMode(DAC_RX_PIN, OUTPUT);
    bsp_UART2_DeInit();
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHAN));
    if (STOP_START == 1) //开始
    {
        //前置提示音
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
        //结束提示音
        RingTone(TONE1_5K, ON);
        delay_ms(100);
        RingTone(TONE1_5K, OFF);
    }
    //////////////////////////////////////////////////////////////////
    dac_output_voltage(DAC_CHAN, 0);
    ESP_ERROR_CHECK(dac_output_disable(DAC_CHAN));
    bsp_UART2_Init(9600);
    // A002_Init();

    SPK_SWITCH(AUD, OFF); //==>发射提示
    M62364_SetSingleChannel(A20_LINE_CHAN, 0);
    M62364_SetSingleChannel(TONE_OUT_CHAN, 0);
    M62364_SetSingleChannel(8, 0);
    // delay_ms(5);
}

//长按静噪按键进入常静噪模式的提示音
void Start_ToneSql0(void)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    M62364_SetSingleChannel(A20_LINE_CHAN, Di_Gain);            //修改增益输出"Di"
    M62364_SetSingleChannel(8, 50);                             // toneout输出打开
    //////////////////////////////////////////////////////////////////
    //配置DAC
    pinMode(DAC_RX_PIN, OUTPUT);                                //设置引脚为输出模式，以便于启动DAC
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHAN));               //设置串口引脚
    //////////////////////////////////////////////////
    RingTone(TONE1_5K, ON);
    delay_ms(60);
    RingTone(TONE1_5K, OFF);
    //////////////////////////////////////////////////
    //结束DAC, 重新配置A002
    dac_output_voltage(DAC_CHAN, 0);
    ESP_ERROR_CHECK(dac_output_disable(DAC_CHAN));
    bsp_UART2_Init(9600);                                       //拉高电平,恢复串口2RX通讯, 设置串口引脚
    //////////////////////////////////////////////////////////////////
    M62364_SetSingleChannel(8, 0);                              // toneout输出关闭
    M62364_SetSingleChannel(A20_LINE_CHAN, A20_LEVEL[VOLUME]);  //恢复当前增益大小输出声音
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // delay_ms(5);
}
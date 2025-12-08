#include "bsp_analog.h"
#include "bsp_conio.h"
#include "tim_int.h"

#define PWM_CHAN 1
#define ADC_SAMPLE_NUM 10
volatile uint16_t MY_ADC_VAL[ADC_SAMPLE_NUM]; // adc的数值缓存
extern int LAMP_TIME;

void bsp_PWM_Init(void)
{
    ledcSetup(PWM_CHAN, 24000, 8); // 0~255
    ledcAttachPin(PWM_PIN, PWM_CHAN);
    ledcWrite(PWM_CHAN, 0);
}
void bsp_ADC_Init(void)
{
    pinMode(ADC_PIN, INPUT);
    adcAttachPin(ADC_PIN);    //将引脚连接到ADC
    analogReadResolution(13); //设置aliogRead返回值的分辨率
}


void BackLightSleep(void)
{
    if (LAMP_TIME && bsp_CheckTimer(TMR_FLOW))
        BackLight_SetVal(0);
}
void BackLight_SetVal(u8 val)
{
    val = val * 255 / 100;
    ledcWrite(PWM_CHAN, val);
}

void refreshADCVal(void)
{
    static int i = 0;
    MY_ADC_VAL[i++] = analogRead(ADC_PIN);
    i %= 10;
    // MY_ADC_VAL[i] = analogRead(ADC_PIN);
    // i = (i+1) % 10;
    MY_ADC_VAL[i++] = analogRead(ADC_PIN);
    i %= 10;
}
#define ADC_DEBUG 0
uint32_t Use_ADC(void)
{
#if ADC_DEBUG
    for (int i = 0; i < 10; i++)
        Serial.printf("MY_ADC_VAL[%d]: %d\n", i, MY_ADC_VAL[i]);
#endif
    uint32_t real_val = MY_ADC_VAL[0],
             v0 = MY_ADC_VAL[0],
             max_v0 = MY_ADC_VAL[0],
             min_v0 = MY_ADC_VAL[0];

    for (int i = 1; i < ADC_SAMPLE_NUM; i++)
    {
        max_v0 = (max_v0 > MY_ADC_VAL[i]) ? max_v0 : MY_ADC_VAL[i];
        min_v0 = (min_v0 < MY_ADC_VAL[i]) ? min_v0 : MY_ADC_VAL[i];
        v0 += MY_ADC_VAL[i];
    }
    // memset((void *)MY_ADC_VAL, 0, ADC_SAMPLE_NUM);
    v0 = (v0 - max_v0 - min_v0) / (ADC_SAMPLE_NUM - 2);

#if ADC_DEBUG
    Serial.printf("\n解读V0:%d\n", v0);
#endif
    real_val = v0 * 2563 / 8191 * 321 / 51;
#if ADC_DEBUG
    Serial.printf("\n真实电压:%d\n", real_val);
#endif
    return real_val;
}

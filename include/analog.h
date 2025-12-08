#ifndef __ANALOG_H__
#define __ANALOF_H__

#include "bsp_analog.h"
#define Di_Gain 15
#define Di_Gain_Local 70

//#define PI        3.1415926535
//#define N           (1.5)


extern unsigned char POWER_SELECT_FLAG;

void PWM_Init(void);
void ADC_Init(void);
void DAC_Init(void);


void LightBacklight(void);
int  Get_Battery_Vol(void);
void SineWave_Data(void);
void Start_Tone(unsigned char STOP_START);
void Start_ToneSql0(void);

void test_DAC_always(void);

#endif

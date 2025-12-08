#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_	 
#include "bsp_conio.h" 



void EN_GPIO_Init(void);			// Enable pin initialization
void A002_Init(void);				// A20 control pin initialization
void A002_Deinit(void);
void test_A20Deinit(void);

uint8_t Select_Power(void);
void VDO_SWITCH(unsigned char on_off);          // 6-pin header power output
void MIC_SWITCH(char mic_temp, char on_off);	// MIC enable
void SPK_SWITCH(char spk_temp, char on_off);	// SPK enable

#endif

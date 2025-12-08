#ifndef __KEY_H__
#define __KEY_H__

#include "bsp_conio.h"

void Key_Init(void);
void Key_DeInit(void);
unsigned char VolumeKeyScan(unsigned char mode);	// Volume up/down key scan

#endif

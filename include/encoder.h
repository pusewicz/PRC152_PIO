#ifndef __ENCODER_H__
#define __ENCODER_H__
#include "FCS152_KDU.h"

void    Encoder_Init(void);
void    Encoder_DeInit(void);
int     EncoderClickValidate(void);             // Return encoder click valid value
uint8_t Encoder_Switch_Scan(uint8_t mode);

#endif

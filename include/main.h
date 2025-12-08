#ifndef __MAIN_H__
#define __MAIN_H__

#include "FCS152_KDU.h"

#include "bsp_wifi.h"
#include "bsp_dac.h"
#include "bsp_uart.h"
#include "bsp_ch423.h"
#include "bsp_conio.h"
#include "bsp_timer.h"
#include "bsp_m62364.h"
#include "bsp_storage.h"
#include "bsp_device.h" // Sleep Init
#include "bsp_MatrixKeyBoard.h"

#include "lcd.h"
#include "key.h"    //key:Independent Key, MatrixKeyBoard, EncoderClick/Spin
#include "analog.h" //ADC DAC PWM
#include "encoder.h"
#include "tim_int.h" //Timing
#include "controller.h"

#if FM_EN
#include "rda5807.h"
extern volatile u8 WFM; // FM switch
#endif

extern int TIMES; // Record encoder operation

void VFO_Load_Data(void);
void VFO_Clear(void);   // Main page initialization
void VFO_Refresh(void); // Main page refresh

void Encoder_process(u8 operate);
u8 Event_Matrix(u8 matrix_key); // Main page matrix key trigger event detection 0:no change, 1:reload, 2:save
void Argument_process(u8 key_pro_ret);

int MY_GLOBAL_FUN(void); // Global processing function
int PTT_Control(void);   // Code executed only once after PTT press/release
void SQ_Read_Control(void);
void SQUELCH_Contol(void);

int readWriteValueToKDU(int Cmd);
int PRC152receiveProcess(void);
int KDUCheck(void);            //
int KDU_Processor(void);       // KDU data interaction

void VOL_Reflash(int operate); // Volume setting
void A20_CALLBACK(void);       // A20 data interaction, must process to get A20 data return
void Switch_Dual_Chan(void);   // Dual-watch mode channel switch
void SetNowChanSql0(u8 on);    // Toggle constant squelch state
// Main page function selection
void ShortCut_Menu(void);           // Main page shortcut settings
void ShortCut_MICGAIN_Select(void); // Main page shortcut mic sensitivity
void ShortCut_FM_Select(void);      // Main page shortcut FM radio toggle
void ShortCut_CHAN_Select(void);    // Main page channel switch
int Lock_Screen_KeyBoard(void);     // Lock screen/keyboard

// TX/RX settings
void RT_Menu(void);
void RT_Menu_Clear(void);
int RT_FREQ_Set(int x, int y, double *vfo_freq_temp, int vu_switch);
int RT_SubVoice_Set(int row, int subvoice);               // Sub-audio tone setting
int RT_SubVoice_Matrix_Menu_Select(int subvoice);         // Matrix sub-audio tone setting
int RT_TX_POWER_Set(int power_temp);                      // TX power selection
int RT_GBW_Set(int gbw_temp);                             // Bandwidth selection
int RT_NICKNAME_Set(u8 current_channel, char nn_temp[7]); // Nickname setting
void RT_CHAN_Switch(void);                                // Channel number switch

// Key 2
void Light_Mode_Set(void);

// Key 5: Initialize menu
int Zeroize_All(void);
void Zero_Menu(void);

// Key 7 OPTION menu
void OPTION_Menu(void);
void Key_Test(void); // Test keys

// Key 8 PGM menu
void PGM_Menu(void);

int PGM_AUDIO_Select(u8 row);   // Audio select and mic sensitivity setting
int PGM_SQL_Set(u8 row);        // Squelch level
int PGM_STEP_Set(u8 row);       // Step
int PGM_ENCRPY_Set(u8 row);     // Encryption
int PGM_TOT_Set(u8 row);        // TX timeout
int PGM_LAMP_TIME_Set(u8 row);  // Backlight duration
int PGM_POWEROUT_Set(u8 row);   // 6-pin header power output
int PGM_TONE_Select(u8 row);    // Tone setting
int PGM_RTControl(u8 row);      // WiFi real-time control
int PGM_IdleTime2Sleep(u8 row); // Idle detection time setting


void SHUT(void); // Shutdown all functions
double checkFreqFloat(double freq_buf); // Validation

// void Update_Check(void);			//IAP

// uint16_t Get_JTAG_ID(void);

#endif

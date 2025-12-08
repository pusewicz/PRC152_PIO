#ifndef __BSP_STORAGE_H__
#define __BSP_STORAGE_H__
#include "FCS152_KDU.h"

#ifdef DEBUG
#define RESET_VAL 0x24
#else
#define RESET_VAL 0x55
#endif

#define MEM_LENGTH 42
#define WIFI_SHOW_SIZE 18

void    Init_Storage(bool init);

// Flag bits
void    set_Flag(int flag_number,uint8_t value);
uint8_t get_Flag(int flag_number);

// Scan add
void    set_Scan(uint8_t channel, uint8_t scan);
uint8_t get_Scan(uint8_t channel);

void    save_ChanA(uint8_t chana);
uint8_t load_ChanA(void);
void    save_ChanB(uint8_t chanb);
uint8_t load_ChanB(void);

// Channel number
void    save_CurrentChannel(uint8_t channel);
uint8_t load_CurrentChannel(void);

// Channel parameters
void save_ChannelParameter(uint8_t chan, CHAN_ARV S);
void load_ChannelParameter(uint8_t chan, CHAN_ARV_P L);
//
void load_ChannelParameterStr(uint8_t chan, char *L);
void save_ChannelParameterStr(uint8_t chan, char *S);

// Data initialization
void DATA_Init(void);

// Audio input/output selection
void    save_AudioSelect(uint8_t audio);
uint8_t load_AudioSelect(void);

// Mic sensitivity
void    save_MicLevel(uint8_t mic);
uint8_t load_MicLevel(void);

// Squelch
void    save_Sql(uint8_t sql);
uint8_t load_Sql(void);

// Encryption
void    save_ScramLevel(uint8_t scram);
uint8_t load_ScramLevel(void);

// Step
void    save_Step(uint8_t step);
uint8_t load_Step(void);

// Transmit timeout
void    save_Tot(uint8_t tot);
uint8_t load_Tot(void);

// Backlight brightness
void    save_Backlightness(uint8_t value);
uint8_t load_Backlightness(void);
// Contrast
void    save_ScreenContrast(uint8_t value);
uint8_t load_ScreenContrast(void);

// Backlight mode
void    save_LampTime(uint8_t lamptime);
uint8_t load_LampTime(void);

// 6-pin header output
void    save_VDO(uint8_t vdo);
uint8_t load_VDO(void);


// Global volume
void    save_OverVolume(uint8_t volume);
uint8_t load_OverVolume(void);


// PTT pre-tone
void    save_PreTone(uint8_t pretone);
uint8_t load_PreTone(void);
// PTT end-tone
void    save_EndTone(uint8_t endtone);
uint8_t load_EndTone(void);

// FM radio frequency // 870~1080
void save_FMFreq(int fm_freq);
int  load_FMFreq(void);

int load_WIFIInfo(char* ssid, char* password);
int save_WIFIInfo(char* ssid, char* password);

// Idle time
uint8_t save_IdleTime2Sleep(uint8_t itts);
uint8_t load_IdleTime2Sleep(void);
#endif



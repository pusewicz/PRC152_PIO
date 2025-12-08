/*
	#include "FCS152_KDU.h"
*/
#ifndef __FCS152_KDU_H__
#define __FCS152_KDU_H__
#include "userinclude.h"
#include "bsp_delay.h"
#include "lcd.h"

#define  THISCHIP   THISCHIP_ESP32S2
// #define  DEBUG

//define the major file to include.
#if     (THISCHIP == THISCHIP_STM32F103RET6)
    #include "stm32f1xx.h"
#elif   (THISCHIP == THISCHIP_ESP32S2)
    #include <Arduino.h>
    #include "esp_timer.h"
    #include "esp_rom_sys.h"
    #include "driver/timer.h"
#elif   (THISCHIP == THISCHIP_CM32M101A)
    #include "cm32m101a.h"
#else
#endif

//define the debug mode how to realize.
#if     (THISCHIP != THISCHIP_ESP32S2)	
    #ifdef DEBUG
        #define D_printf(fmt,args...) \
        do \
        {\
            printf(fmt, ##args);\
        }while(0)
        //printf("%s:  %s:  %d\n", __FILE__, __FUNCTION__, __LINE__);
    #else
        #define D_printf(fmt,args...)
    #endif
#else
    #ifdef DEBUG
        #define D_printf(fmt,args...) \
        do \
        {\
            Serial.printf(fmt, ##args);\
        }while(0)
        //printf("%s:  %s:  %d\n", __FILE__, __FUNCTION__, __LINE__);
    #else
        #define D_printf(fmt,args...)
    #endif
#endif
//define some system operate function.
#if     (THISCHIP == THISCHIP_STM32F103RET6)
    #define FeedDog()           IWDG->KR = 0XAAAA
    #define ResetSystem()       NVIC_SystemReset()
#elif   (THISCHIP == THISCHIP_ESP32S2)
    #define FeedDog()           delay_ms(5)
    #define ResetSystem()       ESP.restart()
#elif   (THISCHIP == THISCHIP_CM32M101A)
    #define FeedDog()           IWDG->KEY = 0xAAAA
    #define ResetSystem()       NVIC_SystemReset()
    //#define INTX_DISABLE()    __disable_irq();
    //#define INTX_ENABLE()     __enable_irq();
#else
#endif

//define the flash address to store the BOOTLOADER, APP and FLAG
#if     (THISCHIP == THISCHIP_ESP32S2)              //// No receive address setting needed for now
#elif   (THISCHIP == THISCHIP_CM32M101A)            //// KDU only
/*
    0x0800 0000 ~ 0x0800 4800: 18K boot
        bootloader  : 18K = 18*1024 = 18432 = 0x4800
    0x0800 4800 ~ 0x0800 5000: 2K  USER FLAG
        USERFLAG    : 2K  = 2*1024  = 2048  = 0x0800

    0x0800 5000 ~ 0x0801 2800: 54K App space
        APP RUN     : 54K = 54*1024 = 55296 = 0xD800
    0x0801 2800 ~ 0x0802 0000: 54K Receive new program
        APP RCV     : 54K = 54*1024 = 55296 = 0xD800
*/
    #define     APP_PAGE_SIZE       0xD800          //54K   // Running APP space / Receiving APP space, both 54K
    #define     USER_PAGE_SIZE      0x0800          //2K    // Page size
    #define     KDU_FLAG_ADDR       0x08004800      // Program update info storage address 0X55
    #define     KDU_SIZE_ADDR       0x08004900      // KDU receive data size storage address
    #define     KDU_RUN_ADDR        0x08005000      // KDU run start address
    #define     KDU_RCV_ADDR        0x08012800      // KDU receive start address
#elif   (THISCHIP == THISCHIP_STM32F103RET6)        //// Both 152 and KDU need setting
    #define     KDU_RUN_ADDR        0x08010000      //
    #define     KDU_RCV_ADDR        0x08020000      //
    #define     KDU_FLAG_ADDR       0x08030000      // Program update info storage address 0X55
    
    //Start address for 152 IAP 
    #if !defined UPBOOT
        #define     _152_RUN_ADDR   0x8020000       // 128K space for APP 
    #else
        #define     _152_RUN_ADDR   0x8000000
    #endif
#endif

//deside the device information string
#if     (THISCHIP != STM32F103RET6)
    #define   __NEW__                           //this program base on CM32M101, which is new
#endif
#ifndef   __NEW__
    #define  STR_152         "FCS PRC152"
    #define  STR_KDU         " FCS  KDU "
    #define  VERSION_152     "Rev 1.0.0000" 
    #define  VERSION_KDU     "Rev 1.1.0000"          // Modified for ESP compatibility
#else
    #define  STR_152         "FCS PRC152-N"
    #define  STR_KDU         "FCS KDU-N "
    //2.1: Smart network config changed to one-to-one upgrade/write freq + other fixes
    //2.2.4B18 //2.1.0316 //2.2.3412
    //4507: Release power limit
    //4621: Fix voltage display
    //4823: Fixed a display issue
    //4B18: Fix RT interface setting display issue
    //5328: Fix A20 signal query slow response issue
    //5408: Fix center display issue
    //5507: Remove restriction on using CLR to exit FM interface when receiving signal
    #define  VERSION_152     "Rev 2.2.5507"
    #define  VERSION_KDU     "Rev 2.1.1226"
#endif

#define  VERSION_UPBOOT     "Upgrade Bootloader0.1" // For updating 152 base layer, 152 version
#define  VERSION_BOOT       "BOOT Rev 0.2.0000"      // BOOT version for upgrading 152

//////////////////////////////////////////////////////////////////////////////////
#define     USART1_BUF_SIZE         1024+8+1+50     // Buffer length
#define     USART2_BUF_SIZE         255             // Serial2 buffer length

#define     NO_OPERATE          0
#define     RELOAD_ARG          1
#define     SAVE_SET            2

#define     CLR2LAST            0
#define     ENT2LAST            1
#define     BACK2MAIN           2
#define     NEED2REFRESH        3

#define     FM_EN               1
#define     LCD12832            0 
#define     LCD12864            2 

//The flag of appaddr whether the app should run
#define     NEW_APP             0x55
#define     RUN_APP             0xAA
//#define   WRO_APP             0xAA
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// 8.4V power on voltage upper/lower limit
#define  VOLT8_ON_UPPER         8800
#define  VOLT8_ON_FLOOR         6400    // Too little power, useless to power on
// 12V power on voltage upper/lower limit
#define  VOLTAGE_ON_12_UPPER        13250
#define  VOLTAGE_ON_12_FLOOR        9250

// 8.4V power off voltage upper/lower limit
#define  VOLTAGE_OFF_8_UPPER        8680
#define  VOLTAGE_OFF_8_FLOOR        6100    // Set as low as safe voltage allows
// 12.4V power off voltage upper/lower limit
#define  VOLTAGE_OFF_12_UPPER       13250
#define  VOLTAGE_OFF_12_FLOOR       8900
// Voltage calculation base value
#define  VOLTAGE_8_CAL              6400
#define  VOLTAGE_12_CAL             9600

// Voltage level definition
typedef enum 
{
    VOLTAGE_NORMAL=0,
    VOLTAGE_ERROR,
    VOLTAGE_LOW,
    VOLTAGE_HIGH,
}CheckVoltage; 

typedef enum
{
    FAILED = 0,
    PASSED = !FAILED
} Status;

typedef enum 
{
    IN=0,
    TOP,
    SIDE
}AudioSelect;

typedef enum
{
    TONE2K,
    TONE1_5K,
}ToneClass;

typedef struct	//32
{
    volatile u8 CHAN;
    volatile u8 RS;
    volatile u8 TS;
    volatile u8 POWER;
    volatile u8 GBW;
    volatile u8 SCAN;
    
    volatile double RX_FREQ;
    volatile double TX_FREQ;
    volatile char NN[8];        // Extra byte for null terminator '\0'
    
}
CHAN_ARV, *CHAN_ARV_P;

#define ARV_MEM_COUNT 4 
enum
{
    NOW=0,  // Current channel parameters
    TMP,    // Cached channel parameters being sought
    CHANA,  // Dual-watch mode channel A parameters
    CHANB,  // Dual-watch mode channel B parameters
};
extern CHAN_ARV chan_arv[ARV_MEM_COUNT];
enum
{
    MAIN_MODE = 0,
    BIG_MODE,
    DUAL_MODE
};
enum recv_mess
{
    ASKALL=0,       // Receive
    _ASKALL,        // Send
    
    ASKCHAN,
    _ASKCHAN,
    
    ASKA,
    _ASKA,
    
    ASKB,
    _ASKB,
    
    RELOAD,
    _RELOAD,
    
    RELA,
    _RELA,
    
    RELB,
    _RELB,
    
    SETCHAN,
    _SETCHAN,
    
    NORMAL,
    _SETZERO,
    
    SETHOMEMODE,
    _SETHOMEMODE,
////////////////////////////////////
    SETSTEP,
    _SETSTEP,
    
    SETSQL,
    _SETSQL,
    
    SETAUD,
    _SETAUD,
    
    SETENC,
    _SETENC,
    
    SETTOT,
    _SETTOT,
    
    SETOP,
    _SETVDO,
    
    SETVOLU,
    _SETVOLU,// Volume level
    
    SETTONE,
    _SETTONE,
    
    SETFM,
    _SETFM,

    SETDUALPOS,
    _SETDUALPOS,
};
//
// PGM menu definition
typedef enum
{
    normal_set_mic_gain,
    normal_set_sql,
    normal_set_step,
    normal_set_tot,
    normal_set_lamptime,
    normal_set_powerout,
    normal_set_ptttone,
    normal_set_rcu,
    normal_set_itts,
}normal_set; 

// Data length
#define Length_CHAN             3   // Channel
#define Length_RX               9   // RX frequency
#define Length_TX               9   // TX frequency
#define Length_RS               3   // RX sub-audio tone
#define Length_TS               3   // TX sub-audio tone
#define Length_POWER            1   // Power
#define Length_BW               1   // Bandwidth
#define Length_NN               8   // Nickname
#define Length_SCAN             1   // Scan flag

#define Length_CF               1   // Current channel/frequency mode
#define Length_VU               1   // Current V/U band
#define Length_CHANA            3   // Dual-watch channel A
#define Length_CHANB            3   // Dual-watch channel B

#define Length_VOLUME           1   // Volume
#define Length_STEP             1   // Step
#define Length_SQL              1   // Squelch
#define Length_AUDIO            1   // Audio select
#define Length_MIC              1   // Mic sensitivity
#define Length_ENCRYPTION       1   // TX encryption
#define Length_TOT              1   // TX timeout
#define Length_OUTPOWER         1   // 6-pin header voltage output
#define Length_PRETONE          1   // TX pre-tone
#define Length_ENDTONE          1   // TX end-tone
#define Length_FMFREQ           4   // FM radio frequency
// KDU
#define Length_WFM              1   // FM radio switch
#define Length_FMCHAN           1   // FM radio channel
#define Length_VOLTAGE          3   // Voltage
#define Length_RSSI             3   // Signal strength
#define Length_KEYSQ            1   // Receive signal state
#define Length_KEYSQU           1   // Squelch button state
#define Length_KEYPTT           1   // PTT button state
#define Length_HOMEMODE         1   // Home mode
#define Length_NOWRCVCHAN       3   // Current receiving signal channel (dual-watch mode)
#define Length_NOWSELCHAN       1   // Current selected dual-watch channel // A or B
// MEMORY
#define Length_BACKLIGHTNESS    3   // Backlight brightness
#define Length_FLAGBACKLIGHT    1   // Backlight switch
#define Length_LAMPTIME         1   // Backlight time
#define Length_SCREENCONTRAST   1   // Screen contrast

// AT save data address
// 100-499 Global variables
// 100-199 Settings
// Arrangement order
#define	CHAN_RANK                           0
#define RX_RANK                             (CHAN_RANK          +Length_CHAN      ) //3
#define TX_RANK                             (RX_RANK            +Length_RX        ) //11
#define	RS_RANK                             (TX_RANK            +Length_TX        ) //19
#define	TS_RANK                             (RS_RANK            +Length_RS        ) //22
#define	POWER_RANK                          (TS_RANK            +Length_TS        ) //25
#define	BW_RANK                             (POWER_RANK         +Length_POWER     ) //26
#define	NN_RANK                             (BW_RANK            +Length_BW        ) //27
#define	SCAN_RANK                           (NN_RANK            +Length_NN        ) //35

#define	CF_RANK                             (SCAN_RANK          +Length_SCAN      ) //36
#define VU_RANK                             (CF_RANK            +Length_CF        ) //37
#define CHANA_RANK                          (VU_RANK            +Length_VU        ) //38
#define CHANB_RANK                          (CHANA_RANK         +Length_CHANA     ) //41

#define	VOLUME_RANK                         (CHANB_RANK         +Length_CHANB     ) //44
#define	STEP_RANK                           (VOLUME_RANK        +Length_VOLUME    ) //45
#define	SQL_RANK                            (STEP_RANK          +Length_STEP      ) //46
#define	AUDIO_RANK                          (SQL_RANK           +Length_SQL       ) //47
#define	MIC_RANK                            (AUDIO_RANK         +Length_AUDIO     ) //48
#define ENCRYPTION_RANK                     (MIC_RANK           +Length_MIC       ) //49
#define TOT_RANK                            (ENCRYPTION_RANK    +Length_ENCRYPTION) //50
#define	VDO_RANK                            (TOT_RANK           +Length_TOT       ) //51   
#define	PRETONE_RANK                        (VDO_RANK           +Length_OUTPOWER  ) //52
#define	ENDTONE_RANK                        (PRETONE_RANK       +Length_PRETONE   ) //53
#define	FMFREQ_RANK                         (ENDTONE_RANK       +Length_ENDTONE   ) //54

// KDU use
#define	WFM_RANK                            (FMFREQ_RANK        +Length_FMFREQ    ) //////58
#define FMCHAN_RANK                         (WFM_RANK           +Length_WFM       ) //59

#define VOLTAGE_RANK                        (FMCHAN_RANK        +Length_FMCHAN    ) //60
#define RSSI_RANK                           (VOLTAGE_RANK       +Length_VOLTAGE   ) //63
#define KEY_SQ_RANK                         (RSSI_RANK          +Length_RSSI      ) //66
#define	KEY_SQU_RANK                        (KEY_SQ_RANK        +Length_KEYSQ     ) //67
#define KEY_PTT_RANK                        (KEY_SQU_RANK       +Length_KEYSQU    ) //68

#define HOMEMODE_RANK                       (KEY_PTT_RANK       +Length_KEYPTT    ) //69
#define NOWRCVCHAN_RANK                     (HOMEMODE_RANK      +Length_HOMEMODE  ) //70
#define NOWSELCHAN_RANK                     (NOWRCVCHAN_RANK    +Length_NOWRCVCHAN) //73

// Memory storage use
#define	BACKLIGHTNESS_RANK                  (FMFREQ_RANK        +Length_FMFREQ    ) //////58
#define FLAG_BACKLIGHT_RANK                 (BACKLIGHTNESS_RANK +Length_BACKLIGHTNESS)  //61
#define LAMPTIME_RANK                       (FLAG_BACKLIGHT_RANK+Length_FLAGBACKLIGHT)  //62
#define SCREEN_CONTRAST_RANK                (LAMPTIME_RANK      +Length_LAMPTIME  )     //63

// Send and receive all data, one frame contains all info, different commands extract different content
#define kdu_start_rank                      16 
#define chan_rank                           kdu_start_rank + CHAN_RANK  
#define rx_rank                             kdu_start_rank + RX_RANK    
#define tx_rank                             kdu_start_rank + TX_RANK 
#define rs_rank                             kdu_start_rank + RS_RANK 
#define ts_rank                             kdu_start_rank + TS_RANK 
#define pw_rank                             kdu_start_rank + POWER_RANK 
#define bw_rank                             kdu_start_rank + BW_RANK 
#define nn_rank                             kdu_start_rank + NN_RANK 
#define scan_rank                           kdu_start_rank + SCAN_RANK  

#define cf_rank                             kdu_start_rank + CF_RANK 
#define vu_rank                             kdu_start_rank + VU_RANK 
#define chana_rank                          kdu_start_rank + CHANA_RANK 
#define chanb_rank                          kdu_start_rank + CHANB_RANK 

#define volume_rank                         kdu_start_rank + VOLUME_RANK 
#define step_rank                           kdu_start_rank + STEP_RANK 
#define sql_rank                            kdu_start_rank + SQL_RANK 
#define aud_rank                            kdu_start_rank + AUDIO_RANK 
#define mic_rank                            kdu_start_rank + MIC_RANK 
#define enc_rank                            kdu_start_rank + ENCRYPTION_RANK 
#define tot_rank                            kdu_start_rank + TOT_RANK 
#define vdo_rank                             kdu_start_rank + VDO_RANK 
#define pre_rank                            kdu_start_rank + PRETONE_RANK 
#define end_rank                            kdu_start_rank + ENDTONE_RANK 
#define ffreq_rank                          kdu_start_rank + FMFREQ_RANK    //FM_Freq FM radio frequency

#define wfm_rank                            kdu_start_rank + WFM_RANK       // FM radio switch flag
#define fmchan_rank                         kdu_start_rank + FMCHAN_RANK    // Is FM frequency a channel

#define volt_rank                           kdu_start_rank + VOLTAGE_RANK
#define rssi_rank                           kdu_start_rank + RSSI_RANK      // A20 signal strength
#define sq_rank                             kdu_start_rank + KEY_SQ_RANK    // Signal state
#define squ_rank                            kdu_start_rank + KEY_SQU_RANK   // Squelch state
#define ptt_rank                            kdu_start_rank + KEY_PTT_RANK   // PTT state
#define homemode_rank                       kdu_start_rank + HOMEMODE_RANK
#define nowrcvchan_rank                     kdu_start_rank + NOWRCVCHAN_RANK
#define nowselchan_rank                     kdu_start_rank + NOWSELCHAN_RANK

#define BUF_SIZE                            nowselchan_rank + Length_NOWSELCHAN+1 
//////////////////////////////////////////////////////////////////////////////////////////
//Enable the eeprom
#if (THISCHIP != ESP32S2)
    #define  EN_EEROOM
#endif

#ifdef EN_EEROOM                            // AT save data address

#define RESETADDR                           99
#define SETADDR                             100                         // Base address
#define CURRENT_CHANNEL_ADDR                SETADDR+CHAN_RANK           // Current channel number
#define FLAG_CF_SWITCH_ADDR                 SETADDR+CF_RANK             // 0C, 1F
#define FLAG_VU_SWITCH_ADDR                 SETADDR+VU_RANK             // 0V, 1U
#define CHANA_ADDR                          SETADDR+CHANA_RANK
#define CHANB_ADDR                          SETADDR+CHANB_RANK

#define STEP_ADDR                           SETADDR+STEP_RANK           // Step 5k 10k 12.5k
#define SQ_ADDR                             SETADDR+SQL_RANK            // Squelch level 0-8
#define AUDIO_SELECT_ADDR                   SETADDR+AUDIO_RANK          // Audio output select
#define MIC_LEVEL_ADDR                      SETADDR+MIC_RANK            // Mic sensitivity 0-7
#define SCRAM_LEVEL_ADDR                    SETADDR+ENCRYPTION_RANK     // TX encryption 0-8
#define TOT_ADDR                            SETADDR+TOT_RANK            // TX timeout 0-9min
#define OUTPOWER_ADDR                       SETADDR+VDO_RANK            // 6-pin output
#define PRETONE_ADDR                        SETADDR+PRETONE_RANK        // PTT pre-tone
#define ENDTONE_ADDR                        SETADDR+ENDTONE_RANK        // PTT end-tone
#define OVER_VOLUME_ADDR                    SETADDR+VOLUME_RANK         // Global volume 0-7
//#define FLAG_WFMMOD_ADDR                  SETADDR+WFM_RANK            // FM radio OFF0 ON1
#define FM_RADIO_FREQ_ADDR                  SETADDR+FMFREQ_RANK         // FM radio freq 4 digits
#define LAMPTIME_ADDR                       SETADDR+LAMPTIME_RANK       // Backlight time 0/1 always on/10s
#define BACKLIGHTNESS_ADDR                  SETADDR+BACKLIGHTNESS_RANK  // Backlight brightness 0-100
#define FLAG_BACKLIGHT_ADDR                 SETADDR+FLAG_BACKLIGHT_RANK // Backlight switch
#define ScreenContrast_ADDR                 SETADDR+SCREEN_CONTRAST_RANK// Contrast
#define	DATA_ADDR                           896
//#define MEM_LENGTH                          64
#define V_CHANNEL_ADDR                      DATA_ADDR+CHAN_RANK         // V channel info start address
#define V_RX_ADDR                           DATA_ADDR+RX_RANK           // V RX freq
#define V_TX_ADDR                           DATA_ADDR+TX_RANK           // V TX freq
#define V_RS_ADDR                           DATA_ADDR+RS_RANK           // V RX sub-audio
#define V_TS_ADDR                           DATA_ADDR+TS_RANK           // V TX sub-audio
#define V_TX_POWER_ADDR                     DATA_ADDR+POWER_RANK        // TX power
#define V_GBW_ADDR                          DATA_ADDR+BW_RANK           // Bandwidth
#define V_NN_ADDR                           DATA_ADDR+NN_RANK           // Nickname
#define V_SCAN_ADDR                         DATA_ADDR+SCAN_RANK         // Scan flag
#define U_FREQ_CHANNEL                      100
#define U_CHANNEL_ADDR                      (DATA_ADDR+MEM_LENGTH*U_FREQ_CHANNEL)    // U channel info start address
#define U_RX_ADDR                           U_CHANNEL_ADDR+RX_RANK      // U RX freq
#define U_TX_ADDR                           U_CHANNEL_ADDR+TX_RANK      // U TX freq
#define U_RS_ADDR                           U_CHANNEL_ADDR+RS_RANK      // U RX sub-audio
#define U_TS_ADDR                           U_CHANNEL_ADDR+TS_RANK      // U TX sub-audio
#define U_TX_POWER_ADDR                     U_CHANNEL_ADDR+POWER_RANK   // TX power
#define U_GBW_ADDR                          U_CHANNEL_ADDR+BW_RANK      // Bandwidth
#define U_NN_ADDR                           U_CHANNEL_ADDR+NN_RANK      // Nickname
#define U_SCAN_ADDR                         U_CHANNEL_ADDR+SCAN_RANK    // Scan flag
#else
typedef enum
{
    RESETADDR,
    FLAG_CF_SWITCH_ADDR,
    FLAG_VU_SWITCH_ADDR,
}FLAG;
#endif

#endif

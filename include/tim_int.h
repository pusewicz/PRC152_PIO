#ifndef __TIM_INT_H__
#define __TIM_INT_H__
#include "bsp_timer.h"  


/* Macros for global interrupt control */
#define ENABLE_INT()            /* Enable global interrupt */
#define DISABLE_INT()           /* Disable global interrupt */  

#define TMR_COUNT           12  /* Number of software timers (timer ID range 0 - 3) */

#define TMR_PERIOD_2MS       2  
#define TMR_PERIOD_10MS     10  
#define TMR_PERIOD_15MS     15  
#define TMR_PERIOD_20MS     20  
#define TMR_PERIOD_30MS     30  
#define TMR_PERIOD_40MS     40  
#define TMR_PERIOD_50MS     50  
#define TMR_PERIOD_80MS     80  
#define TMR_PERIOD_100MS    100 
#define TMR_PERIOD_200MS    200 
#define TMR_PERIOD_300MS    300 
#define TMR_PERIOD_500MS    500 
#define TMR_PERIOD_1S       1000    
#define TMR_PERIOD_2S       2000 
#define TMR_PERIOD_3S       3000 
#define TMR_PERIOD_8S       8000 


#define DUAL_SWITCH_TIME        TMR_PERIOD_1S
#define WAIT_KDU_INSERT_TIME    TMR_PERIOD_1S * 1.2   // 4 seconds
#define WAIT_KDU_LEAVE_TIME     TMR_PERIOD_1S * 1.2   // 4 seconds

/* Timer structure, member variables must be volatile, otherwise C compiler optimization may cause issues */
typedef enum
{
    TMR_ONCE_MODE = 0,      /* One-shot mode */
    TMR_AUTO_MODE = 1       /* Auto-repeat mode */
}TMR_MODE_E;

enum
{
    TMR_FLOW=0,			// Backlight sleep
    TMR_KEY_SCAN,		// Key scan interval
    TMR_KEY_SAME,		// Same key press interval
    TMR_RSSI_CTRL,		// Signal strength refresh interval
    TMR_VOLT_REFRESH,	// Battery refresh interval
    TMR_DUAL_REFRESH,	// Dual watch mode normal switch interval
    TMR_FM_CTRL,		// Radio resume time
    TMR_OUT_CTRL,		// Exit settings timer
    TMR_WAIT_KDU,		// Detect KDU insertion
    TMR_POS_BLINK,		// Edit cursor blink
    TMR_ANY				// Single use anywhere
};

typedef struct
{
    volatile uint32_t Count;	/* Counter */
    volatile uint32_t PreLoad;	/* Counter preload value */
    volatile uint8_t  Mode;		/* Counter mode, one-shot */
    volatile uint8_t  Flag;		/* Timer expired flag */
}SOFT_TMR;


void    Timer_Init(void);

void    bsp_InitTimer(void);
int32_t bsp_GetRunTime(void);
void    bsp_StartTimer(uint8_t _id, uint32_t _period);
void    bsp_StartAutoTimer(uint8_t _id, uint32_t _period);
void    bsp_StopTimer(uint8_t _id);
uint8_t bsp_CheckTimer(uint8_t _id);

// void    ReloadOutCal(void);
void    reloadTimer(uint8_t _id);
void    RDA5807_ResumeImmediately(void); 
#endif



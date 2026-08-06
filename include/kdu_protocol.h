/*
	#include "kdu_protocol.h"

	KDU frame layout and channel record, extracted from FCS152_KDU.h for
	host-side testing (Arduino-free; includes only userinclude.h).
*/
#ifndef __KDU_PROTOCOL_H__
#define __KDU_PROTOCOL_H__
#include "userinclude.h"

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
    volatile char NN[8];        //多一位用作结束符'\0'

}
CHAN_ARV, *CHAN_ARV_P;

#define ARV_MEM_COUNT 4
enum
{
    NOW=0,  //当前信道参数
    TMP,    //缓存寻求的信道参数
    CHANA,  //双守模式下信道A参数
    CHANB,  //双守模式下信道B参数
};
extern CHAN_ARV chan_arv[ARV_MEM_COUNT];

//数据长度
#define Length_CHAN             3   //信道
#define Length_RX               9   //接收频率
#define Length_TX               9   //发射频率
#define Length_RS               3   //接收亚音
#define Length_TS               3   //发射亚音
#define Length_POWER            1   //功率
#define Length_BW               1   //带宽
#define Length_NN               8   //别名
#define Length_SCAN             1   //扫描标志

#define Length_CF               1   //当前信道/频率模式
#define Length_VU               1   //当前V/U段
#define Length_CHANA            3   //双守信道A
#define Length_CHANB            3   //双守信道B

#define Length_VOLUME           1   //音量
#define Length_STEP             1   //步进
#define Length_SQL              1   //静噪
#define Length_AUDIO            1	//音频选通
#define Length_MIC              1	//mic灵敏度
#define Length_ENCRYPTION       1	//发射加密
#define Length_TOT              1	//发射限时
#define Length_OUTPOWER         1	//六针头电压输出
#define Length_PRETONE          1   //发射前置提示音
#define Length_ENDTONE          1   //发射结束提示音
#define Length_FMFREQ           4	//收音机频率
//KDU
#define Length_WFM              1   //收音机开关
#define Length_FMCHAN           1   //收音机频道
#define Length_VOLTAGE          3   //电压
#define Length_RSSI             3   //信号强度
#define Length_KEYSQ            1   //接收信号状态
#define Length_KEYSQU           1   //按键静噪状态
#define Length_KEYPTT           1   //按键PTT状态
#define Length_HOMEMODE         1   //主页模式
#define Length_NOWRCVCHAN       3   //当前接收到信号的信道(双守模式下)
#define Length_NOWSELCHAN       1   //当前选中双守模式的信道//A或B
//MEMORY
#define Length_BACKLIGHTNESS    3   //背光亮度
#define Length_FLAGBACKLIGHT    1   //背光开关
#define Length_LAMPTIME         1   //背光时间
#define Length_SCREENCONTRAST   1   //屏幕对比度

//AT保存数据地址
//100-499 全局变量
//100-199 设置
//排列顺序
#define	CHAN_RANK                           0
#define RX_RANK                             (CHAN_RANK          +Length_CHAN      ) //3
#define TX_RANK                             (RX_RANK            +Length_RX        ) //12
#define	RS_RANK                             (TX_RANK            +Length_TX        ) //21
#define	TS_RANK                             (RS_RANK            +Length_RS        ) //24
#define	POWER_RANK                          (TS_RANK            +Length_TS        ) //27
#define	BW_RANK                             (POWER_RANK         +Length_POWER     ) //28
#define	NN_RANK                             (BW_RANK            +Length_BW        ) //29
#define	SCAN_RANK                           (NN_RANK            +Length_NN        ) //37

#define	CF_RANK                             (SCAN_RANK          +Length_SCAN      ) //38
#define VU_RANK                             (CF_RANK            +Length_CF        ) //39
#define CHANA_RANK                          (VU_RANK            +Length_VU        ) //40
#define CHANB_RANK                          (CHANA_RANK         +Length_CHANA     ) //43

#define	VOLUME_RANK                         (CHANB_RANK         +Length_CHANB     ) //46
#define	STEP_RANK                           (VOLUME_RANK        +Length_VOLUME    ) //47
#define	SQL_RANK                            (STEP_RANK          +Length_STEP      ) //48
#define	AUDIO_RANK                          (SQL_RANK           +Length_SQL       ) //49
#define	MIC_RANK                            (AUDIO_RANK         +Length_AUDIO     ) //50
#define ENCRYPTION_RANK                     (MIC_RANK           +Length_MIC       ) //51
#define TOT_RANK                            (ENCRYPTION_RANK    +Length_ENCRYPTION) //52
#define	VDO_RANK                            (TOT_RANK           +Length_TOT       ) //53
#define	PRETONE_RANK                        (VDO_RANK           +Length_OUTPOWER  ) //54
#define	ENDTONE_RANK                        (PRETONE_RANK       +Length_PRETONE   ) //55
#define	FMFREQ_RANK                         (ENDTONE_RANK       +Length_ENDTONE   ) //56

//KDU用
#define	WFM_RANK                            (FMFREQ_RANK        +Length_FMFREQ    ) //////60
#define FMCHAN_RANK                         (WFM_RANK           +Length_WFM       ) //61

#define VOLTAGE_RANK                        (FMCHAN_RANK        +Length_FMCHAN    ) //62
#define RSSI_RANK                           (VOLTAGE_RANK       +Length_VOLTAGE   ) //65
#define KEY_SQ_RANK                         (RSSI_RANK          +Length_RSSI      ) //68
#define	KEY_SQU_RANK                        (KEY_SQ_RANK        +Length_KEYSQ     ) //69
#define KEY_PTT_RANK                        (KEY_SQU_RANK       +Length_KEYSQU    ) //70

#define HOMEMODE_RANK                       (KEY_PTT_RANK       +Length_KEYPTT    ) //71
#define NOWRCVCHAN_RANK                     (HOMEMODE_RANK      +Length_HOMEMODE  ) //72
#define NOWSELCHAN_RANK                     (NOWRCVCHAN_RANK    +Length_NOWRCVCHAN) //75

//记忆存储用
#define	BACKLIGHTNESS_RANK                  (FMFREQ_RANK        +Length_FMFREQ    ) //////60
#define FLAG_BACKLIGHT_RANK                 (BACKLIGHTNESS_RANK +Length_BACKLIGHTNESS)  //63
#define LAMPTIME_RANK                       (FLAG_BACKLIGHT_RANK+Length_FLAGBACKLIGHT)  //64
#define SCREEN_CONTRAST_RANK                (LAMPTIME_RANK      +Length_LAMPTIME  )     //65

//将数据全部发送和接收,一帧数据包含所有信息, 根据命令不同解读即可获取不同内容
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
#define ffreq_rank                          kdu_start_rank + FMFREQ_RANK    //FM_Freq	收音机频率

#define wfm_rank                            kdu_start_rank + WFM_RANK       //收音机开关标志
#define fmchan_rank                         kdu_start_rank + FMCHAN_RANK    //fm频率是否是频道

#define volt_rank                           kdu_start_rank + VOLTAGE_RANK
#define rssi_rank                           kdu_start_rank + RSSI_RANK      //A20信号强度
#define sq_rank                             kdu_start_rank + KEY_SQ_RANK    //信号状态
#define squ_rank                            kdu_start_rank + KEY_SQU_RANK   //静噪状态
#define ptt_rank                            kdu_start_rank + KEY_PTT_RANK   //PTT状态
#define homemode_rank                       kdu_start_rank + HOMEMODE_RANK
#define nowrcvchan_rank                     kdu_start_rank + NOWRCVCHAN_RANK
#define nowselchan_rank                     kdu_start_rank + NOWSELCHAN_RANK

#define BUF_SIZE                            nowselchan_rank + Length_NOWSELCHAN+1

// Compile-time pin of the KDU frame layout. These offsets are the de facto
// protocol shared with the KDU-side firmware and the EEPROM storage layout:
// if an assert fires, a Length_* changed — the KDU firmware and any stored
// settings must change in lockstep. Do not just update the numbers here.
static_assert(RX_RANK         == 3,  "KDU frame offset moved: RX_RANK");
static_assert(TX_RANK         == 12, "KDU frame offset moved: TX_RANK");
static_assert(RS_RANK         == 21, "KDU frame offset moved: RS_RANK");
static_assert(TS_RANK         == 24, "KDU frame offset moved: TS_RANK");
static_assert(POWER_RANK      == 27, "KDU frame offset moved: POWER_RANK");
static_assert(BW_RANK         == 28, "KDU frame offset moved: BW_RANK");
static_assert(NN_RANK         == 29, "KDU frame offset moved: NN_RANK");
static_assert(SCAN_RANK       == 37, "KDU frame offset moved: SCAN_RANK");
static_assert(CF_RANK         == 38, "KDU frame offset moved: CF_RANK");
static_assert(VU_RANK         == 39, "KDU frame offset moved: VU_RANK");
static_assert(CHANA_RANK      == 40, "KDU frame offset moved: CHANA_RANK");
static_assert(CHANB_RANK      == 43, "KDU frame offset moved: CHANB_RANK");
static_assert(VOLUME_RANK     == 46, "KDU frame offset moved: VOLUME_RANK");
static_assert(STEP_RANK       == 47, "KDU frame offset moved: STEP_RANK");
static_assert(SQL_RANK        == 48, "KDU frame offset moved: SQL_RANK");
static_assert(AUDIO_RANK      == 49, "KDU frame offset moved: AUDIO_RANK");
static_assert(MIC_RANK        == 50, "KDU frame offset moved: MIC_RANK");
static_assert(ENCRYPTION_RANK == 51, "KDU frame offset moved: ENCRYPTION_RANK");
static_assert(TOT_RANK        == 52, "KDU frame offset moved: TOT_RANK");
static_assert(VDO_RANK        == 53, "KDU frame offset moved: VDO_RANK");
static_assert(PRETONE_RANK    == 54, "KDU frame offset moved: PRETONE_RANK");
static_assert(ENDTONE_RANK    == 55, "KDU frame offset moved: ENDTONE_RANK");
static_assert(FMFREQ_RANK     == 56, "KDU frame offset moved: FMFREQ_RANK");
static_assert(WFM_RANK        == 60, "KDU frame offset moved: WFM_RANK");
static_assert(FMCHAN_RANK     == 61, "KDU frame offset moved: FMCHAN_RANK");
static_assert(VOLTAGE_RANK    == 62, "KDU frame offset moved: VOLTAGE_RANK");
static_assert(RSSI_RANK       == 65, "KDU frame offset moved: RSSI_RANK");
static_assert(KEY_SQ_RANK     == 68, "KDU frame offset moved: KEY_SQ_RANK");
static_assert(KEY_SQU_RANK    == 69, "KDU frame offset moved: KEY_SQU_RANK");
static_assert(KEY_PTT_RANK    == 70, "KDU frame offset moved: KEY_PTT_RANK");
static_assert(HOMEMODE_RANK   == 71, "KDU frame offset moved: HOMEMODE_RANK");
static_assert(NOWRCVCHAN_RANK == 72, "KDU frame offset moved: NOWRCVCHAN_RANK");
static_assert(NOWSELCHAN_RANK == 75, "KDU frame offset moved: NOWSELCHAN_RANK");
static_assert(BUF_SIZE        == 93, "KDU frame total length changed: BUF_SIZE");
static_assert(BACKLIGHTNESS_RANK   == 60, "storage offset moved: BACKLIGHTNESS_RANK");
static_assert(FLAG_BACKLIGHT_RANK  == 63, "storage offset moved: FLAG_BACKLIGHT_RANK");
static_assert(LAMPTIME_RANK        == 64, "storage offset moved: LAMPTIME_RANK");
static_assert(SCREEN_CONTRAST_RANK == 65, "storage offset moved: SCREEN_CONTRAST_RANK");

#endif

/*
	#include "bsp_json.h"
*/

#ifndef __BSP_JSON_H__
#define __BSP_JSON_H__

enum{
    U0ERR_SUCCESS           = 0,
    U0ERR_RCV_PACKAGE_ERR   = -1,   //接收数据包失败
    U0ERR_PARSE_JSON_ERR    = -2,   //解析json数据包失败
    U0ERR_PREPARE_JSON_ERR  = -3,   //准备json数据包失败
    U0ERR_CMD_NOT_EXIT      = -4,   //指令不存在

};
//json项目和值排序, 方便配置
// #define ITEMSUM 33
typedef enum
{
    Jcmd        ,
    Jcurrent    ,
    Jrx_freq    ,
    Jtx_freq    ,
    Jrs         ,
    Jts         ,
    Jpower      ,
    Jbandwith   ,
    Jnickname   ,
    Jcf         ,
    Juv         ,
    JchanA      ,
    JchanB      ,
    Jvolume     ,
    Jstep       ,
    Jsql        ,
    Jaudio      ,
    Jmic        ,
    Jtot        ,
    Jvdo        ,
    JpreTone    ,
    JendTone    ,
    Jwfm        ,
    JfmFreq     ,
    JfmChan     ,
    Jvoltage    ,
    Jrssi       ,
    JrcvSQ      ,
    JpressPTT   ,
    JpressSQU   ,
    Jhomemode   ,
    JrcvChan    ,
    JselPos     ,

    Jdevice     ,
    Jble        ,
    Jpit        ,

    ITEMSUM
} JSON_Parameter_RANK;

typedef struct 
{
    JSON_Parameter_RANK index;
    char item[16];
    char valStr[16];
}ParameterValue_t, *ParameterValue_p;


int test_cJson(void);

// int compriseSendJson(char *sendbuf, const char *item[], char val[33][16], int size);
// void parseRcvJson(const char *rcvbuf, const char *item[], char val[33][16], int size);

void parseRcvJson(const char *rcvbuf);
int compriseSendJson(char *sendbuf);
#endif


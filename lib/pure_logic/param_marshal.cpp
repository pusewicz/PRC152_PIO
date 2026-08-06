#include "param_marshal.h"
#include <stdio.h>
#include <stdlib.h>

ParameterValue_t parameterValue[ITEMSUM] =
{
    {Jcmd       ,"cmd"      ,"FCS+ASKALL"   },
    {Jcurrent   ,"current"  ,"008"          },
    {Jrx_freq   ,"rx_freq"  ,"435.55000"    },
    {Jtx_freq   ,"tx_freq"  ,"435.55000"    },
    {Jrs        ,"rs"       ,"000"          },
    {Jts        ,"ts"       ,"000"          },
    {Jpower     ,"power"    ,"1"            },
    {Jbandwith  ,"bandwith" ,"1"            },
    {Jnickname  ,"nickname" ,"1234567"      },
    {Jcf        ,"cf"       ,"0"            },
    {Juv        ,"uv"       ,"0"            },
    {JchanA     ,"chanA"    ,"001"          },
    {JchanB     ,"chanB"    ,"002"          },
    {Jvolume    ,"volume"   ,"1"            },
    {Jstep      ,"step"     ,"0"            },
    {Jsql       ,"sql"      ,"4"            },
    {Jaudio     ,"audio"    ,"0"            },
    {Jmic       ,"mic"      ,"1"            },
    {Jtot       ,"tot"      ,"0"            },
    {Jvdo       ,"outPower" ,"0"            },
    {JpreTone   ,"preTone"  ,"1"            },
    {JendTone   ,"endTone"  ,"1"            },
    {Jwfm       ,"wfm"      ,"0"            },
    {JfmFreq    ,"fmFreq"   ,"1036"         },
    {JfmChan    ,"fmChan"   ,"1"            },
    {Jvoltage   ,"voltage"  ,"100"          },
    {Jrssi      ,"rssi"     ,"000"          },
    {JrcvSQ     ,"rcvSQ"    ,"0"            },
    {JpressPTT  ,"pressPTT" ,"0"            },
    {JpressSQU  ,"pressSQU" ,"0"            },
    {Jhomemode  ,"homemode" ,"1"            },
    {JrcvChan   ,"rcvChan"  ,"001"          },
    {JselPos    ,"selChan"  ,"002"          },

    {Jdevice    ,"device"   ,"PRC 152N"     },
    {Jble       ,"ble"      ,"0"            },
    {Jpit       ,"pit"      ,"0"            },
};

/// @brief 	从数组内读取数据赋值给信道
/// @param 	需要赋值的信道
void readChanFromArray(CHAN_ARV_P B)
{
    B->CHAN     = atoi(parameterValue[Jcurrent  ].valStr);
    B->RX_FREQ  = atof(parameterValue[Jrx_freq  ].valStr);
    B->TX_FREQ  = atof(parameterValue[Jtx_freq  ].valStr);
    B->RS       = atoi(parameterValue[Jrs       ].valStr);
    B->TS       = atoi(parameterValue[Jts       ].valStr);
    B->POWER    = atoi(parameterValue[Jpower    ].valStr);
    B->GBW      = atoi(parameterValue[Jbandwith ].valStr);
    // Bounded: valStr holds up to 15 chars but NN is 8 bytes (7 + '\0');
    // an unbounded copy would overflow into the neighboring chan_arv slot.
    snprintf((char *)B->NN, sizeof(B->NN), "%s", parameterValue[Jnickname].valStr);

    //		printf("*****chan:%d\r  rx:%.5lf\r tx:%.5lf\r rs:%d\r ts: %d\r power:%d\r gbw:%d\r nn:%s\n",
    //			    B->CHAN, B->RX_FREQ, B->TX_FREQ, B->RS, B->TS, B->POWER, B->GBW, B->NN);

    //	for(int i = Jcurrent; i<Jnickname+1; i++)
    //		printf("%s\n",parameterValue[i]);
}

/// @brief 	将需要发送的信道数据赋值给数组
/// @param 	需要发送的信道
void writeChanToArray(CHAN_ARV_P B)
{
    sprintf(parameterValue[Jcurrent ].valStr, "%03d", B->CHAN);
    sprintf(parameterValue[Jrx_freq ].valStr, "%3.5f", B->RX_FREQ);
    sprintf(parameterValue[Jtx_freq ].valStr, "%3.5f", B->TX_FREQ);
    sprintf(parameterValue[Jrs      ].valStr, "%03d", B->RS);
    sprintf(parameterValue[Jts      ].valStr, "%03d", B->TS);
    sprintf(parameterValue[Jpower   ].valStr, "%d", B->POWER);
    sprintf(parameterValue[Jbandwith].valStr, "%d", B->GBW);
    // %.7s bounds the READ as well: if NN ever lost its terminator, plain %s
    // would run past the 8-byte field.
    snprintf(parameterValue[Jnickname].valStr, sizeof(parameterValue[Jnickname].valStr),
             "%.7s", (const char *)B->NN);
    //		printf("*****chan:%d\r  rx:%.5lf\r tx:%.5lf\r rs:%d\r ts: %d\r power:%d\r gbw:%d\r nn:%s\n",
    //			    B->CHAN, B->RX_FREQ, B->TX_FREQ, B->RS, B->TS, B->POWER, B->GBW, B->NN);
    //	for(int i = Jcurrent; i<Jnickname+1; i++)
    //		printf("%s\n",parameterValue[i]);
}

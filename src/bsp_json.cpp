
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>
#include <string.h>
#include "bsp_json.h"

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

// void parseRcvJson(const char *rcvbuf, const char *item[], char val[33][16], int size)
void parseRcvJson(const char *rcvbuf)
{
    // printf("rcvbuf:%s\n", rcvbuf);
    DynamicJsonDocument jsonDoc(1024);
    DeserializationError error = deserializeJson(jsonDoc, rcvbuf);
    JsonObject obj = jsonDoc.as<JsonObject>();
    if (error)
    {
        // Serial.print(F("deserializeJson() failed: "));
        // Serial.println(error.f_str());
        return;
    }
    for (int i = 0; i < ITEMSUM; i++)
    {
        // val[i] = jsonDoc[item[i]];
        // sprintf(val[i], "%s",jsonDoc[item[i]]);
        // printf("%s : %s\n", item[i],val[i]);
        // printf("%s : %s\n", item[i], jsonDoc[item[i]]);
        // const char* sensor = obj[item[i]].as<String>();
        // sprintf(val[i], "%s",sensor);
        sprintf(parameterValue[i].valStr, "%s", obj[parameterValue[i].item].as<String>());
        // printf("%s : %s\n", item[i], val[i]);
    }
    // printf("parseRcvJson Over\n");
    if (!jsonDoc.isNull())
    {
        // printf("parseRcvJson jsonDoc.clear\n");
        jsonDoc.clear();
    }
}

// int compriseSendJson(char *sendbuf, const char *item[], char val[33][16], int size)
int compriseSendJson(char *sendbuf)
{
    DynamicJsonDocument doc(1024);
    // JsonObject obj = doc.as<JsonObject>();
    for (int i = 0; i < ITEMSUM; i++)
    {
        // obj[String(item[i])] = val[i];
        doc[String(parameterValue[i].item)] = parameterValue[i].valStr;
        // printf("%s : %s\n", item[i], val[i]);
    }

    serializeJson(doc, Serial);
    // String output;
    // convertFromJson(doc, output);
    // printf("output:%s\n", output);

    // serializeJsonPretty(doc, Serial);
    // sprintf(sendbuf, "%s", output);
    // printf("sendbufSize:%d\n", strlen(sendbuf));
    // convertFromJson(doc, output);
    // printf("sendbuf:%s\n", output);
    // String output;
    // serializeJson(doc, output);
    if (!doc.isNull())
    {
        // printf("compriseSendJson doc.clear\n");
        doc.clear();
    }
    return strlen(sendbuf);
}
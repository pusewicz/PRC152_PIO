
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>
#include <string.h>
#include "bsp_json.h"
#include "param_marshal.h"

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
        // Bounded: JSON values arrive from the KDU UART / WiFi / devconsole and
        // can be arbitrarily long; valStr is 16 bytes. (.c_str() also avoids
        // passing a String object through varargs.)
        snprintf(parameterValue[i].valStr, sizeof(parameterValue[i].valStr),
                 "%s", obj[parameterValue[i].item].as<String>().c_str());
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
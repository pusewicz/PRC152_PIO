#ifndef __BSP_WIFI_H__
#define __BSP_WIFI_H__

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_wifi.h> //用于esp_wifi_restore() 删除保存的wifi信息
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <Update.h>

#define WIFI_PROGRAM    0
#define WIFI_RT_PROGRAM 1



void parseAudioSet(int audioSet);
void parsePttTone(int tone);
void parseNowMode(int nowMode);
void ConfigureToUpdate(void);

void initSoftAP(void);
void initDNS(void);
void initWebServer_PGM(void);
void initWebServer_RCU(void);

void stopWIFIServer(void);
void handleWIFIServer(int mode);
int inputString(int l, int p, char *dst, int maxLimit, int minLimit);
int modifyWiFiInfo(int mode);

extern const char *html_PGM;
extern const char *html_RCU;


#endif

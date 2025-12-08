#include "bsp_wifi.h"
#include "main.h"


char host[WIFI_SHOW_SIZE] = "FCS_Configure";
char password[WIFI_SHOW_SIZE] = "123456789";
int x = 152;
const byte DNS_PORT = 53;       // DNS port number
IPAddress apIP(192, 168, x, 1); // ESP32-AP-IP address
DNSServer dnsServer;            // Create dnsServer instance
WebServer server(80);           // Create WebServer

extern double STEP_LEVEL[];
extern u8
    STEP,
    SQL,
    VOLUME;

// Display WiFi domain name and IP
void LCD_ShowAddressIP(void)
{
    LCD_ShowString0608(0, 1, "IP:", 1, 128);
    char IP_SHOW[128] = {0};
    uint32_t ip_tmp = WiFi.softAPIP();
    sprintf(IP_SHOW, "%d.%d.%d.%d",
            (ip_tmp & 0xff),
            (ip_tmp >> 8) & 0xff,
            (ip_tmp >> 16) & 0xff,
            (ip_tmp >> 24) & 0xff);
    LCD_ShowString0608(18, 1, IP_SHOW, 1, 128);
}

// Homepage access callback function
void handleRoot()
{
    server.sendHeader("Connection", "close");
    server.send_P(200, "text/html", html_PGM);
}
// Write frequency finish redirect page
void myHandleFinish()
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "Writing Finish. Please reboot your device!");
    Serial.println("this is handle all finish");
    Serial.println("WriteFinish.........\n\n\n\n\n\n");
    delay(2000);
    // ESP.restart();
    // ResetSystem();
    SHUT();
}
// Upgrade finish redirect page
void myHandleUpdateFinish()
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "Upgrade FAIL" : "Upgrade Finish. Please reboot your device!");
    Serial.println("UpdateFinish.........");
    delay(2000);
    // ResetSystem();
    // ESP.restart(); // Restart ESP32
    SHUT();
}
// Upgrade handler
void myHandleUpdate()
{
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin())
        { // start with max available size
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        Serial.printf("writed:%d/%d\n", upload.currentSize, upload.totalSize);
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        }
        else
        {
            Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
    }
    else
    {
        Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
    }
}

// Handle frontend get channel parameter, save current channel
void myHandleGetChan()
{
    StaticJsonDocument<512> jsonDoc;
    String chan_str = server.arg("chan");

    chan_arv[TMP].CHAN = chan_str.toInt();
    save_CurrentChannel(chan_arv[TMP].CHAN);
    load_ChannelParameter(chan_arv[TMP].CHAN, &chan_arv[TMP]);
    jsonDoc["chan"] = chan_arv[TMP].CHAN;
    jsonDoc["rx_freq"] = chan_arv[TMP].RX_FREQ;
    jsonDoc["tx_freq"] = chan_arv[TMP].TX_FREQ;
    jsonDoc["rs"] = chan_arv[TMP].RS; // 32;
    jsonDoc["ts"] = chan_arv[TMP].TS; // 23;
    jsonDoc["power"] = chan_arv[TMP].POWER;
    jsonDoc["gbw"] = chan_arv[TMP].GBW; // 1;
    jsonDoc["chan_nn"] = (const char *)chan_arv[TMP].NN;

    String jsonStr;
    convertFromJson(jsonDoc, jsonStr);
    // Serial.println(jsonStr);
    server.send(200, "text/plain", jsonStr);
}
// Handle frontend get all parameters
void myHandleGetAll()
{
    StaticJsonDocument<512> jsonDoc;

    chan_arv[TMP].CHAN = load_CurrentChannel();
    load_ChannelParameter(chan_arv[TMP].CHAN, &chan_arv[TMP]);

    jsonDoc["chan"] = chan_arv[TMP].CHAN;
    jsonDoc["chanA"] = load_ChanA(); // 18; //
    jsonDoc["chanB"] = load_ChanB(); // 26; //

    jsonDoc["rx_freq"] = chan_arv[TMP].RX_FREQ;
    jsonDoc["tx_freq"] = chan_arv[TMP].TX_FREQ;

    jsonDoc["rs"] = chan_arv[TMP].RS; // 32; //
    jsonDoc["ts"] = chan_arv[TMP].TS; // 23; //

    jsonDoc["power"] = chan_arv[TMP].POWER; // 0;
    jsonDoc["gbw"] = chan_arv[TMP].GBW;     // 1;

    jsonDoc["chan_nn"] = (const char *)chan_arv[TMP].NN;

    jsonDoc["cf"] = get_Flag(FLAG_CF_SWITCH_ADDR);
    jsonDoc["vu"] = get_Flag(FLAG_VU_SWITCH_ADDR);

    jsonDoc["aud"] = load_AudioSelect();
    jsonDoc["mic"] = load_MicLevel();
    jsonDoc["sql"] = load_Sql();
    jsonDoc["step"] = load_Step();
    jsonDoc["tot"] = load_Tot();
    jsonDoc["lampTime"] = load_LampTime();
    jsonDoc["topPowerOut"] = load_VDO();
    jsonDoc["preTone"] = load_PreTone();
    jsonDoc["endTone"] = load_EndTone();

    serializeJsonPretty(jsonDoc, Serial);
    String jsonStr;
    convertFromJson(jsonDoc, jsonStr);

    Serial.println("Result:");
    Serial.println(jsonStr);
    server.send(200, "text/plain", jsonStr);
}
// Handle frontend set parameters
void myHandleSet()
{
    // String chan_str = server.arg("chan");
    Serial.println(server.argName(0));
    String par = server.argName(0);

    if (par.equals("rx_freq"))
    {
        Serial.println(server.arg(par).toFloat());
        chan_arv[TMP].RX_FREQ = server.arg(par).toFloat();
        // Can wait for tx return to save together, since modifying rx will always modify tx, but not vice versa
        // save_ChannelParameter(chan_arv[TMP].CHAN, chan_arv[TMP]);
    }
    else if (par.equals("tx_freq"))
    {
        Serial.println(server.arg(par).toFloat());
        chan_arv[TMP].TX_FREQ = server.arg(par).toFloat();
        save_ChannelParameter(chan_arv[TMP].CHAN, chan_arv[TMP]);
    }
    else if (par.equals("rs"))
    {
        Serial.println(server.arg(par).toInt());
        chan_arv[TMP].RS = server.arg(par).toInt();
        save_ChannelParameter(chan_arv[TMP].CHAN, chan_arv[TMP]);
    }
    else if (par.equals("ts"))
    {
        Serial.println(server.arg(par).toInt());
        chan_arv[TMP].TS = server.arg(par).toInt();
        save_ChannelParameter(chan_arv[TMP].CHAN, chan_arv[TMP]);
    }
    else if (par.equals("power"))
    {
        Serial.println(server.arg(par).toInt());
        chan_arv[TMP].POWER = server.arg(par).toInt();
        save_ChannelParameter(chan_arv[TMP].CHAN, chan_arv[TMP]);
    }
    else if (par.equals("gbw"))
    {
        Serial.println(server.arg(par).toInt());
        chan_arv[TMP].GBW = server.arg(par).toInt();
        save_ChannelParameter(chan_arv[TMP].CHAN, chan_arv[TMP]);
    }
    else if (par.equals("chan_nn"))
    {
        Serial.println(server.arg(par).c_str());
        // chan_arv[TMP].NN = (volatile char*)(server.arg(par).c_str());
        strcpy((char *)chan_arv[TMP].NN, server.arg(par).c_str());
        save_ChannelParameter(chan_arv[TMP].CHAN, chan_arv[TMP]);
    }
    // /////////////////////////////////////////////////////////////////////////////////////
    else if (par.equals("nowMode"))
    {
        parseNowMode(server.arg(par).toInt());
    }
    else if (par.equals("audioSet"))
    {
        parseAudioSet(server.arg(par).toInt());
    }
    else if (par.equals("sql"))
    {
        save_Sql(server.arg(par).toInt());
    }
    else if (par.equals("step"))
    {
        save_Step(server.arg(par).toInt());
    }
    else if (par.equals("tot"))
    {
        save_Tot(server.arg(par).toInt());
    }
    else if (par.equals("lampTime"))
    {
        save_LampTime(server.arg(par).toInt());
    }
    else if (par.equals("topPowerOut"))
    {
        save_VDO(server.arg(par).toInt());
    }
    else if (par.equals("pttTone"))
    {
        parsePttTone(server.arg(par).toInt());
    }

    else if (par.equals("chanA"))
    {
        save_ChanA(server.arg(par).toInt());
    }
    else if (par.equals("chanB"))
    {
        Serial.printf("chanB:%d\n", server.arg(par).toInt());
        save_ChanB(server.arg(par).toInt());
    }

    else
    {
        server.send_P(200, "text/plain", "UnKnow Parameter!");
        return;
    }
    server.send_P(200, "text/plain", "OK");
}

// Parse frontend return value and store in NVS
void parseAudioSet(int audioSet)
{
    int aud = 0;
    int mic = 0;
    switch (audioSet)
    {
    case 0:
        aud = 0;
        mic = 1;
        break;

    case 1:
    case 2:
    case 3:
        aud = 1;
        mic = audioSet - 1;
        break;
    case 4:
    case 5:
    case 6:
        aud = 2;
        mic = audioSet - 4;
        break;
    default:
        break;
    }
    save_MicLevel(mic);
    save_AudioSelect(aud);
}
void parsePttTone(int tone)
{
    save_PreTone((3 - tone) / 2);
    save_EndTone((3 - tone) % 2);
}
void parseNowMode(int nowMode)
{
    set_Flag(FLAG_CF_SWITCH_ADDR, nowMode > 0);
    if (nowMode > 0)
        set_Flag(FLAG_VU_SWITCH_ADDR, nowMode - 1);
}

// Initialize AP mode
void initSoftAP(void)
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    // Serial.println(WiFi.softAPIP());
    if (WiFi.softAP((const char *)host, (const char *)password))
    {
        Serial.println("ESP32 SoftAP is right\n");
    }
}
// Initialize DNS server
void initDNS(void)
{
    if (dnsServer.start(DNS_PORT, "*", apIP))
    { // Check if mapping all addresses to ESP8266 IP succeeded
        Serial.println("start dnsserver success.");
    }
    else
        Serial.println("start dnsserver failed.");
}
// Initialize WebServer
void initWebServer_PGM(void)
{
    // MDNS.begin(host);

    // Register links and callback functions
    // server.on("/",handleRoot);
    // The above line must be written in the format below for captive portal to work
    server.on("/", HTTP_GET, handleRoot); // Set homepage callback function
    server.onNotFound(handleRoot);        // Set callback for unhandled HTTP requests

    server.on("/finish", HTTP_POST, myHandleFinish); // Set POST request callback function
    server.on("/update", HTTP_POST, myHandleUpdateFinish, myHandleUpdate);

    server.on("/set", HTTP_POST, myHandleSet);
    server.on("/getAll", HTTP_GET, myHandleGetAll);
    server.on("/getChan", HTTP_GET, myHandleGetChan);

    server.begin(); // Start WebServer
    Serial.println("WebServer started!");
    // MDNS.addService("http", "tcp", 80);
    // Serial.printf("Ready! Open http://%s.local in your browser\n", host);
}
// Stop WiFi
void stopWIFIServer(void)
{
    dnsServer.stop();
    server.stop();
    Serial.println("****************stopWIFIServer******************");
    // Serial.printf("WIFI softAPdisconnect:%d\n\n", WiFi.softAPdisconnect());// Always online, can reopen WiFi
    WiFi.mode(WIFI_OFF);
}
extern int NEED_RESTART;
void ConfigureToUpdate(void)
{
    if (NEED_RESTART == ON)
    {
        LCD_Clear(GLOBAL32);
        LCD_ShowString0608(0, 2, "Please Restart System", 1, 128);
        delay_ms(1500);
        return;
    }
    handleWIFIServer(WIFI_PROGRAM);
}

void handleRoot_RCU()
{
    server.sendHeader("Connection", "close");
    server.send_P(200, "text/html", html_RCU);
}
void myHandleGetNow()
{
    StaticJsonDocument<512> jsonDoc;
    String jsonStr;
    jsonDoc["cf"] = get_Flag(FLAG_CF_SWITCH_ADDR);
    jsonDoc["vu"] = get_Flag(FLAG_VU_SWITCH_ADDR);

    jsonDoc["chan"] = chan_arv[NOW].CHAN;
    jsonDoc["rx_freq"] = chan_arv[NOW].RX_FREQ;
    jsonDoc["tx_freq"] = chan_arv[NOW].TX_FREQ;
    jsonDoc["chan_nn"] = (const char *)chan_arv[NOW].NN;

    jsonDoc["rs"] = menu_subvoice[chan_arv[NOW].RS]; // 32; //
    jsonDoc["ts"] = menu_subvoice[chan_arv[NOW].TS]; // 23; //

    jsonDoc["volume"] = VOLUME; // 0-7; //

    convertFromJson(jsonDoc, jsonStr);
    // Serial.printf("jsonStr:%s\n", jsonStr);
    // serializeJsonPretty(jsonDoc, Serial);
    server.send(200, "text/plain", jsonStr);
}
void myHandlePreP()
{
    if (get_Flag(FLAG_CF_SWITCH_ADDR)) // Frequency mode
    {
        if (get_Flag(FLAG_VU_SWITCH_ADDR)) // U
        {
            if (chan_arv[NOW].RX_FREQ + STEP_LEVEL[STEP] <= 480.0)
                chan_arv[NOW].RX_FREQ += STEP_LEVEL[STEP];
            else
                chan_arv[NOW].RX_FREQ = 400.0;
        }
        else
        {
            if (chan_arv[NOW].RX_FREQ + STEP_LEVEL[STEP] <= 174.0)
                chan_arv[NOW].RX_FREQ += STEP_LEVEL[STEP];
            else
                chan_arv[NOW].RX_FREQ = 136.0;
        }
        chan_arv[NOW].RX_FREQ = checkFreqFloat(chan_arv[NOW].RX_FREQ);
        chan_arv[NOW].TX_FREQ = chan_arv[NOW].RX_FREQ;
        save_ChannelParameter(chan_arv[NOW].CHAN, chan_arv[NOW]);
    }
    else // Channel mode
    {
        if (++chan_arv[NOW].CHAN > 99)
            chan_arv[NOW].CHAN = 1;
        save_CurrentChannel(chan_arv[NOW].CHAN);
        load_ChannelParameter(chan_arv[NOW].CHAN, &chan_arv[NOW]);
    }
    Set_A20(chan_arv[NOW], SQL);
    myHandleGetNow();
}
void myHandlePreN()
{
    if (get_Flag(FLAG_CF_SWITCH_ADDR)) // VFO模式
    {
        if (get_Flag(FLAG_VU_SWITCH_ADDR)) // U
        {
            if (chan_arv[NOW].RX_FREQ - STEP_LEVEL[STEP] >= 400.0)
                chan_arv[NOW].RX_FREQ -= STEP_LEVEL[STEP];
            else
                chan_arv[NOW].RX_FREQ = 480.0;
        }
        else // V
        {
            if (chan_arv[NOW].RX_FREQ - STEP_LEVEL[STEP] >= 136.0)
                chan_arv[NOW].RX_FREQ -= STEP_LEVEL[STEP];
            else
                chan_arv[NOW].RX_FREQ = 174.0;
        }
        chan_arv[NOW].RX_FREQ = checkFreqFloat(chan_arv[NOW].RX_FREQ);
        chan_arv[NOW].TX_FREQ = chan_arv[NOW].RX_FREQ;
        save_ChannelParameter(chan_arv[NOW].CHAN, chan_arv[NOW]);
    }
    else // Channel mode
    {
        if (--chan_arv[NOW].CHAN < 1)
            chan_arv[NOW].CHAN = 99;
        save_CurrentChannel(chan_arv[NOW].CHAN);
        load_ChannelParameter(chan_arv[NOW].CHAN, &chan_arv[NOW]);
    }
    Set_A20(chan_arv[NOW], SQL);
    myHandleGetNow();
}
void myHandleVolP()
{
    VOL_Reflash(1);
    myHandleGetNow();
}
void myHandleVolN()
{
    VOL_Reflash(2);
    myHandleGetNow();
}
// nowmode:
//   0:channel
//   1:VHF
//   2:UHF
void myHandleSwitch()
{
    // nowMode:0/1/2
    String par = server.argName(0);
    parseNowMode(server.arg(par).toInt());

Serial.printf("myHandleSwitch:%d\n", server.arg(par).toInt());
    // Reload data
    if (get_Flag(FLAG_CF_SWITCH_ADDR)) // Frequency mode
    {
        if (get_Flag(FLAG_VU_SWITCH_ADDR))
            chan_arv[NOW].CHAN = 100;
        else
            chan_arv[NOW].CHAN = 0;
    }
    else
        chan_arv[NOW].CHAN = load_CurrentChannel();

    load_ChannelParameter(chan_arv[NOW].CHAN, &chan_arv[NOW]);
    Set_A20(chan_arv[NOW], SQL);
    myHandleGetNow(); // Send data back to client
}

void initWebServer_RCU(void)
{
    // Set homepage callback function
    server.on("/", HTTP_GET, handleRoot_RCU);
    server.onNotFound(handleRoot_RCU); // Set callback for unhandled HTTP requests

    server.on("/getNow", HTTP_POST, myHandleGetNow);

    server.on("/PRE_P", HTTP_POST, myHandlePreP);
    server.on("/PRE_N", HTTP_POST, myHandlePreN);
    server.on("/SWITCH", HTTP_POST, myHandleSwitch);
    server.on("/VOL_P", HTTP_POST, myHandleVolP);
    server.on("/VOL_N", HTTP_POST, myHandleVolN);

    server.begin(); // Start WebServer

    Serial.println("RCU started!");
}

void handleWIFIServer(int mode)
{
    LCD_Clear(GLOBAL32);
    LCD_ShowString0608(52, 2, "WIFI", 1, 128);

    initSoftAP();
    if (mode == WIFI_PROGRAM)
    {
        LCD_ShowString0608(43, 3, "PROGRAM", 1, 128);
        initWebServer_PGM();
    }
    else if (mode == WIFI_RT_PROGRAM)
    {
        LCD_ShowString0608(34, 3, "RT PROGRAM", 1, 128);
        initWebServer_RCU();
    }
    LCD_ShowAddressIP();

    initDNS();
    while (1)
    {
        if (mode == WIFI_RT_PROGRAM)
        {
            PTT_Control();
            SQUELCH_Contol();
            SQ_Read_Control();
            if (bsp_CheckTimer(TMR_FM_CTRL) && WFM)
                RDA5807_Init(ON);
        }

        switch (Encoder_Switch_Scan(0))
        {
        case key_double:
            stopWIFIServer();
            return;

        case key_long:
            D_printf("<<<<<<SHUTING>>>>>>\n");
            stopWIFIServer();
            SHUT();
            break;
        }
        // if (Matrix_KEY_Scan(0) == MATRIX_RESULT_CLR)
        // {
        //     stopWIFIServer();
        //     return;
        // }

        server.handleClient();
        delay(2); // allow the cpu to switch to other tasks
    }
}

extern volatile u8 KDU_INSERT;
//
int inputString(int l, int p, char *dst, int maxLimit, int minLimit)
{
    TIMES = 0;
    unsigned char
        src[maxLimit] = {32},
        key_old = MATRIX_RESULT_ERROR,
        result_matrix = MATRIX_RESULT_ERROR, // Current triggered key
        locate = 0,                          // Current cursor position
        flag_modifyLocate_change = 0,        // Modified position changed, used to refresh press_times
        flag_selectLocate_change = 1,        // Selected position changed, used to refresh cursor display
        flag_selectBit = 0,                  // Encoder use: current mode is position adjust/set character
        press_times = 0,                     // Current key press count
        clear = 0;                           // Edit bar clear flag, 0=not cleared, 1=cleared

    memset(src, 0, maxLimit);
    sprintf((char *)src, "%s", dst);

    if (maxLimit == 7) // This is for setting channel nickname
    {
        LCD_ShowString0608(0, 2, "CN:", 1, 18);
        LCD_ShowPIC0608(60, 2, 0, 1);
    }
    else
    {
        if (WFM)
        {
            WFM = OFF;
            RDA5807_Init(WFM);
        }
    }

    while (1)
    {
        if (maxLimit == 7)
        {
            MY_GLOBAL_FUN();
            if (KDU_INSERT)
                return BACK2MAIN;
        }
        else
            FeedDog();

        switch (Encoder_Switch_Scan(0))
        {
        case key_click: // Encoder confirm: always confirms character setting at current position
            flag_selectBit = !flag_selectBit;
            break;

        case key_double:
            if (minLimit)
            {
                if (strlen((const char *)src) < minLimit)
                {
                    LCD_ShowString0608(0, 3, "INPUT LESS THAN 8 BITS!", 0, 128);
                    delay(1500);
                    LCD_ShowString0608(0, 3, "                       ", 1, 128);
                    continue;
                }
            }
            sprintf((char *)dst, "%s", src);
            D_printf("%s\n", src);
            return ENT2LAST;

        case key_long:
            SHUT();
            break;
        }

        if (flag_selectBit == 0) // Encoder switched to position adjustment
        {
            if (TIMES != 0)
            {
                // locate = (locate + maxLimit - ((-TIMES) % maxLimit)) % maxLimit;

                locate = (locate + TIMES + maxLimit) % maxLimit;
                TIMES = 0;
                flag_selectLocate_change = 1;
            }
        }
        else // Encoder set nickname
        {
            if (TIMES > 0)
            {
                src[locate] = (src[locate] - 32 + TIMES) % 95 + 32;
                TIMES = 0;
                clear = 0;
                LCD_ShowAscii0608(l + locate * 6, p, src[locate], 0);
            }
            else if (TIMES < 0)
            {
                src[locate] = src[locate] + 95 - (-TIMES) % 95 > 127 ? src[locate] - (-TIMES) % 95 : src[locate] + 95 - (-TIMES) % 95;
                
                TIMES = 0;
                clear = 0;
                LCD_ShowAscii0608(l + locate * 6, p, src[locate], 0);
            }
        }

        result_matrix = Matrix_KEY_Scan(0);
        switch (result_matrix)
        {
        case MATRIX_RESULT_CLR:
            if (clear) // Already cleared, return to initial value
                return NO_OPERATE;
            else // Not cleared, clear edit bar
            {
                memset(src, 0, sizeof(src));
                flag_selectLocate_change = 1;
                locate = 0;
                clear = 1;
            }
            break;

        case MATRIX_RESULT_ENT:
            if (minLimit)
            {
                Serial.printf("src lenth: %d\n", strlen((const char *)src));
                if (strlen((const char *)src) < minLimit)
                {
                    LCD_ShowString0608(0, 3, "INPUT LESS THAN 8 BITS!", 0, 128);
                    delay(1500);
                    LCD_ShowString0608(0, 3, "                       ", 1, 128);
                    break;
                }
            }
            sprintf((char *)dst, "%s", src);
            D_printf("%s\n", src);
            return ENT2LAST;

        case MATRIX_RESULT_0:
        case MATRIX_RESULT_1:
        case MATRIX_RESULT_2:
        case MATRIX_RESULT_3:
        case MATRIX_RESULT_4:
        case MATRIX_RESULT_5:
        case MATRIX_RESULT_6:
        case MATRIX_RESULT_7:
        case MATRIX_RESULT_8:
        case MATRIX_RESULT_9:
            clear = 0;
            press_times++;
            press_times %= 9;
            if (key_old != result_matrix || flag_modifyLocate_change)
            {
                flag_modifyLocate_change = 0;
                key_old = result_matrix;
                press_times = 0;
            }
            src[locate] = square_9[result_matrix][press_times];
            LCD_ShowAscii0608(l + locate * 6, 2, src[locate], 0);
            break;

        case MATRIX_RESULT_LEFT:
            locate--;
            flag_selectLocate_change = 1;
            flag_selectBit = 0; // After key position switch, encoder rotation changes to position switch
            if (locate > maxLimit - 1)
                locate = maxLimit - 1;
            break;

        case MATRIX_RESULT_RIGHT:
            locate++;
            flag_selectLocate_change = 1;
            flag_selectBit = 0; // After key position switch, encoder rotation changes to position switch
            if (locate > maxLimit - 1)
                locate = 0;
            break;
        };

         // Cursor position modification
        if (flag_selectLocate_change)
        {
            flag_selectLocate_change = 0;
            flag_modifyLocate_change = 1;
            // LCD_ShowString0608(l, p, "       ", 1, 128);
            // LCD_ShowString0608(l, p, (char *)src, 1, 128);
            // printf("limit:%d\n", l+maxLimit*6);
            LCD_ShowString0608(l, p, "                       ", 1, l + maxLimit * 6);
            LCD_ShowString0608(l, p, (char *)src, 1, l + maxLimit * 6);

            if (src[locate])
                LCD_ShowAscii0608(l + locate * 6, p, src[locate], 0); // Display current selection
                
            else
                LCD_ShowAscii0608(l + locate * 6, p, ' ', 0);
        }
    }
}

int modifyWiFiInfo(int mode)
{
    const char *op[2] = {"ENTER ",
                         "MODIFY"};
    int matrix_result = 0;
    int selectPos = 0;
    int ENSURE = 0;
    int ret = NO_OPERATE;
    char mssid[WIFI_SHOW_SIZE] = {0};
    char mpassword[WIFI_SHOW_SIZE] = {0};
    bool falg_needSave = false;
    LCD_Clear(GLOBAL32);

    memset(mssid, 0, WIFI_SHOW_SIZE);
    memset(mpassword, 0, WIFI_SHOW_SIZE);
    load_WIFIInfo(mssid, mpassword);

    Serial.println("load_WIFIInfo\n");
    Serial.printf("mssid:%s\n", mssid);
    Serial.printf("mpassword:%s\n", mpassword);

    // LCD_ShowString0608(0, 0, "WIFI RCU", 1, 128);
    LCD_ShowString0408(0, 0, "WIFI RCU", 1);
    LCD_ShowMenu31(op, 2, selectPos);
    bsp_StartAutoTimer(TMR_OUT_CTRL, TMR_PERIOD_8S);
    while (1)
    {
        FeedDog();                        // Feed watchdog
        if (bsp_CheckTimer(TMR_OUT_CTRL)) // 8 seconds no operation auto return
        {
            bsp_StopTimer(TMR_OUT_CTRL);
            return BACK2MAIN;
        }
        matrix_result = Matrix_KEY_Scan(0);
        if (matrix_result != MATRIX_RESULT_ERROR)
            reloadTimer(TMR_OUT_CTRL); // Key press resets timer
        switch (matrix_result)
        {
        case MATRIX_RESULT_8:
        case MATRIX_RESULT_CLR:
            bsp_StopTimer(TMR_OUT_CTRL);
            return BACK2MAIN;

        case MATRIX_RESULT_ENT:
            ENSURE = 1;
            break;

        case MATRIX_RESULT_N:
        case MATRIX_RESULT_RIGHT:
        case MATRIX_RESULT_P:
        case MATRIX_RESULT_LEFT:
            TIMES++;
            break;
        default:
            break;
        }
        switch (Encoder_Switch_Scan(0))
        {
        case key_click:
            reloadTimer(TMR_OUT_CTRL);
            ENSURE = 1;
            break;

        case key_double:
            bsp_StopTimer(TMR_OUT_CTRL);
            return BACK2MAIN;

        case key_long:
            D_printf("<<<<<<SHUTING>>>>>>\n");
            SHUT();
            break;
        }

        if (TIMES != 0)
        {
            reloadTimer(TMR_OUT_CTRL); // Key press resets timer
            selectPos = (selectPos + 1) % 2;
            // selectPos = (selectPos - 1 + 2) % 2;
            TIMES = 0;
            LCD_ShowMenu31(op, 2, selectPos);
        }
        if (ENSURE)
        {
            ENSURE = 0;
            switch (selectPos)
            {
            case 0: // Directly enter WiFi RCU/PGM
                // Serial.printf("mssid:%s, mpassword:%s\n", mssid, mpassword);
                break;

            case 1: // Modify WiFi
                while (1)
                {
                    LCD_Clear(EDITZONE32);
                    LCD_ShowString0608(0, 1, "INPUT SSID:    ", 1, 128);
                    ret = inputString(12, 2, mssid, WIFI_SHOW_SIZE, 0);
                    Serial.printf("mssid:%s\n", mssid);
                    if (ret == BACK2MAIN)
                        return BACK2MAIN;
                    else // Regardless of operation, proceed
                    {
                        if (ret == ENT2LAST)
                            falg_needSave = true;
                        LCD_Clear(EDITZONE32);
                        LCD_ShowString0608(0, 1, "INPUT PASSWORD:", 1, 128);
                        ret = inputString(12, 2, mpassword, WIFI_SHOW_SIZE, 8);
                        // Serial.printf("mpassword:%s\n", mpassword);
                        if (ret == NO_OPERATE)
                            continue; // Return to modify ssid
                        else if (ret == BACK2MAIN)
                            return BACK2MAIN;
                        else
                        {
                            falg_needSave = true;
                            break;
                        }
                    }
                }
                break;
            }
            if (strlen(mssid) == 0)
            {
                falg_needSave = true;
                strcpy(mssid, "FCS_Configure");
                // Serial.printf("ssid is empty, correction:%s\n", mssid);
            }
            if (strlen(mpassword) == 0)
            {
                falg_needSave = true;
                strcpy(mpassword, "123456789");
                // Serial.printf("mpassword is empty, correction:%s\n", mpassword);
            }

            // strcpy(host, mssid);
            // strcpy(password, mpassword);
            sprintf(host, "%s\0", mssid);
            sprintf(password, "%s\0", mpassword);
            Serial.printf("host:%s\n", host);
            Serial.printf("password:%s\n", password);

            if (falg_needSave)
                save_WIFIInfo(host, password);
            return ENT2LAST;
        }
    }
}

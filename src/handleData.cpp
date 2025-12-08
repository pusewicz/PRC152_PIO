#include "main.h"
#include "bsp_json.h"

extern u8 RSSI, SC,                         // SCREEN CONTRAST  CHAN = 0,
    STEP, SQL, AUD, MIC, ENC, TOT, BL, VDO, // VDO: output power
    VOLUME, PRE_TONE, END_TONE, ITTS,       // ITTS:Idle Time to Sleep
    MIC_LEVEL[3], WFM_LEVEL[8], A20_LEVEL[8];
extern volatile char Home_Mode;
extern volatile u8 KDU_INSERT;
extern char sele_pos, rcv_chan;
extern int FM_FREQ;
extern u8 FM_CHAN;

extern ParameterValue_t parameterValue[ITEMSUM];

/// @brief  Assign data to be sent to the parameterValue array storing corresponding variables
/// @param  null
void writeOtherValue2buf()
{
    sprintf(parameterValue[Jcf      ].valStr, "%d", get_Flag(FLAG_CF_SWITCH_ADDR));
    sprintf(parameterValue[Juv      ].valStr, "%d", get_Flag(FLAG_VU_SWITCH_ADDR));
    sprintf(parameterValue[JchanA   ].valStr, "%03d", chan_arv[CHANA].CHAN);
    sprintf(parameterValue[JchanB   ].valStr, "%03d", chan_arv[CHANB].CHAN);
    sprintf(parameterValue[Jvolume  ].valStr, "%d", VOLUME);
    sprintf(parameterValue[Jstep    ].valStr, "%d", STEP);
    sprintf(parameterValue[Jsql     ].valStr, "%d", SQL);
    sprintf(parameterValue[Jmic     ].valStr, "%d", MIC);
    sprintf(parameterValue[Jaudio   ].valStr, "%d", AUD);
    sprintf(parameterValue[Jtot     ].valStr, "%d", TOT);
    sprintf(parameterValue[Jvdo     ].valStr, "%d", VDO);
    sprintf(parameterValue[JpreTone ].valStr, "%d", PRE_TONE);
    sprintf(parameterValue[JendTone ].valStr, "%d", END_TONE);
    sprintf(parameterValue[Jwfm     ].valStr, "%d", WFM);
    sprintf(parameterValue[JfmFreq  ].valStr, "%03d", FM_FREQ);
    sprintf(parameterValue[Jhomemode].valStr, "%d", Home_Mode);

    if (PTT_READ == 0)
        RSSI = 100;
    else
    {
        if (A002_SQ_READ)
            RSSI = 0;
        else
            RSSI = Get_A20_RSSI();
    }
    sprintf(parameterValue[Jrssi    ].valStr, "%03d", RSSI);
    sprintf(parameterValue[JfmChan  ].valStr, "%d", FM_CHAN);
    sprintf(parameterValue[Jvoltage ].valStr, "%d", Get_Battery_Vol());
    sprintf(parameterValue[JrcvSQ   ].valStr, "%03d", A002_SQ_READ);
    sprintf(parameterValue[JpressPTT].valStr, "%d", PTT_READ);
    sprintf(parameterValue[JpressSQU].valStr, "%d", SQUELCH_READ);
    sprintf(parameterValue[JrcvChan ].valStr, "%03d", rcv_chan);
    sprintf(parameterValue[JselPos  ].valStr, "%03d", sele_pos);
}

/// @brief  Read data from array and assign to channel
/// @param  Channel to be assigned
void readChanFromArray(CHAN_ARV_P B)
{
    B->CHAN     = atoi(parameterValue[Jcurrent  ].valStr);
    B->RX_FREQ  = atof(parameterValue[Jrx_freq  ].valStr);
    B->TX_FREQ  = atof(parameterValue[Jtx_freq  ].valStr);
    B->RS       = atoi(parameterValue[Jrs       ].valStr);
    B->TS       = atoi(parameterValue[Jts       ].valStr);
    B->POWER    = atoi(parameterValue[Jpower    ].valStr);
    B->GBW      = atoi(parameterValue[Jbandwith ].valStr);
    sprintf((char *)B->NN, "%s", parameterValue[Jnickname].valStr);

    //		printf("*****chan:%d\r  rx:%.5lf\r tx:%.5lf\r rs:%d\r ts: %d\r power:%d\r gbw:%d\r nn:%s\n",
    //			    B->CHAN, B->RX_FREQ, B->TX_FREQ, B->RS, B->TS, B->POWER, B->GBW, B->NN);

    //	for(int i = Jcurrent; i<Jnickname+1; i++)
    //		printf("%s\n",parameterValue[i]);
}

/// @brief  Assign channel data to be sent to array
/// @param  Channel to be sent
void writeChanToArray(CHAN_ARV_P B)
{
    sprintf(parameterValue[Jcurrent ].valStr, "%03d", B->CHAN);
    sprintf(parameterValue[Jrx_freq ].valStr, "%3.5f", B->RX_FREQ);
    sprintf(parameterValue[Jtx_freq ].valStr, "%3.5f", B->TX_FREQ);
    sprintf(parameterValue[Jrs      ].valStr, "%03d", B->RS);
    sprintf(parameterValue[Jts      ].valStr, "%03d", B->TS);
    sprintf(parameterValue[Jpower   ].valStr, "%d", B->POWER);
    sprintf(parameterValue[Jbandwith].valStr, "%d", B->GBW);
    sprintf(parameterValue[Jnickname].valStr, "%s", B->NN);
    //		printf("*****chan:%d\r  rx:%.5lf\r tx:%.5lf\r rs:%d\r ts: %d\r power:%d\r gbw:%d\r nn:%s\n",
    //			    B->CHAN, B->RX_FREQ, B->TX_FREQ, B->RS, B->TS, B->POWER, B->GBW, B->NN);
    //	for(int i = Jcurrent; i<Jnickname+1; i++)
    //		printf("%s\n",parameterValue[i]);
}
//

int readWriteValueToKDU(int Cmd)
{
    static int cf = get_Flag(FLAG_CF_SWITCH_ADDR); // Auxiliary for querying channel parameters
    int fmfreq = 0;
    // parseRcvJson((char *)rx1_buf, parItem, parameterValue, ITEMSUM);
    parseRcvJson((char *)rx1_buf);
    sprintf(parameterValue[Jcmd].valStr, "%s", prefix_buf[Cmd-1]);

    switch (Cmd)
    {
    case _ASKALL:
        writeOtherValue2buf();
        writeChanToArray(&chan_arv[NOW]);
        break;

    case _RELA:
        chan_arv[CHANA].CHAN = atoi(parameterValue[Jcurrent].valStr);
        save_ChanA(chan_arv[CHANA].CHAN);
        load_ChannelParameter(chan_arv[CHANA].CHAN, &chan_arv[CHANA]);
        writeOtherValue2buf();
        writeChanToArray(&chan_arv[CHANA]);
        break;

    case _RELB:
        chan_arv[CHANB].CHAN = atoi(parameterValue[Jcurrent].valStr);
        save_ChanB(chan_arv[CHANB].CHAN);
        load_ChannelParameter(chan_arv[CHANB].CHAN, &chan_arv[CHANB]);
        writeOtherValue2buf();
        writeChanToArray(&chan_arv[CHANB]);
        break;

    case _RELOAD:
        chan_arv[NOW].CHAN = atoi(parameterValue[Jcurrent].valStr);
        cf = atoi(parameterValue[Jcf].valStr);
        if (cf == get_Flag(FLAG_CF_SWITCH_ADDR)) // CF mode unchanged
        {
            if (cf) // FREQ mode: VU switch
                set_Flag(FLAG_VU_SWITCH_ADDR, chan_arv[NOW].CHAN ? 1 : 0);
            else // CHAN mode: channel number switch
                save_CurrentChannel(chan_arv[NOW].CHAN);
        }
        else // Channel <=> Frequency
        {
            set_Flag(FLAG_CF_SWITCH_ADDR, cf);
            if (cf)
                chan_arv[NOW].CHAN = get_Flag(FLAG_VU_SWITCH_ADDR) * 100;
            else
                chan_arv[NOW].CHAN = load_CurrentChannel();
        }
        load_ChannelParameter(chan_arv[NOW].CHAN, &chan_arv[NOW]);
        Set_A20(chan_arv[NOW], SQL);
        writeOtherValue2buf();
        writeChanToArray(&chan_arv[NOW]);
        break;

    case _ASKA:
        load_ChannelParameter(load_ChanA(), &chan_arv[TMP]);
        writeChanToArray(&chan_arv[TMP]);
        break;

    case _ASKB:
        load_ChannelParameter(load_ChanB(), &chan_arv[TMP]);
        writeChanToArray(&chan_arv[TMP]);
        break;

    case _ASKCHAN:
        chan_arv[TMP].CHAN = atoi(parameterValue[Jcurrent].valStr);
        load_ChannelParameter(chan_arv[TMP].CHAN, &chan_arv[TMP]);
        writeChanToArray(&chan_arv[TMP]);
        break;

    case _SETHOMEMODE:
        Home_Mode = atoi(parameterValue[Jhomemode].valStr);
        if (Home_Mode == DUAL_MODE)
            bsp_StartAutoTimer(TMR_DUAL_REFRESH, DUAL_SWITCH_TIME); // Start 500ms frequency switch timer
        else
        {
            bsp_StopTimer(TMR_DUAL_REFRESH); // Stop switch timer
            Set_A20(chan_arv[NOW], SQL);
        }
        break;

    case _SETCHAN:
        readChanFromArray(&chan_arv[NOW]);
        // When KDU keys are pressed too fast: if these two are placed before send, KDU response will be slow;
        //                                    if placed after, cannot process subsequent data and fail to respond to KDU causing connection loss bug
        save_ChannelParameter(chan_arv[NOW].CHAN, chan_arv[NOW]);
        Set_A20(chan_arv[NOW], SQL);
        break;
        ////////////////////////////////////////////////////////////////////////////////////////
    case _SETZERO:
        set_Flag(RESETADDR, ~RESET_VAL);
        SHUT();
        break;

    case _SETSTEP:
        STEP = atoi(parameterValue[Jstep].valStr);
        save_Step(STEP);
        break;

    case _SETSQL:
        if (SQL != atoi(parameterValue[Jsql].valStr))
        {
            SQL = atoi(parameterValue[Jsql].valStr);
            save_Sql(SQL);
            Set_A20(chan_arv[NOW], SQL);
        }
        break;

    case _SETAUD:
        if (AUD != atoi(parameterValue[Jaudio].valStr))
        {
            SPK_SWITCH(AUD, OFF);
            AUD = atoi(parameterValue[Jaudio].valStr);

            if (RcvSignal()) // Signal present, modify directly
                SPK_SWITCH(AUD, ON);
            else
            {
                if (WFM)
                    SPK_SWITCH(AUD, ON);
            }
            //
        }
        if (AUD == 0)
            MIC = 1;
        else
            MIC = atoi(parameterValue[Jmic].valStr);

        save_AudioSelect(AUD);
        save_MicLevel(MIC);
        M62364_SetSingleChannel(4, MIC_LEVEL[MIC]);
        break;

    case _SETENC:
        break;

    case _SETTOT:
        TOT = atoi(parameterValue[Jtot].valStr);
        save_Tot(TOT);
        break;

    case _SETVDO:
        if (VDO != atoi(parameterValue[Jvdo].valStr))
        {
            VDO = atoi(parameterValue[Jvdo].valStr);
            save_VDO(VDO);
            VDO_SWITCH(VDO);
        }
        break;

    case _SETVOLU:
        if (VOLUME != atoi(parameterValue[Jvolume].valStr))
        {
            VOLUME = atoi(parameterValue[Jvolume].valStr);
            save_OverVolume(VOLUME);
            // Note: volume change while receiving signal/FM in use
            if (WFM) // When radio is on, don't interrupt output, modify volume directly
            {
                RDA5807_ResumeImmediately();
                if (!A002_SQ_READ)
                {
                    M62364_SetSingleChannel(WFM_LINE_CHAN, WFM_LEVEL[0]);
                    M62364_SetSingleChannel(A20_LINE_CHAN, A20_LEVEL[VOLUME]);
                }
                else
                    M62364_SetSingleChannel(WFM_LINE_CHAN, WFM_LEVEL[VOLUME]);
            }
            else
            {
                if (A002_SQ_READ)
                    SPK_SWITCH(AUD, 0);
                M62364_SetSingleChannel(WFM_LINE_CHAN, WFM_LEVEL[0]); // 0
                M62364_SetSingleChannel(A20_LINE_CHAN, A20_LEVEL[VOLUME]);
            }
        }
        break;

    case _SETTONE:
        PRE_TONE = atoi(parameterValue[JpreTone].valStr);
        END_TONE = atoi(parameterValue[JendTone].valStr);
        save_PreTone(PRE_TONE);
        save_EndTone(END_TONE);
        break;

    case _SETFM:
        fmfreq = atoi(parameterValue[JfmFreq].valStr);
        if (WFM != atoi(parameterValue[Jwfm].valStr)) // FM on/off toggle
        {
            WFM = atoi(parameterValue[Jwfm].valStr);
            RDA5807_Init(WFM);
            if (WFM == OFF && A002_SQ_READ && PTT_READ)
                SPK_SWITCH(AUD, OFF);
        }
        else // Switch frequency and validate
        {
            RDA5807_Set_Freq(fmfreq);
            if (RDA5807_ReadReg(0xb) & 0x0100)
            {
                FM_CHAN = 1;
                save_FMFreq(fmfreq);
            }
            else
                FM_CHAN = 0;
        }
        FM_FREQ = fmfreq; // Needed for polling
        break;

    case _SETDUALPOS:
        sele_pos = atoi(parameterValue[JselPos].valStr);
        break;
    default:
        break;
    }
    char sendbuf[10];
    // compriseSendJson(sendbuf, parItem, parameterValue, ITEMSUM);
    compriseSendJson(sendbuf);
    return NO_OPERATE;
}



/// @brief  Process data command, reply data
/// @return Operation after data processing: NO_OPERATE for normal operation
int PRC152receiveProcess()
{
    static int NoVal2ExitCal = 0; // 3 timeout receives, return to main page (no data received)
    static int errVal2Exit = 0;   // 3 error data, return to main page (received data but data error)
    static int InsertKDUCal = 0;  // 3 normal receives to enter KDU mode

    // 1. During confirmation insert period, receive countdown exceeds 4S, clear confirmation value and return
    // 2. Detected timer exceeds 1.2S, calculate KDU exit value
    if (bsp_CheckTimer(TMR_WAIT_KDU)) 
    {
        if (NoVal2ExitCal++ >= 2 || (InsertKDUCal>0 && InsertKDUCal<3))
        {
            Serial.printf("*****************NoVal2ExitCal:%d, InsertKDUCal:%d*********************\n", 
                                NoVal2ExitCal, InsertKDUCal);
            InsertKDUCal = 0;
            NoVal2ExitCal = 0;
            KDU_INSERT = OFF;
            bsp_StopTimer(TMR_WAIT_KDU);
            return BACK2MAIN;
        }
    }
    if (UART1_getRcvFlag())
    {
        UART1_dataPreProcess();
        int getCmd = _SETDUALPOS + 1;
        for (int i = _ASKALL; i < _SETDUALPOS + 1; i += 2)
        {
            if (strstr((char *)rx1_buf, prefix_buf[i]))
            {
                getCmd = i;
                break;
            }
        }
// Serial.printf("\n*****************getCmd:%d, _SETDUALPOS:%d, KDU_INSERT:%d\n*****************\n",getCmd, _SETDUALPOS, KDU_INSERT);
        if (getCmd < _SETDUALPOS + 1) //读取
        {
            NoVal2ExitCal = 0;
            errVal2Exit = 0;
            readWriteValueToKDU(getCmd);
            if(KDU_INSERT == OFF)// Not yet entered
            {
                // Serial.printf("\n\n[%d]:    NoVal2ExitCal:%d, InsertKDUCal:%d\n", __LINE__, NoVal2ExitCal, InsertKDUCal);
                if(getCmd == _ASKALL || getCmd == _ASKA ||  getCmd == _ASKB)
                {
                    if (++InsertKDUCal >= 3)
                    {
                        InsertKDUCal = 0;
                        KDU_INSERT = ON;
                        // Serial.printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!![%d]:     NoVal2ExitCal:%d, InsertKDUCal:%d\n", __LINE__, NoVal2ExitCal, InsertKDUCal);
                        return BACK2MAIN;
                    }
                    bsp_StartTimer(TMR_WAIT_KDU, WAIT_KDU_INSERT_TIME);
                }
            }
        }
        else
            errVal2Exit++;

        /////////////////////////////////////////////////////////////////////////////////////////////////
        UART1_EnRCV();
        ClearShut();
        if (errVal2Exit > 3) // 3 consecutive data errors, directly exit KDU handler
        {
            Serial.printf("*****************errVal2Exit:%d*********************\n", 
                                 errVal2Exit);
            errVal2Exit = 0;
            KDU_INSERT = OFF;
            bsp_StopTimer(TMR_WAIT_KDU);
            return BACK2MAIN;
        }
        if(KDU_INSERT)
            bsp_StartAutoTimer(TMR_WAIT_KDU, WAIT_KDU_LEAVE_TIME);
    }

    return NO_OPERATE;
}
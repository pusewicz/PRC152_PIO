#ifndef _RDA5807_H_ 
#define _RDA5807_H_ 
//

#define RDA5807_READ       	0x23  // Read RDA5807
#define RDA5807_WRITE		0x22  // Write RDA5807
#define RDA5807_R00			0X00  // Chip ID register 0x58 
				
#define RDA5807_R02			0X02  // DHIZ[15],DMUTE[14] mute,MONO[13] channel,BASS[12] bass boost,SEEKUP[9],SEEK[8],SKMODE[7], 
#define RDA5807_R03			0X03  // CLK_MODE[6:4] clock source select,SOFTRESET[1] soft reset,ENABLE[0] power enable 
#define RDA5807_R04			0X04  // CHAN[15:6],TUNE[4],BAND[3:2],SPACE[1:0] set frequency bandwidth step 
#define RDA5807_R05			0X05  //STCIEN[14],DE[11],I2Senable[6],
															// INT_MODE[15],SEEKTH[14:8] (auto-search signal strength threshold),LNA_PORT_SEL[7:6]=0b10, 
															// LNA_ICSEL_BIT[5:4],VOLUME[3:0] volume; 
				
#define RDA5807_R0A			0X0A  // STC[14] seek complete SF[13] seek fail readchan[9:0] current channel
#define RDA5807_R0B			0X0B  // RSSI[15:9],FM TRUE[8] current channel is a station 

short RDA5807_ReadReg(unsigned  char addr);
void  RDA5807_WriteReg(unsigned char addr,short val);

void RDA_Power(unsigned char off_on);
//


void RDA5807_Set_Freq(short freq);
void RDA5807_Init(char off_on);
void FM_PageShow(void);
void Radio_Freq_Show(int fm_freq, int mode);
int FM_Freq_Set_Show(int x,int y,int* result);
void Enter_Radio(void);

void test_FM(void);
#endif
//

#ifndef __BSP_CH423_H__
#define __BSP_CH423_H__

// CH423 interface definitions
#define     CH423_I2C_ADDR1     0x40         // CH423 address
#define     CH423_I2C_MASK      0x3E         // CH423 high byte command mask

/*  Set system parameters command */ 

#define     CH423_SYS_CMD     0x4800     // Set system parameters command, default mode
#define     BIT_SLEEP         0X40
#define     BIT_INTENS        0X20          //
#define     BIT_OD_EN         0X10          // 0: push-pull output (can output low and high) 1: open-drain output (can only output low or float) default is push-pull
#define     BIT_X_INT         0x08       // Enable input level change interrupt, 0 disables; 1 and DEC_H=0 enables level change interrupt
#define     BIT_DEC_H         0x04       // Control open-drain output pins high 8-bit chip select decode
#define     BIT_DEC_L         0x02       // Control open-drain output pins low 8-bit chip select decode
#define     BIT_IO_OE         0x01       // Control bidirectional I/O pin tri-state output, 1 enables output

/*  Set low 8-bit open-drain output command */

#define     CH423_OC_L_CMD    0x4400     // Set low 8-bit open-drain output command, default mode
#define     BIT_OC0_L_DAT     0x01       // OC0=0 outputs low, OC0=1 pin floats (high)
#define     BIT_OC1_L_DAT     0x02       // OC1=0 outputs low, OC1=1 pin floats (high)
#define     BIT_OC2_L_DAT     0x04       // OC2=0 outputs low, OC2=1 pin floats (high)
#define     BIT_OC3_L_DAT     0x08       // OC3=0 outputs low, OC3=1 pin floats (high)
#define     BIT_OC4_L_DAT     0x10       // OC4=0 outputs low, OC4=1 pin floats (high)
#define     BIT_OC5_L_DAT     0x20       // OC5=0 outputs low, OC5=1 pin floats (high)
#define     BIT_OC6_L_DAT     0x40       // OC6=0 outputs low, OC6=1 pin floats (high)
#define     BIT_OC7_L_DAT     0x80       // OC7=0 outputs low, OC7=1 pin floats (high)

/*  Set high 8-bit open-drain output command */

#define     CH423_OC_H_CMD    0x4600      // Set high 8-bit open-drain output command, default mode
#define     BIT_OC8_L_DAT     0x01        // OC8=0 outputs low, OC8=1 pin floats (high)
#define     BIT_OC9_L_DAT     0x02        // OC9=0 outputs low, OC9=1 pin floats (high)
#define     BIT_OC10_L_DAT    0x04        // OC10=0 outputs low, OC10=1 pin floats (high)
#define     BIT_OC11_L_DAT    0x08        // OC11=0 outputs low, OC11=1 pin floats (high)
#define     BIT_OC12_L_DAT    0x10        // OC12=0 outputs low, OC12=1 pin floats (high)
#define     BIT_OC13_L_DAT    0x20        // OC13=0 outputs low, OC13=1 pin floats (high)
#define     BIT_OC14_L_DAT    0x40        // OC14=0 outputs low, OC14=1 pin floats (high)
#define     BIT_OC15_L_DAT    0x80        // OC15=0 outputs low, OC15=1 pin floats (high)

/* Set bidirectional I/O command */

#define     CH423_SET_IO_CMD   0x6000    // Set bidirectional I/O command, default mode
#define     BIT_IO0_DAT        0x01      // Write to bidirectional I/O output register, when IO_OE=1, IO0=0 outputs low, =1 outputs high
#define     BIT_IO1_DAT        0x02      // Write to bidirectional I/O output register, when IO_OE=1, IO1=0 outputs low, =1 outputs high
#define     BIT_IO2_DAT        0x04      // Write to bidirectional I/O output register, when IO_OE=1, IO2=0 outputs low, =1 outputs high
#define     BIT_IO3_DAT        0x08      // Write to bidirectional I/O output register, when IO_OE=1, IO3=0 outputs low, =1 outputs high
#define     BIT_IO4_DAT        0x10      // Write to bidirectional I/O output register, when IO_OE=1, IO4=0 outputs low, =1 outputs high
#define     BIT_IO5_DAT        0x20      // Write to bidirectional I/O output register, when IO_OE=1, IO5=0 outputs low, =1 outputs high
#define     BIT_IO6_DAT        0x40      // Write to bidirectional I/O output register, when IO_OE=1, IO6=0 outputs low, =1 outputs high
#define     BIT_IO7_DAT        0x80      // Write to bidirectional I/O output register, when IO_OE=1, IO7=0 outputs low, =1 outputs high

/* Read bidirectional I/O command */
#define CH423_RD_IO_CMD		0x4D	// Read I/O pins current state

void CH423_Init(void);
void CH423_Write( unsigned short cmd );         // Write command
void CH423_WriteByte( unsigned short cmd );     // Write data
unsigned char CH423_ReadByte();                 // Read data

void SetIOChannel(unsigned char IOChannel);
void ClrIOChannel(unsigned char IOChannel);
void SetOCChannel(unsigned char OCChannel);
void ClrOCChannel(unsigned char OCChannel);

//IO
#define        VDO_PIN      5
#define  MIC_IN_EN_PIN      6
#define MIC_OUT_EN_PIN      7
#define  SPK_IN_EN_PIN      1
#define SPK_OUT_EN_PIN      0

//OC
#define POWER_EN_8_CHAN     1
#define POWER_EN_12_CHAN    2
#define FM_AMP_EN_CHAN      8


#define SET_VDO_PIN            SetIOChannel(VDO_PIN)
#define CLR_VDO_PIN            ClrIOChannel(VDO_PIN)

#define SET_MIC_IN_EN_PIN       SetIOChannel (MIC_IN_EN_PIN)
#define CLR_MIC_IN_EN_PIN       ClrIOChannel (MIC_IN_EN_PIN)
#define SET_MIC_OUT_EN_PIN      SetIOChannel (MIC_OUT_EN_PIN)
#define CLR_MIC_OUT_EN_PIN      ClrIOChannel (MIC_OUT_EN_PIN)

#define SET_SPK_IN_EN_PIN       SetIOChannel (SPK_IN_EN_PIN)
#define CLR_SPK_IN_EN_PIN       ClrIOChannel (SPK_IN_EN_PIN)
#define SET_SPK_OUT_EN_PIN      SetIOChannel (SPK_OUT_EN_PIN)
#define CLR_SPK_OUT_EN_PIN      ClrIOChannel (SPK_OUT_EN_PIN)


#define SET_POWER_EN_8_CHAN     SetOCChannel(POWER_EN_8_CHAN)
#define CLR_POWER_EN_8_CHAN     ClrOCChannel(POWER_EN_8_CHAN)
#define SET_POWER_EN_12_CHAN    SetOCChannel(POWER_EN_12_CHAN)
#define CLR_POWER_EN_12_CHAN    ClrOCChannel(POWER_EN_12_CHAN)

#define SET_FM_AMP_EN_CHAN     SetOCChannel(FM_AMP_EN_CHAN)
#define CLR_FM_AMP_EN_CHAN     ClrOCChannel(FM_AMP_EN_CHAN)


#endif 

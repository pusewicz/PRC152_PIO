#include "bsp_iic.h"       
#include "bsp_delay.h"  

#define I2C_DELAY 1

// I2C initialization
void bsp_IIC_Init(void)
{
    pinMode(SDA_PIN, OUTPUT);
    pinMode(SCL_PIN, OUTPUT);
    IIC_SDA1;
    IIC_SCL1;
}

// Generate I2C start signal
void IIC_Start(void)
{
	SDA_OUT();     // SDA line output
	IIC_SDA1;
	IIC_SCL1;
	delay_us(I2C_DELAY);
 	IIC_SDA0;//START:when CLK is high,DATA change form high to low
	delay_us(I2C_DELAY);
	IIC_SCL0;// Hold I2C bus, ready to send or receive data
}
// Generate I2C stop signal
void IIC_Stop(void)
{
	SDA_OUT();// SDA line output
	IIC_SCL0;
	IIC_SDA0;//STOP:when CLK is high DATA change form low to high
 	delay_us(I2C_DELAY);
	IIC_SCL1;
	delay_us(I2C_DELAY);
	IIC_SDA1;// Send I2C bus end signal
}
// Wait for acknowledge signal
// Return: 1 = receive ACK failed
//         0 = receive ACK success
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();      // SDA set to input
	IIC_SDA1;delay_us(I2C_DELAY);
	IIC_SCL1;delay_us(I2C_DELAY);
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL0;// Clock output 0
	return 0;
}
// Generate ACK acknowledge
void IIC_Ack(void)
{
    IIC_SCL0;
    SDA_OUT();
    IIC_SDA0;
    delay_us(I2C_DELAY);
    IIC_SCL1;
    delay_us(I2C_DELAY);
    IIC_SCL0;
}
// Generate NACK (no acknowledge)
void IIC_NAck(void)
{
    IIC_SCL0;
    SDA_OUT();
    IIC_SDA1;
    delay_us(I2C_DELAY);
    IIC_SCL1;
    delay_us(I2C_DELAY);
    IIC_SCL0;
}
// I2C send one byte
// Return: slave acknowledge status
// 1 = ACK received
// 0 = no ACK
void IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
	SDA_OUT();
    IIC_SCL0;// Pull clock low to start data transfer
    for(t=0; t<8; t++)
    {
        //IIC_SDA=(txd&0x80)>>7;
        if((txd&0x80)>>7)IIC_SDA1;
        else IIC_SDA0;
        txd<<=1;
        delay_us(I2C_DELAY);   // These three delays are required for TEA5767
        IIC_SCL1;
        delay_us(I2C_DELAY);
        IIC_SCL0;
        delay_us(I2C_DELAY);
    }
}
// Read 1 byte, send ACK when ack=1, send NACK when ack=0
uint8_t IIC_Read_Byte(uint8_t ack)
{
    uint8_t i,receive=0;
    SDA_IN();// SDA set to input
    for(i=0;i<8;i++ )
    {
        IIC_SCL0;
        delay_us(I2C_DELAY);
        IIC_SCL1;
        receive<<=1;
        if(READ_SDA)receive++;
        delay_us(I2C_DELAY);
    }
    if (!ack)
        IIC_NAck();// Send NACK
    else
        IIC_Ack(); // Send ACK
    return receive;
}



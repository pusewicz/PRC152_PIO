//VCC RX TX GND DTR_RST RTS_tck

#define FREERTOS_CONFIG_XTENSA_H
#include "main.h"
#include "devconsole.h"
extern void menuUpdate(void);
void setup()
{
    UART1_Init(); //初始化串口

    CH423_Init(); // IIC初始化-->CH423初始化-->CH423控制的引脚初始化

    SPK_SWITCH(IN, OFF); //限制开机的杂音

    Key_Init(); //初始化按键:初始化编码器按键; 检测是否需要进入BOOT模式

    Standby_Init(); //确认编码器是否为正常长按;长按则使能3.3V控制引脚  ////POWER_EN_SET;//

    // UART1_Init(); //初始化串口
    ADC_Init();   //检测电压使用
    Timer_Init(); //启动定时器处理 ADC检测电压程序 DAC输出电压定时器中断

    //将控制引脚初始化,并设置状态, 避免出现错误IO状态
    M62364_Init(); // m62364初始化-->禁止声音输出/关闭FM电源

    LCD_Init();
    PWM_Init();
    LCD_ShowPICALL(pic_HARRIS);

    Init_Storage(true);
    enterSecondSystem(); // menuUpdate();    //信道设置需要内存初始化

    EN_GPIO_Init(); // POWER_EN_8_SET;//

    SineWave_Data();
    VFO_Load_Data();
    A002_Init();
    VFO_Clear();

    DevConsole_Init();
    DevConsole_WifiInit();

    // while(1)FeedDog();
    // test_A20Deinit();
    // test_DAC_always();
    // test_FM();
    // while(1)PGM_Menu();
    //     PGM_RTControl();
}
void loop()
{
    VFO_Refresh();
    MY_GLOBAL_FUN();
    Encoder_process(Encoder_Switch_Scan(0));
    Argument_process(Event_Matrix(Matrix_KEY_Scan(0))); //矩阵按键事件处理
    KDU_Processor();                                    // KDU处理
}

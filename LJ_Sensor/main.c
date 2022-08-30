/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B Bare Program, Sample main file
 */

#include <stdio.h>
#include "ls1b.h"
#include "mips.h"
#include "ls1b_gpio.h"

//#define USE_TOUCH
//#define USE_MAN_S
//#define USE_SG180
//#define USE_Temperature
#define USE_Air_Quality

#define BUZZER 49   // 46 高电平触发 5V
#define TOUCH  45   // 47 触发后输出高电平 2V-5V
#define MAN_S  42   // 36 触发后高电平保持一段时间 4.5V-20V
#define SG180  43   // 32 舵机周期20ms 高电平1ms-2ms,3.5V-6V
#define TEMPE  46   // 45 双向温度数据，3V-5.5V
#define Air_Q  44   // 33 双路信号输出（模拟量输出浓度越高电压越高、TTL输出浓度大于设定浓度输出低电平），5V

//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"

#ifdef BSP_USE_FB
  #include "ls1x_fb.h"
  #ifdef XPT2046_DRV
    char LCD_display_mode[] = LCD_800x480;
  #elif defined(GT1151_DRV)
    char LCD_display_mode[] = LCD_480x800;
  #else
    #error "在bsp.h中选择配置 XPT2046_DRV 或者 GT1151_DRV"
           "XPT2046_DRV:  用于800*480 横屏的触摸屏."
           "GT1151_DRV:   用于480*800 竖屏的触摸屏."
           "如果都不选择, 注释掉本 error 信息, 然后自定义: LCD_display_mode[]"
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// 主程序
//-------------------------------------------------------------------------------------------------

int Buzzer_Mode = 0;

void Change_Buzzer_Mode(void)
{
    Buzzer_Mode = 1 - Buzzer_Mode;
    return;
}


#ifdef USE_TOUCH
int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */

    gpio_enable(BUZZER,DIR_OUT);

    gpio_enable(TOUCH,DIR_IN);
    ls1x_install_gpio_isr(TOUCH, INT_TRIG_EDGE_UP,Change_Buzzer_Mode, 0);
    ls1x_enable_gpio_interrupt(TOUCH);

    gpio_write(BUZZER,Buzzer_Mode);
    delay_ms(10);
    while(1)
    {
        gpio_write(BUZZER,Buzzer_Mode);
        delay_ms(10);
    }
    return 0;
}
#endif


#ifdef USE_MAN_S
int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */

    gpio_enable(BUZZER,DIR_OUT);

    gpio_enable(MAN_S,DIR_IN);

    gpio_write(BUZZER,Buzzer_Mode);
    delay_ms(10);
    while(1)
    {
        Buzzer_Mode = gpio_read(MAN_S);
        //printk("%d",Buzzer_Mode);
        gpio_write(BUZZER,Buzzer_Mode);
        delay_ms(10);
    }
    return 0;
}
#endif


#ifdef USE_SG180
int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */

    gpio_enable(SG180,DIR_OUT);

    int num=250;  // 250us-2550us
    while(1)
    {
        if(num>2550)
            num=250;
        else
            num=num+5;
        gpio_write(SG180,1);
        delay_us(num);
        gpio_write(SG180,0);
        delay_us(20000-num);
    }
    return 0;
}
#endif



#ifdef USE_Temperature

int           DQ = 0;       // 引脚电平
unsigned char tempL=0; 		// 设全局变量
unsigned char tempH=0;
unsigned int  sdata;		// 测量到的温度的整数部分
unsigned char xiaoshu1;		// 小数第一位
unsigned char xiaoshu2;		// 小数第二位
unsigned char xiaoshu;		// 两位小数
int fg=1;        			// 温度正负标志
double temperature=0;


void Init_DS18B20(void)
{
	unsigned char x=0;
	gpio_enable(TEMPE,DIR_OUT);
	DQ=1; 				   //DQ先置高
	gpio_write(TEMPE,DQ);
	delay_us(80);          //稍延时
	DQ=0; 				   //发送复位脉冲
	gpio_write(TEMPE,DQ);
	delay_us(800);         //延时（>480us)
	DQ=1; 				   //拉高数据线
	gpio_write(TEMPE,DQ);
	delay_us(50);          //等待（15~60us)
	
	gpio_enable(TEMPE,DIR_IN);
	DQ=gpio_read(TEMPE);
	x=DQ; 					//用X的值来判断初始化有没有成功，18B20存在的话X=0，否则X=1
	//printk("x=%d\n\r",x);  // 注意延时
	delay_us(200);
}


unsigned char ReadOneChar(void)  			//主机数据线先从高拉至低电平1us以上，再使数据线升为高电平，从而产生读信号
{
	unsigned char i=0; 		//每个读周期最短的持续时间为60us，各个读周期之间必须有1us以上的高电平恢复期
	unsigned char dat=0;
	for (i=8;i>0;i--) 		//一个字节有8位
	{
	    gpio_enable(TEMPE,DIR_OUT);
		DQ=1;
		gpio_write(TEMPE,DQ);
		delay_us(10);
		DQ=0;
		gpio_write(TEMPE,DQ);
		dat>>=1;
		DQ=1;
		gpio_write(TEMPE,DQ);
		
		gpio_enable(TEMPE,DIR_IN);
		DQ=gpio_read(TEMPE);
		if(DQ)
		  dat|=0x80;
		delay_us(40);
	}
	return(dat);
}


void WriteOneChar(unsigned char dat)
{
	unsigned char i=0; 		//数据线从高电平拉至低电平，产生写起始信号。15us之内将所需写的位送到数据线上，
	gpio_enable(TEMPE,DIR_OUT);
	for(i=8;i>0;i--) 		//在15~60us之间对数据线进行采样，如果是高电平就写1，低写0发生。
	{
		DQ=0; 				//在开始另一个写周期前必须有1us以上的高电平恢复期。
		gpio_write(TEMPE,DQ);
		DQ=dat&0x01;
		gpio_write(TEMPE,DQ);
		delay_us(50);
		DQ=1;
		gpio_write(TEMPE,DQ);
		dat>>=1;
	}
	delay_us(40);
}


void ReadTemperature(void)
{
	Init_DS18B20(); 					//初始化
	WriteOneChar(0xcc); 				//跳过读序列号的操作
	WriteOneChar(0x44); 				//启动温度转换
	delay_us(1250);                     //转换需要一点时间，延时
	Init_DS18B20(); 					//初始化
	WriteOneChar(0xcc); 				//跳过读序列号的操作
	WriteOneChar(0xbe); 				//读温度寄存器（头两个值分别为温度的低位和高位）
	tempL=ReadOneChar(); 				//读出温度的低位LSB
	tempH=ReadOneChar(); 				//读出温度的高位MSB
	if(tempH>0x7f)      				//最高位为1时温度是负
	{
		tempL=~tempL;					//补码转换，取反加一
		tempH=~tempH+1;
		fg=0;      						//读取温度为负时fg=0
	}
	sdata = tempL/16+tempH*16;      	//整数部分
	xiaoshu1 = (tempL&0x0f)*10/16; 		//小数第一位
	xiaoshu2 = (tempL&0x0f)*100/16%10;	//小数第二位
	xiaoshu=xiaoshu1*10+xiaoshu2; 		//小数两位
	temperature = sdata + xiaoshu*0.01;
	return;
}


int main()
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */
    double temperature_old = 0;
    
    while(1)
	{
		ReadTemperature();
		if(temperature_old != temperature)
		{
		    printk("%.2f\r\n",temperature);
    		temperature_old = temperature;
        }
        delay_ms(100);
	}
    return 0;
}
#endif


#ifdef USE_Air_Quality
int main()
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */

    gpio_enable(Air_Q,DIR_IN);
    while(1)
    {
        if(!(gpio_read(Air_Q)))
            printk("!!!\r\n");
        else
            printk("...\r\n");
        delay_ms(1000);
    }
    return 0;
}
#endif



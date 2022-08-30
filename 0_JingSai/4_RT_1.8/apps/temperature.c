/*
 * temp.c
 *
 * created: 2022/5/26
 *  author: 
 */


#include <rtthread.h>
#include <stdio.h>
#include "bsp.h"
#include "ls1b.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"
#include "ls1b_gpio.h"

#define printf  printk
#define getch   usb_kbd_getc

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        50
static rt_thread_t Temperature1 = RT_NULL;


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


int Temperature_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);

    printf("Refreshes the display when the temperature changes.\r\n");
    printf("Touch the capacitive sensor to exit the program.\r\n");
    double temperature_old = 0;
    gpio_enable(TOUCH,DIR_IN);
    while(!(gpio_read(TOUCH)))
	{
		ReadTemperature();
		if(temperature_old != temperature)
		{
		    printf("The current temperature is %.2f\r\n",temperature);
    		temperature_old = temperature;
        }
        delay_ms(100);
	}
	printf("\r\n");

    change_scheduler_lock(0);
}

int Temperature(void)
{
    Temperature1 = rt_thread_create("Temperature",
                            Temperature_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(Temperature1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(Temperature, Temperature);

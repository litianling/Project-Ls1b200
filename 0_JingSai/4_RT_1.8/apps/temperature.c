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


int           DQ = 0;       // ���ŵ�ƽ
unsigned char tempL=0; 		// ��ȫ�ֱ���
unsigned char tempH=0;
unsigned int  sdata;		// ���������¶ȵ���������
unsigned char xiaoshu1;		// С����һλ
unsigned char xiaoshu2;		// С���ڶ�λ
unsigned char xiaoshu;		// ��λС��
int fg=1;        			// �¶�������־
double temperature=0;


void Init_DS18B20(void)
{
	unsigned char x=0;
	gpio_enable(TEMPE,DIR_OUT);
	DQ=1; 				   //DQ���ø�
	gpio_write(TEMPE,DQ);
	delay_us(80);          //����ʱ
	DQ=0; 				   //���͸�λ����
	gpio_write(TEMPE,DQ);
	delay_us(800);         //��ʱ��>480us)
	DQ=1; 				   //����������
	gpio_write(TEMPE,DQ);
	delay_us(50);          //�ȴ���15~60us)

	gpio_enable(TEMPE,DIR_IN);
	DQ=gpio_read(TEMPE);
	x=DQ; 					//��X��ֵ���жϳ�ʼ����û�гɹ���18B20���ڵĻ�X=0������X=1
	//printk("x=%d\n\r",x);  // ע����ʱ
	delay_us(200);
}


unsigned char ReadOneChar(void)  			//�����������ȴӸ������͵�ƽ1us���ϣ���ʹ��������Ϊ�ߵ�ƽ���Ӷ��������ź�
{
	unsigned char i=0; 		//ÿ����������̵ĳ���ʱ��Ϊ60us������������֮�������1us���ϵĸߵ�ƽ�ָ���
	unsigned char dat=0;
	for (i=8;i>0;i--) 		//һ���ֽ���8λ
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
	unsigned char i=0; 		//�����ߴӸߵ�ƽ�����͵�ƽ������д��ʼ�źš�15us֮�ڽ�����д��λ�͵��������ϣ�
	gpio_enable(TEMPE,DIR_OUT);
	for(i=8;i>0;i--) 		//��15~60us֮��������߽��в���������Ǹߵ�ƽ��д1����д0������
	{
		DQ=0; 				//�ڿ�ʼ��һ��д����ǰ������1us���ϵĸߵ�ƽ�ָ��ڡ�
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
	Init_DS18B20(); 					//��ʼ��
	WriteOneChar(0xcc); 				//���������кŵĲ���
	WriteOneChar(0x44); 				//�����¶�ת��
	delay_us(1250);                     //ת����Ҫһ��ʱ�䣬��ʱ
	Init_DS18B20(); 					//��ʼ��
	WriteOneChar(0xcc); 				//���������кŵĲ���
	WriteOneChar(0xbe); 				//���¶ȼĴ�����ͷ����ֵ�ֱ�Ϊ�¶ȵĵ�λ�͸�λ��
	tempL=ReadOneChar(); 				//�����¶ȵĵ�λLSB
	tempH=ReadOneChar(); 				//�����¶ȵĸ�λMSB
	if(tempH>0x7f)      				//���λΪ1ʱ�¶��Ǹ�
	{
		tempL=~tempL;					//����ת����ȡ����һ
		tempH=~tempH+1;
		fg=0;      						//��ȡ�¶�Ϊ��ʱfg=0
	}
	sdata = tempL/16+tempH*16;      	//��������
	xiaoshu1 = (tempL&0x0f)*10/16; 		//С����һλ
	xiaoshu2 = (tempL&0x0f)*100/16%10;	//С���ڶ�λ
	xiaoshu=xiaoshu1*10+xiaoshu2; 		//С����λ
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

/* ������ msh �����б��� */
MSH_CMD_EXPORT(Temperature, Temperature);

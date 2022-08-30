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

#define BUZZER 49   // 46 �ߵ�ƽ���� 5V
#define TOUCH  45   // 47 ����������ߵ�ƽ 2V-5V
#define MAN_S  42   // 36 ������ߵ�ƽ����һ��ʱ�� 4.5V-20V
#define SG180  43   // 32 �������20ms �ߵ�ƽ1ms-2ms,3.5V-6V
#define TEMPE  46   // 45 ˫���¶����ݣ�3V-5.5V
#define Air_Q  44   // 33 ˫·�ź������ģ�������Ũ��Խ�ߵ�ѹԽ�ߡ�TTL���Ũ�ȴ����趨Ũ������͵�ƽ����5V

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
    #error "��bsp.h��ѡ������ XPT2046_DRV ���� GT1151_DRV"
           "XPT2046_DRV:  ����800*480 �����Ĵ�����."
           "GT1151_DRV:   ����480*800 �����Ĵ�����."
           "�������ѡ��, ע�͵��� error ��Ϣ, Ȼ���Զ���: LCD_display_mode[]"
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// ������
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



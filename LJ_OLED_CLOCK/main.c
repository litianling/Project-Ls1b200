#include "mips.h"
#include "ls1b.h"
#include "bsp.h"
#include "ls1x_i2c_bus.h"
#include "ls1x_rtc.h"

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

typedef unsigned char           uint8_t;

uint8_t space[11] = "          ";
uint8_t aShowTime[9] = "hh:mm:ss";
uint8_t aShowDate[11] = "dd-mm-yyyy";

//-------------------------------------------------------------------------------------------------
// RTC����
//-------------------------------------------------------------------------------------------------
//driver coding
void get_date(int year,int mon,int day)
{
    aShowDate[0]= day / 10 + 48;
    aShowDate[1]= day % 10 + 48;
    aShowDate[3]= mon / 10 + 48;
    aShowDate[4]= mon % 10 + 48;
    aShowDate[6]= year / 1000 + 48;
    aShowDate[7]= (year % 1000)/100 + 48;
    aShowDate[8]= (year % 100)/10 + 48;
    aShowDate[9]= year % 10 + 48;
}

void get_time(int hour,int min,int sec)
{
    aShowTime[0]= hour / 10 + 48;
    aShowTime[1]= hour % 10 + 48;
    aShowTime[3]= min / 10 + 48;
    aShowTime[4]= min % 10 + 48;
    aShowTime[6]= sec / 10 + 48;
    aShowTime[7]= sec % 10 + 48;
}

static void ls1x_rtc_isr(int vector, void *arg)
{
    int device, index;
    if (arg == NULL)
        return;
    device = (int)arg & 0xFF00;
    index  = (int)arg & 0x00FF;
    printk("isr from device=%i, index=%i\r\n", device, index);
}

static void ls1x_rtc_callback(int device, unsigned match, int *stop)
{
    struct tm dt;
    switch (device & 0xFF00)
    {
        case LS1X_TOY:
            ls1x_toymatch_to_tm(&dt, match);
            normalize_tm(&dt, false);
            printk("isr = %i.%i.%i-%i:%i:%i <-\r\n",dt.tm_year, dt.tm_mon, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
            get_date(dt.tm_year,dt.tm_mon,dt.tm_mday);
            OLED_ShowString(22,2,aShowDate);  //�ַ�����ʾ����
            get_time(dt.tm_hour,dt.tm_min,dt.tm_sec);
            OLED_ShowString_Short(28,4,aShowTime,8); //�޶�����8λ
            break;

        case LS1X_RTC:
            printk("rtc%i <-\r\n", device & 0xFF);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
// ������
//-------------------------------------------------------------------------------------------------

int main()
{
    struct tm dt;
    rtc_cfg_t cfg;

    //����RTC��ʼʱ��
    dt.tm_year=2021;
    dt.tm_mon=11;
    dt.tm_mday=5;
    dt.tm_hour=22;
    dt.tm_min=30;
    dt.tm_sec=59;
    ls1x_rtc_init(NULL, &dt);

    dt.tm_sec+=10;
    cfg.interval_ms=1000;       //RTC�жϴ������
    cfg.trig_datetime=NULL;
    cfg.cb=ls1x_rtc_callback;   //�жϻص�����
    cfg.isr=NULL;
    ls1x_rtc_open(DEVICE_TOYMATCH0, &cfg);

    //��ʼ��IIC������OLED����оƬssd1306
    ls1x_i2c_initialize(busI2C0);
    Initial_M096128x64_ssd1306();

    //unsigned char y=0;
    delay_ms(5);
	Picture(1);//��ʾһ��ͼƬ--��ֽ
	OLED_ShowString(22,2,space);  //���ԭͼƬ�м䲿�֣�׼����ʾ������ʱ�䣩
	OLED_ShowString(22,4,space);

    while (1)
    {

    }
    return 0;
}



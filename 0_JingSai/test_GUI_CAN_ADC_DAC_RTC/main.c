/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B Bare Program, Sample main file
 */

#include <stdio.h>
#include <math.h>

#include "ls1b.h"
#include "mips.h"

//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"

#if defined(BSP_USE_CAN0) || defined(BSP_USE_CAN1)
  #include "ls1x_can.h"
#endif

#ifdef BSP_USE_FB
  #include "ls1x_fb.h"
  #include "gui/demo_gui.h"
  #include "gui/simple-gui/simple_gui.h"
  #ifdef XPT2046_DRV
    #include "spi/xpt2046.h"
    char LCD_display_mode[] = LCD_800x480;
  #elif defined(GT1151_DRV)
    #include "i2c/gt1151.h"
    #include "touch.h"
    char LCD_display_mode[] = LCD_480x800;
  #else
    #error "��bsp.h��ѡ������ XPT2046_DRV ���� GT1151_DRV"
           "XPT2046_DRV:  ����800*480 �����Ĵ�����."
           "GT1151_DRV:   ����480*800 �����Ĵ�����."
           "�������ѡ��, ע�͵��� error ��Ϣ, Ȼ���Զ���: LCD_display_mode[]"
  #endif
#endif

#if defined(BSP_USE_GMAC0)
  #include "lwip/netif.h"
  #include "lwip/init.h"
  extern struct netif *p_gmac0_netif;
#endif

//-------------------------------------------------------------------------------------------------

extern void start_my_gui(void);

extern int lx1x_can0_init_transmit(int gui);
extern int ls1x_can0_do_transmit(void *can);
extern int lx1x_can1_init_receive(int gui);
extern int ls1x_can1_do_receive(void *can);

extern int lx1x_i2c0_slaves_initialize(int gui);
extern int ls1x_i2c0_slaves_do_work(void);

extern void ethernetif_input(struct netif *netif);

//-------------------------------------------------------------------------------------------------
// ��ѭ��
//-------------------------------------------------------------------------------------------------

int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */


#ifdef GP7101_DRV
	set_lcd_brightness(70);    			/* ���� LCD ���� */
#endif


#ifdef BSP_USE_FB
	// do_touchscreen_calibrate();      /* ������У�� */
	start_my_gui();						//����GUI��ʾ��*����ʼ��GUI������������Ҫ���󡢻�����ʾ���棨�����ͱ���
#endif

#ifdef BSP_USE_CAN0
    lx1x_can0_init_transmit(1);         // can0��ʼ������ģʽ��*����ʼ����������IO��������ģʽ/�ٶȣ�����GUI����ʾ������ģʽ
#endif
#ifdef BSP_USE_CAN1
    lx1x_can1_init_receive(1);          // can1��ʼ������ģʽ��*����ʼ����������IO��������ģʽ/�ٶȣ�����GUI����ʾ������ģʽ
#endif
#if defined(ADS1015_DRV) || defined(MCP4725_DRV) || defined(RX8010_DRV)     // 4·ADC/1·DAC/RTC
    lx1x_i2c0_slaves_initialize(1);     // I2C�豸��ʼ����*����ʾADC��DAC��RTC����ģʽ/����Ԥ��ʱ��
#endif


    while (1)
    {
#if defined(BSP_USE_GMAC0)
        ethernetif_input(p_gmac0_netif);    // ��*�����ڶ�ȡ���ݰ�
#endif
        delay_ms(10);


#ifdef XPT2046_DRV
        int x, y;
        if (bare_get_touch_point(&x, &y) == 0)  // ��*�����������ꡢУ׼�������ꡢ��ʾ����λ��
        {
            set_focused_button(x, y);       // ������ִ�а�ť�ص�����
        }
#elif defined(GT1151_DRV)

        if (GT1151_Scan(0))
        {
            int x, y;
            x = tp_dev.x[0];
            y = tp_dev.y[0];

            set_focused_button(x, y);
        }

#endif


#ifdef BSP_USE_CAN0
        {
            ls1x_can0_do_transmit(devCAN0);     // ��can0�������ݡ�*���������ݲ�д�ڱ�����
            delay_ms(10); // 50
        }
#endif


#ifdef BSP_USE_CAN1
        {
            ls1x_can1_do_receive(devCAN1);      // ��can1�������ݡ�*���������ݲ�д�ڱ�����
            delay_ms(10); // 50
        }
#endif

// 4· 12bit ADC   1· 12bit DAC
#if defined(ADS1015_DRV) || defined(MCP4725_DRV)
        {
            ls1x_i2c0_slaves_do_work();         // дDAC & ��ADC & ��RTC
            delay_ms(20); // 100
        }
#endif
    }
    /* �������е����� */
    return 0;
}

/*
 * @@ End
 */

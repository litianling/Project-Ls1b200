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
    #error "在bsp.h中选择配置 XPT2046_DRV 或者 GT1151_DRV"
           "XPT2046_DRV:  用于800*480 横屏的触摸屏."
           "GT1151_DRV:   用于480*800 竖屏的触摸屏."
           "如果都不选择, 注释掉本 error 信息, 然后自定义: LCD_display_mode[]"
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
// 主循环
//-------------------------------------------------------------------------------------------------

int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */


#ifdef GP7101_DRV
	set_lcd_brightness(70);    			/* 设置 LCD 亮度 */
#endif


#ifdef BSP_USE_FB
	// do_touchscreen_calibrate();      /* 触摸屏校正 */
	start_my_gui();						//启用GUI显示【*】初始化GUI环境、创建主要对象、绘制显示界面（按键和表格）
#endif

#ifdef BSP_USE_CAN0
    lx1x_can0_init_transmit(1);         // can0初始化发送模式【*】初始化、开启、IO控制器（模式/速度），在GUI上显示名字与模式
#endif
#ifdef BSP_USE_CAN1
    lx1x_can1_init_receive(1);          // can1初始化接收模式【*】初始化、开启、IO控制器（模式/速度），在GUI上显示名字与模式
#endif
#if defined(ADS1015_DRV) || defined(MCP4725_DRV) || defined(RX8010_DRV)     // 4路ADC/1路DAC/RTC
    lx1x_i2c0_slaves_initialize(1);     // I2C设备初始化【*】显示ADC、DAC、RTC工作模式/并且预设时间
#endif


    while (1)
    {
#if defined(BSP_USE_GMAC0)
        ethernetif_input(p_gmac0_netif);    // 【*】网口读取数据包
#endif
        delay_ms(10);


#ifdef XPT2046_DRV
        int x, y;
        if (bare_get_touch_point(&x, &y) == 0)  // 【*】读触摸坐标、校准触摸坐标、显示触摸位置
        {
            set_focused_button(x, y);       // 【！】执行按钮回调函数
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
            ls1x_can0_do_transmit(devCAN0);     // 用can0发送数据【*】发送数据并写在表格中
            delay_ms(10); // 50
        }
#endif


#ifdef BSP_USE_CAN1
        {
            ls1x_can1_do_receive(devCAN1);      // 用can1接收数据【*】接收数据并写在表格中
            delay_ms(10); // 50
        }
#endif

// 4路 12bit ADC   1路 12bit DAC
#if defined(ADS1015_DRV) || defined(MCP4725_DRV)
        {
            ls1x_i2c0_slaves_do_work();         // 写DAC & 读ADC & 读RTC
            delay_ms(20); // 100
        }
#endif
    }
    /* 不会运行到这里 */
    return 0;
}

/*
 * @@ End
 */


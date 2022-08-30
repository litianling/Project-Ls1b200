/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// 32 33 29 28   30 04 07 06
//

/*
 * Loongson 1B Bare Program, Sample main file
 */

#include <stdio.h>

#include "ls1b.h"
#include "mips.h"
#include "ls1b_gpio.h"

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

#define LED1 32
#define LED2 33
#define LED3 29
#define LED4 28
#define LED5 30
#define LED6 04
#define LED7 07
#define LED8 06

#define BTN1 34     // 速度+
#define BTN2 35     // 速度-
#define BTN3 36     // 模式+
#define BTN4 37     // 模式-

int speed = 1;
int mode = 1;

void LED8_init();
void LED8_write(int DATA);
void BUTTON4_init();
void speed_up();
void speed_down();
void mode_up();
void mode_down();

int main(void)
{
    int GPIO1 = 0x00;
    int GPIO2 = 0x01;
    int GPIO3 = 0x00;
    int GPIO4 = 0xff;
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */
    LED8_init();
    BUTTON4_init();
    
    while(1)
    {
        while(mode==1)  // 闪烁
        {
            GPIO1 = 0x00;
            LED8_write(GPIO1);
            delay_ms(500-speed*100);
            GPIO1 = 0xff;
            LED8_write(GPIO1);
            delay_ms(500-speed*100);
        }
        while(mode==2)  // 流水
        {
            if(GPIO2==0x80)
                GPIO2 = 0x01;
            else
                GPIO2 = GPIO2 << 1;
            LED8_write(GPIO2);
            delay_ms(500-speed*100);
        }
        while(mode==3)  // 计数器
        {
            if(GPIO3==0xff)
                GPIO3 = 0x00;
            else
                GPIO3++;
            LED8_write(GPIO3);
            delay_ms(500-speed*100);
        }
        while(mode==4)  // 常亮
        {
            LED8_write(GPIO4);
            delay_ms(500-speed*100);
        }
    }
    
    return 0;
}

void LED8_init()
{
    gpio_enable(LED1,DIR_OUT);
    gpio_enable(LED2,DIR_OUT);
    gpio_enable(LED3,DIR_OUT);
    gpio_enable(LED4,DIR_OUT);
    gpio_enable(LED5,DIR_OUT);
    gpio_enable(LED6,DIR_OUT);
    gpio_enable(LED7,DIR_OUT);
    gpio_enable(LED8,DIR_OUT);
}

void LED8_write(int DATA)
{
    if(DATA & 0x80)         // 检测到1写0，灯亮
        gpio_write(LED1,0);
    else
        gpio_write(LED1,1);   // 检测到0写1，灯灭
        
    if(DATA & 0x40)
        gpio_write(LED2,0);
    else
        gpio_write(LED2,1);

    if(DATA & 0x20)
        gpio_write(LED3,0);
    else
        gpio_write(LED3,1);

    if(DATA & 0x10)
        gpio_write(LED4,0);
    else
        gpio_write(LED4,1);


    if(DATA & 0x08)
        gpio_write(LED5,0);
    else
        gpio_write(LED5,1);

    if(DATA & 0x04)
        gpio_write(LED6,0);
    else
        gpio_write(LED6,1);

    if(DATA & 0x02)
        gpio_write(LED7,0);
    else
        gpio_write(LED7,1);

    if(DATA & 0x01)
        gpio_write(LED8,0);
    else
        gpio_write(LED8,1);
}

void BUTTON4_init()
{
    //设置为输入
    gpio_enable(BTN1,DIR_IN);   
    gpio_enable(BTN2,DIR_IN);   
    gpio_enable(BTN3,DIR_IN);   
    gpio_enable(BTN4,DIR_IN);   

    //使能GPIO中断
    ls1x_enable_gpio_interrupt(BTN1);
    ls1x_enable_gpio_interrupt(BTN2);
    ls1x_enable_gpio_interrupt(BTN3);
    ls1x_enable_gpio_interrupt(BTN4);

    //安装中断向量
    ls1x_install_gpio_isr(BTN1, INT_TRIG_EDGE_UP,speed_up, 0);
    ls1x_install_gpio_isr(BTN2, INT_TRIG_EDGE_UP,speed_down, 0);
    ls1x_install_gpio_isr(BTN3, INT_TRIG_EDGE_UP,mode_up, 0);
    ls1x_install_gpio_isr(BTN4, INT_TRIG_EDGE_UP,mode_down, 0);
}

/* speed 限制在1到4 */
void speed_up()
{
    if (speed<4)
        speed++;
}

void speed_down()
{
    if (speed>1)
        speed--;
}

/* mode 限制在1到4 */
void mode_up()
{
    if (mode<4)
        mode++;
}

void mode_down()
{
    if (mode>1)
        mode--;
}

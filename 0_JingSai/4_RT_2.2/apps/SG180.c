/*
 * SG180_Touch.c
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
static rt_thread_t SG180_1 = RT_NULL;


int SG180_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    fb_cons_clear();

    gpio_enable(SG180,DIR_OUT);
    printf("Input the number 0-5 to set SG180 location:\r\n");

    // 转动角度说明：（2600-350）/5=2250/5=450
    // 350+0*450=350  350+5*450=2600
    // int num=350;  // 350us-2600us
    while(1)
    {
        char num_c = getch();
        if((num_c<'0')||(num_c>'5'))
            break;
        else
            printf("The mode is %c\r\n",num_c);
        int i = 0 ,num = 350 + ((int)num_c-48)*450;
        
        for(;i<50;i++)  // SG180转动1S到指定位置
        {
            gpio_write(SG180,1);
            delay_us(num);
            gpio_write(SG180,0);
            delay_us(20000-num);
        }
    }

    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
}

int SG180_(void)
{
    SG180_1 = rt_thread_create("SG180_",
                            SG180_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(SG180_1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(SG180_, SG180_);

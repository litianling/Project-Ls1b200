/*
 * mouse_draw.c
 *
 * created: 2022/5/23
 *  author: 
 */

#include <rtthread.h>
#include <stdio.h>
#include "bsp.h"
#include "ls1b.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"

#define printf  printk
#define getch   usb_kbd_getc

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        50
static rt_thread_t mouse_draw1 = RT_NULL;

int mouse_draw_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);

    Mouse_logical_coordinates_reset();      // 画布初始化
    fb_cons_clear();
    
    int ch = 0;
    while (ch != 4)                         // 按下滚轮退出
    {
        if (usb_mouse_testc())              // 检测是否有输入
        {
            ch = usb_mouse_getc();          // 读取鼠标输入，返回输入字符
            //printf(" %d \n",ch);          // 1鼠标左键，2鼠标右键，4鼠标滚轮
        }
        //delay_ms(5);
    }
    
    fb_cons_clear();
    change_scheduler_lock(0);
}

int mouse_draw(void)
{
    mouse_draw1 = rt_thread_create("mouse_draw",
                            mouse_draw_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(mouse_draw1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(mouse_draw, mouse draw);

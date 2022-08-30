/*
 * set_bg_color.c
 *
 * created: 2022/4/13
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
static rt_thread_t set_color1 = RT_NULL;

int set_color_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    //extern int picture_soc_lock;
    //picture_soc_lock = 1;
    
    fb_cons_clear();
    
    printf(" input 'b' to set background color... \r\n");
    printf(" input 'f' to set frontground color... \r\n");
    
    char select = getch();
    if((select!='b')&&(select!='f'))
    {
        printf(" Select err !!! \r\n");
        change_scheduler_lock(0);
        return 0;
    }
    
    printf(" LU_BLACK--------0  |  LU_BRT_BLUE--------9 \r\n");
    printf(" LU_BLUE---------1  |  LU_BRT_GREEN------10 \r\n");
    printf(" LU_GREEN--------2  |  LU_BRT_CYAN-------11 \r\n");
    printf(" LU_CYAN---------3  |  LU_BRT_RED--------12 \r\n");
    printf(" LU_RED----------4  |  LU_BRT_VIOLET-----13 \r\n");
    printf(" LU_VIOLET-------5  |  LU_BRT_YELLOW-----14 \r\n");
    printf(" LU_YELLOW-------6  |  LU_BRT_WHITE------15 \r\n");
    printf(" LU_WHITE--------7  |  LU_BTNFACE--------16 \r\n");
    printf(" LU_GREY---------8  |  LU_SILVER---------17 \r\n");
    printf("Please select one color:");
    int  color_num = 0;
    while(1)
    {
        char color_chr = getch();
        printf("%c",color_chr);
        if ((color_chr == '\n')||(color_chr == '\r'))
            break;
        else if((color_chr>='0')&&(color_chr<='9'))
            color_num = color_num*10 + (int)color_chr - 48;
    }
    
    if(select=='b')
    {
        fb_set_bgcolor(color_num,1);
        fb_cons_clear();
        printf("Set background color successfully %d!!! \r\n",color_num);
    }
    else if(select=='f')
    {
        fb_set_fgcolor(color_num,1);
        fb_cons_clear();
        printf("Set frontground color successfully %d!!! \r\n",color_num);
    }
    
    //picture_soc_lock = 0;
    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
}

int set_color(void)
{
    set_color1 = rt_thread_create("set_color",
                            set_color_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(set_color1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(set_color, set color);

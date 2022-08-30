/*
 * set_bg_color.c
 *
 * created: 2022/4/13
 *  author: 
 */

#include "bsp.h"
#include <rtthread.h>

#define printf  printk
#define getch   usb_kbd_getc

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        50
static rt_thread_t set_color1 = RT_NULL;

void exit_set_color(void);
extern int change_scheduler_lock(int val);
extern int show_background;

int set_color_entry()
{
    change_scheduler_lock(1);
    fb_cons_clear();
    
    printf(" input 'b' to set background color... \r\n");
    printf(" input 'f' to set frontground color... \r\n");
    printf(" input 'm' to set mouse color... \r\n");
    
    char select = getch();
    if((select!='b')&&(select!='f')&&(select!='m'))
    {
        printf(" Select err !!! \r\n");
        exit_set_color();
        return -1;
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
    
    int b_color = get_bg_color();
    int f_color = get_fg_color();
    int m_color = usb_mouse_get_color();
    if((color_num==b_color)||(color_num==f_color)||(color_num==m_color))
    {
        printf("Failed to set color! Please set another color!!! \r\n");
        exit_set_color();
        return -1;
    }
    
    if(select=='b')
    {
        fb_set_bgcolor_mydef(color_num);
        fb_cons_clear();
        printf("Set background color successfully %d!!! \r\n",color_num);
    }
    else if(select=='f')
    {
        fb_set_fgcolor_mydef(color_num);
        fb_cons_clear();
        printf("Set frontground color successfully %d!!! \r\n",color_num);
    }
    else if(select=='m')
    {
        usb_mouse_set_color(color_num);
        printf("Set mouse color successfully %d!!! \r\n",color_num);
    }
    
    exit_set_color();
    return 0;
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

void exit_set_color(void)
{
    printf("Please enter key to exit.");
    getch();                                // 阻断退出
    show_background = 1;
    change_scheduler_lock(0);
}

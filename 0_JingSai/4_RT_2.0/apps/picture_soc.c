/*
 * picture_soc.c
 *
 * created: 2022/7/4
 *  author: 
 */

#if 0

#include <rtthread.h>
#include <stdio.h>
#include "bsp.h"
#include "ls1b.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"
#include "picture_soc.h"

#define printf  printk
#define getch   usb_kbd_getc
#define x_max   800
#define y_max   480
#define devide_lie      4   // 800/4 = 200
#define devide_hang     5   // 480/5 = 96

#define THREAD_PRIORITY         21
#define THREAD_STACK_SIZE       1024*8
#define THREAD_TIMESLICE        10
static rt_thread_t picture_soc1 = RT_NULL;

extern void fb_drawrect(int x1, int y1, int x2, int y2, unsigned coloridx);
extern void fb_put_string_center(int x, int y, char *str, unsigned coloridx);
void show_back_ground(void)
{
    int i,j,x1,x2,y1,y2;
    fb_cons_clear();                        // 清楚画布
    for(i=0;i<devide_lie;i++)
    {
        for(j=0;j<devide_hang;j++)
        {
            x1 = (x_max/devide_lie)*i;
            x2 = (x_max/devide_lie)*(i+1) - 1;
            y1 = (y_max/devide_hang)*j;
            y2 = (y_max/devide_hang)*(j+1) - 1;
            fb_drawrect( x1,  y1,  x2,  y2, 15);
        }
    }
    fb_put_string_center(100, 48, "set_color", 15);
    fb_put_string_center(300, 48, "read_disk", 15);
    fb_put_string_center(500, 48, "push_box", 15);
    fb_put_string_center(700, 48, "mouse_draw", 15);
}


int picture_soc_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    
    Mouse_logical_coordinates_reset();      // 画布初始化
    show_back_ground();

    extern int mouse_x;
    extern int mouse_y;

    extern int msh_exec(char *cmd, rt_size_t length);
    int ch = 0;
    picture_soc_lock = 0;
    while (ch != 4)                         // 按下滚轮退出
    {
        if (usb_mouse_testc())              // 检测是否有输入
        {
            ch = usb_mouse_getc();          // 读取鼠标输入，返回输入字符
            //printf(" %d \n",ch);          // ch=1鼠标左键，ch=2鼠标右键，ch=4鼠标滚轮
            if(ch == 1)
            {
                if(mouse_y<96)
                {
                    if(mouse_x<200)
                        msh_exec("set_color",9);
                    else if(mouse_x<400)
                        msh_exec("read_disk",9);
                    else if(mouse_x<600)
                        msh_exec("push_box",8);
                    else
                        msh_exec("mouse_draw",10);
                }
                else if(mouse_y<192) ;
                else if(mouse_y<288) ;
                else if(mouse_y<384) ;
                else ;
                while(picture_soc_lock == 1)        // 图形化操作系统下有程序运行
                {
                    rt_thread_sleep(1000);
                }
                show_back_ground();
            }
        }
        //delay_ms(5);
    }

    Mouse_logical_coordinates_reset();
    fb_cons_clear();
    change_scheduler_lock(0);
}

int picture_soc(void)
{
    picture_soc1 = rt_thread_create("picture_soc",
                            picture_soc_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(picture_soc1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(picture_soc, picture soc);
#endif

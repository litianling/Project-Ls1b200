/*
 * L610_Ctrl.c
 *
 * created: 2022/4/21
 *  author: 
 */
 
#include <rtthread.h>
#include "L610_include/TCP.h"
#include "bsp.h"

#define  getch  usb_kbd_getc
#define  cmd_max_len 40

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024        // 1024*2
#define THREAD_TIMESLICE        50
static rt_thread_t L610_Ctrl1 = RT_NULL;

char * gets_cmd_1(char *s)
{
    int i;
    for(i=0;i<cmd_max_len-3;i++)
    {
        *s = getch();
        if((*s=='\n')||(*s=='\r'))
        {
            printk("\n\r");
            break;
        }
        printk("%c",*s);
        s++;
    }
    *s++ = '\r';
    *s++ = '\n';
    *s = NULL;
    return s;
}

char * gets_cmd_2(char *s)
{
    int i;
    for(i=0;i<cmd_max_len-3;i++)
    {
        *s = getch();
        if((*s=='\n')||(*s=='\r'))
        {
            printk("\n\r");
            break;
        }
        printk("%c",*s);
        s++;
    }
    *s = NULL;
    return s;
}

void L610_Ctrl_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    fb_cons_clear();
    
    const char cmd[cmd_max_len];
    char *cmd1 = cmd;
    int tmo = 0;
    printk("Welcome to L610_Ctrl.\r\n");
    printk("Enter else key to exit.\r\n");
    while(1)
    {
        printk("Please choose cmd 1 have/ 2 no/3 end:");
        char mode = getch();
        printk("\r\n");
        printk("Please input cmd:");
        if(mode=='1')
        {
            gets_cmd_1(cmd1);
            if((*cmd=="\r")||(*cmd=="\n"))
                break;
            send_at_command(cmd);
        }
        else if(mode=='2')
        {
            gets_cmd_2(cmd1);
            send_at_command(cmd);
        }
        else if(mode=='3')
        {
            const char end = 0x1a;
            const char * end1 = &end;
            send_at_command(end1);
        }
        else
        {
            break;
        }
        wait_for_reply(NULL, "OK", 0);
    }

    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
    return;
}

int L610_Ctrl(void)
{
    L610_Ctrl1 = rt_thread_create("L610_Ctrl",
                            L610_Ctrl_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_Ctrl1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(L610_Ctrl, L610 Ctrl);

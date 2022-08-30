/*
 * LBS_Position.c
 *
 * created: 2022/5/23
 *  author: 
 */

#include <rtthread.h>
#include "L610_include/TCP.h"
#include "bsp.h"

#define  getch  usb_kbd_getc
#define  cmd_max_len 40

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024*2
#define THREAD_TIMESLICE        50
static rt_thread_t L610_Position1 = RT_NULL;

void L610_Position_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    fb_cons_clear();
    
    send_at_command("AT+GTSET=\"IPRFMT\",0\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("ATI\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+MIPCALL=1,\"CMNET\"\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    printk("Please select the mode: 1_GPRS 2_WIFIscan.\r\n");
    char mode = getch();
    if(mode == '1')
    {
        send_at_command("AT+GTGIS=6\r\n");
        wait_for_reply(NULL, "OK", 0);
        delay_ms(500);
        wait_for_reply(NULL, "OK", 0);
        delay_ms(500);
    }
    else if(mode == '2')
    {
        send_at_command("AT+GTGIS=7\r\n");
        wait_for_reply(NULL, "OK", 0);
        delay_ms(500);
        wait_for_reply(NULL, "OK", 2000);
        delay_ms(500);
    }

    printk("Please enter key to exit.");
    getch();                                // 阻断退出
    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
    return;
}

int L610_Position(void)
{
    L610_Position1 = rt_thread_create("L610_Position",
                            L610_Position_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_Position1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(L610_Position, L610 LBS Position);

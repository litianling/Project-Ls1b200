/*
 * L610_Check.c
 *
 * created: 2022/4/25
 *  author: 
 */

#include <rtthread.h>
#include "L610_include/TCP.h"
#include "bsp.h"

extern int usb_kbd_getc( void);
#define  getch  usb_kbd_getc
#define  cmd_max_len 40

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024*2
#define THREAD_TIMESLICE        50
static rt_thread_t L610_Check1 = RT_NULL;

void L610_Check_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    extern void fb_cons_clear(void);
    fb_cons_clear();

    printk("Welcome to L610_Check.\r\n");

    send_at_command("AT\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);
    
    send_at_command("ATI\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+CPIN?\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+CSQ?\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+CGREG?\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);
    
    send_at_command("AT+CEREG?\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+MIPCALL?\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+COPS?\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    printk("Please enter key to exit.\r\n");
    getch();                                // 阻断退出
    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
    return;
}

int L610_Check(void)
{
    L610_Check1 = rt_thread_create("L610_Check",
                            L610_Check_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_Check1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(L610_Check, L610 Check);

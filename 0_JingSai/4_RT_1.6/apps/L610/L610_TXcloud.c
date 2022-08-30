/*
 * L610_TXcloud.c
 *
 * created: 2022/4/30
 *  author: 
 */

#include <rtthread.h>
#include <string.h>
#include "bsp.h"
#include "ns16550.h"
#include "L610_TCP.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"

#define  getch  usb_kbd_getc
#define  cmd_max_len 200

#define THREAD_PRIORITY         20
#define THREAD_STACK_SIZE       1024*8
#define THREAD_TIMESLICE        500
static rt_thread_t L610_TXcloud1 = RT_NULL;
int wait_for_reply_cloud(const char *verstr, const char *endstr, int waitms);

int L610_TXcloud_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);

    send_at_command("AT+TCDEVINFOSET=1,\"MQ48NH81TC\",\"weilaiRoad_Lamp001\",\"AyPF0rmwJ+4zyEA0/aqpiQ==\"\r\n");      // 设置平台设备信息（产品ID/设备名称/密钥）
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+TCMQTTCONN=1,20000,240,1,1\r\n");                                               // 设置连接参数并连接
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+TCMQTTSUB=\"$thing/down/property/MQ48NH81TC/weilaiRoad_Lamp001\",1\r\n");       // 订阅上报下行属性标签
    wait_for_reply(NULL, "OK", 0);
    delay_ms(5000);

    send_at_command("AT+TCMQTTPUB=\"$thing/up/property/MQ48NH81TC/weilaiRoad_Lamp001\",1,\"{\"method\":\"report\",\"clientToken\":\"123\",\"params\":{\"power_switch\":0}}\"\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);


    while(1)
    {
        wait_for_reply_cloud(NULL, NULL, 2000);
        delay_ms(1000);
    }


    // never go here
    send_at_command("AT+TCMQTTDISCONN\r\n");                                                            // 断开连接
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    change_scheduler_lock(0);
    return 0;
}

int L610_TXcloud(void)
{
    L610_TXcloud1 = rt_thread_create("L610_TXcloud",
                            L610_TXcloud_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_TXcloud1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(L610_TXcloud, L610 ten xun yun);


//*******************************************************************************

int wait_for_reply_cloud(const char *verstr, const char *endstr, int waitms)
{
    char tmp[400], *ptr = tmp;
    int  rt = 0, tmo;

    memset(tmp, 0, 400);                            // 清除缓冲区, 方便数据观察
    tmo = waitms > 0 ? waitms : 1000;

    if ((endstr == NULL) || (strlen(endstr) == 0))  // 没有设置结束字符则全部接收
    {
        rt = ls1x_uart_read(devUART1, ptr, 400, tmo);
    }
    else
    {
        while (tmo > 0)
        {
            /*
             * 每次读10个字节, 超时2ms: 与UART1的速率相关.
             */
            int readed = ls1x_uart_read(devUART1, ptr, 10, 2);  // 10 bytes; 2 ms

            if (readed > 0)
            {
                ptr += readed;
                rt  += readed;

                if ((rt >= 400) || strstr(tmp, endstr))         // 包含结束字符串
                    break;
            }

            tmo -= 2;
        }
    }

    if (rt <= 2)                                                // 判断是不是超时退出
        return -1;

    /*
     * 接收字符总是以 "\r\n" 结束, 补充遗漏
     */
    if ((tmp[rt-2] != '\r') && (tmp[rt-1] != '\n'))
    {
        rt += ls1x_uart_read(devUART1, ptr, 10, 2);
    }

//    printk(" RX: %s\r\n", tmp);                                 // 输出接收到的信息******输出有长度限制，需要改进（想想磁盘读取）
//    printk("%d",rt);
    int offset=0;
    printk(" RX: ");
    printk("%s\r\n", tmp);
    if(*(tmp+126))
        printk("%s\r\n", tmp+126);
    if(*(tmp+252))
        printk("%s\r\n", tmp+252);
    if(*(tmp+378))
        printk("%s\r\n", tmp+378);

    if ((verstr != NULL) && (strlen(verstr) > 0))               // 有设置判断字符
    {
        if (strstr(tmp, verstr) != NULL)
            return rt;
        else
            return -1;
    }

    return rt;
}

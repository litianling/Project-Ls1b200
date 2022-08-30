/*
 * L610_app_TCP.c
 *
 * created: 2022/4/21
 *  author: 
 */
#include <rtthread.h>
#include <stdio.h>

#include "ls1b.h"
#include "mips.h"
#include "bsp.h"
#include "ns16550.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        50
static rt_thread_t L610_TCP1 = RT_NULL;

int L610_TCP_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);

    l610_init_pre();                        // 通过AT指令检查一堆参数设置
    int count = 0;
    while (count++ < 2)
    {
		l610_init();                        // 通过AT指令对模块各个部分进行初始化
		l610_check_socket();                // 查询当前可用的socket ID，socket ID 都可用说明当前没有 TCP 链接。 AT+MIPOPEN?
		l610_creat_tcp();                   // 通过socket创建TCP连接，AT+MIPOPEN=1,,\"47.92.117.163\",30000,0
		delay_ms(500);

        l610_send_data("a small dog");      // 返回 ASCII 编码
        l610_send_data("Hello Fibocom.");   // 返回 ASCII 编码
		l610_send_data("0123456789ABCDEF"); // 返回 ASCII 编码

		delay_ms(100);
		l610_end_tcp();                     // 关闭TCP连接，假如数据收发结束或者异常，尝试关闭到服务器的 TCP 连接，AT+MIPCLOSE=1
		l610_ip_release();                  // 释放IP，尝试释放模块本次激活后获取的 IP 地址 AT+MIPCALL=0
		delay_ms(500);
    }

    change_scheduler_lock(0);
    return 0;
}

int L610_TCP(void)
{
    L610_TCP1 = rt_thread_create("L610_TCP",
                            L610_TCP_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_TCP1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(L610_TCP, L610 TCP function);

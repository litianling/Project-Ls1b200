
/******************************************************************************
 *                       @ Copyright 2014 - 2020                              *
 *                           www.diysoon.com                                  *
 ******************************************************************************/
#include <string.h>
#include "ns16550.h"
#include "TCP.h"
#include "bsp.h"
//-----------------------------------------------------------------------------
// 以下两个函数实现: 龙芯1B与L610的通讯.
//-----------------------------------------------------------------------------

/*
 * 通过 UART1 向 L610 模块发送 AT 命令
 */
int send_at_command(const char *buf)
{
    int rt;
    rt = ls1x_uart_write(devUART1, buf, strlen(buf), NULL);     // 发送命令核心
    printk(" TX: %s\r\n", buf);                                // 回显发送的命令
    return rt;
}

int send_at_command_TXcloud(const char *buf)
{
    int rt;
    rt = ls1x_uart_write(devUART1, buf, strlen(buf), NULL);     // 发送命令核心
    return rt;
}
/*
 * 从 UART1 读取 L610 模块的应答信息
 *
 * 参数:    verstr:     检查接收的字符串中是否包含该字符串
 *          endstr:     接收的字符串以此结尾
 *          waitms:     接收等待超时
 *
 * 返回:    >0:         包含字符串 verstr
 *          <=0:        接收错误
 */
int wait_for_reply(const char *verstr, const char *endstr, int waitms)
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

    printk(" RX: %s\r\n", tmp);                                 // 输出接收到的信息******输出有长度限制，需要改进（想想磁盘读取）
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


int wait_for_reply_TXcloud(const char *verstr, const char *endstr, int waitms)
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

    
    if ((verstr != NULL) && (strlen(verstr) > 0))               // 有设置判断字符
    {
        if (strstr(tmp, verstr) != NULL)
            return rt;
        else
            return -1;
    }

    return rt;
}
//-----------------------------------------------------------------------------

/**
 * 请求运营商分配IP
 * 对应AT指令：AT+mipcall =
 * 运营商不同本条指令有区别
 */
void l610_ip_allo(void)
{
    int tmo = 0;

	send_at_command("AT+mipcall=1,\"ctnet\"\r\n");     // 请求运营商分配 IP
	wait_for_reply(NULL, "OK", 2000);
	send_at_command("AT+mipcall?\r\n");                // 检查是否分配IP
	
	while ((wait_for_reply("+MIPCALL: 1", "OK", 2000) <= 0) && (tmo++ < 5))
	{
        send_at_command("AT+mipcall=0\r\n");
        wait_for_reply(NULL, "OK", 2000);
        send_at_command("AT+mipcall=1,\"ctnet\"\r\n");
        wait_for_reply(NULL, "OK", 2000);
        send_at_command("AT+mipcall?\r\n");
    }
}

/**
 * 查询当前可用的socket ID
 * socket ID 都可用说明当前没有 TCP 链接。
 * 对应AT指令：AT+MIPOPEN?
 */
void l610_check_socket(void)
{
	send_at_command("AT+MIPOPEN?\r\n");
    wait_for_reply("+MIPOPEN:", "OK", 2000);
}

/**
 * 通过socket创建TCP连接
 * 对应AT指令：AT+MIPOPEN=1,,\"47.92.117.163\",30000,0
 */
void l610_creat_tcp(void)
{
	send_at_command("AT+MIPOPEN=1,,\"120.24.31.181\",3000,0\r\n"); // 创建TCP连接
    //send_at_command("AT+MIPOPEN=1,,\"10.2.55.18\",3000,0\r\n"); // 创建TCP连接
    wait_for_reply("+MIPOPEN: 1,1", "OK", 2000);
}

/**
 * 发送数据：在TCP连接建立后调用才有效，可以向服务器发送数据
 * 需要传入保存数据的数组
 * 对应AT指令：AT+MIPSEND
 */
void l610_send_data(char *data)
{
    char tmp[32];
    snprintf(tmp, 31, "AT+MIPSEND=1,%i\r\n", strlen(data));  // 将字符串导入tmp
    send_at_command(tmp);                                    // 将tmp发送到模块
    send_at_command(data);
    wait_for_reply(NULL, NULL, 1000);
}

/**
 * 关闭TCP连接
 * 假如数据收发结束或者异常，尝试关闭到服务器的 TCP 连接
 * 对应AT指令：AT+MIPCLOSE=1
 */
void l610_end_tcp(void)
{
	send_at_command("AT+MIPCLOSE=1\r\n");
    wait_for_reply("+MIPCLOSE: 1,0", "OK", 2000);
}

/**
 * 释放IP
 * 尝试释放模块本次激活后获取的 IP 地址
 * 对应AT指令：AT+MIPCALL=0
 */
void l610_ip_release(void)
{
	send_at_command("AT+MIPCALL=0\r\n");
    wait_for_reply("+MIPCALL: 0", "OK", 2000);
}

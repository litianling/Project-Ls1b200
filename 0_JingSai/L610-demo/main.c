/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B Bare Program, Sample main file
 */

#include <stdio.h>

#include "ls1b.h"
#include "mips.h"

//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"
#include "ns16550.h"

//-------------------------------------------------------------------------------------------------
// 主程序
//-------------------------------------------------------------------------------------------------

int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */
    ls1x_uart_open(devUART1, NULL);     /* 使用默认参数: 115200,8N1 */

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
    
    return 0;
}


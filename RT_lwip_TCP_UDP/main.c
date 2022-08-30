/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B RT-Thread Application
 */

#include <time.h>

#include "rtthread.h"

//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"

#ifdef BSP_USE_GMAC0
static unsigned char gmac0_ip[4] = {192, 168, 1, 123};
extern void ls1x_initialize_lwip(unsigned char *ip0, unsigned char *ip1);
#endif

//-------------------------------------------------------------------------------------------------
// Simple demo of task
//-------------------------------------------------------------------------------------------------
#include "lwip_test.h"

int main(int argc, char** argv)
{
	rt_kprintf("\r\nWelcome to RT-Thread.\r\n\r\n");

	#ifdef BSP_USE_GMAC0
	{
        ls1x_initialize_lwip(gmac0_ip, NULL);       // 初始化本地IP地址，参数（GMAC0，GAC1），参数为空时用默认地址

        #if (TEST_TCP_SERVER)
        {
            extern void tcp_server_init(void);
            tcp_server_init();                  // 开启tcp服务器
        }
        #endif

        #if (TEST_TCP_CLIENT)
        {
            extern void tcp_client_init(void);
            tcp_client_init();                  // 开启tcp客户端
        }
        #endif

        #if (TEST_UDP_SERVER)
        {
            extern void udp_server_init(void);
            udp_server_init();                  // 开启udp服务器
        }
        #endif

        #if (TEST_UDP_CLIENT)
        {
            extern void udp_client_init(void);
            udp_client_init();                  // 开启udp客户端
        }
        #endif

    }
	#endif

    return 0;

}

/*
 * @@ End
 */

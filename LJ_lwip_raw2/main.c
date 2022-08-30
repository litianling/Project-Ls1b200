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

#if BSP_USE_LWMEM
#include "libc/lwmem.h"
#endif

#ifdef USE_YAFFS2
#include "yaffs2/port/ls1x_yaffs.h"
#endif

#if defined(BSP_USE_GMAC0)
  #include "lwip/tcp_impl.h"
  #include "netif/etharp.h"
  #include "lwip/ip_frag.h"
  #include "lwip/netif.h"
  #include "lwip/init.h"
  extern struct netif *p_gmac0_netif;

  extern void lwip_init(void);
  extern void ls1x_initialize_lwip(unsigned char *lip, unsigned char *ip1);
  extern void ethernetif_input(struct netif *netif);
#endif

#include "lwip_test.h"

//-------------------------------------------------------------------------------------------------
// LWIP查询, 使用 timer
//-------------------------------------------------------------------------------------------------

#if defined(BSP_USE_RTC)

#include "ls1x_rtc.h"

//#define TCP_TMR
#ifdef TCP_TMR

static volatile int m_tcp_timer_flag = 0;

static void rtc1_callback(int device, unsigned match, int *stop)
{
    m_tcp_timer_flag = 1;
}

#endif

static void ls1x_start_lwip_timer(void)
{
    rtc_cfg_t cfg;

    ls1x_rtc_init(NULL, NULL);          /* 必须先初始化RTC */

    cfg.trig_datetime = NULL;
    cfg.isr = NULL;

#ifdef ETHERNETIF_INPUT_TMR
    /*
     * DEVICE_RTCMATCH0 用于 ethernetif_input 定时检测
     */
    cfg.interval_ms = 10;
    cfg.cb = rtc0_callback;
    ls1x_rtc_timer_start(DEVICE_RTCMATCH0, &cfg);
#endif

#ifdef TCP_TMR
    /*
     * DEVICE_RTCMATCH1 用于 tcp_tmr 定时检测
     */
    cfg.interval_ms = TCP_TMR_INTERVAL;
    cfg.cb = rtc1_callback;
    ls1x_rtc_timer_start(DEVICE_RTCMATCH1, &cfg);
#endif
}

#endif

//-------------------------------------------------------------------------------------------------
// 主程序
//-------------------------------------------------------------------------------------------------

static int tcpclient_count = 0;
 
int main(void)
{
    int rdbytes, wrbytes;
   
    printk("\r\nmain() function.\r\n");

    /*
     * 检测 GMAC 1588: 没有
     */
    #if 0
    {
        unsigned int tmp;
        
        tmp = READ_REG32(LS1B_GMAC0_BASE + 0x0700);
        printk("gmac time stamp control register = 0x%08X\r\n", tmp);
        
        tmp = READ_REG32(LS1B_GMAC0_BASE + 0x0718);
        printk("gmac Target Time Low register = 0x%08X\r\n", tmp);
    }
    #endif

    #if BSP_USE_LWMEM
        lwmem_initialize(0);
    #endif

    #ifdef USE_YAFFS2
        yaffs_startup_and_mount(RYFS_MOUNTED_FS_NAME);
    #endif
    
	#if defined(BSP_USE_GMAC0)
	{
	    unsigned char lip[4] = {192, 168, 1, 123};
	    unsigned char rip[4] = {192, 168, 1, 111};

        lwip_init();                        // Initilaize the LwIP stack
        ls1x_initialize_lwip(lip, NULL);    // Initilaize the LwIP & GMAC0 glue

        #if TEST_TCP_SERVER
            tcp_server_initialize(lip);
        #endif

        #if TEST_TCP_CLIENT
            tcp_client_initialize(lip, rip);
        #endif

        #if TEST_UDP_SERVER
            udp_server_initialize(lip);
        #endif

        #if TEST_UDP_CLIENT
            udp_client_initialize(lip, rip);
        #endif
        
        #if (TEST_TCP_SERVER || TEST_TCP_CLIENT) && (defined(TCP_TMR))
            ls1x_start_lwip_timer();
        #endif
        
        
        ftpd_init();
        
    }
	#endif

    /*
     * 裸机主循环
     */
    for (;;)
    {

		#if defined(BSP_USE_GMAC0)
        {
            #ifndef ETHERNETIF_INPUT_TMR
        	    ethernetif_input(p_gmac0_netif);
            #endif

            #if (TEST_TCP_SERVER || TEST_TCP_CLIENT)
                #ifdef TCP_TMR
                if (m_tcp_timer_flag)
                {
                    tcp_tmr();
                    m_tcp_timer_flag = 0;
                }
                #else
                    tcp_tmr();          // lwip tcp 裸机编程必须调用函数 tcp_tmr()
                #endif
            
            #endif

            #if TEST_TCP_SERVER
            {
                char tmpbuf[TCP_SERVER_BUFSIZE];
                
                memset(tmpbuf, 0, TCP_SERVER_BUFSIZE);

                if (tcpsvr_recv_data(tmpbuf, TCP_SERVER_BUFSIZE) > 0)
                {
                    printk("FROM CLIENT: %s\r\n", tmpbuf);
                    
                    sprintf(tmpbuf, "REPLAY: tick count=%i\r\n", get_clock_ticks());
                    wrbytes = strlen(tmpbuf);
                    tcpsvr_send_data(tmpbuf, wrbytes);
                }
            }
            #endif

            #if TEST_TCP_CLIENT
            {
                char tmpbuf[TCP_CLIENT_BUFSIZE];

                if (tcpclient_count < 30)
                {
                    /*
                     * 本次发送...
                     */
                    memset(tmpbuf, 0, TCP_CLIENT_BUFSIZE);
                    sprintf(tmpbuf, "1B tick count=%i\r\n", get_clock_ticks());
                    wrbytes = strlen(tmpbuf);
                    wrbytes = tcpcli_send_data(tmpbuf, wrbytes);
                    // printk("SEND: %s", tmpbuf);

                #ifndef TCP_TMR
                    delay_ms(100);
                #endif
                
                    /*
                     * 上一次发送的回答
                     */
                    memset(tmpbuf, 0, TCP_CLIENT_BUFSIZE);
                    rdbytes = tcpcli_recv_data(tmpbuf, TCP_CLIENT_BUFSIZE);
                    if (rdbytes > 0)
                    {
                        tcpclient_count++;
                        printk("SERVER REPLAY: %s\r\n", tmpbuf);
                    }

                }
                else
                {
                    tcpcli_disconnect();
                }
            }
            #endif
        
            #if TEST_UDP_SERVER
            {
                char tmpbuf[UDP_SERVER_BUFSIZE];
                
                memset(tmpbuf, 0, UDP_SERVER_BUFSIZE);
                
                if (udpsvr_recv_data(tmpbuf, UDP_SERVER_BUFSIZE) > 0)
                {
                    printk("RECV: %s\r\n", tmpbuf);
                    
                    sprintf(tmpbuf, "REPLAY: tick count=%i\r\n", get_clock_ticks());
                    wrbytes = strlen(tmpbuf);
                    udpsvr_send_data(tmpbuf, wrbytes);
                }
            }
            #endif

            #if TEST_UDP_CLIENT
            {
                char tmpbuf[UDP_CLIENT_BUFSIZE];
                
                memset(tmpbuf, 0, UDP_CLIENT_BUFSIZE);
                
                sprintf(tmpbuf, "tick count=%i", get_clock_ticks());
                wrbytes = strlen(tmpbuf);
                wrbytes = udpcli_send_data(tmpbuf, wrbytes);
                //printk("SEND: %s", udpcli_tx_buf);

                delay_ms(250);

                rdbytes = udpcli_recv_data(tmpbuf, UDP_CLIENT_BUFSIZE);
                if (rdbytes > 0)
                {
                    printk("RECV: %s\r\n", tmpbuf);
                }

            }
            #endif

        }
		#endif

        delay_ms(100);
        
        tcp_tmr();
    }

    /*
     * Never goto here!
     */
    return 0;
}

/*
 * @@ End
 */


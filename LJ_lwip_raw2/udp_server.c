/*
 * udp_server.c
 *
 * created: 2021/6/24
 *  author: 
 */

#include "lwip_test.h"

#if (TEST_UDP_SERVER > 0)

#include "lwip/udp.h"

//---------------------------------------------------------------------------------------

static struct udp_pcb *m_udpsvr_pcb = NULL;         // 定义一个UDP的协议控制块

static unsigned int m_udpsvr_flag = 0;

static char udpsvr_rx_buf[UDP_SERVER_BUFSIZE];      // 定义用来发送数据的缓存
static char udpsvr_tx_buf[UDP_SERVER_BUFSIZE];      // 定义用来接收数据的缓存

//---------------------------------------------------------------------------------------

/*
 * 接收到数据包将要调用的函数
 */
static void udp_server_recv_callback(void *arg,
                                     struct udp_pcb *upcb,
                                     struct pbuf *p,
                                     struct ip_addr *addr,
                                     uint16_t port)
{
    if (p != NULL)
    {
        if ((p->tot_len) >= UDP_SERVER_BUFSIZE)         // 如果收的的数据大于缓存
        { 
            memcpy(udpsvr_rx_buf, p->payload, UDP_SERVER_BUFSIZE);
            udpsvr_rx_buf[UDP_SERVER_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(udpsvr_rx_buf, p->payload, p->tot_len);
            udpsvr_rx_buf[p->tot_len] = 0;
        }
        
        m_udpsvr_flag |= LWIP_NEW_DATA;                 // 收到了新的数据
        m_udpsvr_pcb->remote_ip = *addr;                // 记录远程主机的IP和端口号
        m_udpsvr_pcb->remote_port = port;
        pbuf_free(p);
    }
}

/*
 * 发送数据
 */
static void udp_server_send_data(void)
{
    err_t err;
    struct pbuf *tmpbuf;
    
    if ((m_udpsvr_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA)
    {
        tmpbuf = pbuf_alloc(PBUF_TRANSPORT, strlen(udpsvr_tx_buf), PBUF_RAM);
        tmpbuf->payload = udpsvr_tx_buf;
        err = udp_send(m_udpsvr_pcb, tmpbuf);           // 发送数据
        if (err != ERR_OK)
        {
            //do something
        }
        
        m_udpsvr_flag &= ~LWIP_SEND_DATA;               // 清除发送数据的标志
        pbuf_free(tmpbuf);
    }
}

/*
 * 初始化UDP服务器
 */
void udp_server_initialize(unsigned char *lip)
{
    err_t err;
    ip_addr_t local_ip;

    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);
    
    m_udpsvr_pcb = udp_new();                           // 新建一个UDP协议控制块
    if (m_udpsvr_pcb != NULL)
    {
        err = udp_bind(m_udpsvr_pcb, &local_ip/*IP_ADDR_ANY*/, UDP_LOCAL_PORT);

        if (err == ERR_OK)
        {
            udp_recv(m_udpsvr_pcb, udp_server_recv_callback, NULL); // 注册接收回调函数
            printk("udp server started successful.\r\n");
        }
        else
        {
            udp_remove(m_udpsvr_pcb);                   // 删除控制块
            printk("can not bind pcb of udp server\r\n");
        }
    }
}

//---------------------------------------------------------------------------------------

int udpsvr_recv_data(unsigned char *buf, int buflen)
{
    if ((m_udpsvr_flag & LWIP_NEW_DATA) == LWIP_NEW_DATA)
    {
        int thislen = strlen(udpsvr_rx_buf);
        thislen = thislen < buflen ? thislen : buflen;
        memcpy(buf, udpsvr_rx_buf, thislen);
        buf[thislen] = 0;
        m_udpsvr_flag &= ~LWIP_NEW_DATA;        // 清除接受数据的标志
        return thislen;
    }
    
    return 0;
}

int udpsvr_send_data(unsigned char *buf, int buflen)
{
    int thislen = buflen < UDP_SERVER_BUFSIZE-1 ? buflen : UDP_SERVER_BUFSIZE-1;

    memcpy(udpsvr_tx_buf, buf, thislen);
    udpsvr_tx_buf[thislen] = 0;
    m_udpsvr_flag |= LWIP_SEND_DATA;            // 标记有数据需要发送
    udp_server_send_data();
    
    return thislen;
}

#endif // #if TEST_UDP_SERVER



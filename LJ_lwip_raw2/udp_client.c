/*
 * udp_client.c
 *
 * created: 2021/6/24
 *  author: 
 */
 
#include "lwip_test.h"

#if (TEST_UDP_CLIENT > 0)

#include "lwip/udp.h"

//---------------------------------------------------------------------------------------

static struct udp_pcb *m_udpcli_pcb = NULL;         // 定义一个UDP的协议控制块

static unsigned int m_udpcli_flag = 0;

static char udpcli_rx_buf[UDP_CLIENT_BUFSIZE];      // 定义用来发送数据的缓存
static char udpcli_tx_buf[UDP_CLIENT_BUFSIZE];      // 定义用来接收数据的缓存

//---------------------------------------------------------------------------------------

/*
 * 接收到数据包将要调用的函数
 */
static void udp_client_recv_callback(void *arg,
                                     struct udp_pcb *upcb,
                                     struct pbuf *p,
                                     struct ip_addr *addr,
                                     uint16_t port)
{
    if (p != NULL)
    {
        if ((p->tot_len) >= UDP_CLIENT_BUFSIZE)         // 如果收的的数据大于缓存
        {      
            memcpy(udpcli_rx_buf, p->payload, UDP_CLIENT_BUFSIZE);
            udpcli_rx_buf[UDP_CLIENT_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(udpcli_rx_buf, p->payload, p->tot_len);
            udpcli_rx_buf[p->tot_len] = 0;
        }
        
        m_udpcli_flag |= LWIP_NEW_DATA;                 // 收到了新的数据
        pbuf_free(p);
    }
}

/*
 * 发送数据
 */
static void udp_client_send_data(void)
{
    err_t err;
    struct pbuf *tmpbuf;

    if ((m_udpcli_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA)
    {
        int thislen = strlen((char *)udpcli_tx_buf);

        tmpbuf = pbuf_alloc(PBUF_TRANSPORT, thislen, PBUF_RAM);
        tmpbuf->payload = udpcli_tx_buf;

        err = udp_send(m_udpcli_pcb, tmpbuf);           // 发送数据
        if (err != ERR_OK)
        {
            // lwip_log("UDP SERVER发送数据失败!");
        }
        
        m_udpcli_flag &= ~LWIP_SEND_DATA;               // 清除发送数据的标志
        pbuf_free(tmpbuf);
    }
}

/*
 * 初始化UDP客户端
 */
void udp_client_initialize(unsigned char *lip, unsigned char *rip)
{
    err_t err;
    ip_addr_t local_ip, remote_ip;

    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);
    IP4_ADDR(&remote_ip, rip[0], rip[1], rip[2], rip[3]);

    m_udpcli_pcb = udp_new();                           // 新建一个UDP协议控制块
    if (m_udpcli_pcb != NULL)
    {
        udp_bind(m_udpcli_pcb, &local_ip, UDP_LOCAL_PORT);
        err = udp_connect(m_udpcli_pcb, &remote_ip, UDP_REMOTE_PORT);   // 设置连接到远程主机
        if (err == ERR_OK)
        {
            udp_recv(m_udpcli_pcb, udp_client_recv_callback, NULL);     // 注册接收回调函数
        }
        else
        {
            udp_remove(m_udpcli_pcb);
        }
    }
}

//---------------------------------------------------------------------------------------

int udpcli_send_data(unsigned char *buf, int buflen)
{
    int thislen = buflen < UDP_CLIENT_BUFSIZE-1 ? buflen : UDP_CLIENT_BUFSIZE-1;
    memcpy(udpcli_tx_buf, buf, thislen);
    udpcli_tx_buf[thislen] = 0;
    m_udpcli_flag |= LWIP_SEND_DATA;            // 标记有数据需要发送
    udp_client_send_data();

    return thislen;
}

int udpcli_recv_data(unsigned char *buf, int buflen)
{
    if ((m_udpcli_flag & LWIP_NEW_DATA) == LWIP_NEW_DATA)
    {
        int thislen = strlen(udpcli_rx_buf);
        thislen = thislen < buflen ? thislen : buflen;
        memcpy(buf, udpcli_rx_buf, thislen);
        buf[thislen] = 0;
        m_udpcli_flag &= ~LWIP_NEW_DATA;        // 清除接收数据的标志
        return thislen;
    }

    return 0;
}

#endif // #if TEST_UDP_CLIENT



/*
 * tcp_client.c
 *
 * created: 2021/6/24
 *  author: 
 */

#include "lwip_test.h"

#if (TEST_TCP_CLIENT > 0)

#include "lwip/tcp.h"

//---------------------------------------------------------------------------------------

static mytcp_state_t *m_tcpcli_state = NULL;

static struct tcp_pcb *m_tcpcli_pcb = NULL;         // 定义一个TCP的协议控制块

static unsigned int m_tcpcli_flag = 0;

static char tcpcli_rx_buf[TCP_CLIENT_BUFSIZE];      // 定义用来接收数据的缓存
static char tcpcli_tx_buf[TCP_CLIENT_BUFSIZE];      // 定义用来发送数据的缓存

//---------------------------------------------------------------------------------------

static void tcp_client_close(struct tcp_pcb *tpcb, mytcp_state_t *ts);

//---------------------------------------------------------------------------------------

/*
 * 客户端成功连接到远程主机时调用
 */
static const char *respond =  "tcp client connect success\r\n";

static err_t tcp_client_connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    mytcp_state_t *ts = (mytcp_state_t *)arg;
    
    ts->state = MYTCP_STATE_RECVDATA;                   // 可以开始接收数据了
    m_tcpcli_flag |= LWIP_CONNECTED;                    // 标记连接成功了
    tcp_write(tpcb, respond, strlen(respond), 1);       // 回应信息
    return ERR_OK;
}

/*
 * 连接轮询时将要调用的函数
 */
static err_t tcp_client_poll_callback(void *arg, struct tcp_pcb *tpcb)
{
    err_t rt = ERR_OK;
    mytcp_state_t *ts = (mytcp_state_t *)arg;

    if (ts != NULL)                                     // 连接处于空闲可以发送数据
    {
        if ((m_tcpcli_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA)
        {
            tcp_write(tpcb, tcpcli_tx_buf, strlen(tcpcli_tx_buf), 1);
            m_tcpcli_flag &= ~LWIP_SEND_DATA;           // 清除发送数据的标志
        }
    }
    else
    {
        tcp_abort(tpcb);
        rt = ERR_ABRT;
    }

    return rt;
}

/*
 * 客户端接收到数据之后将要调用的函数
 */
static err_t tcp_client_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    err_t rt = ERR_OK;
    mytcp_state_t *ts = (mytcp_state_t *)arg;

    if (p == NULL)
    {
        ts->state = MYTCP_STATE_CLOSED;                 // 连接关闭了
        tcp_client_close(tpcb, ts);
        m_tcpcli_flag &= ~LWIP_CONNECTED;               // 清除连接标志
    }
    else if (err != ERR_OK)
    {   
        if (p != NULL)                                  // 位置错误释放pbuf
        {
            pbuf_free(p);
        }
        rt = err;
    }
    else if (ts->state == MYTCP_STATE_RECVDATA)         // 连接收到了新的数据
    {
        if ((p->tot_len) >= TCP_CLIENT_BUFSIZE)         // 如果收的的数据大于缓存
        {  
            memcpy(tcpcli_rx_buf, p->payload, TCP_CLIENT_BUFSIZE);
            tcpcli_rx_buf[TCP_CLIENT_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(tcpcli_rx_buf, p->payload, p->tot_len);
            tcpcli_rx_buf[p->tot_len] = 0;
        }
        
        m_tcpcli_flag |= LWIP_NEW_DATA;                 // 收到了新的数据
        tcp_recved(tpcb, p->tot_len);                   // 用于获取接收数据的长度, 表示可以获取更多的数据
        pbuf_free(p);                                   // 释放内存
    }
    else if (ts->state == MYTCP_STATE_CLOSED)           // 服务器关闭了
    {
        tcp_recved(tpcb, p->tot_len);                   // 远程端口关闭两次, 垃圾数据
        pbuf_free(p);
    }
    else                                                // 其他未知状态
    {
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    }
    
    return rt;
}

/*
 * 关闭连接
 */
static void tcp_client_close(struct tcp_pcb *tpcb, mytcp_state_t *ts)
{
    tcp_arg(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);

    if (ts != NULL)
    {
        mem_free(ts);
    }

    tcp_close(tpcb);

    m_tcpcli_pcb = NULL;
    m_tcpcli_state = NULL;
}

/*
 * 初始化TCP客户端
 */
void tcp_client_initialize(unsigned char *lip, unsigned char *rip)
{
    err_t err;
    ip_addr_t local_ip, remote_ip;
    
    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);
    IP4_ADDR(&remote_ip, rip[0], rip[1], rip[2], rip[3]);

    m_tcpcli_pcb = tcp_new();                           // 新建一个PCB

    if (m_tcpcli_pcb != NULL)
    {
        m_tcpcli_state = mem_malloc(sizeof(mytcp_state_t));

        tcp_arg(m_tcpcli_pcb, m_tcpcli_state);          // 将程序的协议控制块的状态传递给回调函数

        tcp_bind(m_tcpcli_pcb, &local_ip, TCP_LOCAL_PORT);

        /*
         * 设定TCP的回调函数
         */
        err = tcp_connect(m_tcpcli_pcb, &remote_ip, TCP_REMOTE_PORT, tcp_client_connect_callback);
        if (err == ERR_OK)
        {
            tcp_recv(m_tcpcli_pcb, tcp_client_recv_callback);      // 接收到新的数据的回调函数
            tcp_poll(m_tcpcli_pcb, tcp_client_poll_callback, 0);   // 轮询时调用的回调函数
            printk("tcp client connect to server successful!");
        }
        else
        {
            tcp_client_close(m_tcpcli_pcb, m_tcpcli_state);
            printk("tcp client connect to server fail!");
        }
    }
}

/*
 * 用于连接远程主机
 */
void tcp_client_connect_remotehost(unsigned char *lip, unsigned char *rip)
{
    /*
     * 记住如果此处需要频繁重连的时候记得先关闭已经申请的tcb, 最好将tcb换成全局变量
     */
    // tcp_client_close();
    tcp_client_initialize(lip, rip);
}

void tcpcli_disconnect(void)
{
    if (m_tcpcli_pcb != NULL)
    {
        tcp_client_close(m_tcpcli_pcb, m_tcpcli_state);
    }
}

//---------------------------------------------------------------------------------------

int tcpcli_recv_data(unsigned char *buf, int buflen)
{
    if ((m_tcpcli_flag & LWIP_NEW_DATA) == LWIP_NEW_DATA)
    {
        int thislen = strlen(tcpcli_rx_buf);
        thislen = thislen < buflen ? thislen : buflen;
        memcpy(buf, tcpcli_rx_buf, thislen);
        buf[thislen] = 0;
        m_tcpcli_flag &= ~LWIP_NEW_DATA;        // 清除接受数据的标志
        
        return thislen;
    }

    return 0;
}

int tcpcli_send_data(unsigned char *buf, int buflen)
{
    int thislen = buflen < TCP_CLIENT_BUFSIZE-1 ? buflen : TCP_CLIENT_BUFSIZE-1;
    memcpy(tcpcli_tx_buf, buf, thislen);
    tcpcli_tx_buf[thislen] = 0;
    m_tcpcli_flag |= LWIP_SEND_DATA;            // 标记有数据需要发送

    return thislen;
}

#endif // #if TEST_TCP_CLIENT



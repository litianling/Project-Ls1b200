/*
 * tcp_server.c
 *
 * created: 2021/6/24
 *  author: 
 */

#include "lwip_test.h"

#if (TEST_TCP_SERVER > 0)

#include "lwip/tcp.h"

//---------------------------------------------------------------------------------------

static struct tcp_pcb *m_tcpsvr_pcb = NULL;         // 定义一个TCP的协议控制块

static unsigned int m_tcpsvr_flag = 0;

static char tcpsvr_rx_buf[TCP_SERVER_BUFSIZE];      // 定义用来接收数据的缓存
static char tcpsvr_tx_buf[TCP_SERVER_BUFSIZE];      // 定义用来发送数据的缓存

//---------------------------------------------------------------------------------------

static void tcp_server_close(struct tcp_pcb *tpcb, mytcp_state_t *ts);

//---------------------------------------------------------------------------------------

/*
 * 连接轮询时将要调用的函数
 */
static err_t tcp_server_poll_callback(void *arg, struct tcp_pcb *tpcb)
{
    err_t rt = ERR_OK;

    if (arg != NULL)
    {     
        /* 连接处于空闲可以发送数据
         */
        if ((m_tcpsvr_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA)
        {
            tcp_write(tpcb, tcpsvr_tx_buf, strlen(tcpsvr_tx_buf), 1);
            m_tcpsvr_flag &= ~LWIP_SEND_DATA;       // 清除发送数据的标志
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
 * 服务器接收到数据之后将要调用的函数
 */
static err_t tcp_server_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    err_t rt = ERR_OK;
    mytcp_state_t *ts = (mytcp_state_t *)arg;       // TCP PCB状态

    if (p == NULL)
    {
        ts->state = MYTCP_STATE_CLOSED;             // 连接关闭了
        tcp_server_close(tpcb, ts);
        m_tcpsvr_flag &= ~LWIP_CONNECTED;           // 清除连接标志
    }
    else if (err != ERR_OK)
    {  
        if (p != NULL)                              // 未知错误, 释放pbuf
        {
            pbuf_free(p);
        }
        
        rt = err;                                   // 得到错误
    }
    else if (ts->state == MYTCP_STATE_RECVDATA)     // 连接收到了新的数据
    {
        if ((p->tot_len) >= TCP_SERVER_BUFSIZE)     // 如果收的的数据大于缓存
        {       
            memcpy(tcpsvr_rx_buf, p->payload, TCP_SERVER_BUFSIZE);
            tcpsvr_rx_buf[TCP_SERVER_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(tcpsvr_rx_buf, p->payload, p->tot_len);
            tcpsvr_rx_buf[p->tot_len] = 0;
        }

        m_tcpsvr_flag |= LWIP_NEW_DATA;             // 收到了新的数据

        tcp_recved(tpcb, p->tot_len);               // 用于获取接收数据的长度, 通知LWIP已经读取了数据, 可以获取更多的数据
        pbuf_free(p);                               // 释放内存
    }
    else if (ts->state == MYTCP_STATE_CLOSED)       // 服务器关闭了
    {    
        tcp_recved(tpcb, p->tot_len);               // 远程端口关闭两次, 垃圾数据
        pbuf_free(p);
    }
    else
    {                                               // 其他未知状态
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    }
    
    return rt;
}

/*
 * 连接出错将要调用的函数
 */
static void tcp_server_error_callback(void *arg, err_t err)
{
    if (arg != NULL)
    {
        mem_free(arg);
    }
}

/*
 * 服务器连接成功后将要调用的函数
 */
static const char *respond = "tcp server connect ok!\r\n";

static err_t tcp_server_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    err_t rt;
    mytcp_state_t *ts;

    ts = mem_malloc(sizeof(mytcp_state_t));                 // 申请内存

    if (ts != NULL)
    {
        ts->state = MYTCP_STATE_RECVDATA;                   // 可以接收数据了

        m_tcpsvr_flag |= LWIP_CONNECTED;                    // 已经连接上了
        tcp_write(newpcb, respond, strlen(respond), 1);     // 回应信息

        tcp_arg(newpcb, ts);                                // 将程序的协议控制块的状态传递给回调函数

        tcp_recv(newpcb, tcp_server_recv_callback);         // 接收到新数据的回调函数
        tcp_err(newpcb, tcp_server_error_callback);         // 连接出错的回调函数
        tcp_poll(newpcb, tcp_server_poll_callback, 0);      // 轮询时调用的回调函数
        rt = ERR_OK;
    }
    else
    {
        rt = ERR_MEM;
    }

    return rt;
}

/*
 * 关闭连接
 */
static void tcp_server_close(struct tcp_pcb *tpcb, mytcp_state_t *ts)
{
    if (ts != NULL)
    {
        mem_free(ts);
    }

    tcp_close(tpcb);
}

//---------------------------------------------------------------------------------------

/*
 * 初始化LWIP服务器
 */
void tcp_server_initialize(unsigned char *lip)
{
    err_t err;                                              // LWIP错误信息
    ip_addr_t local_ip;
    
    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);

    m_tcpsvr_pcb = tcp_new();                               // 新建一个TCP协议控制块
    if (m_tcpsvr_pcb != NULL)
    {
        /* 绑定本地所有IP地址和端口号,作为服务器不需要知道客户端的IP
         */
        err = tcp_bind(m_tcpsvr_pcb, &local_ip, TCP_LOCAL_PORT);
        if (err == ERR_OK)
        {
            m_tcpsvr_pcb = tcp_listen(m_tcpsvr_pcb);        // 开始监听端口

            /*
             * 指定监听状态的连接联通之后将要调用的回调函数
             */
            tcp_accept(m_tcpsvr_pcb, tcp_server_accept_callback);
        }
    }
}

//---------------------------------------------------------------------------------------

int tcpsvr_recv_data(unsigned char *buf, int buflen)
{
    if ((m_tcpsvr_flag & LWIP_NEW_DATA) == LWIP_NEW_DATA)
    {
        int thislen = strlen(tcpsvr_rx_buf);
        thislen = thislen < buflen ? thislen : buflen;
        memcpy(buf, tcpsvr_rx_buf, thislen);
        buf[thislen] = 0;
        m_tcpsvr_flag &= ~LWIP_NEW_DATA;        // 清除接受数据的标志
        return thislen;
    }

    return 0;
}

int tcpsvr_send_data(unsigned char *buf, int buflen)
{
    int thislen = buflen < TCP_SERVER_BUFSIZE-1 ? buflen : TCP_SERVER_BUFSIZE-1;
    memcpy(tcpsvr_tx_buf, buf, thislen);
    tcpsvr_tx_buf[thislen] = 0;
    m_tcpsvr_flag |= LWIP_SEND_DATA;            // 标记有数据需要发送

    return thislen;
}

#endif // #if TEST_TCP_SERVER



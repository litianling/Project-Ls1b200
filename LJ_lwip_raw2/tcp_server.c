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

static struct tcp_pcb *m_tcpsvr_pcb = NULL;         // ����һ��TCP��Э����ƿ�

static unsigned int m_tcpsvr_flag = 0;

static char tcpsvr_rx_buf[TCP_SERVER_BUFSIZE];      // ���������������ݵĻ���
static char tcpsvr_tx_buf[TCP_SERVER_BUFSIZE];      // ���������������ݵĻ���

//---------------------------------------------------------------------------------------

static void tcp_server_close(struct tcp_pcb *tpcb, mytcp_state_t *ts);

//---------------------------------------------------------------------------------------

/*
 * ������ѯʱ��Ҫ���õĺ���
 */
static err_t tcp_server_poll_callback(void *arg, struct tcp_pcb *tpcb)
{
    err_t rt = ERR_OK;

    if (arg != NULL)
    {     
        /* ���Ӵ��ڿ��п��Է�������
         */
        if ((m_tcpsvr_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA)
        {
            tcp_write(tpcb, tcpsvr_tx_buf, strlen(tcpsvr_tx_buf), 1);
            m_tcpsvr_flag &= ~LWIP_SEND_DATA;       // ����������ݵı�־
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
 * ���������յ�����֮��Ҫ���õĺ���
 */
static err_t tcp_server_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    err_t rt = ERR_OK;
    mytcp_state_t *ts = (mytcp_state_t *)arg;       // TCP PCB״̬

    if (p == NULL)
    {
        ts->state = MYTCP_STATE_CLOSED;             // ���ӹر���
        tcp_server_close(tpcb, ts);
        m_tcpsvr_flag &= ~LWIP_CONNECTED;           // ������ӱ�־
    }
    else if (err != ERR_OK)
    {  
        if (p != NULL)                              // δ֪����, �ͷ�pbuf
        {
            pbuf_free(p);
        }
        
        rt = err;                                   // �õ�����
    }
    else if (ts->state == MYTCP_STATE_RECVDATA)     // �����յ����µ�����
    {
        if ((p->tot_len) >= TCP_SERVER_BUFSIZE)     // ����յĵ����ݴ��ڻ���
        {       
            memcpy(tcpsvr_rx_buf, p->payload, TCP_SERVER_BUFSIZE);
            tcpsvr_rx_buf[TCP_SERVER_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(tcpsvr_rx_buf, p->payload, p->tot_len);
            tcpsvr_rx_buf[p->tot_len] = 0;
        }

        m_tcpsvr_flag |= LWIP_NEW_DATA;             // �յ����µ�����

        tcp_recved(tpcb, p->tot_len);               // ���ڻ�ȡ�������ݵĳ���, ֪ͨLWIP�Ѿ���ȡ������, ���Ի�ȡ���������
        pbuf_free(p);                               // �ͷ��ڴ�
    }
    else if (ts->state == MYTCP_STATE_CLOSED)       // �������ر���
    {    
        tcp_recved(tpcb, p->tot_len);               // Զ�̶˿ڹر�����, ��������
        pbuf_free(p);
    }
    else
    {                                               // ����δ֪״̬
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    }
    
    return rt;
}

/*
 * ���ӳ���Ҫ���õĺ���
 */
static void tcp_server_error_callback(void *arg, err_t err)
{
    if (arg != NULL)
    {
        mem_free(arg);
    }
}

/*
 * ���������ӳɹ���Ҫ���õĺ���
 */
static const char *respond = "tcp server connect ok!\r\n";

static err_t tcp_server_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    err_t rt;
    mytcp_state_t *ts;

    ts = mem_malloc(sizeof(mytcp_state_t));                 // �����ڴ�

    if (ts != NULL)
    {
        ts->state = MYTCP_STATE_RECVDATA;                   // ���Խ���������

        m_tcpsvr_flag |= LWIP_CONNECTED;                    // �Ѿ���������
        tcp_write(newpcb, respond, strlen(respond), 1);     // ��Ӧ��Ϣ

        tcp_arg(newpcb, ts);                                // �������Э����ƿ��״̬���ݸ��ص�����

        tcp_recv(newpcb, tcp_server_recv_callback);         // ���յ������ݵĻص�����
        tcp_err(newpcb, tcp_server_error_callback);         // ���ӳ���Ļص�����
        tcp_poll(newpcb, tcp_server_poll_callback, 0);      // ��ѯʱ���õĻص�����
        rt = ERR_OK;
    }
    else
    {
        rt = ERR_MEM;
    }

    return rt;
}

/*
 * �ر�����
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
 * ��ʼ��LWIP������
 */
void tcp_server_initialize(unsigned char *lip)
{
    err_t err;                                              // LWIP������Ϣ
    ip_addr_t local_ip;
    
    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);

    m_tcpsvr_pcb = tcp_new();                               // �½�һ��TCPЭ����ƿ�
    if (m_tcpsvr_pcb != NULL)
    {
        /* �󶨱�������IP��ַ�Ͷ˿ں�,��Ϊ����������Ҫ֪���ͻ��˵�IP
         */
        err = tcp_bind(m_tcpsvr_pcb, &local_ip, TCP_LOCAL_PORT);
        if (err == ERR_OK)
        {
            m_tcpsvr_pcb = tcp_listen(m_tcpsvr_pcb);        // ��ʼ�����˿�

            /*
             * ָ������״̬��������֮ͨ��Ҫ���õĻص�����
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
        m_tcpsvr_flag &= ~LWIP_NEW_DATA;        // ����������ݵı�־
        return thislen;
    }

    return 0;
}

int tcpsvr_send_data(unsigned char *buf, int buflen)
{
    int thislen = buflen < TCP_SERVER_BUFSIZE-1 ? buflen : TCP_SERVER_BUFSIZE-1;
    memcpy(tcpsvr_tx_buf, buf, thislen);
    tcpsvr_tx_buf[thislen] = 0;
    m_tcpsvr_flag |= LWIP_SEND_DATA;            // �����������Ҫ����

    return thislen;
}

#endif // #if TEST_TCP_SERVER



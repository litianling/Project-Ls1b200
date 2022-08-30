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

static struct tcp_pcb *m_tcpcli_pcb = NULL;         // ����һ��TCP��Э����ƿ�

static unsigned int m_tcpcli_flag = 0;

static char tcpcli_rx_buf[TCP_CLIENT_BUFSIZE];      // ���������������ݵĻ���
static char tcpcli_tx_buf[TCP_CLIENT_BUFSIZE];      // ���������������ݵĻ���

//---------------------------------------------------------------------------------------

static void tcp_client_close(struct tcp_pcb *tpcb, mytcp_state_t *ts);

//---------------------------------------------------------------------------------------

/*
 * �ͻ��˳ɹ����ӵ�Զ������ʱ����
 */
static const char *respond =  "tcp client connect success\r\n";

static err_t tcp_client_connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    mytcp_state_t *ts = (mytcp_state_t *)arg;
    
    ts->state = MYTCP_STATE_RECVDATA;                   // ���Կ�ʼ����������
    m_tcpcli_flag |= LWIP_CONNECTED;                    // ������ӳɹ���
    tcp_write(tpcb, respond, strlen(respond), 1);       // ��Ӧ��Ϣ
    return ERR_OK;
}

/*
 * ������ѯʱ��Ҫ���õĺ���
 */
static err_t tcp_client_poll_callback(void *arg, struct tcp_pcb *tpcb)
{
    err_t rt = ERR_OK;
    mytcp_state_t *ts = (mytcp_state_t *)arg;

    if (ts != NULL)                                     // ���Ӵ��ڿ��п��Է�������
    {
        if ((m_tcpcli_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA)
        {
            tcp_write(tpcb, tcpcli_tx_buf, strlen(tcpcli_tx_buf), 1);
            m_tcpcli_flag &= ~LWIP_SEND_DATA;           // ����������ݵı�־
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
 * �ͻ��˽��յ�����֮��Ҫ���õĺ���
 */
static err_t tcp_client_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    err_t rt = ERR_OK;
    mytcp_state_t *ts = (mytcp_state_t *)arg;

    if (p == NULL)
    {
        ts->state = MYTCP_STATE_CLOSED;                 // ���ӹر���
        tcp_client_close(tpcb, ts);
        m_tcpcli_flag &= ~LWIP_CONNECTED;               // ������ӱ�־
    }
    else if (err != ERR_OK)
    {   
        if (p != NULL)                                  // λ�ô����ͷ�pbuf
        {
            pbuf_free(p);
        }
        rt = err;
    }
    else if (ts->state == MYTCP_STATE_RECVDATA)         // �����յ����µ�����
    {
        if ((p->tot_len) >= TCP_CLIENT_BUFSIZE)         // ����յĵ����ݴ��ڻ���
        {  
            memcpy(tcpcli_rx_buf, p->payload, TCP_CLIENT_BUFSIZE);
            tcpcli_rx_buf[TCP_CLIENT_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(tcpcli_rx_buf, p->payload, p->tot_len);
            tcpcli_rx_buf[p->tot_len] = 0;
        }
        
        m_tcpcli_flag |= LWIP_NEW_DATA;                 // �յ����µ�����
        tcp_recved(tpcb, p->tot_len);                   // ���ڻ�ȡ�������ݵĳ���, ��ʾ���Ի�ȡ���������
        pbuf_free(p);                                   // �ͷ��ڴ�
    }
    else if (ts->state == MYTCP_STATE_CLOSED)           // �������ر���
    {
        tcp_recved(tpcb, p->tot_len);                   // Զ�̶˿ڹر�����, ��������
        pbuf_free(p);
    }
    else                                                // ����δ֪״̬
    {
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    }
    
    return rt;
}

/*
 * �ر�����
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
 * ��ʼ��TCP�ͻ���
 */
void tcp_client_initialize(unsigned char *lip, unsigned char *rip)
{
    err_t err;
    ip_addr_t local_ip, remote_ip;
    
    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);
    IP4_ADDR(&remote_ip, rip[0], rip[1], rip[2], rip[3]);

    m_tcpcli_pcb = tcp_new();                           // �½�һ��PCB

    if (m_tcpcli_pcb != NULL)
    {
        m_tcpcli_state = mem_malloc(sizeof(mytcp_state_t));

        tcp_arg(m_tcpcli_pcb, m_tcpcli_state);          // �������Э����ƿ��״̬���ݸ��ص�����

        tcp_bind(m_tcpcli_pcb, &local_ip, TCP_LOCAL_PORT);

        /*
         * �趨TCP�Ļص�����
         */
        err = tcp_connect(m_tcpcli_pcb, &remote_ip, TCP_REMOTE_PORT, tcp_client_connect_callback);
        if (err == ERR_OK)
        {
            tcp_recv(m_tcpcli_pcb, tcp_client_recv_callback);      // ���յ��µ����ݵĻص�����
            tcp_poll(m_tcpcli_pcb, tcp_client_poll_callback, 0);   // ��ѯʱ���õĻص�����
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
 * ��������Զ������
 */
void tcp_client_connect_remotehost(unsigned char *lip, unsigned char *rip)
{
    /*
     * ��ס����˴���ҪƵ��������ʱ��ǵ��ȹر��Ѿ������tcb, ��ý�tcb����ȫ�ֱ���
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
        m_tcpcli_flag &= ~LWIP_NEW_DATA;        // ����������ݵı�־
        
        return thislen;
    }

    return 0;
}

int tcpcli_send_data(unsigned char *buf, int buflen)
{
    int thislen = buflen < TCP_CLIENT_BUFSIZE-1 ? buflen : TCP_CLIENT_BUFSIZE-1;
    memcpy(tcpcli_tx_buf, buf, thislen);
    tcpcli_tx_buf[thislen] = 0;
    m_tcpcli_flag |= LWIP_SEND_DATA;            // �����������Ҫ����

    return thislen;
}

#endif // #if TEST_TCP_CLIENT



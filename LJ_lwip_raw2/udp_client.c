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

static struct udp_pcb *m_udpcli_pcb = NULL;         // ����һ��UDP��Э����ƿ�

static unsigned int m_udpcli_flag = 0;

static char udpcli_rx_buf[UDP_CLIENT_BUFSIZE];      // ���������������ݵĻ���
static char udpcli_tx_buf[UDP_CLIENT_BUFSIZE];      // ���������������ݵĻ���

//---------------------------------------------------------------------------------------

/*
 * ���յ����ݰ���Ҫ���õĺ���
 */
static void udp_client_recv_callback(void *arg,
                                     struct udp_pcb *upcb,
                                     struct pbuf *p,
                                     struct ip_addr *addr,
                                     uint16_t port)
{
    if (p != NULL)
    {
        if ((p->tot_len) >= UDP_CLIENT_BUFSIZE)         // ����յĵ����ݴ��ڻ���
        {      
            memcpy(udpcli_rx_buf, p->payload, UDP_CLIENT_BUFSIZE);
            udpcli_rx_buf[UDP_CLIENT_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(udpcli_rx_buf, p->payload, p->tot_len);
            udpcli_rx_buf[p->tot_len] = 0;
        }
        
        m_udpcli_flag |= LWIP_NEW_DATA;                 // �յ����µ�����
        pbuf_free(p);
    }
}

/*
 * ��������
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

        err = udp_send(m_udpcli_pcb, tmpbuf);           // ��������
        if (err != ERR_OK)
        {
            // lwip_log("UDP SERVER��������ʧ��!");
        }
        
        m_udpcli_flag &= ~LWIP_SEND_DATA;               // ����������ݵı�־
        pbuf_free(tmpbuf);
    }
}

/*
 * ��ʼ��UDP�ͻ���
 */
void udp_client_initialize(unsigned char *lip, unsigned char *rip)
{
    err_t err;
    ip_addr_t local_ip, remote_ip;

    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);
    IP4_ADDR(&remote_ip, rip[0], rip[1], rip[2], rip[3]);

    m_udpcli_pcb = udp_new();                           // �½�һ��UDPЭ����ƿ�
    if (m_udpcli_pcb != NULL)
    {
        udp_bind(m_udpcli_pcb, &local_ip, UDP_LOCAL_PORT);
        err = udp_connect(m_udpcli_pcb, &remote_ip, UDP_REMOTE_PORT);   // �������ӵ�Զ������
        if (err == ERR_OK)
        {
            udp_recv(m_udpcli_pcb, udp_client_recv_callback, NULL);     // ע����ջص�����
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
    m_udpcli_flag |= LWIP_SEND_DATA;            // �����������Ҫ����
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
        m_udpcli_flag &= ~LWIP_NEW_DATA;        // ����������ݵı�־
        return thislen;
    }

    return 0;
}

#endif // #if TEST_UDP_CLIENT



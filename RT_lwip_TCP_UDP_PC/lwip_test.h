/*
 * lwip_test.h
 *
 * created: 2022/3/3
 *  author: 
 */

#ifndef _LWIP_TEST_H
#define _LWIP_TEST_H


#endif // _LWIP_TEST_H

/******************************************************************************
 * ����״̬
 */
#define LWIP_CONNECTED          0x0001
#define LWIP_SEND_DATA          0x0002
#define LWIP_NEW_DATA           0x0004

/******************************************************************************
 * ����TCP
 */
#define TCP_LOCAL_PORT          9061        // ���ض˿�
#define TCP_REMOTE_PORT         9060        // Զ�̶˿�

extern char *local_IP;                      // "192.168.1.111"
extern char *remote_IP;                     // "192.168.1.123"

/*
 * TCP ״̬ - ������״̬�ϲ�
 */
#define MYTCP_STATE_NONE        0
#define MYTCP_STATE_RECVDATA    1           // ���յ�������
#define MYTCP_STATE_CLOSED      2           // ���ӹر�

typedef struct mytcp_state
{
    unsigned int state;
} mytcp_state_t;

/*
 * ���� TCP server
 */
#define TEST_TCP_SERVER         0
#if (TEST_TCP_SERVER)

#define TCP_SERVER_BUFSIZE      256         // ���ݻ�������С

extern void tcp_server_init(void);

#endif

/*
 * ���� TCP client
 */
#define TEST_TCP_CLIENT         1
#if (TEST_TCP_CLIENT)

#define TCP_CLIENT_BUFSIZE      256         // ���ݻ�������С

extern void tcp_client_init(void);

#endif

/******************************************************************************
 * ����UDP
 */
#define UDP_LOCAL_PORT          9063        // ���ض˿�
#define UDP_REMOTE_PORT         9062        // Զ�̶˿�

/*
 * ���� UDP server. RT-Thread OK 2021.7.16
 */
#define TEST_UDP_SERVER         0
#if (TEST_UDP_SERVER)

#define UDP_SERVER_BUFSIZE      256         // ���ݻ�������С

extern void udp_server_init(void);

#endif

/*
 * ���� UDP client. RT-Thread OK 2021.7.16
 */
#define TEST_UDP_CLIENT         0
#if (TEST_UDP_CLIENT)

#define UDP_CLIENT_BUFSIZE      256         // ���ݻ�������С

extern void udp_client_init(void);

#endif




/*
 * lwip_test.h
 *
 * created: 2021/6/26
 *  author: 
 */

#ifndef _LWIP_TEST_H
#define _LWIP_TEST_H

/******************************************************************************
 * 网络状态
 */
#define LWIP_CONNECTED          0x0001
#define LWIP_SEND_DATA          0x0002
#define LWIP_NEW_DATA           0x0004

/******************************************************************************
 * 测试TCP
 */
#define TCP_LOCAL_PORT          9060            // 本地端口
#define TCP_REMOTE_PORT         9061            // 远程端口

/*
 * TCP 状态 - 和网络状态合并
 */
#define MYTCP_STATE_NONE        0
#define MYTCP_STATE_RECVDATA    1               // 接收到了数据
#define MYTCP_STATE_CLOSED      2               // 连接关闭

typedef struct mytcp_state
{
    unsigned int state;
} mytcp_state_t;

/*
 * 测试 TCP server
 */
#define TEST_TCP_SERVER         0
#if (TEST_TCP_SERVER > 0)

#define TCP_SERVER_BUFSIZE      256             // 数据缓冲区大小

extern void tcp_server_initialize(unsigned char *lip);
extern int tcpsvr_recv_data(unsigned char *buf, int buflen);
extern int tcpsvr_send_data(unsigned char *buf, int buflen);

#endif

/*
 * 测试 TCP client
 */
#define TEST_TCP_CLIENT         0
#if (TEST_TCP_CLIENT > 0)

#define TCP_CLIENT_BUFSIZE      256             // 数据缓冲区大小

extern void tcp_client_initialize(unsigned char *lip, unsigned char *rip);
extern int tcpcli_recv_data(unsigned char *buf, int buflen);
extern int tcpcli_send_data(unsigned char *buf, int buflen);
extern void tcpcli_disconnect(void);

#endif

/******************************************************************************
 * 测试UDP
 */
#define UDP_LOCAL_PORT          9062            // 本地端口
#define UDP_REMOTE_PORT         9063            // 远程端口

/*
 * 测试 UDP server
 */
#define TEST_UDP_SERVER         0
#if (TEST_UDP_SERVER > 0)

#define UDP_SERVER_BUFSIZE      256             // 数据缓冲区大小

extern void udp_server_initialize(unsigned char *lip);
extern int udpsvr_recv_data(unsigned char *buf, int buflen);
extern int udpsvr_send_data(unsigned char *buf, int buflen);

#endif

/*
 * 测试 UDP client
 */
#define TEST_UDP_CLIENT         0
#if (TEST_UDP_CLIENT > 0)

#define UDP_CLIENT_BUFSIZE      256             // 数据缓冲区大小

extern void udp_client_initialize(unsigned char *lip, unsigned char *rip);
extern int udpcli_send_data(unsigned char *buf, int buflen);
extern int udpcli_recv_data(unsigned char *buf, int buflen);

#endif


/******************************************************************************
 * lwip_test.c
 */
 
 
 

#endif // _LWIP_TEST_H


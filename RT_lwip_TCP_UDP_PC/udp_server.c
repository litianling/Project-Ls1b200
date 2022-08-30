/*
 * udp_server.c
 *
 * created: 2022/3/3
 *  author: 
 */

#include "lwip_test.h"

#if (TEST_UDP_SERVER)

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static char data_buf[UDP_SERVER_BUFSIZE];

static void udp_server_thread(void *arg)
{
	struct sockaddr_in local_addr;                         // ����Զ�̷�������ַ
	int sock_fd,err;                                       // �����׽��֣��������

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);              // �����׽��֣��������͡�Э�飩
	if (sock_fd == -1)
    {
		printk("failed to create sock_fd!\n");
		return;
	}
    else
        printk("succeed to create sock_fd!\n");

	memset(&local_addr, 0, sizeof(local_addr));            // Ϊ���ط�������ַ���ٿռ�
	local_addr.sin_family = AF_INET;                       // ���ط�����IP�������������
	local_addr.sin_addr.s_addr = inet_addr(local_IP);      // ���ط�����IP��ַ192.168.1.111
	local_addr.sin_port = htons(UDP_LOCAL_PORT);           // ���ط������˿ں�9063

	err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));   // ���ط�������ַ��
	if (err == -1)
    {
        closesocket(sock_fd);                              // ��ʧ�ܣ��ر��׽���
		printk("failed to bind ip!\n");
		return;
	}
	else
        printk("succeed to bind ip!\n");

	while (1)
    {
        struct sockaddr from;
        socklen_t fromlen;

        memset(data_buf, 0, UDP_SERVER_BUFSIZE);            // ������ݻ�����
		err = recvfrom(sock_fd, data_buf, UDP_SERVER_BUFSIZE, 0, &from, &fromlen);    // ��������

        if (err > 0)            // �������ղ�ת��
        {
            printk("RECEIVE: %s \n", data_buf);
            sendto(sock_fd, data_buf, err, 0, &from, fromlen);
            printk("    SEND: %s \n", data_buf);
        }
        else if (err <= 0)      // ���մ����������
        {
            delay_ms(100);
            continue;
        }
	}

	/*
     * NEVER GO HERE!
     */
	closesocket(sock_fd);      // �ر��׽���
	printk("udp_server_thread stop!\r\n");  // ����
}

void udp_server_init(void)      // �����߳�
{
	sys_thread_new("udp_server_thread",
                    udp_server_thread,
                    NULL,
                    DEFAULT_THREAD_STACKSIZE,
                    DEFAULT_THREAD_PRIO + 1);
}

#endif

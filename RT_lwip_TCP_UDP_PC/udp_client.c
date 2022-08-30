/*
 * udp_client.c
 *
 * created: 2022/3/3
 *  author: 
 */

#include "lwip_test.h"

#if (TEST_UDP_CLIENT)

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static char data_buf[UDP_CLIENT_BUFSIZE] = "this is a UDP test package";

static void udp_client_thread(void *arg)
{
	struct sockaddr_in remote_addr;                        // ����Զ�̷�������ַ
	int sock_fd,err,count = 0;                             // �����׽��֣�������룬����

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);              // �����׽��֣��������͡�Э�飩
	if (sock_fd == -1)
    {
		printk("failed to create sock_fd!\n");
		return;
	}
    else
        printk("succeed to create sock_fd!\n");

	memset(&remote_addr, 0, sizeof(remote_addr));          // ΪԶ�̷�������ַ���ٿռ�
	remote_addr.sin_family = AF_INET;                      // Զ�̷�����IP�������������
	remote_addr.sin_addr.s_addr = inet_addr(remote_IP);    // Զ�̷�����IP��ַ192.168.1.123
	remote_addr.sin_port = htons(UDP_REMOTE_PORT);         // Զ�̷������˿ں�9062

	while (1)
    {
        struct sockaddr from;
        socklen_t fromlen;
        unsigned int ticks = get_clock_ticks();             // ��ȡʱ��ֵ

        memset(data_buf, 0, UDP_CLIENT_BUFSIZE);            // ������ݻ�����
        snprintf(data_buf, UDP_CLIENT_BUFSIZE-1, "client ticks = %i.", ticks);  // �����ַ����������ݻ�������

		err = sendto(sock_fd,data_buf,strlen(data_buf),     // ��Զ�̷�������������
                     0,(struct sockaddr *)&remote_addr,
                     sizeof(remote_addr));
        if (err > 0)
		    printk("SEND: %s\r\n", data_buf);
        else if (err <= 0)
        {
            delay_ms(1000);
            continue;
        }

		delay_ms(100);

        memset(data_buf, 0, UDP_CLIENT_BUFSIZE);            // ������ݻ�����
		err = recvfrom(sock_fd,data_buf,UDP_CLIENT_BUFSIZE, // ��������
                       MSG_WAITALL,&from,&fromlen);
// MSG_PEEK������һ�����ݣ�MSG_WAITALL����ÿһ�����ݣ�MSG_DONTWAIT������ģʽ

		if (err > 0)
		    printk("  RECEIVE: %s\r\n", data_buf);

        if (++count >= 5)        /* ������־ 5���շ� */
            break;

	}

	closesocket(sock_fd);          // �ر��׽���
	printk("udp_client_thread stop!\r\n");  // ����
}

void udp_client_init(void)          // �����߳�
{
	sys_thread_new("udp_client_thread",
                   udp_client_thread,
                   NULL,
                   DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO + 1);
}

#endif

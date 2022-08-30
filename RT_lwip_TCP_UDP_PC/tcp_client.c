/*
 * tcp_client.c
 *
 * created: 2022/3/3
 *  author: 
 */

#include "lwip_test.h"

#if (TEST_TCP_CLIENT)

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static char data_buf[TCP_CLIENT_BUFSIZE] = "hello, you are connected!\n";

static void tcp_client_thread(void *arg)
{
	struct sockaddr_in remote_addr;                // ����Զ�̷�������ַ
	int sock_fd, err, count=0;                     // �����׽��֣�������룬����

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);     // �����׽��֣��������͡�Э�飩
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
	remote_addr.sin_port = htons(TCP_REMOTE_PORT);         // Զ�̷������˿ں�9060

	err = connect(sock_fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));  // ����Զ�̷��������׽��֡���ַ��
	if (err != ERR_OK)
	{
	    closesocket(sock_fd);
	    printk("failed to connect to server!\n");
		return;
	}
	else
        printk("succeed to connect to server!\n");

	while (1)
    {
        int rdbytes = 0;
        unsigned int ticks = get_clock_ticks();                 // ��ȡʱ��ֵ

        memset(data_buf, 0, TCP_CLIENT_BUFSIZE);                // ������ݻ�����
        snprintf(data_buf, 99, "client ticks = %i.\n", ticks);  // ���ϻ���"\n": ���ն���Ҫ�����ַ����������ݻ�������

		if (send(sock_fd, data_buf, strlen(data_buf), 0) <= 0)  // �������ݻ���������
        {
            delay_ms(1000);
            continue;
        }
        else
        {
            printk("SEND: %s", data_buf);
        }

        memset(data_buf, 0, TCP_CLIENT_BUFSIZE);                   // ������ݻ�����
		rdbytes = recv(sock_fd, data_buf, TCP_CLIENT_BUFSIZE, 0);  // ���շ���������������
		if (rdbytes > 0)
		{
		    printk("  RECEIVE: %s \n", data_buf);                 // ��ӡ����������������
		}

		delay_ms(100);

        if (++count >= 5)        /* ������־ 5���շ� */
            break;
	}

	closesocket(sock_fd);         // �ر��׽���
	printk("tcp_client_thread stop!\r\n");  // ����
}

void tcp_client_init(void)          // �����߳�
{
	sys_thread_new("tcp_client",
                   tcp_client_thread,
                   NULL,
                   DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO + 1);
}

#endif

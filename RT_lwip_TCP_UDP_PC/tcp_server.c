/*
 * tcp_server.c
 *
 * created: 2022/3/3
 *  author: 
 */

#include "lwip_test.h"

#if (TEST_TCP_SERVER)

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static char data_buf[TCP_SERVER_BUFSIZE] = "hello, I'm tcp server!\n";

static void tcp_server_thread(void *arg)
{
	struct sockaddr_in local_addr;                         // ����Զ�̷�������ַ
	int sock_fd, err;                                      // �����׽��֣��������

	sock_fd = socket(AF_INET, SOCK_STREAM, 6);             // �����׽��֣��������͡�Э�飩
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
	local_addr.sin_port = htons(TCP_LOCAL_PORT);           // ���ط������˿ں�9061
    local_addr.sin_len = sizeof(local_addr);               // ���ط�������ַռ�õĴ洢�ռ��С

	err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));  // ���ط�������ַ��
	if (err != ERR_OK)
	{
	    closesocket(sock_fd);                              // ��ʧ�ܣ��ر��׽���
	    printk("failed to bind ip!\n");
		return;
	}
	else
        printk("succeed to bind ip!\n");

    err = listen(sock_fd, 3);               // ��������
	if (err != ERR_OK)
	{
	    closesocket(sock_fd);
	    printk("failed to listen!\n");
		return;
	}
	else
        printk("succeed to listen!\n");

    /*
     * loop first.
     */
	while (1)
    {
        int client_fd;
        struct sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);

        printk("waiting for client access!\n");
        client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, (socklen_t)&addrlen);   // �ͻ��˽���
        if (client_fd > 0)
        {
            printk("client incoming succeed!\r\n");

            /*
             * loop second.
             */
            for (;;)            /* ѭ�����ղ��Զ�ת����ֱ���ͻ��������Ͽ����� */
            {
                memset(data_buf, 0, TCP_SERVER_BUFSIZE);                // ������ݻ�����
                err = recv(client_fd, data_buf, TCP_SERVER_BUFSIZE, 0); // ��������

                if (err > 0)                        // �����������ݲ�ת��
                {
                    printk("RECEIVE: %s \n", data_buf);
                    send(client_fd, data_buf, err, 0);
                    printk("    SEND: %s \n", data_buf);
                }
				else if (err == ERR_CLSD)            // �ͻ��������ж�
				{
					closesocket(client_fd);          // �ر��׽���
					printk("client disconnected.\r\n");
					break;
				}
				else if (err <= 0)                   // �ͻ��������Ͽ�����
				{
					closesocket(client_fd);
					printk("disconnect client succeed!\r\n");
					break;
				}
            }
        }

		delay_ms(200);

	}

	/*
     * NEVER GO HERE!
     */
	closesocket(sock_fd);            // �ر��׽���
	printk("tcp_server_thread stop!\r\n"); // ����
}

void tcp_server_init(void)              // �����߳�
{
	sys_thread_new("tcp_server",
                   tcp_server_thread,
                   NULL,
                   DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO + 1);
}

#endif


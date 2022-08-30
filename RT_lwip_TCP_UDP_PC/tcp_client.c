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
	struct sockaddr_in remote_addr;                // 定义远程服务器地址
	int sock_fd, err, count=0;                     // 定义套接字，错误代码，计数

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);     // 创建套接字（级别、类型、协议）
	if (sock_fd == -1)
    {
		printk("failed to create sock_fd!\n");
		return;
	}
    else
        printk("succeed to create sock_fd!\n");

	memset(&remote_addr, 0, sizeof(remote_addr));          // 为远程服务器地址开辟空间
	remote_addr.sin_family = AF_INET;                      // 远程服务器IP有两个部分组成
	remote_addr.sin_addr.s_addr = inet_addr(remote_IP);    // 远程服务器IP地址192.168.1.123
	remote_addr.sin_port = htons(TCP_REMOTE_PORT);         // 远程服务器端口号9060

	err = connect(sock_fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));  // 连接远程服务器（套接字、地址）
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
        unsigned int ticks = get_clock_ticks();                 // 获取时间值

        memset(data_buf, 0, TCP_CLIENT_BUFSIZE);                // 清除数据缓存区
        snprintf(data_buf, 99, "client ticks = %i.\n", ticks);  // 加上换行"\n": 接收端需要【将字符串放在数据缓存区】

		if (send(sock_fd, data_buf, strlen(data_buf), 0) <= 0)  // 发送数据缓存区数据
        {
            delay_ms(1000);
            continue;
        }
        else
        {
            printk("SEND: %s", data_buf);
        }

        memset(data_buf, 0, TCP_CLIENT_BUFSIZE);                   // 清除数据缓存区
		rdbytes = recv(sock_fd, data_buf, TCP_CLIENT_BUFSIZE, 0);  // 接收服务器发来的数据
		if (rdbytes > 0)
		{
		    printk("  RECEIVE: %s \n", data_buf);                 // 打印服务器发来的数据
		}

		delay_ms(100);

        if (++count >= 5)        /* 结束标志 5次收发 */
            break;
	}

	closesocket(sock_fd);         // 关闭套接字
	printk("tcp_client_thread stop!\r\n");  // 结束
}

void tcp_client_init(void)          // 开启线程
{
	sys_thread_new("tcp_client",
                   tcp_client_thread,
                   NULL,
                   DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO + 1);
}

#endif

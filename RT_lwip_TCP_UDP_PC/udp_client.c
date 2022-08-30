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
	struct sockaddr_in remote_addr;                        // 定义远程服务器地址
	int sock_fd,err,count = 0;                             // 定义套接字，错误代码，计数

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);              // 创建套接字（级别、类型、协议）
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
	remote_addr.sin_port = htons(UDP_REMOTE_PORT);         // 远程服务器端口号9062

	while (1)
    {
        struct sockaddr from;
        socklen_t fromlen;
        unsigned int ticks = get_clock_ticks();             // 获取时间值

        memset(data_buf, 0, UDP_CLIENT_BUFSIZE);            // 清除数据缓存区
        snprintf(data_buf, UDP_CLIENT_BUFSIZE-1, "client ticks = %i.", ticks);  // 【将字符串放在数据缓存区】

		err = sendto(sock_fd,data_buf,strlen(data_buf),     // 向远程服务器发送数据
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

        memset(data_buf, 0, UDP_CLIENT_BUFSIZE);            // 清除数据缓存区
		err = recvfrom(sock_fd,data_buf,UDP_CLIENT_BUFSIZE, // 接收数据
                       MSG_WAITALL,&from,&fromlen);
// MSG_PEEK阻塞第一条数据，MSG_WAITALL阻塞每一条数据，MSG_DONTWAIT非阻塞模式

		if (err > 0)
		    printk("  RECEIVE: %s\r\n", data_buf);

        if (++count >= 5)        /* 结束标志 5次收发 */
            break;

	}

	closesocket(sock_fd);          // 关闭套接字
	printk("udp_client_thread stop!\r\n");  // 结束
}

void udp_client_init(void)          // 开启线程
{
	sys_thread_new("udp_client_thread",
                   udp_client_thread,
                   NULL,
                   DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO + 1);
}

#endif

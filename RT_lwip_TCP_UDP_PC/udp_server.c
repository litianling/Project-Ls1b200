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
	struct sockaddr_in local_addr;                         // 定义远程服务器地址
	int sock_fd,err;                                       // 定义套接字，错误代码

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);              // 创建套接字（级别、类型、协议）
	if (sock_fd == -1)
    {
		printk("failed to create sock_fd!\n");
		return;
	}
    else
        printk("succeed to create sock_fd!\n");

	memset(&local_addr, 0, sizeof(local_addr));            // 为本地服务器地址开辟空间
	local_addr.sin_family = AF_INET;                       // 本地服务器IP有两个部分组成
	local_addr.sin_addr.s_addr = inet_addr(local_IP);      // 本地服务器IP地址192.168.1.111
	local_addr.sin_port = htons(UDP_LOCAL_PORT);           // 本地服务器端口号9063

	err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));   // 本地服务器地址绑定
	if (err == -1)
    {
        closesocket(sock_fd);                              // 绑定失败，关闭套接字
		printk("failed to bind ip!\n");
		return;
	}
	else
        printk("succeed to bind ip!\n");

	while (1)
    {
        struct sockaddr from;
        socklen_t fromlen;

        memset(data_buf, 0, UDP_SERVER_BUFSIZE);            // 清除数据缓存区
		err = recvfrom(sock_fd, data_buf, UDP_SERVER_BUFSIZE, 0, &from, &fromlen);    // 接收数据

        if (err > 0)            // 正常接收并转发
        {
            printk("RECEIVE: %s \n", data_buf);
            sendto(sock_fd, data_buf, err, 0, &from, fromlen);
            printk("    SEND: %s \n", data_buf);
        }
        else if (err <= 0)      // 接收错误继续接收
        {
            delay_ms(100);
            continue;
        }
	}

	/*
     * NEVER GO HERE!
     */
	closesocket(sock_fd);      // 关闭套接字
	printk("udp_server_thread stop!\r\n");  // 结束
}

void udp_server_init(void)      // 开启线程
{
	sys_thread_new("udp_server_thread",
                    udp_server_thread,
                    NULL,
                    DEFAULT_THREAD_STACKSIZE,
                    DEFAULT_THREAD_PRIO + 1);
}

#endif

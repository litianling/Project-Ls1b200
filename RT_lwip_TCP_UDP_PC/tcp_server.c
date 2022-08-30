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
	struct sockaddr_in local_addr;                         // 定义远程服务器地址
	int sock_fd, err;                                      // 定义套接字，错误代码

	sock_fd = socket(AF_INET, SOCK_STREAM, 6);             // 创建套接字（级别、类型、协议）
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
	local_addr.sin_port = htons(TCP_LOCAL_PORT);           // 本地服务器端口号9061
    local_addr.sin_len = sizeof(local_addr);               // 本地服务器地址占用的存储空间大小

	err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));  // 本地服务器地址绑定
	if (err != ERR_OK)
	{
	    closesocket(sock_fd);                              // 绑定失败，关闭套接字
	    printk("failed to bind ip!\n");
		return;
	}
	else
        printk("succeed to bind ip!\n");

    err = listen(sock_fd, 3);               // 开启监听
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
        client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, (socklen_t)&addrlen);   // 客户端接入
        if (client_fd > 0)
        {
            printk("client incoming succeed!\r\n");

            /*
             * loop second.
             */
            for (;;)            /* 循环接收并自动转发，直至客户端主动断开连接 */
            {
                memset(data_buf, 0, TCP_SERVER_BUFSIZE);                // 清除数据缓存区
                err = recv(client_fd, data_buf, TCP_SERVER_BUFSIZE, 0); // 接收数据

                if (err > 0)                        // 正常接收数据并转发
                {
                    printk("RECEIVE: %s \n", data_buf);
                    send(client_fd, data_buf, err, 0);
                    printk("    SEND: %s \n", data_buf);
                }
				else if (err == ERR_CLSD)            // 客户端连接中断
				{
					closesocket(client_fd);          // 关闭套接字
					printk("client disconnected.\r\n");
					break;
				}
				else if (err <= 0)                   // 客户端主动断开连接
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
	closesocket(sock_fd);            // 关闭套接字
	printk("tcp_server_thread stop!\r\n"); // 结束
}

void tcp_server_init(void)              // 开启线程
{
	sys_thread_new("tcp_server",
                   tcp_server_thread,
                   NULL,
                   DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO + 1);
}

#endif


===============================
===============================

RT本地串口输出分析

rt_kprintf在RTT4/src/kservice.c中实现，调用了rt_hw_console_output(rt_log_buf);输出缓存数据

在上述文件中弱定义空函数RT_WEAK void rt_hw_console_output(const char *str)
该函数真正实现是在ls1x-drv/console/console.c文件中，并且调用console_putstr输出字符串。

console_putstr又调用console_putch函数输出单个字符。

===============================

将RT输出重定向到屏幕（增加一个输出路径，原控制台路径不删除）

在main.c与bsp.c文件中打开与屏幕相关的定义
在主函数中初始化fb的使用  fb_open();  fb_cons_clear();
在ls1x-drv/console/console.c文件中console_putstr()函数下调用fb_cons_putc(*s);函数
【注】由于在启动引导代码中就要输出RT版本号，先于fb_cons_putc(*s)所以要加限制条件
	在ls1x-drv/fb/ls1x_fb_utils.c添加全局变量fb_print_lock=1，在fb_open函数解锁，在fb_cons_putc判断

===============================
===============================

RT本地串口输入分析

在RTT4/components/shell.c文件下507行的函数finish_thread_entry就是空闲时输入命令行的钩子函数。
在该函数中调用了ch = finsh_getchar();从控制台获取字符，finsh_getchar又调用了rt_hw_console_getchar()函数。
该rt_hw_console_getchar()函数在ls1x-drv/console/console.c文件中实现，调用console_getch()函数……

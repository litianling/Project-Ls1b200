/*
 * L610_app_TCP.c
 *
 * created: 2022/4/21
 *  author: 
 */
#include <rtthread.h>
#include <stdio.h>

#include "ls1b.h"
#include "mips.h"
#include "bsp.h"
#include "ns16550.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        50
static rt_thread_t L610_TCP1 = RT_NULL;

int L610_TCP_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);

    l610_init_pre();                        // ͨ��ATָ����һ�Ѳ�������
    int count = 0;
    while (count++ < 2)
    {
		l610_init();                        // ͨ��ATָ���ģ��������ֽ��г�ʼ��
		l610_check_socket();                // ��ѯ��ǰ���õ�socket ID��socket ID ������˵����ǰû�� TCP ���ӡ� AT+MIPOPEN?
		l610_creat_tcp();                   // ͨ��socket����TCP���ӣ�AT+MIPOPEN=1,,\"47.92.117.163\",30000,0
		delay_ms(500);

        l610_send_data("a small dog");      // ���� ASCII ����
        l610_send_data("Hello Fibocom.");   // ���� ASCII ����
		l610_send_data("0123456789ABCDEF"); // ���� ASCII ����

		delay_ms(100);
		l610_end_tcp();                     // �ر�TCP���ӣ����������շ����������쳣�����Թرյ��������� TCP ���ӣ�AT+MIPCLOSE=1
		l610_ip_release();                  // �ͷ�IP�������ͷ�ģ�鱾�μ�����ȡ�� IP ��ַ AT+MIPCALL=0
		delay_ms(500);
    }

    change_scheduler_lock(0);
    return 0;
}

int L610_TCP(void)
{
    L610_TCP1 = rt_thread_create("L610_TCP",
                            L610_TCP_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_TCP1);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(L610_TCP, L610 TCP function);

/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date           Author       Notes 
 * 2018-08-24     yangjie      the first version 
 */ 

/*
 * 程序清单：创建/删除、初始化线程
 *
 * 这个例子会创建两个线程，一个动态线程，一个静态线程。
 * 一个线程在运行完毕后自动被系统删除，另一个线程一直打印计数。
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

/* �� �� 1 �� �� �� �� �� */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count=0;

    while (1)
    {
        /* �߳�1���õ����ȼ����У�һֱ��ӡ����ֵ*/
        rt_kprintf("thread1 count: %d\n", count ++);
        rt_thread_mdelay(500);

    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
static char thread3_stack[1024];
static struct rt_thread thread3;
/* �� �� 2 �� �� */
static void thread2_entry(void *param)
{
    rt_uint32_t count = 0;

    /* �� �� 2 ӵ �� �� �� �� �� �� ���� �� �� ռ �� �� 1 �� �� �� ִ �� */
    for (count = 0; count < 10 ; count++)
    {
        /* �� �� 2 �� ӡ �� �� ֵ */
        rt_kprintf("thread2 count: %d\n", count);
    }
    rt_kprintf("thread2 exit\n");
    /* �� �� 2 �� �� �� �� �� Ҳ �� �� �� �� ϵ ͳ �� �� */
}

static void thread3_entry(void *param)
{
    rt_uint32_t count = 0;

    /* �� �� 3 ӵ �� �� �� �� �� �� ���� �� �� ռ �� �� 1 �� �� �� ִ �� */
    for (count = 10; count < 25 ; count++)
    {
        /* �� �� 2 �� ӡ �� �� ֵ */
        rt_kprintf("thread3 count: %d\n", count-10);
    }
    rt_kprintf("thread3 exit\n");
    /* �� �� 2 �� �� �� �� �� Ҳ �� �� �� �� ϵ ͳ �� �� */
}

/* �� �� ʾ �� */
int thread_sample(void)
{
    /* �� �� �� �� 1�� �� �� �� thread1�� �� �� �� thread1_entry*/
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    
    /* �� �� �� �� �� �� �� �� �飬 �� �� �� �� �� �� */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    /* �� ʼ �� �� �� 2�� �� �� �� thread2�� �� �� �� thread2_entry */
    rt_thread_init(&thread2,
                   "thread2",
                   thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack),
                   THREAD_PRIORITY - 2, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);

rt_thread_init(&thread3,
                   "thread3",
                   thread3_entry,
                   RT_NULL,
                   &thread3_stack[0],
                   sizeof(thread3_stack),
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    rt_thread_startup(&thread3);
    return 0;
}

/* �� �� �� msh �� �� �� �� �� */
MSH_CMD_EXPORT(thread_sample, thread sample);
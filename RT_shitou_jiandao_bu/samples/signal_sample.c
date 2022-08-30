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
 * 程序清单：信号例程
 *
 * 这个例子会创建一个线程，线程安装信号，然后给这个线程发送信号。
 *
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5
#define MAXSEM 2

static rt_uint32_t set, i;

static rt_thread_t left_tid = RT_NULL;
static rt_thread_t right_tid = RT_NULL;
//static rt_thread_t tid3 = RT_NULL;

static rt_uint32_t array[MAXSEM];

struct rt_semaphore sem_lock;
struct rt_semaphore sem_empty;


/* 线程1的信号处理函数 */
void thread1_signal_handler(int sig)
{
    rt_kprintf("thread1 received signal %d\n", sig);
}

/* 线程1的入口函数 */
static void thread_entry(void *parameter)
{
    int cnt = 0;
    while(cnt < 10){
        rt_sem_take(&sem_empty, RT_WAITING_FOREVER);
        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        if(set % MAXSEM == 0){
            array[set%MAXSEM] = (cnt + 1) % 3;
            if((cnt + 1) % 3 == 0)
                rt_kprintf("the %d hao chu de shi shitou\n", set%MAXSEM);
            else if((cnt + 1) % 3 == 1)
                rt_kprintf("the %d hao chu de shi jiandao\n", set%MAXSEM);
            else if((cnt + 1) % 3 == 2)
                rt_kprintf("the %d hao chu de shi bu\n", set%MAXSEM);
        }
        else{
            array[set%MAXSEM] = (cnt + 2) % 3;
            if((cnt + 2) % 3 == 0)
                rt_kprintf("the %d hao chu de shi shitou\n", set%MAXSEM);
            else if((cnt + 2) % 3 == 1)
                rt_kprintf("the %d hao chu de shi jiandao\n", set%MAXSEM);
            else if((cnt + 2) % 3 == 2)
                rt_kprintf("the %d hao chu de shi bu\n", set%MAXSEM);
        }
            
        i++;
        set++;
        if(i % 2 == 0)
        {
            if(array[0] == 0 && array[1] == 1)
                rt_kprintf("shi tou ying\n", set%MAXSEM);
            if(array[0] == 1 && array[1] == 2)
                rt_kprintf("jiandao ying\n", set%MAXSEM);
            if(array[0] == 0 && array[1] == 2)
                rt_kprintf("bu ying\n", set%MAXSEM);
            if(array[0] == array[1])
                rt_kprintf("ping\n", set%MAXSEM);
            if(array[0] == 1 && array[1] == 0)
                rt_kprintf("shitou ying\n", set%MAXSEM);
            if(array[0] == 2 && array[1] == 1)
                rt_kprintf("jiandao\n", set%MAXSEM);
            if(array[0] == 2 && array[1] == 0)
                rt_kprintf("buying\n", set%MAXSEM);
        }
        rt_sem_release(&sem_lock);
        rt_sem_release(&sem_empty);
        cnt++;
        
        rt_thread_mdelay(20);
    }
    /* 安装信号 */
    


    //rt_signal_install(SIGUSR1, thread1_signal_handler);
    //rt_signal_unmask(SIGUSR1);



    /* 运行10次 */
    //while (cnt < 10)
    //{
        /* 线程1采用低优先级运行，一直打印计数值 */
        //rt_kprintff("thread1 count : %d\n", cnt);

        //cnt++;
        //rt_thread_mdelay(100);
    //}
}

/* 信号示例的初始化 */
int signal_sample(void)
{
    /* 创建线程1 */
    //tid1 = rt_thread_create("thread1",
                            //thread1_entry, RT_NULL,
                            //THREAD_STACK_SIZE,
                            //THREAD_PRIORITY, THREAD_TIMESLICE);
    
    //if (tid1 != RT_NULL)
        //rt_thread_startup(tid1);

    //rt_thread_mdelay(300);

    /* 发送信号 SIGUSR1 给线程1 */
    //rt_thread_kill(tid1, SIGUSR1);
    
    set = 0;
    i = 0;
    
    rt_sem_init(&sem_lock, "lock", 1, RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_empty, "lock", MAXSEM, RT_IPC_FLAG_FIFO);

    /* ��ʼ�������ź��� */
    //rt_sem_init()
    left_tid = rt_thread_create("left",
                                    thread_entry, RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (left_tid != RT_NULL)
    {
        rt_thread_startup(left_tid);
    }
    else
    {
        rt_kprintf("create thread left failed");
        return -1;
    }
    
    right_tid = rt_thread_create("right",
                                    thread_entry, RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (right_tid != RT_NULL)
    {
        rt_thread_startup(right_tid);
    }
    else
    {
        rt_kprintf("create thread right failed");
        return -1;
    }
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(signal_sample, signal sample);

/*
 * barber_sample.c
 *
 * created: 2022/1/12
 *  author:
 */

#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_sem_t quhao = RT_NULL;        // 指向信号量的指针
static rt_sem_t jiaohao = RT_NULL;      // 指向信号量的指针
static rt_mutex_t mutex = RT_NULL;  // 指向互斥量的指针
static rt_uint8_t waitings = 0; // 等待人数;
static rt_uint8_t chairs = 10;   // 空余椅子数;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
static void rt_thread1_entry(void *parameter)
{// thread1为理发师线程;
    static rt_err_t result;
    while(1)
    {
        result = rt_sem_take(quhao, RT_WAITING_FOREVER);  // P()操作;
        rt_kprintf("barber waits quhao.\n");
        if (result != RT_EOK)
        {
            rt_kprintf("barber P(quhao) failed.\n");
            rt_sem_delete(quhao);
            return;
        }
        else
        {// 如果P到了quhao; 每条语句后的rt_kprintf()语句起到了类似注释的作用;
            rt_mutex_take(mutex, RT_WAITING_FOREVER); rt_kprintf("barber P(mutex).\n");
            waitings--; rt_kprintf("waitings--. waitings=%d\n", waitings);
            rt_mutex_release(mutex); rt_kprintf("barber V(mutex).\n");
            rt_sem_release(jiaohao); rt_kprintf("barber V(jiaohao).\n");
            rt_kprintf("barber cut hair.\n");
            rt_thread_mdelay(666);
        }
    }
}

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t i;

static void rt_thread2_entry(void *parameter)
{// thread2为顾客进程; 每条语句后的rt_kprintf()语句起到了类似注释的作用;
    static rt_err_t result;

    rt_mutex_take(mutex, RT_WAITING_FOREVER); rt_kprintf("customer%d P(mutex).\n", i);

    if(waitings<chairs) {
        waitings++; rt_kprintf("waitings++. waitings=%d\n", waitings);
        rt_mutex_release(mutex); rt_kprintf("customer%d V(mutex).\n", i);
        rt_sem_release(quhao); rt_kprintf("customer%d V(quhao).\n", i);
        result = rt_sem_take(quhao, RT_WAITING_FOREVER);  // P(jiaohao)
        rt_kprintf("customer%d P(jiaohao).\n", i);
        if (result != RT_EOK) {
            rt_kprintf("customer%d P(jiaohao) failed.\n", i);
            rt_sem_delete(jiaohao);
            return;
        }
        else {// 如果P到了jiaohao
            rt_kprintf("customer%d has a hair cut.\n", i);
        }
    }
    else {
        rt_mutex_release(mutex); rt_kprintf("customer%d V(mutex).\n", i);
    }
}

/* 信号量示例的初始化 */
int barber_sample()
{
    /* 创建动态信号量quhao，初始值是0 */
    quhao = rt_sem_create("quhao", 0, RT_IPC_FLAG_FIFO);
    if (quhao == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore 'quhao' failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create 'quhao' done. dynamic semaphore value = 0.\n");
    }

    /* 创建动态信号量jiaohao，初始值是0 */
    jiaohao = rt_sem_create("jiaohao", 0, RT_IPC_FLAG_FIFO);
    if (jiaohao == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore 'jiaohao' failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create 'jiaohao' done. dynamic semaphore value = 0.\n");
    }

    /* 创建一个动态互斥量 */
    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create 'mutex' done.\n");
    }

    rt_thread_init(&thread1,
                   "thread1",
                   rt_thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);

    // 创建30个顾客线程;
    for(i=0; i<30; i++)
    {
        // 设计两种顾客到来时间;
        if(i%2==0)
            rt_thread_mdelay(300);
        else
            rt_thread_mdelay(700);
        struct rt_thread thread2;
        rt_thread_t tid;
        /* 创建线程*/
        tid = rt_thread_create("thread2",
                            rt_thread2_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY-1, THREAD_TIMESLICE);
         /* 如果获得线程控制块，启动这个线程 */
        if (tid != RT_NULL)
            rt_thread_startup(tid);

        return 0;
    }
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(barber_sample, barber sample);


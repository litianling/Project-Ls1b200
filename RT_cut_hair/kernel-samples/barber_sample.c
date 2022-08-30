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

static rt_sem_t quhao = RT_NULL;        // ָ���ź�����ָ��
static rt_sem_t jiaohao = RT_NULL;      // ָ���ź�����ָ��
static rt_mutex_t mutex = RT_NULL;  // ָ�򻥳�����ָ��
static rt_uint8_t waitings = 0; // �ȴ�����;
static rt_uint8_t chairs = 10;   // ����������;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
static void rt_thread1_entry(void *parameter)
{// thread1Ϊ��ʦ�߳�;
    static rt_err_t result;
    while(1)
    {
        result = rt_sem_take(quhao, RT_WAITING_FOREVER);  // P()����;
        rt_kprintf("barber waits quhao.\n");
        if (result != RT_EOK)
        {
            rt_kprintf("barber P(quhao) failed.\n");
            rt_sem_delete(quhao);
            return;
        }
        else
        {// ���P����quhao; ÿ�������rt_kprintf()�����������ע�͵�����;
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
{// thread2Ϊ�˿ͽ���; ÿ�������rt_kprintf()�����������ע�͵�����;
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
        else {// ���P����jiaohao
            rt_kprintf("customer%d has a hair cut.\n", i);
        }
    }
    else {
        rt_mutex_release(mutex); rt_kprintf("customer%d V(mutex).\n", i);
    }
}

/* �ź���ʾ���ĳ�ʼ�� */
int barber_sample()
{
    /* ������̬�ź���quhao����ʼֵ��0 */
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

    /* ������̬�ź���jiaohao����ʼֵ��0 */
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

    /* ����һ����̬������ */
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

    // ����30���˿��߳�;
    for(i=0; i<30; i++)
    {
        // ������ֹ˿͵���ʱ��;
        if(i%2==0)
            rt_thread_mdelay(300);
        else
            rt_thread_mdelay(700);
        struct rt_thread thread2;
        rt_thread_t tid;
        /* �����߳�*/
        tid = rt_thread_create("thread2",
                            rt_thread2_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY-1, THREAD_TIMESLICE);
         /* �������߳̿��ƿ飬��������߳� */
        if (tid != RT_NULL)
            rt_thread_startup(tid);

        return 0;
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(barber_sample, barber sample);


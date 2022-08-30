/*
 * movieTheater_sample.c
 *
 * created: 2022/1/11
 *  author: 
 */

#include <rtthread.h>
#include <stdlib.h>
#include <time.h>
#define THREAD_PRIORITY         25
#define THREAD_TIMESLICE        5
#define THREAD_STACK_SIZE	1024
/* ָ���ź�����ָ�� */
static rt_sem_t movie=RT_NULL;//��ʾ��Ӱ���ź�����Ϊͬ���ź���
ALIGN(RT_ALIGN_SIZE)
static rt_mutex_t  mutex[3];//�����ź���
static rt_uint32_t count[3];//ÿ�ֵ�Ӱ���ڹۿ�����������Ҫ��ʼ��Ϊ0
static void rt_thread_entry(void *parameter)/*�����߳�*/
{
    rt_err_t result;
    rt_uint32_t id=(rt_uint32_t)parameter;
    /*��ȡ������idΪ�ù��ڹۿ���Ӱ�����*/
    rt_mutex_take(mutex[id], RT_WAITING_FOREVER);
    /*��Ϊcount[id]�ǻ�����Դ��������Ҫ�û����ź���*/
    count[id]++;
    rt_kprintf("A customer comes in to watch movie%d.\n",id);
    if(count[id]==1)
    {
        result = rt_sem_take(movie, RT_WAITING_FOREVER);
        /* ���÷�ʽ�ȴ��ź���,������ȴ���һ���ۿ���һ����Ӱ�Ĺ���������ܷ��µ�Ӱ */
        if (result != RT_EOK)
        {
            rt_kprintf(" failed.\n");
            rt_sem_delete(movie);
            return;
        }
        rt_kprintf("Movie%d. starts now\n",id);
    }
    rt_mutex_release(mutex[id]);
    //����Ӱ
    int delayTime;
    delayTime = rand()%200+20;
    rt_thread_mdelay(delayTime);
    rt_mutex_take(mutex[id], RT_WAITING_FOREVER);
    /*��Ϊcount[id]�ǻ�����Դ��������Ҫ�û����ź���*/
    count[id]--;
    rt_kprintf("A customer finishes watching movie%d and leave.\n",id);
    if(count[id]==0)
    {
        rt_kprintf("Movie%d. ends now\n",id);
        rt_sem_release(movie);
    }
    rt_mutex_release(mutex[id]);
}
int movie_theater_sample()
{
    /* �ź����ĳ�ʼ�� */
    movie=rt_sem_create("moviesem", 1, RT_IPC_FLAG_FIFO);//movie��ʼ��Ϊ1
    if (movie == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    int i=0;
    for(i=0;i<3;i++)
    {
        mutex[i]=RT_NULL;
        mutex[i] = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
        if (mutex[i] == RT_NULL)
        {
            rt_kprintf("create dynamic semaphore failed.\n");
            return -1;
        }
    }
    /*�������ĳ�ʼ��*/
    for(i=0;i<3;i++)
        count[i]=0;
    for(i=0;i<10;i++)/*����10���ۿ����߳�*/
    {
        rt_thread_mdelay(10);
        struct rt_thread thread;
        rt_thread_t tid;
        int num;
        num = rand();
        /* �����߳�,����ۿ�movie1,2,3�������ǰ��*/
        tid = rt_thread_create("thread",
                            rt_thread_entry, (void*)(num%3),
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
         /* �������߳̿��ƿ飬��������߳� */
        if (tid != RT_NULL)
            rt_thread_startup(tid);
    }
    return 0;
}
 
/* ������ msh �����б��� */
MSH_CMD_EXPORT(movie_theater_sample, movie theater);


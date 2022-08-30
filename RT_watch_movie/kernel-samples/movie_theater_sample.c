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
/* 指向信号量的指针 */
static rt_sem_t movie=RT_NULL;//表示电影的信号量，为同步信号量
ALIGN(RT_ALIGN_SIZE)
static rt_mutex_t  mutex[3];//互斥信号量
static rt_uint32_t count[3];//每种电影正在观看的人数，需要初始化为0
static void rt_thread_entry(void *parameter)/*观众线程*/
{
    rt_err_t result;
    rt_uint32_t id=(rt_uint32_t)parameter;
    /*获取参数，id为该观众观看电影的序号*/
    rt_mutex_take(mutex[id], RT_WAITING_FOREVER);
    /*因为count[id]是互斥资源，所有需要用互斥信号量*/
    count[id]++;
    rt_kprintf("A customer comes in to watch movie%d.\n",id);
    if(count[id]==1)
    {
        result = rt_sem_take(movie, RT_WAITING_FOREVER);
        /* 永久方式等待信号量,即必须等待上一个观看上一个电影的观众走完才能放新电影 */
        if (result != RT_EOK)
        {
            rt_kprintf(" failed.\n");
            rt_sem_delete(movie);
            return;
        }
        rt_kprintf("Movie%d. starts now\n",id);
    }
    rt_mutex_release(mutex[id]);
    //看电影
    int delayTime;
    delayTime = rand()%200+20;
    rt_thread_mdelay(delayTime);
    rt_mutex_take(mutex[id], RT_WAITING_FOREVER);
    /*因为count[id]是互斥资源，所有需要用互斥信号量*/
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
    /* 信号量的初始化 */
    movie=rt_sem_create("moviesem", 1, RT_IPC_FLAG_FIFO);//movie初始化为1
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
    /*计数器的初始化*/
    for(i=0;i<3;i++)
        count[i]=0;
    for(i=0;i<10;i++)/*创建10个观看者线程*/
    {
        rt_thread_mdelay(10);
        struct rt_thread thread;
        rt_thread_t tid;
        int num;
        num = rand();
        /* 创建线程,假设观看movie1,2,3的人随机前来*/
        tid = rt_thread_create("thread",
                            rt_thread_entry, (void*)(num%3),
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
         /* 如果获得线程控制块，启动这个线程 */
        if (tid != RT_NULL)
            rt_thread_startup(tid);
    }
    return 0;
}
 
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(movie_theater_sample, movie theater);


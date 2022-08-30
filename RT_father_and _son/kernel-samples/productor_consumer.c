/*
 * productor_consumer.c
 *
 * created: 2022/1/12
 *  author: 
 */
#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_PRIORITY 0
#define THREAD_STACK_SIZE 2048
#define THREAD_TIMESLICE 5

struct rt_semaphore mutex;//������

struct rt_semaphore empty;//�ź���
struct rt_semaphore apple;
struct rt_semaphore banana;

static rt_thread_t productor_tid = RT_NULL;
static rt_thread_t consumer_son_tid = RT_NULL;
static rt_thread_t consumer_daughter_tid = RT_NULL;

void productor_entry(void *parameter)
{
    int num = 10;
    while (num--)
    {
        rt_sem_take(&empty, RT_WAITING_FOREVER);//ͬ��
        rt_sem_take(&mutex, RT_WAITING_FOREVER);//����
        int now = rand() % 2;           //���Ƿ�ʲôˮ��
        if(now == 0)
        {
            rt_sem_release(&apple);
            rt_kprintf("father put a apple\n");
        }
        else
        {
            rt_sem_release(&banana);
            rt_kprintf("father put a banana\n");
        }
        rt_sem_release(&mutex);
        rt_thread_mdelay(200);
    }
}
void consumer_son_entry(void *parameter)
{
    while(1)
    {
        rt_sem_take(&apple, RT_WAITING_FOREVER);//ͬ��
        rt_sem_take(&mutex, RT_WAITING_FOREVER);//����
        rt_kprintf("son get a apple\n");
        rt_sem_release(&empty);         //ȡ��ƻ��
        rt_sem_release(&mutex);
        rt_thread_mdelay(200);
    }
}
void consumer_daughter_entry(void *parameter)
{
    while(1)
    {
        rt_sem_take(&banana, RT_WAITING_FOREVER);//ͬ��
        rt_sem_take(&mutex, RT_WAITING_FOREVER);//����
        rt_kprintf("daughter get a banana\n");
        rt_sem_release(&empty);         //ȡ���㽶
        rt_sem_release(&mutex);
        rt_thread_mdelay(200);
    }
}
int productor_consumer(void)
{
    rt_sem_init(&mutex, "mutex", 1, RT_IPC_FLAG_FIFO);      //��ʼ���ź���
    rt_sem_init(&empty, "empty", 2, RT_IPC_FLAG_FIFO);
    rt_sem_init(&apple, "apple", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&banana, "banana", 0, RT_IPC_FLAG_FIFO);
    
    //����
    productor_tid = rt_thread_create("father",
                                      productor_entry, RT_NULL,
                                      THREAD_STACK_SIZE,
                                      THREAD_PRIORITY, THREAD_TIMESLICE);
    if (productor_tid != RT_NULL)
        rt_thread_startup(productor_tid);

    //����
    consumer_son_tid = rt_thread_create("son",
                                      consumer_son_entry, RT_NULL,
                                      THREAD_STACK_SIZE,
                                      THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (consumer_son_tid != RT_NULL)
        rt_thread_startup(consumer_son_tid);
        
    //Ů��
    consumer_daughter_tid = rt_thread_create("daughter",
                                      consumer_daughter_entry, RT_NULL,
                                      THREAD_STACK_SIZE,
                                      THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (consumer_daughter_tid != RT_NULL)
        rt_thread_startup(consumer_daughter_tid);

    return 0;
}
/* �� �� �� msh �� �� �� �� �� */
MSH_CMD_EXPORT(productor_consumer, productor_consumer sample);


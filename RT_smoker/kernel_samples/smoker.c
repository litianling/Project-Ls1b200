/*
 * smoker.c
 *
 * created: 2022/1/12
 *  author: 
 */

#include <rtthread.h>
#include <rtdef.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_PRIORITY 3
#define THREAD_STACK_SIZE 1024
#define THREAD_TIMESLICE 5

static rt_thread_t provider_tid = RT_NULL;
static rt_thread_t tobacco_tid = RT_NULL;
static rt_thread_t paper_tid = RT_NULL;
static rt_thread_t glue_tid = RT_NULL;


struct rt_semaphore sem_offer1, sem_offer2, sem_offer3, sem_finish;

void provider_thread_entry(void *parameter){
    while(1){
        int r = rand() % 3;
        //rt_kprintf("%d", r);
        if(r == 0){
            rt_kprintf("Tobacco and paper are provided...\n");
            rt_sem_release(&sem_offer1);
        }
        else if(r == 1){
            rt_kprintf("Tobacco and glue are provided...\n");
            rt_sem_release(&sem_offer2);
        }
        else if(r == 2){
            rt_kprintf("Paper and glue are provided...\n");
            rt_sem_release(&sem_offer3);
        }
        rt_thread_mdelay(200);
        rt_sem_take(&sem_finish, RT_WAITING_FOREVER);
    }
    rt_kprintf("provider exit!\n");
}

void tobacco_thread_entry(void *parameter){
    while(1){
        rt_sem_take(&sem_offer3, RT_WAITING_FOREVER);
        rt_kprintf("People who own tobacco started smoking...\n");
        rt_thread_mdelay(200);
        rt_sem_release(&sem_finish);
    }
    rt_kprintf("Tobacco owner complete smoking...\n");
    rt_kprintf("tobacco owner exit!\n");
}

void paper_thread_entry(void *parameter){
    while(1){
        rt_sem_take(&sem_offer2, RT_WAITING_FOREVER);
        rt_kprintf("People who own paper started smoking...\n");
        rt_thread_mdelay(200);
        rt_sem_release(&sem_finish);
    }
    rt_kprintf("paper owner complete smoking...\n");
    rt_kprintf("paper owner exit!\n");
}

void glue_thread_entry(void *parameter){
    while(1){
        rt_sem_take(&sem_offer1, RT_WAITING_FOREVER);
        rt_kprintf("People who own glue started smoking...\n");
        rt_thread_mdelay(200);
        rt_sem_release(&sem_finish);
    }
    rt_kprintf("glue owner complete smoking...\n");
    rt_kprintf("glue owner exit!\n");
}

int smoker(void){
    //初始化信号量
    rt_sem_init(&sem_offer1, "offer1", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_offer2, "offer2", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_offer3, "offer3", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_finish, "finish", 0, RT_IPC_FLAG_FIFO);
    
    //创建提供者进线程
    provider_tid = rt_thread_create("provider",
                                    provider_thread_entry,
                                    RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY - 1,
                                    THREAD_TIMESLICE
    );
    if(provider_tid != RT_NULL){
        rt_thread_startup(provider_tid);
    }
    
    //创建拥有烟草的人的线程
    tobacco_tid = rt_thread_create("tobacco",
                                    tobacco_thread_entry,
                                    RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY + 1,
                                    THREAD_TIMESLICE
    );
    if(tobacco_tid != RT_NULL){
        rt_thread_startup(tobacco_tid);
    }
    
    //创建拥有纸的人的线程
    paper_tid = rt_thread_create("paper",
                                    paper_thread_entry,
                                    RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY + 1,
                                    THREAD_TIMESLICE
    );
    if(paper_tid != RT_NULL){
        rt_thread_startup(paper_tid);
    }
    
    //创建拥有胶水的人的线程
    glue_tid = rt_thread_create("glue",
                                    glue_thread_entry,
                                    RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY + 1,
                                    THREAD_TIMESLICE
    );
    if(glue_tid != RT_NULL){
        rt_thread_startup(glue_tid);
    }
    return 0;
}


MSH_CMD_EXPORT(smoker, smoker question);







































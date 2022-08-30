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
 * ç¨‹åºæ¸…å•ï¼šåˆ›å»º/åˆ é™¤ã€åˆå§‹åŒ–çº¿ç¨‹
 *
 * è¿™ä¸ªä¾‹å­ä¼šåˆ›å»ºä¸¤ä¸ªçº¿ç¨‹ï¼Œä¸€ä¸ªåŠ¨æ€çº¿ç¨‹ï¼Œä¸€ä¸ªé™æ€çº¿ç¨‹ã€‚
 * ä¸€ä¸ªçº¿ç¨‹åœ¨è¿è¡Œå®Œæ¯•åè‡ªåŠ¨è¢«ç³»ç»Ÿåˆ é™¤ï¼Œå¦ä¸€ä¸ªçº¿ç¨‹ä¸€ç›´æ‰“å°è®¡æ•°ã€‚
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

/* Ïß ³Ì 1 µÄ Èë ¿Ú º¯ Êı */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count=0;

    while (1)
    {
        /* Ïß³Ì1²ÉÓÃµÍÓÅÏÈ¼¶ÔËĞĞ£¬Ò»Ö±´òÓ¡¼ÆÊıÖµ*/
        rt_kprintf("thread1 count: %d\n", count ++);
        rt_thread_mdelay(500);

    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
static char thread3_stack[1024];
static struct rt_thread thread3;
/* Ïß ³Ì 2 Èë ¿Ú */
static void thread2_entry(void *param)
{
    rt_uint32_t count = 0;

    /* Ïß ³Ì 2 Óµ ÓĞ ×î ¸ß µÄ ÓÅ ÏÈ ¼¶£¬ ÒÔ ÇÀ Õ¼ Ïß ³Ì 1 ¶ø »ñ µÃ Ö´ ĞĞ */
    for (count = 0; count < 10 ; count++)
    {
        /* Ïß ³Ì 2 ´ò Ó¡ ¼Æ Êı Öµ */
        rt_kprintf("thread2 count: %d\n", count);
    }
    rt_kprintf("thread2 exit\n");
    /* Ïß ³Ì 2 ÔË ĞĞ ½á Êø ºó Ò² ½« ×Ô ¶¯ ±» Ïµ Í³ ÍÑ Àë */
}

static void thread3_entry(void *param)
{
    rt_uint32_t count = 0;

    /* Ïß ³Ì 3 Óµ ÓĞ ½Ï ¸ß µÄ ÓÅ ÏÈ ¼¶£¬ ÒÔ ÇÀ Õ¼ Ïß ³Ì 1 ¶ø »ñ µÃ Ö´ ĞĞ */
    for (count = 10; count < 25 ; count++)
    {
        /* Ïß ³Ì 2 ´ò Ó¡ ¼Æ Êı Öµ */
        rt_kprintf("thread3 count: %d\n", count-10);
    }
    rt_kprintf("thread3 exit\n");
    /* Ïß ³Ì 2 ÔË ĞĞ ½á Êø ºó Ò² ½« ×Ô ¶¯ ±» Ïµ Í³ ÍÑ Àë */
}

/* Ïß ³Ì Ê¾ Àı */
int thread_sample(void)
{
    /* ´´ ½¨ Ïß ³Ì 1£¬ Ãû ³Æ ÊÇ thread1£¬ Èë ¿Ú ÊÇ thread1_entry*/
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    
    /* Èç ¹û »ñ µÃ Ïß ³Ì ¿Ø ÖÆ ¿é£¬ Æô ¶¯ Õâ ¸ö Ïß ³Ì */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    /* ³õ Ê¼ »¯ Ïß ³Ì 2£¬ Ãû ³Æ ÊÇ thread2£¬ Èë ¿Ú ÊÇ thread2_entry */
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

/* µ¼ ³ö µ½ msh Ãü Áî ÁĞ ±í ÖĞ */
MSH_CMD_EXPORT(thread_sample, thread sample);

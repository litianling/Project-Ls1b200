/*
 * read_disc.c
 *
 * created: 2022/4/11
 *  author: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bsp.h"
#include "ls1b.h"
#include "ls1b_gpio.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"
#include "usb.h"
#include "blk.h"

#define  getch  usb_kbd_getc

#include <rtthread.h>
#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       0x00800000  //2M
#define THREAD_TIMESLICE        500

static rt_thread_t read_disk1 = RT_NULL;
extern void dos_test(struct blk_desc *desc);

void read_disk_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    fb_cons_clear();
    
    int cur_msc;                            // 当前大容量存储器=>有无USB设备连接，有0，无-1
    struct blk_desc *msc_desc;              // 指针--->大容量存储器设备描述符
    cur_msc = usb_stor_scan(1);             // 扫描USB存储设备【*】参数1为显示模式，如果有USB连接返回当前大容量存储器（编号）从0开始，没有返回-1
    msc_desc = get_usb_msc_blk_dev(cur_msc);// 根据编号cur_msc在usb_dev_desc找到相应的大容量存储器设备描述符【*】

    if(msc_desc!=NULL)                      // 如果这个设备描述符非空
    {
        int rdcount;
        unsigned char *blk_buf;
        dos_test(msc_desc);                 // 用设备描述符进行操作【*】
    }
    
    printf("Please enter key to exit.");
    getch();                                // 阻断退出
    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
}

int read_disk(void)
{
    read_disk1 = rt_thread_create("read_disk",
                            read_disk_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(read_disk1);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(read_disk, read disk);

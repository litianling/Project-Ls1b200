/*
 * read_disc.c
 *
 * created: 2022/4/11
 *  author: 
 */

#include "bsp.h"
#include <rtthread.h>

#define  getch  usb_kbd_getc

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       0x00800000  //2M
#define THREAD_TIMESLICE        500

static rt_thread_t read_disk1 = RT_NULL;
extern void dos_test(struct blk_desc *desc);

char * gets_filename(char *s)
{
    int i=0;
    for(;i<38;i++)
    {
        *s = getch();
        if((*s=='\r')||(*s=='\n'))
            break;
        printk("%c",*s);
        s++;
    }
    *s = NULL;
    return;
}

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
        char cmd[40];
        while(1)
        {
            printk("Please input cmd(ls/cat/exit):");
            gets_filename(cmd);
            printk("\n\r");
            if((cmd[0]=='l')&&(cmd[1]=='s'))
                dos_read_dir(msc_desc,cmd+3);  // 用设备描述符进行操作【*】
            else if((cmd[0]=='c')&&(cmd[1]=='a')&&(cmd[2]=='t'))
                dos_read(msc_desc,cmd+4);
            else if((cmd[0]=='e')&&(cmd[1]=='x')&&(cmd[2]=='i')&&(cmd[3]=='t'))
                break;
        }
    }
    
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

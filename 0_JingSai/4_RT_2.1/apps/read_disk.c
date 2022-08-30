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
    
    int cur_msc;                            // ��ǰ�������洢��=>����USB�豸���ӣ���0����-1
    struct blk_desc *msc_desc;              // ָ��--->�������洢���豸������
    cur_msc = usb_stor_scan(1);             // ɨ��USB�洢�豸��*������1Ϊ��ʾģʽ�������USB���ӷ��ص�ǰ�������洢������ţ���0��ʼ��û�з���-1
    msc_desc = get_usb_msc_blk_dev(cur_msc);// ���ݱ��cur_msc��usb_dev_desc�ҵ���Ӧ�Ĵ������洢���豸��������*��

    if(msc_desc!=NULL)                      // �������豸�������ǿ�
    {
        int rdcount;
        unsigned char *blk_buf;
        dos_test(msc_desc);                 // ���豸���������в�����*��
    }
    
    printf("Please enter key to exit.");
    getch();                                // ����˳�
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

/* ������ msh �����б��� */
MSH_CMD_EXPORT(read_disk, read disk);

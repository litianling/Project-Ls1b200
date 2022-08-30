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
    
    int cur_msc;                            // ��ǰ�������洢��=>����USB�豸���ӣ���0����-1
    struct blk_desc *msc_desc;              // ָ��--->�������洢���豸������
    cur_msc = usb_stor_scan(1);             // ɨ��USB�洢�豸��*������1Ϊ��ʾģʽ�������USB���ӷ��ص�ǰ�������洢������ţ���0��ʼ��û�з���-1
    msc_desc = get_usb_msc_blk_dev(cur_msc);// ���ݱ��cur_msc��usb_dev_desc�ҵ���Ӧ�Ĵ������洢���豸��������*��


    if(msc_desc!=NULL)                      // �������豸�������ǿ�
    {
        char cmd[40];
        while(1)
        {
            printk("Please input cmd(ls/cat/exit):");
            gets_filename(cmd);
            printk("\n\r");
            if((cmd[0]=='l')&&(cmd[1]=='s'))
                dos_read_dir(msc_desc,cmd+3);  // ���豸���������в�����*��
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

/* ������ msh �����б��� */
MSH_CMD_EXPORT(read_disk, read disk);

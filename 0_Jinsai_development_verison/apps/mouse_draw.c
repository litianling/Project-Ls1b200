/*
 * mouse_draw.c
 *
 * created: 2022/5/23
 *  author: 
 */

#include "bsp.h"
#include <rtthread.h>

#define printf  printk
#define getch   usb_kbd_getc

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        50
static rt_thread_t mouse_draw1 = RT_NULL;

void mouse_draw_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    usb_mouse_draw_unlock();

    Mouse_logical_coordinates_reset();      // ������ʼ�� ���ù��λ��
    fb_cons_clear();
    
    int ch = 0;
    while (ch != 4)                         // ���¹����˳�
    {
        if (usb_mouse_testc())              // ����Ƿ�������
        {
            ch = usb_mouse_getc();          // ��ȡ������룬���������ַ�
            //printf(" %d \n",ch);          // ch=1��������ch=2����Ҽ���ch=4������
        }
        //delay_ms(5);
    }
    
    fb_cons_clear();
    extern int show_background;
    show_background = 1;
    usb_mouse_draw_lock();
    change_scheduler_lock(0);
}

int mouse_draw(void)
{
    mouse_draw1 = rt_thread_create("mouse_draw",
                            mouse_draw_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(mouse_draw1);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(mouse_draw, mouse draw);

/*
 * L610_Telephone.c
 *
 * created: 2022/4/25
 *  author: 
 */

#include <rtthread.h>
#include "L610_include/TCP.h"
#include "bsp.h"

#define  getch  usb_kbd_getc
#define  cmd_max_len 20

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024*2
#define THREAD_TIMESLICE        50
static rt_thread_t L610_Telephone1 = RT_NULL;

void exit_Telephone(void)
{
    printk("Please enter key to exit.");
    getch();                                // ����˳�
    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
}

void L610_Telephone_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    fb_cons_clear();

    printk("\r\n");
    printk(" Mode 1: Call up\r\n");
    printk(" Mode 2: Press Key\r\n");
    printk(" Mode 3: Answer the phone\r\n");
    printk(" Mode 4: Hang up the phone\r\n");
    printk("Please select mode:\r\n");
    char mode = getch();

    if(mode=='1')
    {
        send_at_command("AT+GTSET=\"GPRSFIRST\",0\r\n");    // �������ȣ����籣�棩
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);

        send_at_command("AT+GTSET=\"CALLBREAK\",1\r\n");    // �����ж����ݣ����籣�棩
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);

        send_at_command("AT+CLIP=1\r\n");                   // ��ʾ������루���粻���棩
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);

        printk("Please input the telephone number:\r\n");
        char telephone[cmd_max_len];
        char *telephone1 = telephone;
        gets_cmd_2(telephone1);
        char cmd[32];
        snprintf(cmd, 31, "ATD%s;\r\n", telephone);          // ���ַ�������tmp
        send_at_command(cmd);                               // ���Ͳ���
        wait_for_reply(NULL, "OK", 0);
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);

        send_at_command("AT+CLVL=5\r\n");                   // ����speaker����
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);
        
        exit_Telephone();
        return 0;
    }
    else if(mode=='2')
    {
        printk("Please input the key:\r\n");
        char key = getch();
        char cmd[32];
        
        snprintf(cmd, 31, "AT+CLVL=%c\r\n", key);           // ���ַ�������cmd
        send_at_command(cmd);                               // ����ģ�ⰴ��
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);
        
        exit_Telephone();
        return 0;
    }
    else if(mode=='3')
    {
        send_at_command("ATA\r\n");                         // �����绰
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);
        
        exit_Telephone();
        return 0;
    }
    else if(mode=='4')
    {
        send_at_command("ATH\r\n");                         // �Ҷϵ绰
        wait_for_reply(NULL, "OK", 0);
        delay_ms(1000);
        
        exit_Telephone();
        return 0;
    }
    
    exit_Telephone();
    return;
}

int L610_Telephone(void)
{
    L610_Telephone1 = rt_thread_create("L610_Telephone",
                            L610_Telephone_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_Telephone1);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(L610_Telephone, L610 Telephone);


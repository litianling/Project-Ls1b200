/*
 * L610_TXcloud.c
 *
 * created: 2022/4/30
 *  author: 
 */

#include <rtthread.h>
#include <string.h>
#include "bsp.h"
#include "ns16550.h"
#include "L610_TCP.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"

#define  getch  usb_kbd_getc
#define  cmd_max_len 200

#define THREAD_PRIORITY         20
#define THREAD_STACK_SIZE       1024*8
#define THREAD_TIMESLICE        500
static rt_thread_t L610_TXcloud1 = RT_NULL;
int wait_for_reply_cloud(const char *verstr, const char *endstr, int waitms);

int L610_TXcloud_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);

    send_at_command("AT+TCDEVINFOSET=1,\"MQ48NH81TC\",\"weilaiRoad_Lamp001\",\"AyPF0rmwJ+4zyEA0/aqpiQ==\"\r\n");      // ����ƽ̨�豸��Ϣ����ƷID/�豸����/��Կ��
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+TCMQTTCONN=1,20000,240,1,1\r\n");                                               // �������Ӳ���������
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    send_at_command("AT+TCMQTTSUB=\"$thing/down/property/MQ48NH81TC/weilaiRoad_Lamp001\",1\r\n");       // �����ϱ��������Ա�ǩ
    wait_for_reply(NULL, "OK", 0);
    delay_ms(5000);

    send_at_command("AT+TCMQTTPUB=\"$thing/up/property/MQ48NH81TC/weilaiRoad_Lamp001\",1,\"{\"method\":\"report\",\"clientToken\":\"123\",\"params\":{\"power_switch\":0}}\"\r\n");
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);


    while(1)
    {
        wait_for_reply_cloud(NULL, NULL, 2000);
        delay_ms(1000);
    }


    // never go here
    send_at_command("AT+TCMQTTDISCONN\r\n");                                                            // �Ͽ�����
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);

    change_scheduler_lock(0);
    return 0;
}

int L610_TXcloud(void)
{
    L610_TXcloud1 = rt_thread_create("L610_TXcloud",
                            L610_TXcloud_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_TXcloud1);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(L610_TXcloud, L610 ten xun yun);


//*******************************************************************************

int wait_for_reply_cloud(const char *verstr, const char *endstr, int waitms)
{
    char tmp[400], *ptr = tmp;
    int  rt = 0, tmo;

    memset(tmp, 0, 400);                            // ���������, �������ݹ۲�
    tmo = waitms > 0 ? waitms : 1000;

    if ((endstr == NULL) || (strlen(endstr) == 0))  // û�����ý����ַ���ȫ������
    {
        rt = ls1x_uart_read(devUART1, ptr, 400, tmo);
    }
    else
    {
        while (tmo > 0)
        {
            /*
             * ÿ�ζ�10���ֽ�, ��ʱ2ms: ��UART1���������.
             */
            int readed = ls1x_uart_read(devUART1, ptr, 10, 2);  // 10 bytes; 2 ms

            if (readed > 0)
            {
                ptr += readed;
                rt  += readed;

                if ((rt >= 400) || strstr(tmp, endstr))         // ���������ַ���
                    break;
            }

            tmo -= 2;
        }
    }

    if (rt <= 2)                                                // �ж��ǲ��ǳ�ʱ�˳�
        return -1;

    /*
     * �����ַ������� "\r\n" ����, ������©
     */
    if ((tmp[rt-2] != '\r') && (tmp[rt-1] != '\n'))
    {
        rt += ls1x_uart_read(devUART1, ptr, 10, 2);
    }

//    printk(" RX: %s\r\n", tmp);                                 // ������յ�����Ϣ******����г������ƣ���Ҫ�Ľ���������̶�ȡ��
//    printk("%d",rt);
    int offset=0;
    printk(" RX: ");
    printk("%s\r\n", tmp);
    if(*(tmp+126))
        printk("%s\r\n", tmp+126);
    if(*(tmp+252))
        printk("%s\r\n", tmp+252);
    if(*(tmp+378))
        printk("%s\r\n", tmp+378);

    if ((verstr != NULL) && (strlen(verstr) > 0))               // �������ж��ַ�
    {
        if (strstr(tmp, verstr) != NULL)
            return rt;
        else
            return -1;
    }

    return rt;
}

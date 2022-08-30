/*
 * L610_ShortMessage.c
 *
 * created: 2022/4/25
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
#include "pdu.h"

#define  getch  usb_kbd_getc
#define  cmd_max_len 40

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       4096*2
#define THREAD_TIMESLICE        50
static rt_thread_t L610_ShortMessage1 = RT_NULL;
int wait_for_reply_read_message(const char *verstr, const char *endstr, int waitms);

int L610_ShortMessage_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);

    while(1)
    {
        printk("\r\n");
        printk(" Mode 0: Set up\r\n");
        printk(" Mode 1: Send TXT messages\r\n");
        printk(" Mode 2: Send PDU messages\r\n");
        printk(" Mode 3: Read SMS\r\n");
        printk(" Mode 4: Delete SMS\r\n");
        printk(" Mode 5: Report\r\n");
        printk(" Else: Exit SMS mode\r\n");
        printk("Please select mode:\r\n");
        char mode = getch();
        printk("mode is %c\r\n",mode);
        
        if(mode=='0')
        {
            send_at_command("AT+CSCA?\r\n");            // ��ѯ�������ĺ���
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
            
            send_at_command("AT+CPMS=\"SM\"\r\n");      // �������ȴ洢��SIM��
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
            
            send_at_command("AT+CNMI=2,1,0,0,0\r\n");   // �ϱ��յ��Ķ��ű��
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);

            send_at_command("AT+CNMI=2,2,0,0,0\r\n");   // �ϱ��յ��Ķ�������
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
        }
        else if(mode=='1')
        {
            send_at_command("AT+CMGF=1\r\n");           // �ı�����ģʽ
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
            
            send_at_command("AT+CSMP=17,167,0,0\r\n");  // ��Ч��24Сʱ
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
            
            printk("Please input the telephone number:\r\n");
            char telephone[cmd_max_len];
            char *telephone1 = telephone;
            gets_cmd_2(telephone1);
            char cmd[32];
            snprintf(cmd, 31, "AT+CMGS=\"%s\"\r\n", telephone);
            send_at_command(cmd);                       // ���ý��պ���-----------
            wait_for_reply(NULL, ">", 0);
            delay_ms(1000);
            
            printk("Please input the SMS content:\r\n");
            char content[cmd_max_len];
            char *content1 = content;
            gets_cmd_2(content1);
            char cmd1[32];
            snprintf(cmd1, 31, "%s\r\n", content);
            send_at_command(cmd1);                      // ���ö�������-----------
            wait_for_reply(NULL, NULL, 0);
            delay_ms(1000);
            
            const char end = 0x1a;
            const char * end1 = &end;
            send_at_command(end1);                      // ���ͽ�����
            wait_for_reply(NULL, NULL, 0);              // �ս�����
            delay_ms(1000);
            wait_for_reply(NULL, "OK", 0);              // �շ�����λ��
            delay_ms(1000);
            wait_for_reply(NULL, NULL, 0);              // ���ϱ���������
            delay_ms(1000);
        }
        else if(mode=='2')
        {
            send_at_command("AT+CMGF=0\r\n");           // PDU����ģʽ
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
            
            send_at_command("AT+CMGS=18\r\n");          // ���ͳ���
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
            
            send_at_command("0891683108100005F011000B918108215684F40008B0044F60597D");           // PDU����ģʽ
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
        }
        else if(mode=='3')
        {
            printk("Please input the location(1-9):\r\n");
            char cmd[32];
            char location;
            int location_num = 0;
            location = getch();
            printk("%c",location);
            while((location>='0')&&(location<='9'))
            {
                location_num = location_num*10 + location-48;
                location = getch();
                printk("%c",location);
            }
            snprintf(cmd, 31, "AT+CMGR=%d\r\n", location_num);
            send_at_command(cmd);
            //wait_for_reply(NULL, "OK", 0);
            wait_for_reply_read_message(NULL, "OK", 0);
            delay_ms(1000);
        }
        else if(mode=='4')
        {
            printk("Please input the location(1-9):\r\n");
            char cmd[32];
            char location;
            int location_num = 0;
            location = getch();
            printk("%c",location);
            while((location>='0')&&(location<='9'))
            {
                location_num = location_num*10 + location-48;
                location = getch();
                printk("%c",location);
            }
            snprintf(cmd, 31, "AT+CMGD=%d\r\n", location_num);
            send_at_command(cmd);
            wait_for_reply(NULL, "OK", 0);
            delay_ms(1000);
        }
        else if(mode=='5')
        {
            wait_for_reply(NULL, "OK", 0);              // �ȴ��ϱ���Ӧ
            delay_ms(1000);
        }
        else
        {
            change_scheduler_lock(0);
            return 0;
        }
    }
    
    change_scheduler_lock(0);
    return 0;
}

int L610_ShortMessage(void)
{
    L610_ShortMessage1 = rt_thread_create("L610_ShortMessage",
                            L610_ShortMessage_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_ShortMessage1);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(L610_ShortMessage, L610 ShortMessage);


void strcpy_message_data(char * src,char * dest)
{
    int begin=0;
    while(src!=NULL)
    {
        if((*src=='E')&&(*(src+1)=='R'))
        {
            *dest++ = NULL;
            return ;
        }
        
        if(((*src>='0')&&(*src<='9')) || ((*src>='A')&&(*src<='F')) || ((*src>='a')&&(*src<='f')))
        {
            if((*src=='0')&&(*(src+1)=='8'))
                begin=1;
                
            if(begin==1)
                *dest++ = *src++;
            else
                src++;
        }
        else if(begin==1)
            break;
        else
            src++;
    }
    *dest++ = NULL;
    return ;
}

int wait_for_reply_read_message(const char *verstr, const char *endstr, int waitms)
{
    char tmp[200], *ptr = tmp;
    int  rt = 0, tmo;

    memset(tmp, 0, 200);                            // ���������, �������ݹ۲�
    tmo = waitms > 0 ? waitms : 1000;

    if ((endstr == NULL) || (strlen(endstr) == 0))  // û�����ý����ַ���ȫ������
    {
        rt = ls1x_uart_read(devUART1, ptr, 200, tmo);
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

                if ((rt >= 200) || strstr(tmp, endstr))         // ���������ַ���
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
    printk(" RX: %s\r\n", tmp);                                 // ������յ�����Ϣ


    SM_PARA	sSmsPara;
    char message_data[200];
    delay_ms(1000);
    strcpy_message_data(tmp,message_data);
    //printk("  begin= %s",message_data);
    PDU_Decode( (INT8U *)message_data, strlen(message_data), &sSmsPara );
	printk("\r\n%s\r\n", (char*)sSmsPara.u8TP_UD);

    return rt;
}



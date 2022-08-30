
/******************************************************************************
 *                       @ Copyright 2014 - 2020                              *
 *                           www.diysoon.com                                  *
 ******************************************************************************/
#include <string.h>
#include "ns16550.h"
#include "TCP.h"
#include "bsp.h"
//-----------------------------------------------------------------------------
// ������������ʵ��: ��о1B��L610��ͨѶ.
//-----------------------------------------------------------------------------

/*
 * ͨ�� UART1 �� L610 ģ�鷢�� AT ����
 */
int send_at_command(const char *buf)
{
    int rt;
    rt = ls1x_uart_write(devUART1, buf, strlen(buf), NULL);     // �����������
    printk(" TX: %s\r\n", buf);                                // ���Է��͵�����
    return rt;
}

int send_at_command_TXcloud(const char *buf)
{
    int rt;
    rt = ls1x_uart_write(devUART1, buf, strlen(buf), NULL);     // �����������
    return rt;
}
/*
 * �� UART1 ��ȡ L610 ģ���Ӧ����Ϣ
 *
 * ����:    verstr:     �����յ��ַ������Ƿ�������ַ���
 *          endstr:     ���յ��ַ����Դ˽�β
 *          waitms:     ���յȴ���ʱ
 *
 * ����:    >0:         �����ַ��� verstr
 *          <=0:        ���մ���
 */
int wait_for_reply(const char *verstr, const char *endstr, int waitms)
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

    printk(" RX: %s\r\n", tmp);                                 // ������յ�����Ϣ******����г������ƣ���Ҫ�Ľ���������̶�ȡ��
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


int wait_for_reply_TXcloud(const char *verstr, const char *endstr, int waitms)
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

    
    if ((verstr != NULL) && (strlen(verstr) > 0))               // �������ж��ַ�
    {
        if (strstr(tmp, verstr) != NULL)
            return rt;
        else
            return -1;
    }

    return rt;
}
//-----------------------------------------------------------------------------

/**
 * ������Ӫ�̷���IP
 * ��ӦATָ�AT+mipcall =
 * ��Ӫ�̲�ͬ����ָ��������
 */
void l610_ip_allo(void)
{
    int tmo = 0;

	send_at_command("AT+mipcall=1,\"ctnet\"\r\n");     // ������Ӫ�̷��� IP
	wait_for_reply(NULL, "OK", 2000);
	send_at_command("AT+mipcall?\r\n");                // ����Ƿ����IP
	
	while ((wait_for_reply("+MIPCALL: 1", "OK", 2000) <= 0) && (tmo++ < 5))
	{
        send_at_command("AT+mipcall=0\r\n");
        wait_for_reply(NULL, "OK", 2000);
        send_at_command("AT+mipcall=1,\"ctnet\"\r\n");
        wait_for_reply(NULL, "OK", 2000);
        send_at_command("AT+mipcall?\r\n");
    }
}

/**
 * ��ѯ��ǰ���õ�socket ID
 * socket ID ������˵����ǰû�� TCP ���ӡ�
 * ��ӦATָ�AT+MIPOPEN?
 */
void l610_check_socket(void)
{
	send_at_command("AT+MIPOPEN?\r\n");
    wait_for_reply("+MIPOPEN:", "OK", 2000);
}

/**
 * ͨ��socket����TCP����
 * ��ӦATָ�AT+MIPOPEN=1,,\"47.92.117.163\",30000,0
 */
void l610_creat_tcp(void)
{
	send_at_command("AT+MIPOPEN=1,,\"120.24.31.181\",3000,0\r\n"); // ����TCP����
    //send_at_command("AT+MIPOPEN=1,,\"10.2.55.18\",3000,0\r\n"); // ����TCP����
    wait_for_reply("+MIPOPEN: 1,1", "OK", 2000);
}

/**
 * �������ݣ���TCP���ӽ�������ò���Ч���������������������
 * ��Ҫ���뱣�����ݵ�����
 * ��ӦATָ�AT+MIPSEND
 */
void l610_send_data(char *data)
{
    char tmp[32];
    snprintf(tmp, 31, "AT+MIPSEND=1,%i\r\n", strlen(data));  // ���ַ�������tmp
    send_at_command(tmp);                                    // ��tmp���͵�ģ��
    send_at_command(data);
    wait_for_reply(NULL, NULL, 1000);
}

/**
 * �ر�TCP����
 * ���������շ����������쳣�����Թرյ��������� TCP ����
 * ��ӦATָ�AT+MIPCLOSE=1
 */
void l610_end_tcp(void)
{
	send_at_command("AT+MIPCLOSE=1\r\n");
    wait_for_reply("+MIPCLOSE: 1,0", "OK", 2000);
}

/**
 * �ͷ�IP
 * �����ͷ�ģ�鱾�μ�����ȡ�� IP ��ַ
 * ��ӦATָ�AT+MIPCALL=0
 */
void l610_ip_release(void)
{
	send_at_command("AT+MIPCALL=0\r\n");
    wait_for_reply("+MIPCALL: 0", "OK", 2000);
}

/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B Bare Program, Sample main file
 */

#include <stdio.h>

#include "ls1b.h"
#include "mips.h"

//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"
#include "ns16550.h"

//-------------------------------------------------------------------------------------------------
// ������
//-------------------------------------------------------------------------------------------------

int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */
    ls1x_uart_open(devUART1, NULL);     /* ʹ��Ĭ�ϲ���: 115200,8N1 */

    l610_init_pre();                        // ͨ��ATָ����һ�Ѳ�������
    
    int count = 0;
    
    while (count++ < 2)
    {
		l610_init();                        // ͨ��ATָ���ģ��������ֽ��г�ʼ��
		l610_check_socket();                // ��ѯ��ǰ���õ�socket ID��socket ID ������˵����ǰû�� TCP ���ӡ� AT+MIPOPEN?
		l610_creat_tcp();                   // ͨ��socket����TCP���ӣ�AT+MIPOPEN=1,,\"47.92.117.163\",30000,0
		delay_ms(500);                      

        l610_send_data("a small dog");      // ���� ASCII ����
        l610_send_data("Hello Fibocom.");   // ���� ASCII ����
		l610_send_data("0123456789ABCDEF"); // ���� ASCII ����

		delay_ms(100);
		l610_end_tcp();                     // �ر�TCP���ӣ����������շ����������쳣�����Թرյ��������� TCP ���ӣ�AT+MIPCLOSE=1
		l610_ip_release();                  // �ͷ�IP�������ͷ�ģ�鱾�μ�����ȡ�� IP ��ַ AT+MIPCALL=0
		delay_ms(500);
    }
    
    return 0;
}


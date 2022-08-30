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
#include "libc/lwmem.h"
#include "usb.h"
#include "blk.h"


/*
 * print buffer as hex char
 */
void print_hex(char *p, int size)
{
	char hexval[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	int i, j;
	char ch_hi, ch_lo;
	uint32_t paddr = (uint32_t)p;
	j = paddr % 4;

	for (i = 1; i <= size + j; i++)
	{
		if (i <= j)
		{
			printk("  ");
		}
		else
		{
			ch_hi = hexval[(((uint8_t)p[0] >> 4) & 0x0F)];
			ch_lo = hexval[((uint8_t)p[0] & 0x0F)];

			printk("%c%c", ch_hi, ch_lo);

			p++;
		}

		if ((i % 4) == 0) printk(" ");
		if ((i % 32) == 0) printk("\n");
	}

	printk("\n");
}

//-------------------------------------------------------------------------------------------------
// ������
//-------------------------------------------------------------------------------------------------

// static unsigned char buf[0x020000]; // desc->blksz = 0x020000
extern void dos_test(struct blk_desc *desc);

int main(void)
{
    int cur_msc;                            // ��ǰ�������洢��=>����USB�豸���ӣ���0����-1
    struct blk_desc *msc_desc;              // ָ��--->�������洢���豸������
    unsigned int beginTicks;                // ��ʼ��ʱ��ֵ

    printk("\r\nmain() function.\r\n");
    lwmem_initialize(0);                    // �Ѵ洢���ռ��ʼ��Ϊ��ջ��*������0����

    LS1B_MUX_CTRL1 &= ~MUX_CTRL1_USB_SHUT;  // GPIO_MAX_CTRL1[11]=0����USB
    delay_ms(1);
    LS1B_MUX_CTRL1 |= MUX_CTRL1_USB_RESET;  // GPIO_MAX_CTRL1[31]=1��USB��λ
    delay_ms(1);

    beginTicks = get_clock_ticks();         // ��ȡ����ʼִ�е�ʱ��ֵ��ms��
    usb_init();                             // USBģ���ʼ����*��

#if (BSP_USE_MASS)                          // ���ʹ����USB�������洢�豸
    cur_msc = usb_stor_scan(1);             // ɨ��USB�洢�豸��*������1Ϊ��ʾģʽ�������USB���ӷ��ص�ǰ�������洢������ţ���0��ʼ��û�з���-1
    msc_desc = get_usb_msc_blk_dev(cur_msc);// ���ݱ��cur_msc��usb_dev_desc�ҵ���Ӧ�Ĵ������洢���豸��������*��
    
    while(msc_desc!=NULL)                   // �������豸�������ǿ�
    {
        int rdcount;
        unsigned char *blk_buf;
        printk("\n USB device is %d \n",cur_msc);
        dos_test(msc_desc);                 // ���豸���������в�����*��
        cur_msc++;
        msc_desc = get_usb_msc_blk_dev(cur_msc); // ��ȡ��һ���豸��*��
    }
#endif

#if (BSP_USE_MOUSE)                           // ����Ƿ�ʹ���˼���
    drv_usb_mouse_init();                     // ���̵�������ʼ��
    while (1)
    {
        if (usb_mouse_testc())                // ����Ƿ�������
        {
            int ch = usb_mouse_getc();        // ��ȡ�������룬���������ַ�
                       // ��ӡ�����ַ�
        }
        delay_ms(5);
    }
#endif

    usb_stop();                             // ����USB��ʹ�á�*��
    printk("Total ticks: %i\r\n",  get_clock_ticks() - beginTicks); // ����ʹ��ʱ��
    /*
     * OHCI: 2039 ticks
     * EHCI: 1302 ticks
     */
    printk("usb stop\r\n");                 // USBʹ�ý���
    return 0;
}




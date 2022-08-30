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

#ifdef BSP_USE_FB
  #include "ls1x_fb.h"
  #ifdef XPT2046_DRV
    char LCD_display_mode[] = LCD_800x480;
  #elif defined(GT1151_DRV)
    char LCD_display_mode[] = LCD_480x800;
  #else
    #error "��bsp.h��ѡ������ XPT2046_DRV ���� GT1151_DRV"
           "XPT2046_DRV:  ����800*480 �����Ĵ�����."
           "GT1151_DRV:   ����480*800 �����Ĵ�����."
           "�������ѡ��, ע�͵��� error ��Ϣ, Ȼ���Զ���: LCD_display_mode[]"
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// ������
//-------------------------------------------------------------------------------------------------

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

/* LTL_DEBUG �����Ʒ�� */
    unsigned char i;                    // i�ǿ�����USB�豸�ӿں�
    for (i = 0; i < USB_MAX_DEVICE; i++)// USB_MAX_DEVICE=32
    {
        struct usb_device *dev;         // ����USB�豸�ṹָ��
        dev = usb_get_dev_index(i);     // �����ӿڻ�ȡ�豸ָ�롾*��
        if(dev!=NULL)
        {
            printk("\n devnum=%d prod=%s \n",dev->devnum,dev->prod);
        }
    }


#if (BSP_USE_MASS)                          // ���ʹ����USB�������洢�豸
    cur_msc = usb_stor_scan(1);             // ɨ��USB�洢�豸��*������1Ϊ��ʾģʽ�������USB���ӷ��ص�ǰ�������洢������ţ���0��ʼ��û�з���-1
    msc_desc = get_usb_msc_blk_dev(cur_msc);// ���ݱ��cur_msc��usb_dev_desc�ҵ���Ӧ�Ĵ������洢���豸��������*��

    while(msc_desc!=NULL)                   // �������豸�������ǿ�
    {
        int rdcount;
        unsigned char *blk_buf;
        dos_test(msc_desc);                 // ���豸���������в�����*��
        cur_msc++;
        msc_desc = get_usb_msc_blk_dev(cur_msc); // ��ȡ��һ���豸��*��
    }

#endif

#if (BSP_USE_KBD)                           // ����Ƿ�ʹ���˼���
    drv_usb_kbd_init();                     // ���̵�������ʼ����*��ֻ�͵�һ�����̴������ӣ�һ����ʼ��һ�����˳���
    puts("please input:");
    while (1)
    {
        if (usb_kbd_testc())                // ����Ƿ������롾*������һ���ַ��Ƿ��ڶ�����,������
        {
            int ch = usb_kbd_getc();        // ��ȡ�������룬���������ַ���*��[�˴��Ѿ�����ʾ��]
            //printk("%c\n", ch);             // ��ӡ�����ַ�
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

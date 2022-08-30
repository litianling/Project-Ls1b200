/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B RT-Thread Application
 */


//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"
#include "ns16550.h"
#include "ls1b_gpio.h"

#define XPT2046_DRV
//#define LTL_DEBUG

#if defined(BSP_USE_FB)
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
// main function
//-------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    ls1x_drv_init();            /* Initialize device drivers */
    rt_ls1x_drv_init();         /* Initialize device drivers for RTT */
    install_3th_libraries(); 	/* Install 3th libraies */
    ls1x_uart_open(devUART1, NULL);

    fb_open();
    fb_cons_clear();
    fb_set_fgcolor(15,1);       // ������ɫ

    rt_show_version();
    rt_kprintf("\r\n Welcome to RT-Thread.\r\n\r\n");

    lwmem_initialize(0);                    // �Ѵ洢���ռ��ʼ��Ϊ��ջ��*������0����
    LS1B_MUX_CTRL1 &= ~MUX_CTRL1_USB_SHUT;  // GPIO_MAX_CTRL1[11]=0����USB
    delay_ms(1);
    LS1B_MUX_CTRL1 |= MUX_CTRL1_USB_RESET;  // GPIO_MAX_CTRL1[31]=1��USB��λ
    delay_ms(1);
    usb_init();                             // USBģ���ʼ����*��
    drv_usb_kbd_init();                     // ���̵�������ʼ����*��ֻ�͵�һ�����̴������ӣ�һ����ʼ��һ�����˳���
    drv_usb_mouse_init();                   // ����������ʼ��

/* LTL_DEBUG �����Ʒ�� */
#ifdef LTL_DEBUG
    unsigned char i;                    // i�ǿ�����USB�豸�ӿں�
    for (i = 0; i < USB_MAX_DEVICE; i++)// USB_MAX_DEVICE=32
    {
        struct usb_device *dev;         // ����USB�豸�ṹָ��
        dev = usb_get_dev_index(i);     // �����ӿڻ�ȡ�豸ָ�롾*��
        if(dev!=NULL)
        {
            printk("  devnum=%d prod=%s \n",dev->devnum,dev->prod);
        }
    }
    printk("\n");
#endif

    gpio_enable(Air_Q,DIR_IN);
    gpio_enable(SG180,DIR_OUT);
    gpio_enable(BUZZER,DIR_OUT);
    gpio_enable(MAN_S,DIR_IN);
    gpio_enable(TEMPE,DIR_IN);
    gpio_enable(TOUCH,DIR_IN);
    gpio_write(BUZZER,0);
    return 0;
}

int scheduler_lock = 0;

void change_scheduler_lock(int val)
{
    scheduler_lock = val;
    printk("scheduler_lock is %d \n",scheduler_lock);
}

int get_scheduler_lock()
{
    return scheduler_lock;
}


/*
 * @@ End
 */


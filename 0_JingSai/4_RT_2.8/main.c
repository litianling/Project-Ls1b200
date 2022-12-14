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


#if defined(BSP_USE_FB)
  #include "ls1x_fb.h"
  #ifdef XPT2046_DRV
    char LCD_display_mode[] = LCD_800x480;
  #elif defined(GT1151_DRV)
    char LCD_display_mode[] = LCD_480x800;
  #else
    char LCD_display_mode[] = LCD_800x480;
  #endif
#endif


//-------------------------------------------------------------------------------------------------
// main function
//-------------------------------------------------------------------------------------------------
int change_scheduler_lock(int val);

int main(int argc, char** argv)
{
    change_scheduler_lock(1);
    ls1x_drv_init();            /* Initialize device drivers */
    rt_ls1x_drv_init();         /* Initialize device drivers for RTT */
    install_3th_libraries(); 	/* Install 3th libraies */
    ls1x_uart_open(devUART1, NULL);

    fb_open();
    fb_set_fgcolor_mydef(15);   // 修改前景颜色
    fb_set_bgcolor_mydef(1);    // 修改背景颜色
    fb_cons_clear();

    rt_show_version();
    rt_kprintf("\r\n Welcome to RT-Thread.\r\n\r\n");

    lwmem_initialize(0);                    // 把存储器空间初始化为堆栈【*】参数0无用
    LS1B_MUX_CTRL1 &= ~MUX_CTRL1_USB_SHUT;  // GPIO_MAX_CTRL1[11]=0，打开USB
    delay_ms(1);
    LS1B_MUX_CTRL1 |= MUX_CTRL1_USB_RESET;  // GPIO_MAX_CTRL1[31]=1，USB复位
    delay_ms(1);
    usb_init();                             // USB模块初始化【*】
    drv_usb_kbd_init();                     // 键盘的驱动初始化【*】只和第一个键盘创建连接（一旦初始化一个就退出）
    drv_usb_mouse_init();                   // 鼠标的驱动初始化


/* LTL_DEBUG 输出产品号 */
#ifdef LTL_DEBUG
    unsigned char i;                    // i是开发板USB设备接口号
    for (i = 0; i < USB_MAX_DEVICE; i++)// USB_MAX_DEVICE=32
    {
        struct usb_device *dev;         // 定义USB设备结构指针
        dev = usb_get_dev_index(i);     // 遍历接口获取设备指针【*】
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
    
    change_scheduler_lock(0);
    return 0;
}

int scheduler_lock = 0;

int change_scheduler_lock(int val)
{
    scheduler_lock = val;
    printk("scheduler_lock is %d \n",scheduler_lock);
    return 0;
}

int get_scheduler_lock()
{
    return scheduler_lock;
}


/*
 * @@ End
 */


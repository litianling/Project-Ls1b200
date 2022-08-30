/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B RT-Thread Application
 */

#include <time.h>

#include "rtthread.h"
//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"
//#include "libc/lwmem.h"
#include "usb.h"
#include "blk.h"

#define XPT2046_DRV
#define LTL_DEBUG

#if defined(BSP_USE_FB)
  #include "ls1x_fb.h"
  #ifdef XPT2046_DRV
    char LCD_display_mode[] = LCD_800x480;
  #elif defined(GT1151_DRV)
    char LCD_display_mode[] = LCD_480x800;
  #else
    #error "在bsp.h中选择配置 XPT2046_DRV 或者 GT1151_DRV"
           "XPT2046_DRV:  用于800*480 横屏的触摸屏."
           "GT1151_DRV:   用于480*800 竖屏的触摸屏."
           "如果都不选择, 注释掉本 error 信息, 然后自定义: LCD_display_mode[]"
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// Simple demo of task
//-------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    ls1x_drv_init();            /* Initialize device drivers */
    rt_ls1x_drv_init();         /* Initialize device drivers for RTT */
    install_3th_libraries(); 	/* Install 3th libraies */

    fb_open();
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

    //usb_stop();                             // 结束USB的使用【*】
    //printk("usb stop\r\n");                 // USB使用结束

/* LTL_DEBUG 输出产品号 */
#ifdef LTL_DEBUG
    unsigned char i;                    // i是开发板USB设备接口号
    for (i = 0; i < USB_MAX_DEVICE; i++)// USB_MAX_DEVICE=32
    {
        struct usb_device *dev;         // 定义USB设备结构指针
        dev = usb_get_dev_index(i);     // 遍历接口获取设备指针【*】
        if(dev!=NULL)
        {
            printk("\n devnum=%d prod=%s \n",dev->devnum,dev->prod);
        }
    }
#endif


#if 0//(BSP_USE_KBD)                           // 检测是否使用了键盘
    printk("please input:");
    while (1)
    {
        if (usb_kbd_testc())                // 检测是否有输入【*】测试一个字符是否在队列中,带回显,去掉回显
        {
            int ch = usb_kbd_getc();        // 读取键盘输入，返回输入字符【*】[此处已经有显示了]
            printk("%c\n", ch);             // 打印输入字符
        }
        delay_ms(5);
    }
#endif

    return 0;
}

/*
 * @@ End
 */


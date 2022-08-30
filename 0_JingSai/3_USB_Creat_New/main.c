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
    #error "在bsp.h中选择配置 XPT2046_DRV 或者 GT1151_DRV"
           "XPT2046_DRV:  用于800*480 横屏的触摸屏."
           "GT1151_DRV:   用于480*800 竖屏的触摸屏."
           "如果都不选择, 注释掉本 error 信息, 然后自定义: LCD_display_mode[]"
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// 主程序
//-------------------------------------------------------------------------------------------------

extern void dos_test(struct blk_desc *desc);

int main(void)
{
    int cur_msc;                            // 当前大容量存储器=>有无USB设备连接，有0，无-1
    struct blk_desc *msc_desc;              // 指针--->大容量存储器设备描述符
    unsigned int beginTicks;                // 开始的时间值

    printk("\r\nmain() function.\r\n");
    lwmem_initialize(0);                    // 把存储器空间初始化为堆栈【*】参数0无用

    LS1B_MUX_CTRL1 &= ~MUX_CTRL1_USB_SHUT;  // GPIO_MAX_CTRL1[11]=0，打开USB
    delay_ms(1);
    LS1B_MUX_CTRL1 |= MUX_CTRL1_USB_RESET;  // GPIO_MAX_CTRL1[31]=1，USB复位
    delay_ms(1);

    beginTicks = get_clock_ticks();         // 获取程序开始执行的时间值（ms）
    usb_init();                             // USB模块初始化【*】

/* LTL_DEBUG 输出产品号 */
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


#if (BSP_USE_MASS)                          // 如果使用了USB大容量存储设备
    cur_msc = usb_stor_scan(1);             // 扫描USB存储设备【*】参数1为显示模式，如果有USB连接返回当前大容量存储器（编号）从0开始，没有返回-1
    msc_desc = get_usb_msc_blk_dev(cur_msc);// 根据编号cur_msc在usb_dev_desc找到相应的大容量存储器设备描述符【*】

    while(msc_desc!=NULL)                   // 如果这个设备描述符非空
    {
        int rdcount;
        unsigned char *blk_buf;
        dos_test(msc_desc);                 // 用设备描述符进行操作【*】
        cur_msc++;
        msc_desc = get_usb_msc_blk_dev(cur_msc); // 读取下一个设备【*】
    }

#endif

#if (BSP_USE_KBD)                           // 检测是否使用了键盘
    drv_usb_kbd_init();                     // 键盘的驱动初始化【*】只和第一个键盘创建连接（一旦初始化一个就退出）
    puts("please input:");
    while (1)
    {
        if (usb_kbd_testc())                // 检测是否有输入【*】测试一个字符是否在队列中,带回显
        {
            int ch = usb_kbd_getc();        // 读取键盘输入，返回输入字符【*】[此处已经有显示了]
            //printk("%c\n", ch);             // 打印输入字符
        }
        delay_ms(5);
    }
#endif

    usb_stop();                             // 结束USB的使用【*】
    printk("Total ticks: %i\r\n",  get_clock_ticks() - beginTicks); // 返回使用时间
    /*
     * OHCI: 2039 ticks
     * EHCI: 1302 ticks
     */
    printk("usb stop\r\n");                 // USB使用结束
    return 0;
}

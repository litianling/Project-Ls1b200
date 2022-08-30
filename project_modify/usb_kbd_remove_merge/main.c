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
// 主程序
//-------------------------------------------------------------------------------------------------

// static unsigned char buf[0x020000]; // desc->blksz = 0x020000
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

#if (BSP_USE_MASS)                          // 如果使用了USB大容量存储设备
    cur_msc = usb_stor_scan(1);             // 扫描USB存储设备【*】参数1为显示模式，如果有USB连接返回当前大容量存储器（编号）从0开始，没有返回-1
    msc_desc = get_usb_msc_blk_dev(cur_msc);// 根据编号cur_msc在usb_dev_desc找到相应的大容量存储器设备描述符【*】
    
    while(msc_desc!=NULL)                   // 如果这个设备描述符非空
    {
        int rdcount;
        unsigned char *blk_buf;
        printk("\n USB device is %d \n",cur_msc);
        dos_test(msc_desc);                 // 用设备描述符进行操作【*】
        cur_msc++;
        msc_desc = get_usb_msc_blk_dev(cur_msc); // 读取下一个设备【*】
    }
#endif

#if (BSP_USE_MOUSE)                           // 检测是否使用了键盘
    drv_usb_mouse_init();                     // 键盘的驱动初始化
    while (1)
    {
        if (usb_mouse_testc())                // 检测是否有输入
        {
            int ch = usb_mouse_getc();        // 读取键盘输入，返回输入字符
                       // 打印输入字符
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




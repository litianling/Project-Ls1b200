/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B Bare Program, Sample main file
 */

#include <stdio.h>
#include <math.h>

#include "ls1b.h"
#include "mips.h"

//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"

#ifdef BSP_USE_FB
  #include "ls1x_fb.h"
  #include "gui/demo_gui.h"
  #include "gui/simple-gui/simple_gui.h"
  #ifdef XPT2046_DRV
    #include "spi/xpt2046.h"
    char LCD_display_mode[] = LCD_800x480;
  #elif defined(GT1151_DRV)
    #include "i2c/gt1151.h"
    #include "touch.h"
    char LCD_display_mode[] = LCD_480x800;
  #else
    char LCD_display_mode[] = LCD_800x480;
  #endif
#endif

//-------------------------------------------------------------------------------------------------

extern void start_my_gui(void);

//-------------------------------------------------------------------------------------------------
// 主循环
//-------------------------------------------------------------------------------------------------


char str[] = {0x47,0xC9,0xBD,0xCE,0xF7,0xCA,0xA1,0x20,0xD1,0xF4,0xC8,0xAA,0xCA,0xD0,0x20};


int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */


	//start_desktop_gui();						//启用GUI显示【*】初始化GUI环境、创建主要对象、绘制显示界面（按键和表格）

    fb_cons_clear();
    
    char *chinese=str;
    fb_draw_gb2312_char(0,0,chinese);
    //fb_textout(0,0,chinese);

    //desktop_gui_show();

    while (1);
    return 0;
}

/*
 * @@ End
 */


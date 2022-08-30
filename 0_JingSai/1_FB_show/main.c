#include <stdio.h>
#include "ls1b.h"
#include "mips.h"
#include "stdlib.h"
#include "string.h"

#define u8  unsigned char
#define u16 unsigned int


//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"
#define BSP_USE_FB
#define XPT2046_DRV

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
// 函数实现
//------------------------------------------------------------------------------------------------

void put_text(char *s, int i, unsigned coloridx)    //字符串输出
{
    //i = i+3;
    //fb_put_string_center(400, i*20 ,s , coloridx);
    fb_put_string(0, i*20 ,s , coloridx);                   // 列-行-颜色
}

void show_menu()    //菜单
{
    //fb_drawrect(90,40,720,360, 7);    绘制矩形
    //put_text("---------------------------------------------------------------------------",0,7);
    put_text("欢迎来到贪吃蛇大冒险游戏！",1,4);      // 红色
    put_text("游戏说明：",2, 7);                     // 白色
    put_text("使用btn1、btn2选择游戏模式，btn3进行确认",3,7);
    put_text("进入游戏后使用 btn1:向上、btn2:向下、btn3:向左、btn4:向右 对贪吃蛇进行操控！",4, 7);
    //put_text("---------------------------------------------------------------------------",5,7);
    //put_text("模式选择：",6,7);
    //put_text("---------------------------------------------------------------------------",7,7);
    put_text("-----------------------------  李天凌 制  --------------------------------",14,7);
}


//-------------------------------------------------------------------------------------------------
// 主程序
//-------------------------------------------------------------------------------------------------

int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */

    fb_open();
    fb_cons_clear();

    int i=0;
    while(1)
    {
        iprintf("Hello i am LTL! %d\r\n",i);
        i++;
        //fb_cons_puts(buffer);
		delay_ms(500);
	}
    return 0;
}

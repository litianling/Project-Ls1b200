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
    #error "��bsp.h��ѡ������ XPT2046_DRV ���� GT1151_DRV"
           "XPT2046_DRV:  ����800*480 �����Ĵ�����."
           "GT1151_DRV:   ����480*800 �����Ĵ�����."
           "�������ѡ��, ע�͵��� error ��Ϣ, Ȼ���Զ���: LCD_display_mode[]"
  #endif
#endif


//-------------------------------------------------------------------------------------------------
// ����ʵ��
//------------------------------------------------------------------------------------------------

void put_text(char *s, int i, unsigned coloridx)    //�ַ������
{
    //i = i+3;
    //fb_put_string_center(400, i*20 ,s , coloridx);
    fb_put_string(0, i*20 ,s , coloridx);                   // ��-��-��ɫ
}

void show_menu()    //�˵�
{
    //fb_drawrect(90,40,720,360, 7);    ���ƾ���
    //put_text("---------------------------------------------------------------------------",0,7);
    put_text("��ӭ����̰���ߴ�ð����Ϸ��",1,4);      // ��ɫ
    put_text("��Ϸ˵����",2, 7);                     // ��ɫ
    put_text("ʹ��btn1��btn2ѡ����Ϸģʽ��btn3����ȷ��",3,7);
    put_text("������Ϸ��ʹ�� btn1:���ϡ�btn2:���¡�btn3:����btn4:���� ��̰���߽��вٿأ�",4, 7);
    //put_text("---------------------------------------------------------------------------",5,7);
    //put_text("ģʽѡ��",6,7);
    //put_text("---------------------------------------------------------------------------",7,7);
    put_text("-----------------------------  ������ ��  --------------------------------",14,7);
}


//-------------------------------------------------------------------------------------------------
// ������
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

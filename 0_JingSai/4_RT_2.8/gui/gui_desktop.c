
#include "bsp.h"

#ifdef BSP_USE_FB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls1x_fb.h"
#include "simple-gui/simple_gui.h"

#define BUTTON_WIDTH		100
#define BUTTON_HEIGHT       50


/******************************************************************************
 * ��Ҫ��ƿ�ܳ���
 ******************************************************************************/

#define MAIN_GROUP	0x00010000

static TGrid *grid_main = NULL;

static int create_desktop_grid(void)
{
	TColumn *p_column;
	TRect    rect;

	rect.left   = 20;
	rect.top    = 20;
	rect.right  = 507;
	rect.bottom = 459;
	grid_main = create_grid(&rect, 12, 4, MAIN_GROUP | 0x0001, MAIN_GROUP);

	if (grid_main == NULL)
		return -1;

	/* column 0 */
	p_column = grid_get_column(grid_main, 0);
	p_column->align = align_center;            // ɾ�����о��������
	grid_set_column_title(p_column, "���");
	grid_set_column_width(grid_main, 0, 34);

	/* column 1 */
	p_column = grid_get_column(grid_main, 1);
	p_column->align = align_center;
	grid_set_column_title(p_column, "Ӧ�ó���");
	grid_set_column_width(grid_main, 1, 210);

	/* column 2 */
	p_column = grid_get_column(grid_main, 2);
	p_column->align = align_center;
	grid_set_column_title(p_column, "���");
	grid_set_column_width(grid_main, 2, 34);

	/* column 3 */
	p_column = grid_get_column(grid_main, 3);
	p_column->align = align_center;
	grid_set_column_title(p_column, "Ӧ�ó���");
	grid_set_column_width(grid_main, 3, 210);
	
	return 0;
}


static int create_desktop_buttons(void)
{
	TRect rect;

	rect.left   = 540;
	rect.top    = 35;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0001, MAIN_GROUP, "1", NULL);
	rect.left   = 660;
	rect.top    = 35;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0002, MAIN_GROUP, "2", NULL);

	rect.left   = 540;
	rect.top    = 95;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0003, MAIN_GROUP, "3", NULL);
	rect.left   = 660;
	rect.top    = 95;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0004, MAIN_GROUP, "4", NULL);

	rect.left   = 540;
	rect.top    = 155;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0005, MAIN_GROUP, "5", NULL);
	rect.left   = 660;
	rect.top    = 155;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0006, MAIN_GROUP, "6", NULL);

	rect.left   = 540;
	rect.top    = 215;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0007, MAIN_GROUP, "7", NULL);
	rect.left   = 660;
	rect.top    = 215;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0008, MAIN_GROUP, "8", NULL);
	
	rect.left   = 540;
	rect.top    = 275;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x0009, MAIN_GROUP, "9", NULL);
	rect.left   = 660;
	rect.top    = 275;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x000a, MAIN_GROUP, "10", NULL);
	
	rect.left   = 540;
	rect.top    = 335;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x000b, MAIN_GROUP, "11", NULL);
	rect.left   = 660;
	rect.top    = 335;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x000c, MAIN_GROUP, "12", NULL);
	
	rect.left   = 540;
	rect.top    = 395;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x000d, MAIN_GROUP, "13", NULL);
	rect.left   = 660;
	rect.top    = 395;
	rect.right  = rect.left + BUTTON_WIDTH;
	rect.bottom = rect.top + BUTTON_HEIGHT;
	new_button(&rect, MAIN_GROUP | 0x000e, MAIN_GROUP, "14", NULL);

	return 0;
}

int create_desktop_objects(void)
{
    ls1x_dc_ioctl(devDC, IOCTRL_FB_CLEAR_BUFFER, (void *)GetColor(cidxSILVER));   // �����Ļ
	if (get_buttons_count(MAIN_GROUP) == 0)
		create_desktop_buttons();                    // ����һϵ�а�����ֻ���������ԣ���û��ʾ��
	if (grid_main == NULL)
		create_desktop_grid();                       // �������Ҳ���������ԣ���û����ʾ��
	set_gui_active_group(MAIN_GROUP);
	return 0;
}

/******************************************************************************
 * ��Ҫ���û��ӿڳ���
 ******************************************************************************/

/* �ڱ������ʾ�ı���Ϣ */
void gui_drawtext_in_grid(int row, int col, const char *str)
{
    if (!ls1x_dc_started())
        return;
    if (grid_main != NULL)
        grid_set_cell_text(grid_main, row, col, str);
}


/* ���ò���ʾ���� */
void show_desktop_gui(void)
{
    if (fb_open() != 0)
		return;
    if (!ls1x_dc_started())
        return;

    int bgcolor_num = get_bg_color();  // ������ɫ
    int fgcolor_num = get_fg_color();

    init_simple_gui_queue();        // ��ʼ��gui ������*����ʼ��GUI���С���񡢰�����
    create_desktop_objects();	        // �����������*�������������������
    paint_desktop_gui();            // ��������ͼ��

    gui_drawtext_in_grid(0, 0, "1");    gui_drawtext_in_grid(0, 2, "2");
    gui_drawtext_in_grid(1, 0, "3");    gui_drawtext_in_grid(1, 2, "4");
    gui_drawtext_in_grid(2, 0, "5");    gui_drawtext_in_grid(2, 2, "6");
    gui_drawtext_in_grid(3, 0, "7");    gui_drawtext_in_grid(3, 2, "8");
    gui_drawtext_in_grid(4, 0, "9");    gui_drawtext_in_grid(4, 2, "10");
    gui_drawtext_in_grid(5, 0, "11");   gui_drawtext_in_grid(5, 2, "12");
    gui_drawtext_in_grid(6, 0, "13");   gui_drawtext_in_grid(6, 2, "14");

    gui_drawtext_in_grid(0, 1, "Set color");        gui_drawtext_in_grid(0, 3, "Mouse draw");
    gui_drawtext_in_grid(1, 1, "Read disk");        gui_drawtext_in_grid(1, 3, "Push box");
    gui_drawtext_in_grid(2, 1, "SG180");            gui_drawtext_in_grid(2, 3, "Human sensor");
    gui_drawtext_in_grid(3, 1, "Airquality");       gui_drawtext_in_grid(3, 3, "Temperature");
    gui_drawtext_in_grid(4, 1, "L610 Ctrl");        gui_drawtext_in_grid(4, 3, "L610 Check");
    gui_drawtext_in_grid(5, 1, "L610 Telephone");   gui_drawtext_in_grid(5, 3, "L610 Short Message");
    gui_drawtext_in_grid(6, 1, "LBS Position");     gui_drawtext_in_grid(6, 3, "L610 TXcloud safety");

    fb_set_bgcolor_mydef(bgcolor_num);  // �ָ���ɫ
    fb_set_fgcolor_mydef(fgcolor_num);
}

/* ����GUI��ʾ */
void desktop_gui_show(void)
{
    int bgcolor_num = get_bg_color();  // ������ɫ
    int fgcolor_num = get_fg_color();
    
    ls1x_dc_ioctl(devDC, IOCTRL_FB_CLEAR_BUFFER, (void *)GetColor(cidxSILVER));   // �����Ļ
    paint_active_buttons_mydefine();
    paint_active_grids_mydefine();
    
    fb_set_bgcolor_mydef(bgcolor_num);  // �ָ���ɫ
    fb_set_fgcolor_mydef(fgcolor_num);
}

/******************************************************************************
 * ������ж�
 ******************************************************************************/
static const int button_x[]={540,660,540,660,540,660,540,660,540,660,540,660,540,660};
static const int button_y[]={35,35,95,95,155,155,215,215,275,275,335,335,395,395};
extern int mouse_x,mouse_y;

int click_in_button(int button)
{
    if (button>14)
        return 0;

    int index = button-1;
    if((mouse_x>button_x[index])&&(mouse_x<button_x[index]+BUTTON_WIDTH))
        if((mouse_y>button_y[index])&&(mouse_y<button_y[index]+BUTTON_HEIGHT))
            return 1;

    return 0;
}

#endif


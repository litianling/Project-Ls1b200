/*
 * usb_mouse.h
 *
 * created: 2022/5/23
 *  author: 
 */

#ifndef _USB_MOUSE_H
#define _USB_MOUSE_H


#endif // _USB_MOUSE_H

#define COL_GAP				0		/* 列字符间距 */
#define ROW_GAP				0		/* 行字符间距 */

#define CONS_FONT_WIDTH		8
#define CONS_FONT_HEIGHT    8

int Col_max = 800/CONS_FONT_WIDTH - 1;
int Row_max = 480/CONS_FONT_HEIGHT - 1;

int Col = 0;
int Row = 0;


int mouse_x = 0;
int mouse_y = 0;




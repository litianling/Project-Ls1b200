/*
 * oled_driver.h
 *
 * created: 2021/11/5
 *  author:
 */

#ifndef _OLED_DRIVER_H
#define _OLED_DRIVER_H

unsigned char cmd_buf[2] = {0x00, 0x00};
unsigned char data_buf[2] = {0x40, 0x00};
unsigned int Addr = 0b0111100;
int rw = 0;
typedef unsigned char           uint8_t;

void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Initial_M096128x64_ssd1306(void);

void Picture(int i);
void fill_picture(unsigned char fill_Data);

void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr);
void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr);
void OLED_ShowString_Short(unsigned char  x,unsigned char  y, unsigned char  *chr,unsigned char  l);

#endif // _OLED_DRIVER_H



/*
 * scanf.h
 *
 * created: 2021/10/26
 *  author: 
 */

#ifndef _SCANF_H
#define _SCANF_H

#include "bsp.h"
#include <stdarg.h>
#include <stddef.h>

#endif // _SCANF_H

extern void getch(char character);
extern void uart3_initialize(int baudrate, int databits, char eccmode, int stopbits);
extern int uart3_read(unsigned char *buf, int size);
int scanf(const char* format, ...);



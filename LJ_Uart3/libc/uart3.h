/*
 * uart3.h
 *
 * created: 2021/10/23
 *  author:
 */

#ifndef _UART3_H
#define _UART3_H


#endif // _UART3_H

#include "bsp.h"
#include "mips.h"
#include "cpu.h"
#include "stdio.h"
#include "ls1b.h"
#include "ls1b_irq.h"


#define UART_FIFO_SIZE  16

typedef struct
{
    union
	{
    	volatile unsigned char dat;     // 0x00 Êý¾Ý¼Ä´æÆ÷          // when DLAB=0, read as receive buffer
                                                                    // when DLAB=0, write as send buffer
    	volatile unsigned char dll;     // 0x00 ·ÖÆµÖµµÍ×Ö½Ú¼Ä´æÆ÷  // when DLAB=1, rw as dll
	} R0;
    union
	{
    	volatile unsigned char ier;     // 0x01 ÖÐ¶ÏÊ¹ÄÜ¼Ä´æÆ÷      // when DLAB=0, rw as ier
    	volatile unsigned char dlh;     // 0x01 ·ÖÆµÖµ¸ß×Ö½Ú¼Ä´æÆ÷  // when DLAB=1, rw as dlh
	} R1;
    union
	{
		volatile unsigned char isr;     // 0x02 ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷      // when DLAB=X, read as isr
		volatile unsigned char fcr;     // 0x02 FIFO¿ØÖÆ¼Ä´æÆ÷      // when DLAB=X, write as fcr
	} R2;
    volatile unsigned char lcr;         // 0x03 ÏßÂ·¿ØÖÆ¼Ä´æÆ÷      // XXX bit(7) is DLAB
    volatile unsigned char mcr;         // 0x04 MODEM¿ØÖÆ¼Ä´æÆ÷
    volatile unsigned char lsr;         // 0x05 ÏßÂ·×´Ì¬¼Ä´æÆ÷
    volatile unsigned char msr;         // 0x06 MODEM×´Ì¬¼Ä´æÆ÷
} HW_UART_t;


int uart3_initialize(int baudrate, int databits, char eccmode, int stopbits);
int uart3_read(unsigned char *buf, int size);
int uart3_write(unsigned char *buf, int size);

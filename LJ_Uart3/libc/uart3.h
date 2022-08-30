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
    	volatile unsigned char dat;     // 0x00 ���ݼĴ���          // when DLAB=0, read as receive buffer
                                                                    // when DLAB=0, write as send buffer
    	volatile unsigned char dll;     // 0x00 ��Ƶֵ���ֽڼĴ���  // when DLAB=1, rw as dll
	} R0;
    union
	{
    	volatile unsigned char ier;     // 0x01 �ж�ʹ�ܼĴ���      // when DLAB=0, rw as ier
    	volatile unsigned char dlh;     // 0x01 ��Ƶֵ���ֽڼĴ���  // when DLAB=1, rw as dlh
	} R1;
    union
	{
		volatile unsigned char isr;     // 0x02 �ж�״̬�Ĵ���      // when DLAB=X, read as isr
		volatile unsigned char fcr;     // 0x02 FIFO���ƼĴ���      // when DLAB=X, write as fcr
	} R2;
    volatile unsigned char lcr;         // 0x03 ��·���ƼĴ���      // XXX bit(7) is DLAB
    volatile unsigned char mcr;         // 0x04 MODEM���ƼĴ���
    volatile unsigned char lsr;         // 0x05 ��·״̬�Ĵ���
    volatile unsigned char msr;         // 0x06 MODEM״̬�Ĵ���
} HW_UART_t;


int uart3_initialize(int baudrate, int databits, char eccmode, int stopbits);
int uart3_read(unsigned char *buf, int size);
int uart3_write(unsigned char *buf, int size);

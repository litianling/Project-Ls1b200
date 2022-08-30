/*
 * bsp_start.S
 *
 * created: 2022/1/27
 *  author: Li TianLing
 */

/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���
*   s0  ����������arr����ַ
*   s1  ��������-->begin
*   s2  ��������-->end
*   s3  ��ת�����Ĵ���
*   t1  �ڱ�i
*   t2  �ڱ�i+1
*   t3  �ڱ�iָ���ֵ����arr[i]
*   t4  �ڱ�i+1ָ���ֵ����arr[i+1]
*   t5  �ڱ�j
*   t6  �ڱ�jָ���ֵ����arr[j]
*   t7  ��ʱ����temp
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//���ݶ�
    .data
    arr:  .half 3,8,1,5,2,4,6,7    // ���������ݣ����飩
	len:  .half 8                  // ���������ݳ���

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    s4, ra      /* ���ص�ַ */

    la  s0,arr          // ���ش��������ݻ���ַ��������
    la  s1,0            // begin��ʼ��Ϊ0
    la  s2,len          // ���ء�����end(һ�����������ֽ�)  (8-1)*2=14
    lh  s2,(s2)
    sub s2,1
    sll s2,1
    
    move t1,s1          // �ڱ�i
    add t2,t1,2         // �ڱ�i+1�����ַ�����
for_0_begin:            // Ѱ����������
    add v0,s0,t1        // �����ڱ�iָ���ֵarr[i]��t3
    lh  t3,(v0)
    add v0,s0,t2        // �����ڱ�i+1ָ���ֵarr[i+1]��t4
    lh  t4,(v0)
    
    sub s3,t3,t4        // arr[i]>arr[i+1]��������t3>t4����Ѱ��
    bgtz s3,for_0_end
    
    add t1,2            // �ڱ��ƶ�����ʼѭ��
    add t2,2
    b   for_0_begin
    nop
for_0_end:


    move t5,t2
for_1_begin:            // begin������end-1ʱ����������������һ����
    sub s3,t5,s2        // ����У�飬t5>s2����ѭ��1,�����������ݱ�����end����
    bgtz s3,for_1_end
    nop

    add v0,s0,t5        // ��arr[j]ֱ�ӵ���temp(t7)���ƿ�t6
    lh  t7,(v0)

    move t2,t5
    sub  t1,t2,2
for_2_begin:            
    sub s3,t1,s1        // t1<s1��������i<begin����ѭ��2
    bltz s3,for_2_end
    nop

    add v0,s0,t1        // ��arr[i]����t3
    lh  t3,(v0)
    sub s3,t3,t7        // arr[i]<=temp��������t3<=t7����ѭ��2
    blez s3,for_2_end
    nop

    add v0,s0,t2        // ����arr[i]д��arr[i+1]
    sh  t3,(v0)

    sub t1,2            // i-2����ʼѭ��2
    sub t2,2            // (i-1)-2
    b for_2_begin
    nop
for_2_end:

    add v0,s0,t2        // ���temp����arr[i+1]
    sh  t7,(v0)

    add t5,2            // j+2,��ʼѭ��1
    b for_1_begin
    nop
for_1_end:


finish:
    nop

    move    ra, s4
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)

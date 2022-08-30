/*
 * bsp_start.S
 *
 * created: 2022/1/26
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
*   t2  �ڱ�j����i-1��
*   t3  �ڱ�iָ���ֵ����arr[i]
*   t4  �ڱ�jָ���ֵ����arr[j]
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
    
for_1_begin:            // end������begin+1ʱ�������������begin
    sub s3,s2,s1        // ����У�飬end<=begin,����ѭ��1
    blez s3,for_1_end
    nop

    move t1,s1          // �ڱ�i��j��ʼ��
    add  t1,2
    move t2,s1

for_2_begin:            // i��begin��end֮��ѭ������
    sub s3,s2,t1        // ����У�飬end<i,����ѭ��2
    bltz s3,for_2_end
    nop

    add v0,s0,t1        // ��arr[i]����t3
    lh  t3,(v0)
    add v0,s0,t2        // ��arr[j]����t4
    lh  t4,(v0)

    sub s3,t3,t4        // ���arr[i]>arr[j],������
    bgez s3,no_if
    nop

    add v0,s0,t2        // ����arr[i]��arr[j]����
    sh  t3,(v0)
    add v0,s0,t1        
    sh  t4,(v0)

no_if:

    add t1,2            // i+2��j+2����ʼѭ��2
    add t2,2            
    b for_2_begin
    nop
for_2_end:


    sub s2,2            // end-2,��ʼѭ��1
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
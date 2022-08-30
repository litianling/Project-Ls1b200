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
*   t2  ��־j
*   t3  �ڱ�iָ���ֵ����arr[i]
*   t4  ��־jָ���ֵ����arr[j]
*   t5  beginָ���ֵ����arr[begin]
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

for_1_begin:            // begin������end-1ʱ����������������һ����
    sub s3,s2,s1        // ����У�飬end<=begin,����ѭ��1
    blez s3,for_1_end
    nop
    
    move t1,s1
    move t2,s1
    
for_2_begin:            // i��begin��end֮��ѭ������
    sub s3,s2,t1        // ����У�飬end<i,����ѭ��2
    bltz s3,for_2_end
    nop
    
    add v0,s0,t1        // ��arr[i]����t3
    lh  t3,(v0)
    add v0,s0,t2        // ��arr[j]����t4
    lh  t4,(v0)
    
    sub s3,t3,t4        // ���arr[i]>arr[j],��Сֵ����
    bgez s3,no_if
    nop
    
    move t2,t1          // ����arr[i]���Ϊ��Сֵ
    
no_if:
    
    add t1,2            // i+2����ʼѭ��2
    b for_2_begin
    nop
for_2_end:
    
    add v0,s0,s1        // ��arr[begin]���ص�t5��
    lh  t5,(v0)
    add v0,s0,t2        // ��arr[j]���ص�t4��
    lh  t4,(v0)
    add v0,s0,t2        // ��t5��ֵ���浽arr[j]��
    sh  t5,(v0)
    add v0,s0,s1        // ��t4��ֵ���浽arr[begin]��
    sh  t4,(v0)
    
    add s1,2            // begin+2,��ʼѭ��1
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
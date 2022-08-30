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
*   s1  increment
*   s2  ��������-->len
*   s3  ��ת�����Ĵ���
*   t1  �ڱ�i
*   t2  �ڱ�i+increment
*   t3  �ڱ�iָ���ֵ����arr[i]
*   t4  �ڱ�i+incrementָ���ֵ����arr[i+increment]
*   t5  �ڱ�j
*   t6  �ڱ�jָ���ֵ����arr[j]
*   t7  ��ʱ����temp
*   t8  �ڱ�j-increment
*   t9  �ڱ�j-incrementָ���ֵ����arr[j-increment]
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

    la  s5,3
    la  s0,arr          // ���ش��������ݻ���ַ��������
    la  s2,len          // ���ء�����len(һ�����������ֽ�)  8*2=16
    lh  s2,(s2)
    sll s2,1

    move s1,s2
while_begin:
    sub  s3,s1,2        // increment<=2����ѭ��
    blez s3,while_end   
    nop

    div  s1,6           // increment=(increment/3)����ȡ��+1,���ǰ���ռ���ֽڣ�����increment=(increment/6)����ȡ��*2+2
    mflo s1
    sll  s1,1
    add  s1,2

    move t5,s1          // �ڱ�j��ֵ
for_1_begin:            
    sub s3,t5,s2        // ����У�飬t5>=s2����ѭ��1,��j������len-1����
    bgez s3,for_1_end
    nop

if_begin:
    sub t8,t5,s1        // ����j-increment
    add v0,s0,t8        // ��arr[j-increment]���ص�t9
    lh  t9,(v0)
    add v0,s0,t5        // ��arr[j]���ص�t6
    lh  t6,(v0)

    sub s3,t9,t6        // ���arr[j-increment]<=arr[j],��ִ��if
    blez s3,if_end
    nop

    move t7,t6          // ��arr[j]ֱ�ӵ���temp(t7)
    
    move t2,t5          // �ڱ�i��ʼ�����ڱ�j-increment��
    sub  t1,t2,s1
for_2_begin:
    sub s3,t1,zero      // t1<zero��������i<0����ѭ��2
    bltz s3,for_2_end
    nop

    add v0,s0,t1        // ��arr[i]����t3
    lh  t3,(v0)
    sub s3,t3,t7        // arr[i]<=temp��������t3<=t7����ѭ��2
    blez s3,for_2_end
    nop

    add v0,s0,t2        // ����arr[i]д��arr[i+increment]
    sh  t3,(v0)

    sub t1,s1           // i-increment����ʼѭ��2
    sub t2,s1           // (i+increment)-increment
    b for_2_begin
    nop
for_2_end:

    add v0,s0,t2        // ���temp����arr[i+increment]
    sh  t7,(v0)

if_end:

    add t5,2            // j+2,��ʼѭ��1
    b for_1_begin
    nop
for_1_end:

    b while_begin
    nop
while_end:

finish:
    nop

    move    ra, s4
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)
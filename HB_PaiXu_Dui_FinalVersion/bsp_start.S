/*
 * bsp_start.S
 *
 * created: 2022/2/1
 *  author: Li TianLing
 */


/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���
*   v1  ��ת�����Ĵ���
*   s0  ����������arr����ַ              ��������
*   s1  ����len                          ��������
*   s2  ����size
*   s3  ����root
*   s4  ����l_leaf
*   s5  ����r_leaf
*   s6  ����max
*   s7  ����root_1      ��������ʱroot��������
*   t1  arr[len]
*   t2  arr[size]
*   t3  arr[root]
*   t4  arr[l_leaf]
*   t5  arr[r_leaf]
*   t6  arr[max]
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//���ݶ�
    .data
    arr:    .half 3,4,2,8,9,5,1     // ���������ݣ����飩
	len:    .half 7                 // ���������ݳ���

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* ���ص�ַ */

    la  s0,arr          // ��arr����ַ���ص�s0��������
    la  v0,len          // ������len���ص�s1��������
    lh  s1,(v0)
    sll s1,1
    
    move s2,s1          // size = len
    div s1,4            // root_1 = (len/4)*2-2
    mflo s7
    mul s7,2
    sub s7,2
for_root_begin:         // ���������
    bltz s7,for_root_end// root_1<0������ѭ��
    nop
    
Adjust_heap_1_begin:
    move s3,s7          // root=root_1
Digui_1_begin:
    mul s4,s3,2         // l_leaf=root*2+2
    add s4,2
    add s5,s4,2         // r_leaf=l_leaf+2
    move s6,s3          // max=root
    
    add v0,s0,s3        // ��arr[root]���ص�t3
    lh  t3,(v0)
    add v0,s0,s4        // ��arr[l_leaf]���ص�t4
    lh  t4,(v0)
    add v0,s0,s5        // ��arr[r_leaf]���ص�t5
    lh  t5,(v0)
    add v0,s0,s6        // ��arr[max]���ص�t6
    lh  t6,(v0)

if_l_leaf_1_begin:
    sub v1,s4,s2        // l_leaf>=size��ִ��if
    bgez v1,if_l_leaf_1_end
    nop
    sub v1,t4,t6
    blez v1,if_l_leaf_1_end   // arr[l_leaf]<=arr[max]��ִ��if
    nop
    
    move s6,s4          // max=l_leaf
if_l_leaf_1_end:

if_r_leaf_1_begin:
    sub v1,s5,s2        // r_leaf>=size��ִ��if
    bgez v1,if_r_leaf_1_end
    nop
    add v0,s0,s6        // max���ܸı䣬���½�arr[max]���ص�t6
    lh  t6,(v0)
    sub v1,t5,t6
    blez v1,if_r_leaf_1_end   // arr[r_leaf]<=arr[max]��ִ��if
    nop

    move s6,s5          // max=r_leaf
if_r_leaf_1_end:

if_not_equal_1_begin:
    beq s3,s6,if_not_equal_1_end    // max==root��ִ�д�if
    nop

    add v0,s0,s6        // max�ı䣬���½�arr[max]���ص�t6
    lh  t6,(v0)
    add v0,s0,s3        // ��t6���浽arr[root]
    sh  t6,(v0)
    add v0,s0,s6        // ��t3���浽arr[max]
    sh  t3,(v0)
    
    move s3,s6          // root = max
    b   Digui_1_begin
    nop
if_not_equal_1_end:

Adjust_heap_1_end:
    
    sub s7,2            // root_1-2
    b   for_root_begin
    nop
for_root_end:

    nop
    nop

    la  s7,0            // root_1 = 0
    la  v0,len          // ���¼��س���len���ص�s1����53��len���ģ�
    lh  s1,(v0)
    sll s1,1
    sub s2,s1,2         // size=len-2
for_size_begin:
    bltz s2,for_size_end    // size<0����
    nop
    
    add v0,s0,zero      // ��arr[0]���ص�t1
    lh  t1,(v0)
    add v0,s0,s2        // ��arr[size]���ص�t2
    lh  t2,(v0)
    add v0,s0,zero      // ��t2���浽arr[0]
    sh  t2,(v0)
    add v0,s0,s2        // ��t1���浽arr[size]
    sh  t1,(v0)
    
Adjust_heap_2_begin:
    move s3,s7          // root=root_1
Digui_2_begin:
    mul s4,s3,2         // l_leaf=root*2+2
    add s4,2
    add s5,s4,2         // r_leaf=l_leaf+2
    move s6,s3          // max=root

    add v0,s0,s3        // ��arr[root]���ص�t3
    lh  t3,(v0)
    add v0,s0,s4        // ��arr[l_leaf]���ص�t4
    lh  t4,(v0)
    add v0,s0,s5        // ��arr[r_leaf]���ص�t5
    lh  t5,(v0)
    add v0,s0,s6        // ��arr[max]���ص�t6
    lh  t6,(v0)

if_l_leaf_2_begin:
    sub v1,s4,s2        // l_leaf>=size��ִ��if
    bgez v1,if_l_leaf_2_end
    nop
    sub v1,t4,t6
    blez v1,if_l_leaf_2_end   // arr[l_leaf]<=arr[max]��ִ��if
    nop

    move s6,s4          // max=l_leaf
if_l_leaf_2_end:

if_r_leaf_2_begin:
    sub v1,s5,s2        // r_leaf>=size��ִ��if
    bgez v1,if_r_leaf_2_end
    nop
    add v0,s0,s6        // max���ܸı䣬���½�arr[max]���ص�t6
    lh  t6,(v0)
    sub v1,t5,t6
    blez v1,if_r_leaf_2_end   // arr[r_leaf]<=arr[max]��ִ��if
    nop

    move s6,s5          // max=r_leaf
if_r_leaf_2_end:

if_not_equal_2_begin:
    beq s3,s6,if_not_equal_2_end    // max==root��ִ�д�if
    nop

    add v0,s0,s6        // max�ı䣬���½�arr[max]���ص�t6
    lh  t6,(v0)
    add v0,s0,s3        // ��t6���浽arr[root]
    sh  t6,(v0)
    add v0,s0,s6        // ��t3���浽arr[max]
    sh  t3,(v0)

    move s3,s6          // root = max
    b   Digui_2_begin
    nop
if_not_equal_2_end:

Adjust_heap_2_end:

    sub s2,2            // size-2
    b   for_size_begin
    nop
for_size_end:


finish:
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)

/*
 * bsp_start.S
 *
 * created: 2022/2/1
 *  author: Li TianLing
 */


/**************************************
*
*   v0  存储器寻址辅助寄存器
*   v1  跳转辅助寄存器
*   s0  待排序数据arr基地址              （常量）
*   s1  参数len                          （常量）
*   s2  参数size
*   s3  参数root
*   s4  参数l_leaf
*   s5  参数r_leaf
*   s6  参数max
*   s7  参数root_1      函数调用时root辅助参数
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
//数据段
    .data
    arr:    .half 3,4,2,8,9,5,1     // 待排序数据（数组）
	len:    .half 7                 // 待排序数据长度

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* 返回地址 */

    la  s0,arr          // 将arr基地址加载到s0（常量）
    la  v0,len          // 将长度len加载到s1（常量）
    lh  s1,(v0)
    sll s1,1
    
    move s2,s1          // size = len
    div s1,4            // root_1 = (len/4)*2-2
    mflo s7
    mul s7,2
    sub s7,2
for_root_begin:         // 构建大根堆
    bltz s7,for_root_end// root_1<0结束此循环
    nop
    
Adjust_heap_1_begin:
    move s3,s7          // root=root_1
Digui_1_begin:
    mul s4,s3,2         // l_leaf=root*2+2
    add s4,2
    add s5,s4,2         // r_leaf=l_leaf+2
    move s6,s3          // max=root
    
    add v0,s0,s3        // 将arr[root]加载到t3
    lh  t3,(v0)
    add v0,s0,s4        // 将arr[l_leaf]加载到t4
    lh  t4,(v0)
    add v0,s0,s5        // 将arr[r_leaf]加载到t5
    lh  t5,(v0)
    add v0,s0,s6        // 将arr[max]加载到t6
    lh  t6,(v0)

if_l_leaf_1_begin:
    sub v1,s4,s2        // l_leaf>=size不执行if
    bgez v1,if_l_leaf_1_end
    nop
    sub v1,t4,t6
    blez v1,if_l_leaf_1_end   // arr[l_leaf]<=arr[max]不执行if
    nop
    
    move s6,s4          // max=l_leaf
if_l_leaf_1_end:

if_r_leaf_1_begin:
    sub v1,s5,s2        // r_leaf>=size不执行if
    bgez v1,if_r_leaf_1_end
    nop
    add v0,s0,s6        // max可能改变，重新将arr[max]加载到t6
    lh  t6,(v0)
    sub v1,t5,t6
    blez v1,if_r_leaf_1_end   // arr[r_leaf]<=arr[max]不执行if
    nop

    move s6,s5          // max=r_leaf
if_r_leaf_1_end:

if_not_equal_1_begin:
    beq s3,s6,if_not_equal_1_end    // max==root不执行此if
    nop

    add v0,s0,s6        // max改变，重新将arr[max]加载到t6
    lh  t6,(v0)
    add v0,s0,s3        // 将t6保存到arr[root]
    sh  t6,(v0)
    add v0,s0,s6        // 将t3保存到arr[max]
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
    la  v0,len          // 重新加载长度len加载到s1（第53行len被改）
    lh  s1,(v0)
    sll s1,1
    sub s2,s1,2         // size=len-2
for_size_begin:
    bltz s2,for_size_end    // size<0结束
    nop
    
    add v0,s0,zero      // 将arr[0]加载到t1
    lh  t1,(v0)
    add v0,s0,s2        // 将arr[size]加载到t2
    lh  t2,(v0)
    add v0,s0,zero      // 将t2保存到arr[0]
    sh  t2,(v0)
    add v0,s0,s2        // 将t1保存到arr[size]
    sh  t1,(v0)
    
Adjust_heap_2_begin:
    move s3,s7          // root=root_1
Digui_2_begin:
    mul s4,s3,2         // l_leaf=root*2+2
    add s4,2
    add s5,s4,2         // r_leaf=l_leaf+2
    move s6,s3          // max=root

    add v0,s0,s3        // 将arr[root]加载到t3
    lh  t3,(v0)
    add v0,s0,s4        // 将arr[l_leaf]加载到t4
    lh  t4,(v0)
    add v0,s0,s5        // 将arr[r_leaf]加载到t5
    lh  t5,(v0)
    add v0,s0,s6        // 将arr[max]加载到t6
    lh  t6,(v0)

if_l_leaf_2_begin:
    sub v1,s4,s2        // l_leaf>=size不执行if
    bgez v1,if_l_leaf_2_end
    nop
    sub v1,t4,t6
    blez v1,if_l_leaf_2_end   // arr[l_leaf]<=arr[max]不执行if
    nop

    move s6,s4          // max=l_leaf
if_l_leaf_2_end:

if_r_leaf_2_begin:
    sub v1,s5,s2        // r_leaf>=size不执行if
    bgez v1,if_r_leaf_2_end
    nop
    add v0,s0,s6        // max可能改变，重新将arr[max]加载到t6
    lh  t6,(v0)
    sub v1,t5,t6
    blez v1,if_r_leaf_2_end   // arr[r_leaf]<=arr[max]不执行if
    nop

    move s6,s5          // max=r_leaf
if_r_leaf_2_end:

if_not_equal_2_begin:
    beq s3,s6,if_not_equal_2_end    // max==root不执行此if
    nop

    add v0,s0,s6        // max改变，重新将arr[max]加载到t6
    lh  t6,(v0)
    add v0,s0,s3        // 将t6保存到arr[root]
    sh  t6,(v0)
    add v0,s0,s6        // 将t3保存到arr[max]
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


/*
 * bsp_start.S
 *
 * created: 2022/1/26
 *  author: Li TianLing
 */

/**************************************
*
*   v0  存储器寻址辅助寄存器
*   s0  待排序数据arr基地址
*   s1  参数传递-->begin
*   s2  参数传递-->end
*   s3  跳转辅助寄存器
*   t1  哨兵i
*   t2  哨兵j（即i-1）
*   t3  哨兵i指向的值，即arr[i]
*   t4  哨兵j指向的值，即arr[j]
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//数据段
    .data
    arr:  .half 3,8,1,5,2,4,6,7    // 待排序数据（数组）
	len:  .half 8                  // 待排序数据长度

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    s4, ra      /* 返回地址 */

    la  s0,arr          // 加载待排序数据基地址（常量）
    la  s1,0            // begin初始化为0
    la  s2,len          // 加载、计算end(一个半字两个字节)  (8-1)*2=14
    lh  s2,(s2)
    sub s2,1
    sll s2,1
    
for_1_begin:            // end遍历至begin+1时结束，不会遍历begin
    sub s3,s2,s1        // 参数校验，end<=begin,结束循环1
    blez s3,for_1_end
    nop

    move t1,s1          // 哨兵i和j初始化
    add  t1,2
    move t2,s1

for_2_begin:            // i在begin和end之间循环遍历
    sub s3,s2,t1        // 参数校验，end<i,结束循环2
    bltz s3,for_2_end
    nop

    add v0,s0,t1        // 将arr[i]读入t3
    lh  t3,(v0)
    add v0,s0,t2        // 将arr[j]读入t4
    lh  t4,(v0)

    sub s3,t3,t4        // 如果arr[i]>arr[j],不交换
    bgez s3,no_if
    nop

    add v0,s0,t2        // 否则将arr[i]和arr[j]交换
    sh  t3,(v0)
    add v0,s0,t1        
    sh  t4,(v0)

no_if:

    add t1,2            // i+2，j+2，开始循环2
    add t2,2            
    b for_2_begin
    nop
for_2_end:


    sub s2,2            // end-2,开始循环1
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

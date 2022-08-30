/*
 * bsp_start.S
 * 本程序是冒泡排序改进版，加入了有序数组检测，介于冒泡和插入之间
 * created: 2022/1/27
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
*   t2  哨兵i+1
*   t3  哨兵i指向的值，即arr[i]
*   t4  哨兵i+1指向的值，即arr[i+1]
*   t5  哨兵j
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

    move t1,s1          // 哨兵i
    add t2,t1,2         // 哨兵i+1（半字翻倍）
for_0_begin:            // 寻找有序数组
    add v0,s0,t1        // 加载哨兵i指向的值arr[i]到t3
    lh  t3,(v0)
    add v0,s0,t2        // 加载哨兵i+1指向的值arr[i+1]到t4
    lh  t4,(v0)

    sub s3,t3,t4        // arr[i]>arr[i+1]结束，即t3>t4结束寻找
    bgtz s3,for_0_end

    add t1,2            // 哨兵移动，开始循环
    add t2,2
    b   for_0_begin
    nop
for_0_end:


    move t5,t2
for_1_begin:            // begin遍历至end-1时结束，不会遍历最后一个数
    sub s3,t5,s2        // 参数校验，t5>s2结束循环1,即待插入数据遍历到end结束
    bgtz s3,for_1_end
    nop

    move t2,t5
    sub  t1,t2,2
for_2_begin:            // 某一待冒泡数据
    sub s3,t1,s1        // t1<s1结束，即i<begin结束循环2
    bltz s3,for_2_end
    nop

    add v0,s0,t1        // 将arr[i]读入t3
    lh  t3,(v0)
    add v0,s0,t2        // 将arr[i+1]读入t4
    lh  t4,(v0)

    sub s3,t3,t4        // 如果arr[i]<=arr[i+1],不冒泡
    blez s3,no_if
    nop

    add v0,s0,t1        //否则交叉存储，用以冒泡，将t4存入arr[i]
    sh  t4,(v0)
    add v0,s0,t2        //否则交叉存储，用以冒泡，将t3存入arr[i+1]
    sh  t3,(v0)


no_if:

    sub t1,2            // i-2，开始循环2
    sub t2,2            // (i-1)-2
    b for_2_begin
    nop
for_2_end:

    add t5,2            // j+2,开始循环1
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

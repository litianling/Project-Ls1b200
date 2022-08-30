/*
 * bsp_start.S
 *
 * created: 2022/1/25
 *  author: 
 */

/**************************************
*
*   v0  存储器寻址辅助寄存器
*   s0  待排序数据arr基地址
*   s1  参数传递-->begin
*   s2  参数传递-->end
*   s3  跳转辅助寄存器
*   s4  递归缓存区buf基地址
*   s5  递归缓存区buf偏移地址（指针）
*   t0  基准数
*   t1  哨兵i
*   t2  哨兵j
*   t3  哨兵i指向的值，即arr[i]
*   t4  哨兵j指向的值，即arr[j]
*   t5  交换寄存器t（改进后无用）
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//数据段
    .data
    arr:  .half 3,8,1,5,2,4,6,7     // 待排序数据（数组）
	len:  .half 8                   // 待排序数据个数
	buf:  .space 8                  // 递归缓存区（栈）

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    s6, ra      /* 返回地址 */
    
    la  s4,buf          // 加载递调用缓存区基地址（常量）
    la  s5,0            // 初始化递归调用偏移地址（变量）
    la  s0,arr          // 加载待排序数据基地址（常量）
    
    la  s1,0            // 初始化第一次参数传递begin的值
    la  s2,len          // 初始化第一次参数传递end的值
    lw  s2,(s2)
    sub s2,1
    sll s2,s2,1         // 因为是.half数据类型，一个半字占两个字节所以s2*2
    
    j Quick_Sort_start  // 开始第一次排序
    nop

loop:                   // 递归调用排序
    blez s5,finish      // 递归调用结束条件，如果s5<=0结束递归
    nop

    sub  s5,2           // end 出栈
    add v0,s4,s5
    lh  s2,(v0)
    sub  s5,2           // begin 出栈
    add v0,s4,s5
    lh  s1,(v0)
    // b Quick_Sort_start // 开始递归
    // nop


Quick_Sort_start:
    sub s3,s2,s1
    blez s3,Quick_Sort_end  // 参数校验：如果s2<=s1则结束此函数（不入栈）
    nop
    
    add v0,s0,s1    // 选定基准数
    lh  t0,(v0)
    la  t1,0        // 将传入参数begin加载到哨兵i
    add t1,s1
    la  t2,0        // 将传入参数end加载到哨兵j
    add t2,s2
    
while_i_j_begin:
    beq  t1,t2,while_i_j_end    // 一轮遍历结束条件：两哨兵相遇
    nop
    
while_arr_j_begin:              // 哨兵j先移动，找到小于基准数的数值
    add v0,s0,t2                // 将哨兵j指向的值arr[j]加载到t4寄存器
    lh  t4,(v0)
    sub s3,t4,t0                // 找到了小于基准数(t0)的值(t4)，即t4<t0结束寻找
    bltz s3,while_arr_j_end 
    nop
    
    sub s3,t2,t1                // 参数校验：如果哨兵j与哨兵i相遇或者跑到哨兵i之前就结束寻找，即t2<=t1跳转
    blez s3,while_arr_j_end  
    nop
    
    sub  t2,2                   // 哨兵j向前移动
    b    while_arr_j_begin      // 继续寻找
    nop
while_arr_j_end:
    
while_arr_i_begin:              // 哨兵i后移动，找到大于基准数的数值
    add v0,s0,t1                // 将哨兵i指向的值arr[i]加载到t3寄存器
    lh  t3,(v0)
    sub s3,t3,t0                // 找到了大于基准数(t0)的值(t3)，即t3>t0结束寻找
    bgtz s3,while_arr_i_end     
    nop
    
    sub s3,t2,t1                // 参数校验：如果哨兵i与哨兵j相遇或者跑到哨兵j之后就结束寻找，即t1>=t2跳转
    blez s3,while_arr_i_end
    nop
    
    add  t1,2                   // 哨兵i向后移动
    b    while_arr_i_begin      // 继续寻找
    nop
while_arr_i_end:

if_i_j_begin:                   // 如果两个哨兵没有相遇，就交换两个哨兵所指向的值
    sub s3,t2,t1                // 如果两哨兵相遇或错位，则不执行这部分，即t2<=t1跳转
    blez s3,if_i_j_end          
    nop

/*
    add v0,s0,t1    // 将arr[i]赋值给t3
    lh  t3,(v0)
    la  t5,0        // 将t3赋值给t5
    add t5,t3
    
    add v0,s0,t2    // 将arr[j]赋值给t4
    lh  t4,(v0)
    la  t3,0
    add t3,t4       // 将t4赋值给t3
    add v0,s0,t1    // 将t3存到arr[i]
    sh  t3,(v0)

    la  t4,0        // 将t5赋值给t4
    add t4,t5
    add v0,s0,t2    // 将t4存到arr[j]
    sh  t4,(v0)
*/
    add v0,s0,t1    // 将哨兵j指向的值(t4)存储到arr[i]
    sh  t4,(v0)
    add v0,s0,t2    // 将哨兵i指向的值(t3)存储到arr[j]
    sh  t3,(v0)

if_i_j_end:
    
    b  while_i_j_begin          // 开始下一轮遍历
    nop
while_i_j_end:
    
    /*
    add v0,s0,t1    // arr[i]=>t3
    lh  t3,(v0)
    add v0,s0,s1    // t3=>arr[begin]
    sh  t3,(v0)
    
    
    la  t3,0        // t3 = t0 (tmp)
    add t3,t0
    add v0,s0,t1    // t3=>arr[i]
    sh  t3,(v0)
    */
    
    add v0,s0,s1    // 将哨兵i指向的值(t3)存到arr[begin]
    sh  t3,(v0)
    add v0,s0,t1    // 将基准数(t0)存储到arr[i]
    sh  t0,(v0)

    
//********************************** 入栈
    add v0,s4,s5    // 入栈begin
    sh  s1,(v0)
    add s5,2
    sub t1,2        // 入栈i-2
    add v0,s4,s5
    sh  t1,(v0)
    add s5,2
    add t2,2        // 入栈j+2
    add v0,s4,s5
    sh  t2,(v0)
    add s5,2
    add v0,s4,s5    // 入栈end
    sh  s2,(v0)
    add s5,2
//**********************************

Quick_Sort_end:
    b  loop
    nop
    
finish:
    nop
    
    move    ra, s6
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)
    
    
    
    

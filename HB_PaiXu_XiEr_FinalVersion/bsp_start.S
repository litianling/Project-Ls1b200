/*
 * bsp_start.S
 *
 * created: 2022/1/27
 *  author: Li TianLing
 */


/**************************************
*
*   v0  存储器寻址辅助寄存器
*   s0  待排序数据arr基地址
*   s1  increment
*   s2  参数传递-->len
*   s3  跳转辅助寄存器
*   t1  哨兵i
*   t2  哨兵i+increment
*   t3  哨兵i指向的值，即arr[i]
*   t4  哨兵i+increment指向的值，即arr[i+increment]
*   t5  哨兵j
*   t6  哨兵j指向的值，即arr[j]
*   t7  临时变量temp
*   t8  哨兵j-increment
*   t9  哨兵j-increment指向的值，即arr[j-increment]
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

    la  s5,3
    la  s0,arr          // 加载待排序数据基地址（常量）
    la  s2,len          // 加载、计算len(一个半字两个字节)  8*2=16
    lh  s2,(s2)
    sll s2,1

    move s1,s2
while_begin:
    sub  s3,s1,2        // increment<=2结束循环
    blez s3,while_end   
    nop

    div  s1,6           // increment=(increment/3)向下取整+1,但是半字占两字节，所以increment=(increment/6)向下取整*2+2
    mflo s1
    sll  s1,1
    add  s1,2

    move t5,s1          // 哨兵j赋值
for_1_begin:            
    sub s3,t5,s2        // 参数校验，t5>=s2结束循环1,即j遍历到len-1结束
    bgez s3,for_1_end
    nop

if_begin:
    sub t8,t5,s1        // 计算j-increment
    add v0,s0,t8        // 将arr[j-increment]加载到t9
    lh  t9,(v0)
    add v0,s0,t5        // 将arr[j]加载到t6
    lh  t6,(v0)

    sub s3,t9,t6        // 如果arr[j-increment]<=arr[j],不执行if
    blez s3,if_end
    nop

    move t7,t6          // 将arr[j]直接导入temp(t7)
    
    move t2,t5          // 哨兵i初始化（哨兵j-increment）
    sub  t1,t2,s1
for_2_begin:
    sub s3,t1,zero      // t1<zero结束，即i<0结束循环2
    bltz s3,for_2_end
    nop

    add v0,s0,t1        // 将arr[i]读入t3
    lh  t3,(v0)
    sub s3,t3,t7        // arr[i]<=temp结束，即t3<=t7结束循环2
    blez s3,for_2_end
    nop

    add v0,s0,t2        // 否则将arr[i]写入arr[i+increment]
    sh  t3,(v0)

    sub t1,s1           // i-increment，开始循环2
    sub t2,s1           // (i+increment)-increment
    b for_2_begin
    nop
for_2_end:

    add v0,s0,t2        // 最后将temp插入arr[i+increment]
    sh  t7,(v0)

if_end:

    add t5,2            // j+2,开始循环1
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

/*
 * bsp_start.S
 *
 * created: 2022/1/28
 *  author: Li TianLing
 */

/**************************************
*
*   v0  存储器寻址辅助寄存器
*   v1  跳转辅助寄存器
*   s0  参数len（也是length）           （常量）
*   s1  待排序数据arr基地址             （常量）
*   s2  桶内元素个数记录数组record基地址（常量）
*   s3  桶barrel基地址                  （常量）
*   s4  参数max_scale                   （常量）
*   s5  参数mid_scale
*   s6  参数min_scale
*   t1  哨兵i
*   t2  哨兵j
*   t3  哨兵k
*   t4  待排序数组遍历arr[i]
*   t5  某桶内元素记录record[j]
*   t6  桶barrel[j*len+k]
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//数据段
    .data
    arr:    .half 33,58,999,0,233,54,133,7  // 待排序数据（数组）
    record: .space 20                       // 分配十个半字，就是20个字节
    barrel: .space 160                      // 分配80个半字，就是160个字节
	len:    .half 8                         // 待排序数据长度

    max_scale: .half 1000                   // max_scale

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* 返回地址 */

    la  v0,len          // 将长度len加载到s0（常量）
    lh  s0,(v0)
    sll s0,1
    la  s1,arr          // 将arr基地址加载到s1（常量）
    la  s2,record       // 将record基地址加载到s2（常量）
    la  s3,barrel       // 将barrel基地址加载到s3（常量）
    la  v0,max_scale    // 将max_scale加载到s4（常量）
    lh  s4,(v0)

    la  s6,1            // 将min_scale赋值为1
while_begin:
    sub v1,s6,s4        // min_scale>=max_scale就结束循环
    bgez v1,while_end
    nop

    mul s5,s6,10        // mid_scale初始化为min_scale*10
    la  t1,0            // 哨兵i初始化为0
for_pre_arr_begin:
    sub v1,t1,s0
    bgez v1,for_pre_arr_end
    nop
    
    add v0,s1,t1        // 将arr[i]的值加载到t4中
    lh  t4,(v0)
    div t4,s5           // 将arr[i]/mid_scale的余数赋值给哨兵j
    mfhi t2
    div t2,s6           // 将哨兵j/min_scale的商赋值给哨兵j
    mflo t2
    mul t2,2            // 哨兵j翻倍，【注】j是余数不是地址要翻倍
    
    add v0,s2,t2        // 将record[j]的值加载到t5中
    lh  t5,(v0)
    move t3,t5          // 将record[j]赋值给哨兵k，【注】recor[j]本来就是偶数地址，不用翻倍
    add v0,s1,t1        // 将arr[i]的值加载到t4中
    lh  t4,(v0)
    move t6,t4          // 将arr[i]赋值给t6
    mul v0,t2,s0        // 将t6的值保存到barrel[j*len+k]
    srl v0,1            // 【注】j和len乘积是四倍，所以要右移一位除以二
    add v0,s3
    add v0,t3
    sh  t6,(v0)
    
    add t5,2            // record[j]+2
    add v0,s2,t2        // 保存
    sh  t5,(v0)

    add t1,2            // 哨兵i+2
    b   for_pre_arr_begin
    nop
for_pre_arr_end:
    
    la  t2,0            // 哨兵j复位
    la  t1,0            // 哨兵i复位
    
for_after_arr_begin:
    sub v1,t1,s0
    bgez v1,for_after_arr_end
    nop
    
for_record_begin:
    add v0,s2,t2        // 将record[j]加载到t5
    lh  t5,(v0)
    bne t5,zero,for_record_end  // 非空就结束
    nop
    
    add t2,2            // 哨兵j+2
    b   for_record_begin
    nop
for_record_end:
    
    la  t3,0            // 哨兵k初始化为0
for_barrel_begin:
    // record[j]不用重新加载

    sub v1,t3,t5        // 哨兵k>=record[j]结束
    bgez v1,for_barrel_end
    nop
    
    mul v0,t2,s0        // barrel[j*len+k]加载到t6
    srl v0,1            // 【注】j和len乘积是四倍，所以要右移一位除以二   *************************srl
    add v0,s3
    add v0,t3
    lh  t6,(v0)
    move t4,t6          // 将t6赋值给t4
    add v0,s1,t1        // 将t4保存到arr[i]
    sh  t4,(v0)
    
    add t1,2            // 哨兵i+2
    add t3,2            // 哨兵k+2
    b   for_barrel_begin
    nop
for_barrel_end:
    
    la  t5,0            // 将record[j]清除
    add v0,s2,t2
    sh  t5,(v0)
    
    b   for_after_arr_begin
    nop
for_after_arr_end:
    
    move s6,s5          // min_scale = mid_scale

    b   while_begin
    nop
while_end:

finish:
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)



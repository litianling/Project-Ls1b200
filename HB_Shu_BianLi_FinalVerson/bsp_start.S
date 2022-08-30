/*
 * bsp_start.S
 *
 * created: 2022/2/14
 *  author: Li TianLing
 */

/**************************************
*
*   v0  存储器寻址辅助寄存器(偏移地址)
*
*   a0->a3 数据区基地址
*
*   s0  当前节点i
*   s1  左叶子节点2*1+1
*   s2  右叶子节点2*i+2
*
*   t0  长度len     （常数）
*   t1  （坐标）缓存区指针
*   t2  输出指针
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"


//-----------------------------------------------------------------------------
//数据段——半字存储，每个数据占两个字节
    .data
    tree:   .half 20,19,17,16,18,10,15,14,12,13,0,9,7,6,8
    len:    .half 15        // 长度
    stack:  .space 30       // （坐标）缓存堆栈
    lev_out: .space 30      // 层序输出
    pre_out: .space 30      // 先序输出
    mid_out: .space 30      // 中序输出
    aft_out: .space 30      // 后序输出

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra          /* 返回地址 */

    la a0,lev_out
    la a1,pre_out
    la a2,mid_out
    la a3,aft_out
    la  t0,len              // 将长度len加载到t0
    lh  t0,(t0)
    mul t0,2

/* 层序遍历 */
    la  t2,0                // 输出指针初始化
print_lev_begin:
    la  s0,0                // 先从根节点开始遍历
for_lev_begin:
    bge s0,t0,for_lev_end   // 节点超出范围结束
    nop

if_pre_begin:
    lh  s3,tree(s0)         // 将tree[i]加载到s3
    beq s3,zero,if_pre_end  // 剔除空元素
    nop

    sh  s3,lev_out(t2)      // 否则将其导出到lev_out
    add t2,2
if_pre_end:

    add s0,2                // 遍历下一个节点
    b   for_lev_begin
    nop
for_lev_end:
print_lev_end:


/* 前序遍历 */
    la  t1,0                // （坐标）缓存区指针初始化
    sh  zero,stack(t1)      // 先将根节点坐标入栈
    add t1,2
    la  t2,0                // 输出指针初始化
print_pre_begin:
    blez t1,print_pre_end   // 缓存区堆栈指针小于等于0结束遍历
    nop
    sub t1,2                // 加载缓存区中坐标
    lh  s0,stack(t1)
    bge s0,t0,print_pre_begin // 当前节点超出范围，换下一个节点重新遍历
    nop

    lh  s3,tree(s0)         // 将tree[i]加载到s3
    sh  s3,pre_out(t2)      // 将其导出到pre_out
    add t2,2

    mul s2,s0,2             // 右叶子节点坐标2*i+4（half类型）
    add s2,4
    lh  s3,tree(s2)         // 将tree[右]加载到s3
    beq s3,zero,pre_jump_right  // 为空不入栈
    nop
    sh  s2,stack(t1)        // 将右叶子节点坐标入栈
    add t1,2
pre_jump_right:

    mul s1,s0,2             // 左叶子节点坐标2*i+2（half类型）
    add s1,2
    lh  s3,tree(s1)         // 将tree[左]加载到s3
    beq s3,zero,pre_jump_left  // 为空不入栈
    nop
    sh  s1,stack(t1)        // 将左叶子节点坐标入栈
    add t1,2
pre_jump_left:

    b   print_pre_begin
    nop
print_pre_end:


/* 中序遍历 */
    la  t1,0                // （坐标）缓存区指针初始化
    la  t2,0                // 输出指针初始化
    la  s0,0                // 当前节点从根节点开始
print_mid_begin:

while_go_left_1_begin:
    mul s1,s0,2             // 左节点坐标2*i+2
    add s1,2
    bge s1,t0,while_go_left_1_end
    nop                     // 当前节点超出范围，结束

    lh  s3,tree(s1)         // 将tree[左]加载到s3
    beq s3,zero,while_go_left_1_end
    nop                     // 为空结束

    sh  s0,stack(t1)        // 节点坐标i入栈
    add t1,2

    mul s0,2                // i=i*2+2访问左节点
    add s0,2
    b   while_go_left_1_begin
    nop
while_go_left_1_end:

while_1_begin:
    lh  s3,tree(s0)         // 将tree[i]加载到s3
    sh  s3,mid_out(t2)      // 输出
    add t2,2

if_mid_begin:
    mul s2,s0,2             // 右节点坐标2*i+4
    add s2,4
    bge s2,t0,else_if_mid   // 当前节点超出范围跳转
    nop

    lh  s3,tree(s2)         // 将tree[右]加载到s3
    beq s3,zero,else_if_mid
    nop                     // 为空结束

    mul s0,2                // i=i*2+4访问右节点
    add s0,4
while_go_left_2_begin:
    mul s1,s0,2             // 左节点坐标2*i+2
    add s1,2
    bge s1,t0,while_go_left_2_end
    nop                     // 当前节点超出范围，结束

    lh  s3,tree(s1)         // 将tree[左]加载到s3
    beq s3,zero,while_go_left_2_end
    nop                     // 为空结束

    sh  s0,stack(t1)        // 节点坐标i入栈
    add t1,2

    mul s0,2                // i=i*2+2访问左节点
    add s0,2
    b   while_go_left_2_begin
    nop
while_go_left_2_end:

    b   if_mid_end          // 退出if
    nop
else_if_mid:
    beq t1,zero,else_mid    // 堆栈为空跳转
    nop

    sub t1,2
    lh  s0,stack(t1)

    b   if_mid_end          // 退出if
    nop
else_mid:
    b   while_1_end
    nop

if_mid_end:

    b   while_1_begin
    nop
while_1_end:

print_mid_end:


/* 后序遍历 */
    la  t1,0                // （坐标）缓存区指针初始化
    sh  zero,stack(t1)      // 先将根节点坐标入栈
    add t1,2
    move t2,t0               // 输出指针初始化到最后一格
    sub t2,2
print_aft_begin:
    blez t1,print_aft_end   // 缓存区堆栈指针小于0结束遍历
    nop
    sub t1,2                // 加载缓存区中坐标
    lh  s0,stack(t1)
    bge s0,t0,print_aft_begin // 当前节点超出范围重新遍历
    nop

    lh  s3,tree(s0)         // 将tree[i]加载到s3
    sh  s3,aft_out(t2)      // 将其导出到pre_out
    sub t2,2

    mul s1,s0,2             // 左叶子节点坐标2*i+2（half类型）
    add s1,2
    lh  s3,tree(s1)         // 将tree[左]加载到s3
    beq s3,zero,aft_jump_left  // 为空不入栈
    nop
    sh  s1,stack(t1)        // 将左叶子节点坐标入栈
    add t1,2
aft_jump_left:

    mul s2,s0,2             // 右叶子节点坐标2*i+4（half类型）
    add s2,4
    lh  s3,tree(s2)         // 将tree[右]加载到s3
    beq s3,zero,aft_jump_right  // 为空不入栈
    nop
    sh  s2,stack(t1)        // 将右叶子节点坐标入栈
    add t1,2
aft_jump_right:

    b   print_aft_begin
    nop
print_aft_end:

finish:
    nop
    nop

    //结束
    move    ra, k0
    j       ra
	nop

	.set reorder
ENDFRAME(bsp_start)


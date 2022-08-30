/*
 * bsp_start.S
 *
 * created: 2022/2/19
 * author: Li TianLing
 * 链表表示无向图的邻接表
 */


/**************************************
*
*   v0  存储器寻址辅助寄存器
*   v1  跳转辅助寄存器
*
*   s0  无向图关系graph_ralation基地址  （常量）
*   s1  无向图邻接表graph_buffer基地址  （常量）
*   s2  遍历输出邻接表graph_out基地址   （常量）
*   s3  无向图节点数node_number         （常量）（坐标.半字翻倍）
*   s4  无向图边数max_edge_number       （常量）
*   s5  无向图邻接表大小max_graph_size  （常量）（坐标.半字翻倍）
*
*   t0  参数src     （源节点）
*   t1  参数dest    （目标节点）
*   t2  参数graph_label （节点存储位置）
*   t3  参数check
*   t4  哨兵i
*   t5  无向图关系最大坐标
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//数据段
    .data
    graph_ralation: .half 0,1,0,4,1,2,1,3,1,4,2,3,3,4 // 无向图关系（两个一组2*max_edge_number个数据）
    graph_buffer:   .space 66    // 链表存储图的邻接表（半字存储2*max_graph_size）
    graph_out:      .space 66    // 链表遍历输出
    
    node_number:        .half 5  // 无向图节点数
    max_edge_number:    .half 7  // 无向图边数
    max_graph_size:     .half 33 // 无向图邻接表大小（半字存储）

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* 返回地址 */

    la  s0,graph_ralation   // 将graph_ralation基地址加载到s0（常量）
    la  s1,graph_buffer     // 将graph_buffer基地址加载到s1  （常量）
    la  s2,graph_out        // 将graph_out基地址加载到s2     （常量）
    
    la  v0,node_number      // 将node_number加载到s3    （常量）
    lh  s3,(v0)
    sll s3,1
    la  v0,max_edge_number  // 将max_edge_number加载到s4（常量）
    lh  s4,(v0)
    la  v0,max_graph_size   // 将max_graph_size加载到s5 （常量）
    lh  s5,(v0)
    sll s5,1

    move t2,s3              // 初始化graph_label=node_number
    la  t4,0                // 哨兵i，遍历关系数组
    move t5,s4              // 关系数组最大坐标=边长*2*2
    sll t5,2
for_add_all_node_begin:
    bge t4,t5,for_add_all_node_end // t4>=t5,遍历完关系矩阵，节点添加完成
    nop
    
    lh  t0,graph_ralation(t4) // 加载源节点src
    sll t0,1                  // src*2j用于坐标计算
    add t4,2
    lh  t1,graph_ralation(t4) // 加载目标节点dest
    sll t1,1                  // dest*2j用于坐标计算
    add t4,2
    
//ADD_ONE_NODE_BEGIN:

if_src_begin:
    lh  s6,graph_buffer(t0) // 加载graph[src]
    bne s6,zero,else_sec    // 非空跳转到else
    nop
    
    sh  t2,graph_buffer(t0) // graph[src]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t1,1                // dest/2用于存储
    sh  t1,graph_buffer(t2) // graph[graph_label]=dest
    sll t1,1                // dest*2用于坐标计算
    add t2,2                // graph_label=graph_label+2
    
    b   if_sec_end
    nop
else_sec:

    move t3,t0              // 初始化check=src
while_src_begin:
    lh  s6,graph_buffer(t3) // 将graph[check]加载到s6
    beq s6,zero,while_src_end // 为0跳转，结束while
    nop
    move t3,s6              // check=graph[check]
    b   while_src_begin
    nop
while_src_end:
    sh  t2,graph_buffer(t3) // graph[check]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t1,1                // dest/2用于存储
    sh  t1,graph_buffer(t2) // graph[graph_label]=dest
    sll t1,1                // dest*2用于坐标计算
    add t2,2                // graph_label=graph_label+2
    
if_sec_end:

//add_src_node_end_&_add_dest_node_begin:

if_dest_begin:
    lh  s6,graph_buffer(t1) // 加载graph[dest]
    bne s6,zero,else_dest   // 非空跳转到else
    nop

    sh  t2,graph_buffer(t1) // graph[dest]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t0,1                // src/2用于存储
    sh  t0,graph_buffer(t2) // graph[graph_label]=src
    sll t0,1                // src*2用于坐标计算
    add t2,2                // graph_label=graph_label+2

    b   if_dest_end
    nop
else_dest:

    move t3,t1              // 初始化check=dest
while_dest_begin:
    lh  s6,graph_buffer(t3) // 将graph[check]加载到s6
    beq s6,zero,while_dest_end // 为0跳转，结束while
    nop
    move t3,s6              // check=graph[check]
    b   while_dest_begin
    nop
while_dest_end:
    sh  t2,graph_buffer(t3) // graph[check]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t0,1                // src/2用于存储
    sh  t0,graph_buffer(t2) // graph[graph_label]=src
    sll t0,1                // src*2用于坐标计算
    add t2,2                // graph_label=graph_label+2

if_dest_end:

//ADD_ONE_NODE_END:

    b   for_add_all_node_begin
    nop
for_add_all_node_end:


//PRINT_GRAPH_BEGIN:

    la  t2,0                // 初始化graph_label=0，辅助输出
    la  t4,0                // i=0
for_i_begin:
    bge t4,s3,for_i_end    // i>=node_number,遍历完所有节点，结束
    nop
    
    srl t4,1                // i/2，数据输出
    sh  t4,graph_out(t2)    // graph_out[graph_label]=i
    add t2,2                // graph_label=graph_label+2
    sll t4,1                // i*2，坐标计算
    
    lh  t3,graph_buffer(t4) // check=graph[i]
for_check_begin:
    beq t3,zero,for_check_end // check=0结束
    nop
    
    add t3,2                // check=check+2,用于输出数据
    lh  s6,graph_buffer(t3) // 将graph[check]加载到s6
    sub t3,2                // check=check-2,恢复check
    sh  s6,graph_out(t2)    // 将t6保存到graph_out[graph_label]
    add t2,2                // graph_label=graph_label+2

    lh  t3,graph_buffer(t3) // check=graph[check]
    b   for_check_begin
    nop
for_check_end:
    
    add t2,4                // graph_label=graph_label+6    【注】：两个0作为结束标志
    
    add t4,2                // i=i+2
    b   for_i_begin
    nop
for_i_end:

//PRINT_GRAPH_END:

    /* 程序运行结束 */
finish:
    nop
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)





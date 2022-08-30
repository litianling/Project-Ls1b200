/*
 * bsp_start.S
 *
 * created: 2022/2/23
 *  author: Li TianLig
 */

/**************************************
*
*   v0  存储器寻址辅助寄存器
*   v1  跳转辅助寄存器or观测标志
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
*   t2  参数graph_label （节点存储位置）（之后用于DFS）
*   t3  参数check
*   t4  哨兵i
*   t5  无向图关系最大坐标
*   t6  DFS and BFS输出位置
*   t7  buffer_flag
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

#define GRAPH_OUT
#define GRAPH_DFS
#define GRAPH_BFS

//-----------------------------------------------------------------------------
//数据段
    .data

    graph_ralation: .half 0,1,0,4,1,2,1,3,1,4,2,3,3,4 // 无向图关系（两个一组2*max_edge_number个数据）
    graph_buffer:   .space 66    // 链表存储图的邻接表（半字存储2*max_graph_size）
    visited:        .space 10     // 已观测标志
#ifdef GRAPH_OUT
    graph_out:      .space 66    // 遍历链表输出
#endif
#ifdef GRAPH_DFS
    graph_dfs:      .space 10     // 深度优先搜索
#endif
#ifdef GRAPH_BFS
    graph_bfs:      .space 10     // 广度优先搜索
    node_buffer:    .space 10     // 缓存堆栈
#endif
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
#ifdef GRAPH_OUT
    la  s2,graph_out        // 将graph_out基地址加载到s2     （常量）
#endif
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


#ifdef GRAPH_OUT    /* PRINT_GRAPH_BEGIN */
    la  t2,0                // 初始化graph_label=0，辅助输出
    la  t4,0                // i=0
for_i_begin:
    bge t4,s3,for_i_end     // i>=node_number,遍历完所有节点，结束
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
#endif          /* PRINT_GRAPH_END */


#ifdef GRAPH_DFS    /* PRINT_DFS_BEGIN */
/* 默认从0开始搜索,v1=1代表已观测 */
    la  v1,1                // 观测标志初始化
    la  t6,0                // DFS输出位置初始化
    sh  zero,graph_dfs(t6)  // 保存第一个节点0
    add t6,2
    sh  v1,visited(zero)    // visited[0]=1
    lh  t3,graph_buffer(zero)// check=graph[node]
    
while_DFS_begin:
    beq t3,zero,while_DFS_end// check=0表示无连接点，结束while
    nop
    
if_DFS_begin:
    add v0,t3,2             // v0=check+2
    lh  v0,graph_buffer(v0) // v0=graph[v0]
    sll v0,1                // v0*2坐标
    lh  v0,visited(v0)      // v0=visited[v0]
    beq v0,v1,if_DFS_end    // v0=1表示已观测，结束if
    nop
    
    add v0,t3,2             // v0=graph[check+2]
    lh  v0,graph_buffer(v0)
    sh  v0,graph_dfs(t6)    // 保存v0
    add t6,2
    sll v0,1                // v0*2坐标
    sh  v1,visited(v0)      // 标记为已观测
    
    move t3,v0              // check=graph[check+2]
    lh  t3,graph_buffer(t3) // check=graph[check]
    
    b   while_DFS_begin     // conntinue
    nop
if_DFS_end:
    
    lh  t3,graph_buffer(t3) // check=graph[check]
    
    b   while_DFS_begin
    nop
while_DFS_end:
#endif          /* PRINT_DFS_END */


#ifdef GRAPH_BFS
    la  t4,0                // i=0
for_clean_visited_begin:
    bge t4,s3,for_clean_visited_end
    nop
    
    sh  zero,visited(t4)
    
    add t4,2
    b   for_clean_visited_begin
    nop
for_clean_visited_end:

/* PRINT_BFS_BEGIN */
    la  v1,1                // 观测标志初始化
    la  t6,0                // BFS输出位置初始化
    la  t7,0                // buffer_flag初始化
    sh  zero,graph_bfs(t6)  // 保存第一个节点0
    add t6,2
    sh  v1,visited(zero)    // visited[0]=1
    sh  zero,node_buffer(t7)// 开始节点入栈
    add t7,2
    
while_buffer_begin:
    beqz t7,while_buffer_end// buffer_flag=0堆栈为空，结束while
    nop
    
    sub t7,2                // 出栈
    lh  t3,node_buffer(t7)  // check=node_buffer[buffer_flag]
    sll t3,1                // check*2坐标
    lh  t3,graph_buffer(t3) // check=graph[check]
    
while_BFS_begin:
    beq t3,zero,while_BFS_end// check=0表示无连接点，结束while
    nop
    
if_BFS_begin:
    add v0,t3,2             // v0=check+2
    lh  v0,graph_buffer(v0) // v0=graph[v0]
    sll v0,1                // v0*2坐标
    lh  v0,visited(v0)      // v0=visited[v0]
    beq v0,v1,if_BFS_end    // v0=1表示已观测，结束if
    nop
    
    add v0,t3,2             // v0=graph[check+2]
    lh  v0,graph_buffer(v0)
    sh  v0,graph_bfs(t6)    // 保存v0
    add t6,2
    sh  v0,node_buffer(t7)  // 入栈v0
    add t7,2
    sll v0,1                // v0*2坐标
    sh  v1,visited(v0)      // 标记为已观测
if_BFS_end:
    
    lh  t3,graph_buffer(t3) // check=graph[check]
    
    b   while_BFS_begin
    nop
while_BFS_end:
    
    b   while_buffer_begin
    nop
while_buffer_end:
#endif              /* PRINT_BFS_END */

    /* 程序运行结束 */
#ifdef GRAPH_DFS
    la  s6,graph_dfs
#endif
#ifdef GRAPH_BFS
    la  s7,graph_bfs
#endif
finish:
    nop
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)

/*
 * bsp_start.S
 *
 * created: 2022/2/19
 * author: Li TianLing
 * ������ʾ����ͼ���ڽӱ�
 */


/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���
*   v1  ��ת�����Ĵ���
*
*   s0  ����ͼ��ϵgraph_ralation����ַ  ��������
*   s1  ����ͼ�ڽӱ�graph_buffer����ַ  ��������
*   s2  ��������ڽӱ�graph_out����ַ   ��������
*   s3  ����ͼ�ڵ���node_number         ��������������.���ַ�����
*   s4  ����ͼ����max_edge_number       ��������
*   s5  ����ͼ�ڽӱ���Сmax_graph_size  ��������������.���ַ�����
*
*   t0  ����src     ��Դ�ڵ㣩
*   t1  ����dest    ��Ŀ��ڵ㣩
*   t2  ����graph_label ���ڵ�洢λ�ã�
*   t3  ����check
*   t4  �ڱ�i
*   t5  ����ͼ��ϵ�������
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//���ݶ�
    .data
    graph_ralation: .half 0,1,0,4,1,2,1,3,1,4,2,3,3,4 // ����ͼ��ϵ������һ��2*max_edge_number�����ݣ�
    graph_buffer:   .space 66    // �����洢ͼ���ڽӱ������ִ洢2*max_graph_size��
    graph_out:      .space 66    // �����������
    
    node_number:        .half 5  // ����ͼ�ڵ���
    max_edge_number:    .half 7  // ����ͼ����
    max_graph_size:     .half 33 // ����ͼ�ڽӱ���С�����ִ洢��

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* ���ص�ַ */

    la  s0,graph_ralation   // ��graph_ralation����ַ���ص�s0��������
    la  s1,graph_buffer     // ��graph_buffer����ַ���ص�s1  ��������
    la  s2,graph_out        // ��graph_out����ַ���ص�s2     ��������
    
    la  v0,node_number      // ��node_number���ص�s3    ��������
    lh  s3,(v0)
    sll s3,1
    la  v0,max_edge_number  // ��max_edge_number���ص�s4��������
    lh  s4,(v0)
    la  v0,max_graph_size   // ��max_graph_size���ص�s5 ��������
    lh  s5,(v0)
    sll s5,1

    move t2,s3              // ��ʼ��graph_label=node_number
    la  t4,0                // �ڱ�i��������ϵ����
    move t5,s4              // ��ϵ�����������=�߳�*2*2
    sll t5,2
for_add_all_node_begin:
    bge t4,t5,for_add_all_node_end // t4>=t5,�������ϵ���󣬽ڵ��������
    nop
    
    lh  t0,graph_ralation(t4) // ����Դ�ڵ�src
    sll t0,1                  // src*2j�����������
    add t4,2
    lh  t1,graph_ralation(t4) // ����Ŀ��ڵ�dest
    sll t1,1                  // dest*2j�����������
    add t4,2
    
//ADD_ONE_NODE_BEGIN:

if_src_begin:
    lh  s6,graph_buffer(t0) // ����graph[src]
    bne s6,zero,else_sec    // �ǿ���ת��else
    nop
    
    sh  t2,graph_buffer(t0) // graph[src]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t1,1                // dest/2���ڴ洢
    sh  t1,graph_buffer(t2) // graph[graph_label]=dest
    sll t1,1                // dest*2�����������
    add t2,2                // graph_label=graph_label+2
    
    b   if_sec_end
    nop
else_sec:

    move t3,t0              // ��ʼ��check=src
while_src_begin:
    lh  s6,graph_buffer(t3) // ��graph[check]���ص�s6
    beq s6,zero,while_src_end // Ϊ0��ת������while
    nop
    move t3,s6              // check=graph[check]
    b   while_src_begin
    nop
while_src_end:
    sh  t2,graph_buffer(t3) // graph[check]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t1,1                // dest/2���ڴ洢
    sh  t1,graph_buffer(t2) // graph[graph_label]=dest
    sll t1,1                // dest*2�����������
    add t2,2                // graph_label=graph_label+2
    
if_sec_end:

//add_src_node_end_&_add_dest_node_begin:

if_dest_begin:
    lh  s6,graph_buffer(t1) // ����graph[dest]
    bne s6,zero,else_dest   // �ǿ���ת��else
    nop

    sh  t2,graph_buffer(t1) // graph[dest]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t0,1                // src/2���ڴ洢
    sh  t0,graph_buffer(t2) // graph[graph_label]=src
    sll t0,1                // src*2�����������
    add t2,2                // graph_label=graph_label+2

    b   if_dest_end
    nop
else_dest:

    move t3,t1              // ��ʼ��check=dest
while_dest_begin:
    lh  s6,graph_buffer(t3) // ��graph[check]���ص�s6
    beq s6,zero,while_dest_end // Ϊ0��ת������while
    nop
    move t3,s6              // check=graph[check]
    b   while_dest_begin
    nop
while_dest_end:
    sh  t2,graph_buffer(t3) // graph[check]=graph_label
    add t2,2                // graph_label=graph_label+2
    srl t0,1                // src/2���ڴ洢
    sh  t0,graph_buffer(t2) // graph[graph_label]=src
    sll t0,1                // src*2�����������
    add t2,2                // graph_label=graph_label+2

if_dest_end:

//ADD_ONE_NODE_END:

    b   for_add_all_node_begin
    nop
for_add_all_node_end:


//PRINT_GRAPH_BEGIN:

    la  t2,0                // ��ʼ��graph_label=0���������
    la  t4,0                // i=0
for_i_begin:
    bge t4,s3,for_i_end    // i>=node_number,���������нڵ㣬����
    nop
    
    srl t4,1                // i/2���������
    sh  t4,graph_out(t2)    // graph_out[graph_label]=i
    add t2,2                // graph_label=graph_label+2
    sll t4,1                // i*2���������
    
    lh  t3,graph_buffer(t4) // check=graph[i]
for_check_begin:
    beq t3,zero,for_check_end // check=0����
    nop
    
    add t3,2                // check=check+2,�����������
    lh  s6,graph_buffer(t3) // ��graph[check]���ص�s6
    sub t3,2                // check=check-2,�ָ�check
    sh  s6,graph_out(t2)    // ��t6���浽graph_out[graph_label]
    add t2,2                // graph_label=graph_label+2

    lh  t3,graph_buffer(t3) // check=graph[check]
    b   for_check_begin
    nop
for_check_end:
    
    add t2,4                // graph_label=graph_label+6    ��ע��������0��Ϊ������־
    
    add t4,2                // i=i+2
    b   for_i_begin
    nop
for_i_end:

//PRINT_GRAPH_END:

    /* �������н��� */
finish:
    nop
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)




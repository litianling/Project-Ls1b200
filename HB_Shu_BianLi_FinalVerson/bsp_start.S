/*
 * bsp_start.S
 *
 * created: 2022/2/14
 *  author: Li TianLing
 */

/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���(ƫ�Ƶ�ַ)
*
*   a0->a3 ����������ַ
*
*   s0  ��ǰ�ڵ�i
*   s1  ��Ҷ�ӽڵ�2*1+1
*   s2  ��Ҷ�ӽڵ�2*i+2
*
*   t0  ����len     ��������
*   t1  �����꣩������ָ��
*   t2  ���ָ��
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"


//-----------------------------------------------------------------------------
//���ݶΡ������ִ洢��ÿ������ռ�����ֽ�
    .data
    tree:   .half 20,19,17,16,18,10,15,14,12,13,0,9,7,6,8
    len:    .half 15        // ����
    stack:  .space 30       // �����꣩�����ջ
    lev_out: .space 30      // �������
    pre_out: .space 30      // �������
    mid_out: .space 30      // �������
    aft_out: .space 30      // �������

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra          /* ���ص�ַ */

    la a0,lev_out
    la a1,pre_out
    la a2,mid_out
    la a3,aft_out
    la  t0,len              // ������len���ص�t0
    lh  t0,(t0)
    mul t0,2

/* ������� */
    la  t2,0                // ���ָ���ʼ��
print_lev_begin:
    la  s0,0                // �ȴӸ��ڵ㿪ʼ����
for_lev_begin:
    bge s0,t0,for_lev_end   // �ڵ㳬����Χ����
    nop

if_pre_begin:
    lh  s3,tree(s0)         // ��tree[i]���ص�s3
    beq s3,zero,if_pre_end  // �޳���Ԫ��
    nop

    sh  s3,lev_out(t2)      // �����䵼����lev_out
    add t2,2
if_pre_end:

    add s0,2                // ������һ���ڵ�
    b   for_lev_begin
    nop
for_lev_end:
print_lev_end:


/* ǰ����� */
    la  t1,0                // �����꣩������ָ���ʼ��
    sh  zero,stack(t1)      // �Ƚ����ڵ�������ջ
    add t1,2
    la  t2,0                // ���ָ���ʼ��
print_pre_begin:
    blez t1,print_pre_end   // ��������ջָ��С�ڵ���0��������
    nop
    sub t1,2                // ���ػ�����������
    lh  s0,stack(t1)
    bge s0,t0,print_pre_begin // ��ǰ�ڵ㳬����Χ������һ���ڵ����±���
    nop

    lh  s3,tree(s0)         // ��tree[i]���ص�s3
    sh  s3,pre_out(t2)      // ���䵼����pre_out
    add t2,2

    mul s2,s0,2             // ��Ҷ�ӽڵ�����2*i+4��half���ͣ�
    add s2,4
    lh  s3,tree(s2)         // ��tree[��]���ص�s3
    beq s3,zero,pre_jump_right  // Ϊ�ղ���ջ
    nop
    sh  s2,stack(t1)        // ����Ҷ�ӽڵ�������ջ
    add t1,2
pre_jump_right:

    mul s1,s0,2             // ��Ҷ�ӽڵ�����2*i+2��half���ͣ�
    add s1,2
    lh  s3,tree(s1)         // ��tree[��]���ص�s3
    beq s3,zero,pre_jump_left  // Ϊ�ղ���ջ
    nop
    sh  s1,stack(t1)        // ����Ҷ�ӽڵ�������ջ
    add t1,2
pre_jump_left:

    b   print_pre_begin
    nop
print_pre_end:


/* ������� */
    la  t1,0                // �����꣩������ָ���ʼ��
    la  t2,0                // ���ָ���ʼ��
    la  s0,0                // ��ǰ�ڵ�Ӹ��ڵ㿪ʼ
print_mid_begin:

while_go_left_1_begin:
    mul s1,s0,2             // ��ڵ�����2*i+2
    add s1,2
    bge s1,t0,while_go_left_1_end
    nop                     // ��ǰ�ڵ㳬����Χ������

    lh  s3,tree(s1)         // ��tree[��]���ص�s3
    beq s3,zero,while_go_left_1_end
    nop                     // Ϊ�ս���

    sh  s0,stack(t1)        // �ڵ�����i��ջ
    add t1,2

    mul s0,2                // i=i*2+2������ڵ�
    add s0,2
    b   while_go_left_1_begin
    nop
while_go_left_1_end:

while_1_begin:
    lh  s3,tree(s0)         // ��tree[i]���ص�s3
    sh  s3,mid_out(t2)      // ���
    add t2,2

if_mid_begin:
    mul s2,s0,2             // �ҽڵ�����2*i+4
    add s2,4
    bge s2,t0,else_if_mid   // ��ǰ�ڵ㳬����Χ��ת
    nop

    lh  s3,tree(s2)         // ��tree[��]���ص�s3
    beq s3,zero,else_if_mid
    nop                     // Ϊ�ս���

    mul s0,2                // i=i*2+4�����ҽڵ�
    add s0,4
while_go_left_2_begin:
    mul s1,s0,2             // ��ڵ�����2*i+2
    add s1,2
    bge s1,t0,while_go_left_2_end
    nop                     // ��ǰ�ڵ㳬����Χ������

    lh  s3,tree(s1)         // ��tree[��]���ص�s3
    beq s3,zero,while_go_left_2_end
    nop                     // Ϊ�ս���

    sh  s0,stack(t1)        // �ڵ�����i��ջ
    add t1,2

    mul s0,2                // i=i*2+2������ڵ�
    add s0,2
    b   while_go_left_2_begin
    nop
while_go_left_2_end:

    b   if_mid_end          // �˳�if
    nop
else_if_mid:
    beq t1,zero,else_mid    // ��ջΪ����ת
    nop

    sub t1,2
    lh  s0,stack(t1)

    b   if_mid_end          // �˳�if
    nop
else_mid:
    b   while_1_end
    nop

if_mid_end:

    b   while_1_begin
    nop
while_1_end:

print_mid_end:


/* ������� */
    la  t1,0                // �����꣩������ָ���ʼ��
    sh  zero,stack(t1)      // �Ƚ����ڵ�������ջ
    add t1,2
    move t2,t0               // ���ָ���ʼ�������һ��
    sub t2,2
print_aft_begin:
    blez t1,print_aft_end   // ��������ջָ��С��0��������
    nop
    sub t1,2                // ���ػ�����������
    lh  s0,stack(t1)
    bge s0,t0,print_aft_begin // ��ǰ�ڵ㳬����Χ���±���
    nop

    lh  s3,tree(s0)         // ��tree[i]���ص�s3
    sh  s3,aft_out(t2)      // ���䵼����pre_out
    sub t2,2

    mul s1,s0,2             // ��Ҷ�ӽڵ�����2*i+2��half���ͣ�
    add s1,2
    lh  s3,tree(s1)         // ��tree[��]���ص�s3
    beq s3,zero,aft_jump_left  // Ϊ�ղ���ջ
    nop
    sh  s1,stack(t1)        // ����Ҷ�ӽڵ�������ջ
    add t1,2
aft_jump_left:

    mul s2,s0,2             // ��Ҷ�ӽڵ�����2*i+4��half���ͣ�
    add s2,4
    lh  s3,tree(s2)         // ��tree[��]���ص�s3
    beq s3,zero,aft_jump_right  // Ϊ�ղ���ջ
    nop
    sh  s2,stack(t1)        // ����Ҷ�ӽڵ�������ջ
    add t1,2
aft_jump_right:

    b   print_aft_begin
    nop
print_aft_end:

finish:
    nop
    nop

    //����
    move    ra, k0
    j       ra
	nop

	.set reorder
ENDFRAME(bsp_start)

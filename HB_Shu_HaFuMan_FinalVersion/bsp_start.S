/*
 * bsp_start.S
 *
 * created: 2022/2/10
 *  author: Li TianLing
 */


/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���
*   v1  ��ת�����Ĵ���
*
*   s0  ����������basic_date����ַ  ��������
*   s1  ��������HF_Tree����ַ       ��������
*   s2  ����������HF_code����ַ     ��������
*   s3  ���뻺��ռ�code����ַ      ��������
*   s4  ���������ݸ���              ��������
*
*   t0  ���������ܽڵ���m     ��half����*2��
*   t1  ѭ���ڱ�i
*   t2  ѭ���ڱ�j
*   t3  ��Сֵ������s1
*   t4  ��Сֵ������s2
*   t5  ��Сֵ����ɸѡmin
*   t6  ����start
*   t7  ����c
*   t8  ����p
*   t9  ����len
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//���ݶ�
    .data
    basic_date: .half  1,2,3,4,5,6,7,8  // ���������ݣ����飩
    HF_Tree:    .space 128              // ��������
    HF_code:    .space 144              // ����������
    code:       .space 16               // ���������뻺��ռ�
	number:     .half  8                // ���������ݳ���

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* ���ص�ַ */

    la  s0,basic_date   // ��basic_date����ַ���ص�s0��������
    la  s1,HF_Tree      // ��HF_Tree����ַ���ص�s0��������
    la  s2,HF_code      // ��HF_code����ַ���ص�s0��������
    la  s3,code         // ��code����ַ���ص�s0��������
    la  v0,number       // ������numer���ص�s4��������
    lh  s4,(v0)
    sll s4,1

    /* ��ʼ������������ */
CREAT_HAFFF_BEGIN:
    mul t0,s4,2         // m=number*2-2
    sub t0,2

    la  t1,2            // i=2
for_leaf_copy_begin:    // ��Ȩֵ����n��Ҷ�ӽڵ�
    sub v1,t1,s4        // i>number����
    bgtz v1,for_leaf_copy_end
    nop
    
    sub v0,t1,2         // ��basic_date[i-2]���ص�s5
    add v0,s0
    lh  s5,(v0)
    mul v0,t1,4         // ��s5���浽HF_Tree[i*4]
    add v0,s1
    sh  s5,(v0)
    
    add t1,2
    b   for_leaf_copy_begin
    nop
for_leaf_copy_end:
    
    add t1,s4,2             // i=number+2
for_creat_hafftree_begin:   // ������������
    sub v1,t1,t0            // i>m����
    bgtz v1,for_creat_hafftree_end
    nop
    
    /* ��ʼ��ѡ��Сֵ */
SELECT_BEGIN:
    sub t9,t1,2             // len=i-2
    la  t2,2                // j=2
for_initial_min_begin_1:    // ��ʼ��min�������е�һ���޸��ڵ������
    sub v1,t2,t9            // j>len����
    bgtz v1,for_initial_min_end_1
    nop

if_no_father_begin_1:       // ����޸��ڵ�
    mul v0,t2,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[j*4+2]���ص�s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    
    bne s5,zero,if_no_father_end_1 // ���ڵ㲻Ϊ0�Ͳ�ִ�д�if
    nop
    
    move t5,t2              // ����min=j
    b   for_initial_min_end_1 // ������forѭ��
    nop
if_no_father_end_1:

    add t2,2                // j=j+2
    b   for_initial_min_begin_1
    nop
for_initial_min_end_1:

    add t2,t5,2             // j=min+2
for_select_min_begin_1:     // ����С������
    sub v1,t2,t9            // j>len����
    bgtz v1,for_select_min_end_1
    nop

if_is_min_begin_1:
    mul v0,t2,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[j*4+2]���ص�s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    bne s5,zero,if_is_min_end_1  // ���ڸ��ڵ�ͽ���
    nop
    
    mul v0,t2,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[j*4]���ص�s5
    add v0,s1
    lh  s5,(v0)
    mul v0,t5,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[min*4]���ص�s6
    add v0,s1
    lh  s6,(v0)
    sub v1,s5,s6
    bgez v1,if_is_min_end_1 // HF_Tree[j*4]>=HF_Tree[min*4]�ͽ���
    nop
    
    move t5,t2              // ����min=j
if_is_min_end_1:

    add t2,2                // j=j+2
    b   for_select_min_begin_1
    nop
for_select_min_end_1:

    move t3,t5              // ��һ����Сֵѡȡ����s1=min

    la  t2,2                // j=2
for_initial_min_begin_2:    // ��ʼ��min�������е�һ���޸��ڵ������
    sub v1,t2,t9            // j>len����
    bgtz v1,for_initial_min_end_2
    nop

if_no_father_begin_2:       // ����޸��ڵ�
    mul v0,t2,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[j*4+2]���ص�s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    bne s5,zero,if_no_father_end_2 // ���ڵ㲻Ϊ0�Ͳ�ִ�д�if
    nop

    beq t2,t3,if_no_father_end_2    // j=s1Ҳ��ִ�д�if
    nop

    move t5,t2              // ����min=j
    b   for_initial_min_end_2 // ������forѭ��
    nop
if_no_father_end_2:

    add t2,2                // j=j+2
    b   for_initial_min_begin_2
    nop
for_initial_min_end_2:


    add t2,t5,2             // j=min+2
for_select_min_begin_2:     // ����С������
    sub v1,t2,t9            // j>len����
    bgtz v1,for_select_min_end_2
    nop

if_is_min_begin_2:
    mul v0,t2,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[j*4+2]���ص�s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    bne s5,zero,if_is_min_end_2  // ���ڸ��ڵ�ͽ���
    nop

    mul v0,t2,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[j*4]���ص�s5
    add v0,s1
    lh  s5,(v0)
    mul v0,t5,4             // ���ýڵ�ĸ��ڵ�����HF_Tree[min*4]���ص�s6
    add v0,s1
    lh  s6,(v0)
    sub v1,s5,s6
    bgez v1,if_is_min_end_2 // HF_Tree[j*4]>=HF_Tree[min*4]�ͽ���
    nop

    beq t2,t3,if_is_min_end_2    // j=s1Ҳ����
    nop

    move t5,t2              // ����min=j
if_is_min_end_2:

    add t2,2                // j=j+2
    b   for_select_min_begin_2
    nop
for_select_min_end_2:

    move t4,t5              // s2=min

SELECT_END:

    mul v0,t3,4             // ��HF_Tree[s1*4]���ص�s5
    add v0,s1
    lh  s5,(v0)
    mul v0,t4,4             // ��HF_Tree[s2*4]���ص�s6
    add v0,s1
    lh  s6,(v0)
    add s7,s5,s6            // s7=s5+s6
    mul v0,t1,4             // ��s7���浽HF_Tree[i*4]
    add v0,s1
    sh  s7,(v0)
    
    mul v0,t3,4             // ��i���浽HF_Tree[s1*4+2]��s1�ĸ��ڵ㣩
    add v0,2
    add v0,s1
    sh  t1,(v0)
    mul v0,t4,4             // ��i���浽HF_Tree[s2*4+2]��s2�ĸ��ڵ㣩
    add v0,2
    add v0,s1
    sh  t1,(v0)
    mul v0,t1,4             // ��s1���浽HF_Tree[i*4+4]
    add v0,4
    add v0,s1
    sh  t3,(v0)
    mul v0,t1,4             // ��s2���浽HF_Tree[i*4+6]
    add v0,6
    add v0,s1
    sh  t4,(v0)
    
    add t1,2                // i=i+2
    b   for_creat_hafftree_begin
    nop
for_creat_hafftree_end:
    
CREAT_HAFFF_END:

    /* ��ӡ�������� */
    nop
    nop
    
    /* ��ʼ���������� */
HAFF_CODING_BEGIN:
    la  s5,2                // ��0��1���룬2��ʾ����
    sub v0,s4,2             // code[number-2]=2
    add v0,s3
    sh  s5,(v0)
    
    la  t1,2                // i=2
for_coding_begin:
    sub v1,t1,s4            // i>numberʱ����
    bgtz v1,for_coding_end
    nop
    
    sub t6,s4,2             // start=number-2
    move t7,t1              // c=i
    mul v0,t7,4             // p=HF_Tree[c*4+2]
    add v0,2
    add v0,s1
    lh  t8,(v0)
    
while_begin:
    beq t8,zero,while_end   // p=0����
    nop
    
if_left_child_begin:
    mul v0,t8,4             // ��HF_Tree[p*4+4]���ص�s5
    add v0,4
    add v0,s1
    lh  s5,(v0)             // HF_Tree[p*4+4]!=c��ת��else
    bne s5,t7,else_right_child_begin
    nop
    
    sub t6,2                // start=start-2
    la  s6,0                // s6=0
    add v0,s3,t6            // ��s6���浽code[start]
    sh  s6,(v0)

    b   if_child_end
    nop
else_right_child_begin:
    sub t6,2                // start=start-2
    la  s6,1                // s6=1
    add v0,s3,t6            // ��s6���浽code[start]
    sh  s6,(v0)
if_child_end:
    
    move t7,t8              // c=p
    mul v0,t7,4             // p=HF_Tree[c*4+2]
    add v0,2
    add v0,s1
    lh  t8,(v0)
    
    b   while_begin
    nop
while_end:
    
    la  t2,0                // j=0
    la  s5,2                // s5�Ĵ�����ֵΪ2
for_copy_code_begin:
    add v0,s3,t6            // ��code[start]���ص�s6
    lh  s6,(v0)
    beq s5,s6,for_copy_code_end // s6Ϊ2������ɽ���
    nop

    mul v0,t1,8             // ��s6���浽HF_code[i*8+j]
    add v0,t2
    add v0,s2
    sh  s6,(v0)

    add t6,2                // start=start+2
    add t2,2                // j=j+2
    b   for_copy_code_begin
    nop
for_copy_code_end:
    
    /* ����һ�����������־ */
    la s6,2                 // ������־2
    mul v0,t1,8             // ��s6���浽HF_code[i*8+j]
    add v0,t2
    add v0,s2
    sh  s6,(v0)
    
    add t1,2                // i=i+2
    b   for_coding_begin
    nop
for_coding_end:

HAFF_CODING_END:

    /* �������н��� */
finish:
    nop
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)


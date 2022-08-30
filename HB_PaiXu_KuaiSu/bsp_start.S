/*
 * bsp_start.S
 *
 * created: 2022/1/25
 *  author: 
 */

/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���
*   s0  ����������arr����ַ
*   s1  ��������-->begin
*   s2  ��������-->end
*   s3  ��ת�����Ĵ���
*   s4  �ݹ黺����buf����ַ
*   s5  �ݹ黺����bufƫ�Ƶ�ַ��ָ�룩
*   t0  ��׼��
*   t1  �ڱ�i
*   t2  �ڱ�j
*   t3  �ڱ�iָ���ֵ����arr[i]
*   t4  �ڱ�jָ���ֵ����arr[j]
*   t5  �����Ĵ���t���Ľ������ã�
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//���ݶ�
    .data
    arr:  .half 3,8,1,5,2,4,6,7     // ���������ݣ����飩
	len:  .half 8                   // ���������ݸ���
	buf:  .space 8                  // �ݹ黺������ջ��

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    s6, ra      /* ���ص�ַ */
    
    la  s4,buf          // ���صݵ��û���������ַ��������
    la  s5,0            // ��ʼ���ݹ����ƫ�Ƶ�ַ��������
    la  s0,arr          // ���ش��������ݻ���ַ��������
    
    la  s1,0            // ��ʼ����һ�β�������begin��ֵ
    la  s2,len          // ��ʼ����һ�β�������end��ֵ
    lw  s2,(s2)
    sub s2,1
    sll s2,s2,1         // ��Ϊ��.half�������ͣ�һ������ռ�����ֽ�����s2*2
    
    j Quick_Sort_start  // ��ʼ��һ������
    nop

loop:                   // �ݹ��������
    blez s5,finish      // �ݹ���ý������������s5<=0�����ݹ�
    nop

    sub  s5,2           // end ��ջ
    add v0,s4,s5
    lh  s2,(v0)
    sub  s5,2           // begin ��ջ
    add v0,s4,s5
    lh  s1,(v0)
    // b Quick_Sort_start // ��ʼ�ݹ�
    // nop


Quick_Sort_start:
    sub s3,s2,s1
    blez s3,Quick_Sort_end  // ����У�飺���s2<=s1������˺���������ջ��
    nop
    
    add v0,s0,s1    // ѡ����׼��
    lh  t0,(v0)
    la  t1,0        // ���������begin���ص��ڱ�i
    add t1,s1
    la  t2,0        // ���������end���ص��ڱ�j
    add t2,s2
    
while_i_j_begin:
    beq  t1,t2,while_i_j_end    // һ�ֱ����������������ڱ�����
    nop
    
while_arr_j_begin:              // �ڱ�j���ƶ����ҵ�С�ڻ�׼������ֵ
    add v0,s0,t2                // ���ڱ�jָ���ֵarr[j]���ص�t4�Ĵ���
    lh  t4,(v0)
    sub s3,t4,t0                // �ҵ���С�ڻ�׼��(t0)��ֵ(t4)����t4<t0����Ѱ��
    bltz s3,while_arr_j_end 
    nop
    
    sub s3,t2,t1                // ����У�飺����ڱ�j���ڱ�i���������ܵ��ڱ�i֮ǰ�ͽ���Ѱ�ң���t2<=t1��ת
    blez s3,while_arr_j_end  
    nop
    
    sub  t2,2                   // �ڱ�j��ǰ�ƶ�
    b    while_arr_j_begin      // ����Ѱ��
    nop
while_arr_j_end:
    
while_arr_i_begin:              // �ڱ�i���ƶ����ҵ����ڻ�׼������ֵ
    add v0,s0,t1                // ���ڱ�iָ���ֵarr[i]���ص�t3�Ĵ���
    lh  t3,(v0)
    sub s3,t3,t0                // �ҵ��˴��ڻ�׼��(t0)��ֵ(t3)����t3>t0����Ѱ��
    bgtz s3,while_arr_i_end     
    nop
    
    sub s3,t2,t1                // ����У�飺����ڱ�i���ڱ�j���������ܵ��ڱ�j֮��ͽ���Ѱ�ң���t1>=t2��ת
    blez s3,while_arr_i_end
    nop
    
    add  t1,2                   // �ڱ�i����ƶ�
    b    while_arr_i_begin      // ����Ѱ��
    nop
while_arr_i_end:

if_i_j_begin:                   // ��������ڱ�û���������ͽ��������ڱ���ָ���ֵ
    sub s3,t2,t1                // ������ڱ��������λ����ִ���ⲿ�֣���t2<=t1��ת
    blez s3,if_i_j_end          
    nop

/*
    add v0,s0,t1    // ��arr[i]��ֵ��t3
    lh  t3,(v0)
    la  t5,0        // ��t3��ֵ��t5
    add t5,t3
    
    add v0,s0,t2    // ��arr[j]��ֵ��t4
    lh  t4,(v0)
    la  t3,0
    add t3,t4       // ��t4��ֵ��t3
    add v0,s0,t1    // ��t3�浽arr[i]
    sh  t3,(v0)

    la  t4,0        // ��t5��ֵ��t4
    add t4,t5
    add v0,s0,t2    // ��t4�浽arr[j]
    sh  t4,(v0)
*/
    add v0,s0,t1    // ���ڱ�jָ���ֵ(t4)�洢��arr[i]
    sh  t4,(v0)
    add v0,s0,t2    // ���ڱ�iָ���ֵ(t3)�洢��arr[j]
    sh  t3,(v0)

if_i_j_end:
    
    b  while_i_j_begin          // ��ʼ��һ�ֱ���
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
    
    add v0,s0,s1    // ���ڱ�iָ���ֵ(t3)�浽arr[begin]
    sh  t3,(v0)
    add v0,s0,t1    // ����׼��(t0)�洢��arr[i]
    sh  t0,(v0)

    
//********************************** ��ջ
    add v0,s4,s5    // ��ջbegin
    sh  s1,(v0)
    add s5,2
    sub t1,2        // ��ջi-2
    add v0,s4,s5
    sh  t1,(v0)
    add s5,2
    add t2,2        // ��ջj+2
    add v0,s4,s5
    sh  t2,(v0)
    add s5,2
    add v0,s4,s5    // ��ջend
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
    
    
    
    
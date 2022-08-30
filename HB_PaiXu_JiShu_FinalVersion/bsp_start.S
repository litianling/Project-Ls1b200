/*
 * bsp_start.S
 *
 * created: 2022/1/28
 *  author: Li TianLing
 */

/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���
*   v1  ��ת�����Ĵ���
*   s0  ����len��Ҳ��length��           ��������
*   s1  ����������arr����ַ             ��������
*   s2  Ͱ��Ԫ�ظ�����¼����record����ַ��������
*   s3  Ͱbarrel����ַ                  ��������
*   s4  ����max_scale                   ��������
*   s5  ����mid_scale
*   s6  ����min_scale
*   t1  �ڱ�i
*   t2  �ڱ�j
*   t3  �ڱ�k
*   t4  �������������arr[i]
*   t5  ĳͰ��Ԫ�ؼ�¼record[j]
*   t6  Ͱbarrel[j*len+k]
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//���ݶ�
    .data
    arr:    .half 33,58,999,0,233,54,133,7  // ���������ݣ����飩
    record: .space 20                       // ����ʮ�����֣�����20���ֽ�
    barrel: .space 160                      // ����80�����֣�����160���ֽ�
	len:    .half 8                         // ���������ݳ���

    max_scale: .half 1000                   // max_scale

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* ���ص�ַ */

    la  v0,len          // ������len���ص�s0��������
    lh  s0,(v0)
    sll s0,1
    la  s1,arr          // ��arr����ַ���ص�s1��������
    la  s2,record       // ��record����ַ���ص�s2��������
    la  s3,barrel       // ��barrel����ַ���ص�s3��������
    la  v0,max_scale    // ��max_scale���ص�s4��������
    lh  s4,(v0)

    la  s6,1            // ��min_scale��ֵΪ1
while_begin:
    sub v1,s6,s4        // min_scale>=max_scale�ͽ���ѭ��
    bgez v1,while_end
    nop

    mul s5,s6,10        // mid_scale��ʼ��Ϊmin_scale*10
    la  t1,0            // �ڱ�i��ʼ��Ϊ0
for_pre_arr_begin:
    sub v1,t1,s0
    bgez v1,for_pre_arr_end
    nop
    
    add v0,s1,t1        // ��arr[i]��ֵ���ص�t4��
    lh  t4,(v0)
    div t4,s5           // ��arr[i]/mid_scale��������ֵ���ڱ�j
    mfhi t2
    div t2,s6           // ���ڱ�j/min_scale���̸�ֵ���ڱ�j
    mflo t2
    mul t2,2            // �ڱ�j��������ע��j���������ǵ�ַҪ����
    
    add v0,s2,t2        // ��record[j]��ֵ���ص�t5��
    lh  t5,(v0)
    move t3,t5          // ��record[j]��ֵ���ڱ�k����ע��recor[j]��������ż����ַ�����÷���
    add v0,s1,t1        // ��arr[i]��ֵ���ص�t4��
    lh  t4,(v0)
    move t6,t4          // ��arr[i]��ֵ��t6
    mul v0,t2,s0        // ��t6��ֵ���浽barrel[j*len+k]
    srl v0,1            // ��ע��j��len�˻����ı�������Ҫ����һλ���Զ�
    add v0,s3
    add v0,t3
    sh  t6,(v0)
    
    add t5,2            // record[j]+2
    add v0,s2,t2        // ����
    sh  t5,(v0)

    add t1,2            // �ڱ�i+2
    b   for_pre_arr_begin
    nop
for_pre_arr_end:
    
    la  t2,0            // �ڱ�j��λ
    la  t1,0            // �ڱ�i��λ
    
for_after_arr_begin:
    sub v1,t1,s0
    bgez v1,for_after_arr_end
    nop
    
for_record_begin:
    add v0,s2,t2        // ��record[j]���ص�t5
    lh  t5,(v0)
    bne t5,zero,for_record_end  // �ǿվͽ���
    nop
    
    add t2,2            // �ڱ�j+2
    b   for_record_begin
    nop
for_record_end:
    
    la  t3,0            // �ڱ�k��ʼ��Ϊ0
for_barrel_begin:
    // record[j]�������¼���

    sub v1,t3,t5        // �ڱ�k>=record[j]����
    bgez v1,for_barrel_end
    nop
    
    mul v0,t2,s0        // barrel[j*len+k]���ص�t6
    srl v0,1            // ��ע��j��len�˻����ı�������Ҫ����һλ���Զ�   *************************srl
    add v0,s3
    add v0,t3
    lh  t6,(v0)
    move t4,t6          // ��t6��ֵ��t4
    add v0,s1,t1        // ��t4���浽arr[i]
    sh  t4,(v0)
    
    add t1,2            // �ڱ�i+2
    add t3,2            // �ڱ�k+2
    b   for_barrel_begin
    nop
for_barrel_end:
    
    la  t5,0            // ��record[j]���
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


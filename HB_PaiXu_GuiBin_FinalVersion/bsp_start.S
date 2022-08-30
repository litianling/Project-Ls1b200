/*
 * bsp_start.S
 *
 * created: 2022/1/29
 *  author: Li TianLing
 */

/**************************************
*
*   v0  �洢��Ѱַ�����Ĵ���
*   v1  ��ת�����Ĵ���
*   s0  ����len��Ҳ��length��           ��������
*   s1  ����������arr����ַ             ��������
*   s2  ���껺������lebal����ַ         ��������
*   s3  ��ʱ����temparr����ַ           ��������
*   s4  �ڱ�left
*   s5  �ڱ�right
*   s6  �ڱ�mid   (С�ļ���)(left+right)/4*2
*   s7  �ڱ�leb_start
*   s8  �ڱ�leb_end
*   t1  �ڱ�l_pose
*   t2  �ڱ�r_pose
*   t3  �ڱ�pose
*   t4  arr[l_pose]
*   t5  arr[r_pose]
*   t6  temparr[left]
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//���ݶ�
    .data
    arr:    .half 9,5,2,7,12,4,3,1,11       // ���������ݣ����飩
    lebal:  .space 36                       // ����18�����֣�����36���ֽ�
    temparr:.space 18                       // ����9�����֣�����18���ֽ�
	len:    .half 9                         // ���������ݳ���

//-----------------------------------------------------------------------------
//�����
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* ���淵�ص�ַ */

    /* ��ʼ��ʼ�� */
    la  v0,len          // ������2*len���ص�s0��������
    lh  s0,(v0)
    sll s0,1
    la  s1,arr          // ��arr����ַ���ص�s1��������
    la  s2,lebal        // ��lebal����ַ���ص�s2��������
    la  s3,temparr      // ��temparr����ַ���ص�s3��������

    sub t4,s0,2         // ��2*len-2���浽lebal[2]
    add v0,s2,2
    sh  t4,(v0)
    
    la  s7,0            // lebal_start��ʼ��Ϊ0
    la  s8,4            // lebal_end��ʼ��Ϊ4

    /* ��ʼ���� */
for_split_begin:
    add v0,s2,s7        // ��lebal[leb_start]���ص�left
    lh  s4,(v0)
    add s7,2            // leb_start����
    add v0,s2,s7        // ��lebal[leb_start]���ص�right
    lh  s5,(v0)
    sub s7,2            // leb_start��λ
    
    sub v1,s5,s4        // ���right<=left+2�ͽ���
    sub v1,2
    blez v1,for_split_end
    nop
    
    add s7,4            // ����start���ƣ�left��right�Ѽ��أ������ظ�����
    
Merge_sort_1_begin:
    add s6,s4,s5        // ���м�� mid=(left+right)/4*2
    div s6,4
    mflo s6
    mul s6,2

if_left_mid_begin:
    beq s4,s6,if_left_mid_end   // ���left==mid����if
    nop

    add v0,s2,s8        // ��left���浽lebal[leb_end]
    sh  s4,(v0)
    add s8,2            // leb_end����
    
    add v0,s2,s8        // ��mid���浽lebal[leb_end]
    sh  s6,(v0)
    add s8,2            // leb_end����
if_left_mid_end:

if_mid_right_beging:
    add v1,s6,2
    beq v1,s5,if_mid_right_end  // ���mid+2==right����if
    nop
    
    add v0,s2,s8        // ��mid+2���浽lebal[leb_end]
    sh  v1,(v0)
    add s8,2            // leb_end����

    add v0,s2,s8        // ��right���浽lebal[leb_end]
    sh  s5,(v0)
    add s8,2            // leb_end����
if_mid_right_end:

Merge_sort_1_end:
    
    b   for_split_begin
    nop
for_split_end:


    nop
    nop
    
    /* ��ʼ�ϲ� */
for_merge_begin:
    blez s8,for_merge_end   // leb_end<=0�ϲ�����
    nop
    
    sub s8,2        // leb_endǰ��
    add v0,s2,s8    // ��lebal[leb_end]���ص�right
    lh  s5,(v0)

    sub s8,2        // leb_endǰ��
    add v0,s2,s8    // ��lebal[leb_end]���ص�right
    lh  s4,(v0)
    
    
Merge_sort_2_begin:
    add s6,s4,s5        // mid=(left+right)/4*2
    div s6,4
    mflo s6
    mul s6,2
    
    move t1,s4          // ����������һ��δ����Ԫ�� l_pose=left
    add t2,s6,2         // ����Ұ�����һ��δ����Ԫ�� r_pose=mid+2
    move t3,s4          // ��ʱ����Ԫ�ص��±� pose = left
    
while_1_begin:          /* �ϲ� ���Ұ�������ʣ��Ԫ�� */
    sub v1,t1,s6        // l_pose>mid����
    bgtz v1,while_1_end
    nop
    sub v1,t2,s5        // r_pose>right����
    bgtz v1,while_1_end
    nop
    
if_pose_begin:
    add v0,s1,t1        // ��arr[l_pose]���ص�t4
    lh  t4,(v0)
    add v0,s1,t2        // ��arr[r_pose]���ص�t5
    lh  t5,(v0)
    sub v1,t4,t5        // ���arr[l_pose]>=arr[r_pose],��ת��else
    bgez v1,else_if_pose
    nop
    
    add v0,s3,t3        // ��t4(�Ѽ��ع���arr[l_pose])���浽temparr[pose]
    sh  t4,(v0)
    add t1,2            // l_pose����
    add t3,2            // pose����
    
    b   end_if_pose
    nop
else_if_pose:

    add v0,s3,t3        // ��t5(�Ѽ��ع���arr[r_pose])���浽temparr[pose]
    sh  t5,(v0)
    add t2,2            // r_pose����
    add t3,2            // pose����

end_if_pose:
    
    b   while_1_begin
    nop
while_1_end:

while_2_begin:          /* �ϲ� �������ʣ���Ұ���Ϊ�� */
    sub v1,t1,s6        // l_pose>mid����
    bgtz v1,while_2_end
    nop

    add v0,s1,t1        // ��arr[l_pose]���ص�t4
    lh  t4,(v0)
    add v0,s3,t3        // ��t4���浽temparr[pose]
    sh  t4,(v0)
    add t1,2            // l_pose����
    add t3,2            // pose����

    b   while_2_begin
    nop
while_2_end:
    
while_3_begin:          /* �ϲ� �Ұ�����ʣ�������Ϊ�� */
    sub v1,t2,s5        // r_pose>right����
    bgtz v1,while_3_end
    nop

    add v0,s1,t2        // ��arr[r_pose]���ص�t5
    lh  t5,(v0)
    add v0,s3,t3        // ��t5���浽temparr[pose]
    sh  t5,(v0)
    add t2,2            // r_pose����
    add t3,2            // pose����

    b   while_3_begin
    nop
while_3_end:

while_4_begin:          /* ����ʱ�����кϲ���Ԫ�ظ��ƻ�ԭ������ */
    sub v1,s4,s5        // left>right����
    bgtz v1,while_4_end
    nop
    
    add v0,s3,s4        // ��temp[left]���ص�t6
    lh  t6,(v0)
    add v0,s1,s4        // ��t6���浽arr[left]
    sh  t6,(v0)
    add s4,2            // left�����

    b   while_4_begin
    nop
while_4_end:

Merge_sort_2_end:
    
    b   for_merge_begin
    nop
for_merge_end:


finish:
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)

/*
 * bsp_start.S
 *
 * created: 2022/1/29
 *  author: Li TianLing
 */

/**************************************
*
*   v0  存储器寻址辅助寄存器
*   v1  跳转辅助寄存器
*   s0  参数len（也是length）           （常量）
*   s1  待排序数据arr基地址             （常量）
*   s2  坐标缓存数组lebal基地址         （常量）
*   s3  临时数组temparr基地址           （常量）
*   s4  哨兵left
*   s5  哨兵right
*   s6  哨兵mid   (小心计算)(left+right)/4*2
*   s7  哨兵leb_start
*   s8  哨兵leb_end
*   t1  哨兵l_pose
*   t2  哨兵r_pose
*   t3  哨兵pose
*   t4  arr[l_pose]
*   t5  arr[r_pose]
*   t6  temparr[left]
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//数据段
    .data
    arr:    .half 9,5,2,7,12,4,3,1,11       // 待排序数据（数组）
    lebal:  .space 36                       // 分配18个半字，就是36个字节
    temparr:.space 18                       // 分配9个半字，就是18个字节
	len:    .half 9                         // 待排序数据长度

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* 保存返回地址 */

    /* 开始初始化 */
    la  v0,len          // 将长度2*len加载到s0（常量）
    lh  s0,(v0)
    sll s0,1
    la  s1,arr          // 将arr基地址加载到s1（常量）
    la  s2,lebal        // 将lebal基地址加载到s2（常量）
    la  s3,temparr      // 将temparr基地址加载到s3（常量）

    sub t4,s0,2         // 将2*len-2保存到lebal[2]
    add v0,s2,2
    sh  t4,(v0)
    
    la  s7,0            // lebal_start初始化为0
    la  s8,4            // lebal_end初始化为4

    /* 开始划分 */
for_split_begin:
    add v0,s2,s7        // 将lebal[leb_start]加载到left
    lh  s4,(v0)
    add s7,2            // leb_start后移
    add v0,s2,s7        // 将lebal[leb_start]加载到right
    lh  s5,(v0)
    sub s7,2            // leb_start归位
    
    sub v1,s5,s4        // 如果right<=left+2就结束
    sub v1,2
    blez v1,for_split_end
    nop
    
    add s7,4            // 否则start后移，left与right已加载，无需重复加载
    
Merge_sort_1_begin:
    add s6,s4,s5        // 找中间点 mid=(left+right)/4*2
    div s6,4
    mflo s6
    mul s6,2

if_left_mid_begin:
    beq s4,s6,if_left_mid_end   // 如果left==mid结束if
    nop

    add v0,s2,s8        // 将left保存到lebal[leb_end]
    sh  s4,(v0)
    add s8,2            // leb_end后移
    
    add v0,s2,s8        // 将mid保存到lebal[leb_end]
    sh  s6,(v0)
    add s8,2            // leb_end后移
if_left_mid_end:

if_mid_right_beging:
    add v1,s6,2
    beq v1,s5,if_mid_right_end  // 如果mid+2==right结束if
    nop
    
    add v0,s2,s8        // 将mid+2保存到lebal[leb_end]
    sh  v1,(v0)
    add s8,2            // leb_end后移

    add v0,s2,s8        // 将right保存到lebal[leb_end]
    sh  s5,(v0)
    add s8,2            // leb_end后移
if_mid_right_end:

Merge_sort_1_end:
    
    b   for_split_begin
    nop
for_split_end:


    nop
    nop
    
    /* 开始合并 */
for_merge_begin:
    blez s8,for_merge_end   // leb_end<=0合并结束
    nop
    
    sub s8,2        // leb_end前移
    add v0,s2,s8    // 将lebal[leb_end]加载到right
    lh  s5,(v0)

    sub s8,2        // leb_end前移
    add v0,s2,s8    // 将lebal[leb_end]加载到right
    lh  s4,(v0)
    
    
Merge_sort_2_begin:
    add s6,s4,s5        // mid=(left+right)/4*2
    div s6,4
    mflo s6
    mul s6,2
    
    move t1,s4          // 标记左半区第一个未排序元素 l_pose=left
    add t2,s6,2         // 标记右半区第一个未排序元素 r_pose=mid+2
    move t3,s4          // 临时数组元素的下标 pose = left
    
while_1_begin:          /* 合并 左右半区都有剩余元素 */
    sub v1,t1,s6        // l_pose>mid结束
    bgtz v1,while_1_end
    nop
    sub v1,t2,s5        // r_pose>right结束
    bgtz v1,while_1_end
    nop
    
if_pose_begin:
    add v0,s1,t1        // 将arr[l_pose]加载到t4
    lh  t4,(v0)
    add v0,s1,t2        // 将arr[r_pose]加载到t5
    lh  t5,(v0)
    sub v1,t4,t5        // 如果arr[l_pose]>=arr[r_pose],跳转到else
    bgez v1,else_if_pose
    nop
    
    add v0,s3,t3        // 将t4(已加载过的arr[l_pose])保存到temparr[pose]
    sh  t4,(v0)
    add t1,2            // l_pose后移
    add t3,2            // pose后移
    
    b   end_if_pose
    nop
else_if_pose:

    add v0,s3,t3        // 将t5(已加载过的arr[r_pose])保存到temparr[pose]
    sh  t5,(v0)
    add t2,2            // r_pose后移
    add t3,2            // pose后移

end_if_pose:
    
    b   while_1_begin
    nop
while_1_end:

while_2_begin:          /* 合并 左半区有剩余右半区为空 */
    sub v1,t1,s6        // l_pose>mid结束
    bgtz v1,while_2_end
    nop

    add v0,s1,t1        // 将arr[l_pose]加载到t4
    lh  t4,(v0)
    add v0,s3,t3        // 将t4保存到temparr[pose]
    sh  t4,(v0)
    add t1,2            // l_pose后移
    add t3,2            // pose后移

    b   while_2_begin
    nop
while_2_end:
    
while_3_begin:          /* 合并 右半区有剩余左半区为空 */
    sub v1,t2,s5        // r_pose>right结束
    bgtz v1,while_3_end
    nop

    add v0,s1,t2        // 将arr[r_pose]加载到t5
    lh  t5,(v0)
    add v0,s3,t3        // 将t5保存到temparr[pose]
    sh  t5,(v0)
    add t2,2            // r_pose后移
    add t3,2            // pose后移

    b   while_3_begin
    nop
while_3_end:

while_4_begin:          /* 把临时数组中合并后元素复制回原来数组 */
    sub v1,s4,s5        // left>right结束
    bgtz v1,while_4_end
    nop
    
    add v0,s3,s4        // 将temp[left]加载到t6
    lh  t6,(v0)
    add v0,s1,s4        // 将t6保存到arr[left]
    sh  t6,(v0)
    add s4,2            // left向后移

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


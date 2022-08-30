/*
 * bsp_start.S
 *
 * created: 2022/2/10
 *  author: Li TianLing
 */


/**************************************
*
*   v0  存储器寻址辅助寄存器
*   v1  跳转辅助寄存器
*
*   s0  待编码数据basic_date基地址  （常量）
*   s1  哈夫曼树HF_Tree基地址       （常量）
*   s2  哈夫曼编码HF_code基地址     （常量）
*   s3  编码缓存空间code基地址      （常量）
*   s4  待编码数据个数              （常量）
*
*   t0  哈夫曼树总节点数m     （half类型*2）
*   t1  循环哨兵i
*   t2  循环哨兵j
*   t3  最小值点坐标s1
*   t4  最小值点坐标s2
*   t5  最小值辅助筛选min
*   t6  参数start
*   t7  参数c
*   t8  参数p
*   t9  参数len
*
**************************************/

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

//-----------------------------------------------------------------------------
//数据段
    .data
    basic_date: .half  1,2,3,4,5,6,7,8  // 待编码数据（数组）
    HF_Tree:    .space 128              // 哈夫曼树
    HF_code:    .space 144              // 哈夫曼编码
    code:       .space 16               // 哈夫曼编码缓存空间
	number:     .half  8                // 待排序数据长度

//-----------------------------------------------------------------------------
//代码段
    .text
FRAME(bsp_start,sp,0,ra)
	.set noreorder
    move    k0, ra      /* 返回地址 */

    la  s0,basic_date   // 将basic_date基地址加载到s0（常量）
    la  s1,HF_Tree      // 将HF_Tree基地址加载到s0（常量）
    la  s2,HF_code      // 将HF_code基地址加载到s0（常量）
    la  s3,code         // 将code基地址加载到s0（常量）
    la  v0,number       // 将个数numer加载到s4（常量）
    lh  s4,(v0)
    sll s4,1

    /* 开始构建哈夫曼树 */
CREAT_HAFFF_BEGIN:
    mul t0,s4,2         // m=number*2-2
    sub t0,2

    la  t1,2            // i=2
for_leaf_copy_begin:    // 将权值赋给n个叶子节点
    sub v1,t1,s4        // i>number结束
    bgtz v1,for_leaf_copy_end
    nop
    
    sub v0,t1,2         // 将basic_date[i-2]加载到s5
    add v0,s0
    lh  s5,(v0)
    mul v0,t1,4         // 将s5保存到HF_Tree[i*4]
    add v0,s1
    sh  s5,(v0)
    
    add t1,2
    b   for_leaf_copy_begin
    nop
for_leaf_copy_end:
    
    add t1,s4,2             // i=number+2
for_creat_hafftree_begin:   // 构建哈夫曼树
    sub v1,t1,t0            // i>m结束
    bgtz v1,for_creat_hafftree_end
    nop
    
    /* 开始挑选最小值 */
SELECT_BEGIN:
    sub t9,t1,2             // len=i-2
    la  t2,2                // j=2
for_initial_min_begin_1:    // 初始化min，数组中第一个无父节点的坐标
    sub v1,t2,t9            // j>len结束
    bgtz v1,for_initial_min_end_1
    nop

if_no_father_begin_1:       // 如果无父节点
    mul v0,t2,4             // 将该节点的父节点坐标HF_Tree[j*4+2]加载到s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    
    bne s5,zero,if_no_father_end_1 // 父节点不为0就不执行此if
    nop
    
    move t5,t2              // 否则min=j
    b   for_initial_min_end_1 // 并跳出for循环
    nop
if_no_father_end_1:

    add t2,2                // j=j+2
    b   for_initial_min_begin_1
    nop
for_initial_min_end_1:

    add t2,t5,2             // j=min+2
for_select_min_begin_1:     // 找最小点坐标
    sub v1,t2,t9            // j>len结束
    bgtz v1,for_select_min_end_1
    nop

if_is_min_begin_1:
    mul v0,t2,4             // 将该节点的父节点坐标HF_Tree[j*4+2]加载到s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    bne s5,zero,if_is_min_end_1  // 存在父节点就结束
    nop
    
    mul v0,t2,4             // 将该节点的父节点坐标HF_Tree[j*4]加载到s5
    add v0,s1
    lh  s5,(v0)
    mul v0,t5,4             // 将该节点的父节点坐标HF_Tree[min*4]加载到s6
    add v0,s1
    lh  s6,(v0)
    sub v1,s5,s6
    bgez v1,if_is_min_end_1 // HF_Tree[j*4]>=HF_Tree[min*4]就结束
    nop
    
    move t5,t2              // 否则min=j
if_is_min_end_1:

    add t2,2                // j=j+2
    b   for_select_min_begin_1
    nop
for_select_min_end_1:

    move t3,t5              // 第一个最小值选取结束s1=min

    la  t2,2                // j=2
for_initial_min_begin_2:    // 初始化min，数组中第一个无父节点的坐标
    sub v1,t2,t9            // j>len结束
    bgtz v1,for_initial_min_end_2
    nop

if_no_father_begin_2:       // 如果无父节点
    mul v0,t2,4             // 将该节点的父节点坐标HF_Tree[j*4+2]加载到s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    bne s5,zero,if_no_father_end_2 // 父节点不为0就不执行此if
    nop

    beq t2,t3,if_no_father_end_2    // j=s1也不执行此if
    nop

    move t5,t2              // 否则min=j
    b   for_initial_min_end_2 // 并跳出for循环
    nop
if_no_father_end_2:

    add t2,2                // j=j+2
    b   for_initial_min_begin_2
    nop
for_initial_min_end_2:


    add t2,t5,2             // j=min+2
for_select_min_begin_2:     // 找最小点坐标
    sub v1,t2,t9            // j>len结束
    bgtz v1,for_select_min_end_2
    nop

if_is_min_begin_2:
    mul v0,t2,4             // 将该节点的父节点坐标HF_Tree[j*4+2]加载到s5
    add v0,2
    add v0,s1
    lh  s5,(v0)
    bne s5,zero,if_is_min_end_2  // 存在父节点就结束
    nop

    mul v0,t2,4             // 将该节点的父节点坐标HF_Tree[j*4]加载到s5
    add v0,s1
    lh  s5,(v0)
    mul v0,t5,4             // 将该节点的父节点坐标HF_Tree[min*4]加载到s6
    add v0,s1
    lh  s6,(v0)
    sub v1,s5,s6
    bgez v1,if_is_min_end_2 // HF_Tree[j*4]>=HF_Tree[min*4]就结束
    nop

    beq t2,t3,if_is_min_end_2    // j=s1也结束
    nop

    move t5,t2              // 否则min=j
if_is_min_end_2:

    add t2,2                // j=j+2
    b   for_select_min_begin_2
    nop
for_select_min_end_2:

    move t4,t5              // s2=min

SELECT_END:

    mul v0,t3,4             // 将HF_Tree[s1*4]加载到s5
    add v0,s1
    lh  s5,(v0)
    mul v0,t4,4             // 将HF_Tree[s2*4]加载到s6
    add v0,s1
    lh  s6,(v0)
    add s7,s5,s6            // s7=s5+s6
    mul v0,t1,4             // 将s7保存到HF_Tree[i*4]
    add v0,s1
    sh  s7,(v0)
    
    mul v0,t3,4             // 将i保存到HF_Tree[s1*4+2]（s1的父节点）
    add v0,2
    add v0,s1
    sh  t1,(v0)
    mul v0,t4,4             // 将i保存到HF_Tree[s2*4+2]（s2的父节点）
    add v0,2
    add v0,s1
    sh  t1,(v0)
    mul v0,t1,4             // 将s1保存到HF_Tree[i*4+4]
    add v0,4
    add v0,s1
    sh  t3,(v0)
    mul v0,t1,4             // 将s2保存到HF_Tree[i*4+6]
    add v0,6
    add v0,s1
    sh  t4,(v0)
    
    add t1,2                // i=i+2
    b   for_creat_hafftree_begin
    nop
for_creat_hafftree_end:
    
CREAT_HAFFF_END:

    /* 打印哈夫曼树 */
    nop
    nop
    
    /* 开始哈夫曼编码 */
HAFF_CODING_BEGIN:
    la  s5,2                // 用0和1编码，2表示结束
    sub v0,s4,2             // code[number-2]=2
    add v0,s3
    sh  s5,(v0)
    
    la  t1,2                // i=2
for_coding_begin:
    sub v1,t1,s4            // i>number时结束
    bgtz v1,for_coding_end
    nop
    
    sub t6,s4,2             // start=number-2
    move t7,t1              // c=i
    mul v0,t7,4             // p=HF_Tree[c*4+2]
    add v0,2
    add v0,s1
    lh  t8,(v0)
    
while_begin:
    beq t8,zero,while_end   // p=0结束
    nop
    
if_left_child_begin:
    mul v0,t8,4             // 将HF_Tree[p*4+4]加载到s5
    add v0,4
    add v0,s1
    lh  s5,(v0)             // HF_Tree[p*4+4]!=c跳转到else
    bne s5,t7,else_right_child_begin
    nop
    
    sub t6,2                // start=start-2
    la  s6,0                // s6=0
    add v0,s3,t6            // 将s6保存到code[start]
    sh  s6,(v0)

    b   if_child_end
    nop
else_right_child_begin:
    sub t6,2                // start=start-2
    la  s6,1                // s6=1
    add v0,s3,t6            // 将s6保存到code[start]
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
    la  s5,2                // s5寄存器赋值为2
for_copy_code_begin:
    add v0,s3,t6            // 将code[start]加载到s6
    lh  s6,(v0)
    beq s5,s6,for_copy_code_end // s6为2复制完成结束
    nop

    mul v0,t1,8             // 将s6保存到HF_code[i*8+j]
    add v0,t2
    add v0,s2
    sh  s6,(v0)

    add t6,2                // start=start+2
    add t2,2                // j=j+2
    b   for_copy_code_begin
    nop
for_copy_code_end:
    
    /* 补上一个编码结束标志 */
    la s6,2                 // 结束标志2
    mul v0,t1,8             // 将s6保存到HF_code[i*8+j]
    add v0,t2
    add v0,s2
    sh  s6,(v0)
    
    add t1,2                // i=i+2
    b   for_coding_begin
    nop
for_coding_end:

HAFF_CODING_END:

    /* 程序运行结束 */
finish:
    nop
    nop

    move    ra, k0
    j       ra
	nop
    .set reorder
ENDFRAME(bsp_start)



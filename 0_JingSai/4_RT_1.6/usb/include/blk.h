/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef BLK_H
#define BLK_H

#include "part.h"

//-------------------------------------------------------------------------------------------------
// #include "efi.h"
//-------------------------------------------------------------------------------------------------

typedef struct
{
    unsigned char b[16];
} efi_guid_t __attribute__((aligned(8)));

#ifdef CONFIG_SYS_64BIT_LBA
typedef uint64_t    lbaint_t;
#define LBAFlength  "ll"
#else
typedef unsigned long lbaint_t;    //\\ 一些定义和 part.h 有冲突
#define LBAFlength  "l"
#endif

#define LBAF "%"  LBAFlength "x"
#define LBAFU "%" LBAFlength "u"

#define BLK_VEN_SIZE        40
#define BLK_PRD_SIZE        20
#define BLK_REV_SIZE        8

//-------------------------------------------------------------------------------------------------
// Identifies the partition table type (ie. MBR vs GPT GUID) signature
//-------------------------------------------------------------------------------------------------

enum sig_type 
{
    SIG_TYPE_NONE,
    SIG_TYPE_MBR,
    SIG_TYPE_GUID,
    SIG_TYPE_COUNT          /* Number of signature types */
};

/*
 * 对于驱动程序模型 (CONFIG_BLK)，这是 uclass 平台数据，可通过 dev_get_uclass_platdata(dev) 访问
 */
struct blk_desc
{
    /*
     * TODO：使用驱动模型，我们应该能够使用父设备的 uclass 代替。
     */
    unsigned        if_type;        /* 接口类型 */
    int             devnum;         /* 设备号 */
    unsigned char   part_type;      /* 分区类型 */
    unsigned char   target;         /* 目标 SCSI ID */ //小型计算机接口ID
    unsigned char   lun;            /* 目标逻辑单元号 */
    unsigned char   hwpart;         /* 硬件分区，例如 用于 eMMC */
    unsigned char   type;           /* 设备类型 */
    unsigned char   removable;      /* 可移除设备 */
#ifdef CONFIG_LBA48
    /* 设备可以用48位地址 (ATA/ATAPI v7) */
    unsigned char   lba48;
#endif
    lbaint_t        lba;            /* 块存储器数量 */
    unsigned long   blksz;          /* 块存储器大小 */
    int             log2blksz;      /* 方便计算的块存储器大小: log2(blksz) */
    char            vendor[BLK_VEN_SIZE + 1];   /* 设备供应商字符串 */
    char            product[BLK_PRD_SIZE + 1];  /* 设备产品编号 */
    char            revision[BLK_REV_SIZE + 1]; /* 固件版本 */
    enum sig_type   sig_type;                   /* 分区表签名类型 */
    union
    {
        //uint32_t    mbr_sig;        /* MBR 整数签名 */
        char        mbr_sig;
        efi_guid_t  guid_sig;       /* GPT GUID 签名 */
    };
#if (CONFIG_BLK_ENABLED)
    /*
     * 现在我们有一些以 struct blk_desc 作为参数的函数。
     * 该字段允许他们查找关联的设备。 删除这些功能后，我们可以删除此字段。
     */
    struct udevice  *bdev;
#else
    unsigned long  (*block_read)(struct blk_desc *block_dev,
                                 lbaint_t start,
                                 lbaint_t blkcnt,
                                 void *buffer);
    unsigned long  (*block_write)(struct blk_desc *block_dev,
                                  lbaint_t start,
                                  lbaint_t blkcnt,
                                  const void *buffer);
    unsigned long  (*block_erase)(struct blk_desc *block_dev,
                                  lbaint_t start,
                                  lbaint_t blkcnt);
    void            *priv;      /* 驱动程序私有结构指针 */
#endif
} blk_desc_t;

#define BLOCK_CNT(size, blk_desc)        (PAD_COUNT(size, blk_desc->blksz))
#define PAD_TO_BLOCKSIZE(size, blk_desc) (PAD_SIZE(size, blk_desc->blksz))

//-------------------------------------------------------------------------------------------------
// Identifies the partition table type (ie. MBR vs GPT GUID) signature
//-------------------------------------------------------------------------------------------------

#include <errno.h>
/*
 * 这些函数应该采用 struct udevice 而不是 struct blk_desc，
  * 但这便于迁移到驱动程序模型。 添加“d”前缀
  * 到函数操作，这样可以保留 blk_read() 等
  * 具有正确参数的函数。
 */
static inline unsigned long blk_dread(struct blk_desc *block_dev,
                                      lbaint_t start,
			                          lbaint_t blkcnt,
                                      void *buffer)
{
	return block_dev->block_read(block_dev, start, blkcnt, buffer);;  // 读块存储器【*】
}

static inline unsigned long blk_dwrite(struct blk_desc *block_dev,
                                       lbaint_t start,
			                           lbaint_t blkcnt,
                                       const void *buffer)
{
	return block_dev->block_write(block_dev, start, blkcnt, buffer);
}

#endif



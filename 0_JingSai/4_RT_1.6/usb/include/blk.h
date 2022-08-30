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
typedef unsigned long lbaint_t;    //\\ һЩ����� part.h �г�ͻ
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
 * ������������ģ�� (CONFIG_BLK)������ uclass ƽ̨���ݣ���ͨ�� dev_get_uclass_platdata(dev) ����
 */
struct blk_desc
{
    /*
     * TODO��ʹ������ģ�ͣ�����Ӧ���ܹ�ʹ�ø��豸�� uclass ���档
     */
    unsigned        if_type;        /* �ӿ����� */
    int             devnum;         /* �豸�� */
    unsigned char   part_type;      /* �������� */
    unsigned char   target;         /* Ŀ�� SCSI ID */ //С�ͼ�����ӿ�ID
    unsigned char   lun;            /* Ŀ���߼���Ԫ�� */
    unsigned char   hwpart;         /* Ӳ������������ ���� eMMC */
    unsigned char   type;           /* �豸���� */
    unsigned char   removable;      /* ���Ƴ��豸 */
#ifdef CONFIG_LBA48
    /* �豸������48λ��ַ (ATA/ATAPI v7) */
    unsigned char   lba48;
#endif
    lbaint_t        lba;            /* ��洢������ */
    unsigned long   blksz;          /* ��洢����С */
    int             log2blksz;      /* �������Ŀ�洢����С: log2(blksz) */
    char            vendor[BLK_VEN_SIZE + 1];   /* �豸��Ӧ���ַ��� */
    char            product[BLK_PRD_SIZE + 1];  /* �豸��Ʒ��� */
    char            revision[BLK_REV_SIZE + 1]; /* �̼��汾 */
    enum sig_type   sig_type;                   /* ������ǩ������ */
    union
    {
        //uint32_t    mbr_sig;        /* MBR ����ǩ�� */
        char        mbr_sig;
        efi_guid_t  guid_sig;       /* GPT GUID ǩ�� */
    };
#if (CONFIG_BLK_ENABLED)
    /*
     * ����������һЩ�� struct blk_desc ��Ϊ�����ĺ�����
     * ���ֶ��������ǲ��ҹ������豸�� ɾ����Щ���ܺ����ǿ���ɾ�����ֶΡ�
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
    void            *priv;      /* ��������˽�нṹָ�� */
#endif
} blk_desc_t;

#define BLOCK_CNT(size, blk_desc)        (PAD_COUNT(size, blk_desc->blksz))
#define PAD_TO_BLOCKSIZE(size, blk_desc) (PAD_SIZE(size, blk_desc->blksz))

//-------------------------------------------------------------------------------------------------
// Identifies the partition table type (ie. MBR vs GPT GUID) signature
//-------------------------------------------------------------------------------------------------

#include <errno.h>
/*
 * ��Щ����Ӧ�ò��� struct udevice ������ struct blk_desc��
  * �������Ǩ�Ƶ���������ģ�͡� ��ӡ�d��ǰ׺
  * �������������������Ա��� blk_read() ��
  * ������ȷ�����ĺ�����
 */
static inline unsigned long blk_dread(struct blk_desc *block_dev,
                                      lbaint_t start,
			                          lbaint_t blkcnt,
                                      void *buffer)
{
	return block_dev->block_read(block_dev, start, blkcnt, buffer);;  // ����洢����*��
}

static inline unsigned long blk_dwrite(struct blk_desc *block_dev,
                                       lbaint_t start,
			                           lbaint_t blkcnt,
                                       const void *buffer)
{
	return block_dev->block_write(block_dev, start, blkcnt, buffer);
}

#endif



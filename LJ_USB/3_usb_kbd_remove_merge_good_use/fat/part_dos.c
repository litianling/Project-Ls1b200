// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Raymond Lo, lo@routefree.com
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Support for harddisk partitions.
 *
 * To be compatible with LinuxPPC and Apple we use the standard Apple
 * SCSI disk partitioning scheme. For more information see:
 * http://developer.apple.com/techpubs/mac/Devices/Devices-126.html#MARKER-14-92
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

//#include "linux_kernel.h"
#include "linux.h"
#include "part.h"
#include "blk.h"
#include "memalign.h"
#include "part_dos.h"

//-------------------------------------------------------------------------------------------------

struct partition
{
	unsigned char boot_ind;		    /* 0x80 - active */
	unsigned char head;		        /* starting head */
	unsigned char sector;		    /* starting sector */
	unsigned char cyl;		    	/* starting cylinder */
	unsigned char sys_ind;		    /* What partition type */
	unsigned char end_head;		    /* end head */
	unsigned char end_sector;	    /* end sector */
	unsigned char end_cyl;		    /* end cylinder */
	unsigned int  start_sect;	    /* starting sector counting from 0 */
	unsigned int  nr_sects;         /* nr of sectors in partition */
} __attribute__ ((packed)); // __packed;

#define MSDOS_MBR_BOOT_CODE_SIZE 	440

typedef struct _legacy_mbr
{
	unsigned char boot_code[MSDOS_MBR_BOOT_CODE_SIZE];
	unsigned int /*__le32*/ unique_mbr_signature;
	unsigned short /*__le16*/ unknown;
	struct partition partition_record[4];
	unsigned short /*__le16*/ signature;
} legacy_mbr_t __attribute__((packed)); // __packed legacy_mbr;

//-------------------------------------------------------------------------------------------------

#define DOS_PART_DEFAULT_SECTOR     512

/* should this be configurable? It looks like it's not very common at all
 * to use large numbers of partitions
 */
#define MAX_EXT_PARTS 	            256

/* 将小端格式的 char[4] 转换为主机格式整数 */
static inline unsigned int le32_to_int(unsigned char *le32)
{
    return ((le32[3] << 24) +
	        (le32[2] << 16) +
			(le32[1] << 8) +
			le32[0] );
}

static inline int is_extended(int part_type)
{
    return (part_type == 0x5 ||
	        part_type == 0xf ||
			part_type == 0x85);
}

static int get_bootable(dos_partition_t *p)
{
	int ret = 0;
/* TODO
	if (p->sys_ind == 0xef)
		ret |= PART_EFI_SYSTEM_PARTITION;
	if (p->boot_ind == 0x80)
		ret |= PART_BOOTABLE;
*/

	return ret;
}

static void print_one_part(dos_partition_t *p, lbaint_t ext_part_sector,
			               int part_num, unsigned int disksig)
{
	lbaint_t lba_start = ext_part_sector + le32_to_int (p->start4);    // 【*】
	lbaint_t lba_size  = le32_to_int (p->size4);                       // 【*】

    puts("\r\nFound DOS patition info: \r\n");
    printf("  Part number: %i\r\n", part_num);
    printf("  Start Sector: %i\r\n", lba_start);
    printf("  Num Sectors: %i\r\n", lba_size);
    printf("  UUID: %08x-%02x\r\n", disksig, part_num);
    printf("  Type: %02x%s%s\r\n\r\n", p->sys_ind,
           (is_extended(p->sys_ind) ? " Extd" : ""),
		   (get_bootable(p) ? " Boot" : ""));
}

static int test_block_type(unsigned char *buffer)
{
	int slot;
	struct dos_partition *p;
	int part_count = 0;

	if ((buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) ||
	    (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) )
	{
		return (-1);
	} /*根本没有DOS签名 */

	p = (struct dos_partition *)&buffer[DOS_PART_TBL_OFFSET];

	/* 检查引导指示灯是否有效并计算分区数. */
	for (slot = 0; slot < 4; ++slot, ++p)
	{
		if (p->boot_ind != 0 && p->boot_ind != 0x80)
			break;
		if (p->sys_ind)
			++part_count;
	}

	/*
	 * 如果分区表无效或为空，请检查这是否是 DOS PBR
	 */
	if (slot != 4 || !part_count)
	{
		if (!strncmp((char *)&buffer[DOS_PBR_FSTYPE_OFFSET], "FAT", 3) ||
		    !strncmp((char *)&buffer[DOS_PBR32_FSTYPE_OFFSET], "FAT32", 5))
			return DOS_PBR; /* This is a DOS PBR and not an MBR */
	}

	if (slot == 4)
	{
    	return DOS_MBR;	/* 这是一个 DOS MBR */
    }

	/* 这既不是 DOS MBR 也不是 DOS PBR */
	return -1;
}

static int part_test_dos(struct blk_desc *dev_desc)
{
#ifndef CONFIG_SPL_BUILD
	ALLOC_CACHE_ALIGN_BUFFER(legacy_mbr_t, mbr,
			DIV_ROUND_UP(dev_desc->blksz, sizeof(legacy_mbr_t)));

	if (blk_dread(dev_desc, 0, 1, (unsigned long *)mbr) != 1)
		return -1;

	if (test_block_type((unsigned char *)mbr) != DOS_MBR)
		return -1;

	if (dev_desc->sig_type == SIG_TYPE_NONE &&
	    mbr->unique_mbr_signature != 0)
    {
		dev_desc->sig_type = SIG_TYPE_MBR;
		dev_desc->mbr_sig = mbr->unique_mbr_signature;
	}

#else
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);

	if (blk_dread(dev_desc, 0, 1, (ulong *)buffer) != 1)
		return -1;

	if (test_block_type(buffer) != DOS_MBR)
		return -1;
		
#endif

	return 0;
}

/*  打印与其扩展分区表相关的分区 */
static void print_partition_extended(struct blk_desc *dev_desc,
				                     lbaint_t ext_part_sector,
				                     lbaint_t relative,
				                     int part_num, unsigned int disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i;

	/* 设置最大递归级别 */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\r\n");
		return;
    }

	if (blk_dread(dev_desc, ext_part_sector, 1, (unsigned long *)buffer) != 1)     // 读块存储器【*】
	{
		printf("** Can't read partition table on %d:" LBAFU " **\r\n",
			   dev_desc->devnum, ext_part_sector);
		return;
	}

	i=test_block_type(buffer);                                                     // 检查分区类型【*】
	if (i != DOS_MBR)
	{
		printf("bad MBR sector signature 0x%02x%02x\r\n",
			   buffer[DOS_PART_MAGIC_OFFSET],
			   buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return;
	}

	if (!ext_part_sector)
		disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);                   // char转int【*】

	/* 打印所有主/逻辑分区 */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++)
	{
		/* fdisk 不显示不在 MBR 中的扩展分区  */
		if ((pt->sys_ind != 0) &&
		    (ext_part_sector == 0 || !is_extended (pt->sys_ind)) )
		{
			print_one_part(pt, ext_part_sector, part_num, disksig);                // 输出提示信息【*】
		}

		/* 反向 engr fdisk part# 分配规则！ */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) )
		{
			part_num++;
		}
	}

	/* 打印扩展分区 */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++)
	{
		if (is_extended (pt->sys_ind))                                           // 是不是相关区域【*】
		{
			lbaint_t lba_start = le32_to_int (pt->start4) + relative;            // char转int【*】

			print_partition_extended(dev_desc, lba_start,                        // 递归调用----打印与其扩展分区表相关的分区【*】
									 ext_part_sector == 0  ? lba_start : relative,
									 part_num, disksig);
		}
	}

	return;
}

/*  打印与其扩展分区表相关的分区 */
static int part_get_info_extended(struct blk_desc *dev_desc,
				                  lbaint_t ext_part_sector,
                                  lbaint_t relative,
								  int part_num,
                                  int which_part,
								  struct disk_partition *info,
                                  unsigned disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);                  // 分配内存【*】
	dos_partition_t *pt;                                                               // 某分区开始结束等信息
	int i;
	int dos_type;

	/* 设置最大递归级别--检测嵌套的dos分区 */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\n");
		return -1;
    }

    // 读取分区表
	if (blk_dread(dev_desc, ext_part_sector, 1, (unsigned long *)buffer) != 1)         // 读块存储器分区表等信息【*】
	{
		printf("** Can't read partition table on %d:" LBAFU " **\n",
			   dev_desc->devnum, ext_part_sector);
		return -1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 || buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa)
	{
		printf ("bad MBR sector signature 0x%02x%02x\n",
			     buffer[DOS_PART_MAGIC_OFFSET],
				 buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return -1;
	}

#if (CONFIG_PARTITION_UUIDS)
	if (!ext_part_sector)
		disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);
#endif

	/* 打印所有主/逻辑分区 */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);                           // 强制数据类型转换（缓存区中存储分区表的偏移量）
	for (i = 0; i < 4; i++, pt++)
	{
		/*
		 * fdisk 不显示不在 MBR 中的扩展分区
		 */
		if (((pt->boot_ind & ~0x80) == 0) &&
		    (pt->sys_ind != 0) &&
		    (part_num == which_part) &&
		    (ext_part_sector == 0 || is_extended(pt->sys_ind) == 0))                   // 是不是扩展的分区【*】
		{
			info->blksz = DOS_PART_DEFAULT_SECTOR;
			info->start = (lbaint_t)(ext_part_sector + le32_to_int(pt->start4));       // 将小端格式的 char[4] 转换为主机格式整数【*】
			info->size  = (lbaint_t)le32_to_int(pt->size4);                            // 同上【*】
			part_set_generic_name(dev_desc, part_num, (char *)info->name);             // 设置通用名称【*】
			/* sprintf(info->type, "%d, pt->sys_ind); */
			strcpy((char *)info->type, "loongson");                                    // 字符串拷贝【*】
			info->bootable = get_bootable(pt);                                         // 返回0【*】无用
#if (CONFIG_PARTITION_UUIDS)
			sprintf(info->uuid, "%08x-%02x", disksig, part_num);
#endif
			// info->sys_ind = pt->sys_ind;
			return 0;
		}

		/* 反向 engr fdisk part# 分配规则! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)))                         // 同上【*】此条件为不是拓展分区
		{
			part_num++;
		}
	}

	/* 遍历扩展分区 */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);                          
	for (i = 0; i < 4; i++, pt++)
	{
		if (is_extended (pt->sys_ind))                                                // 同上【*】
		{
			lbaint_t lba_start = le32_to_int (pt->start4) + relative;                 // 同上【*】

			return part_get_info_extended(dev_desc, lba_start,                        // 递归调用----打印与其扩展分区表相关的分区【*】
										  ext_part_sector == 0 ? lba_start : relative,
										  part_num, which_part, info, disksig);
		}
	}

	/* 如果未找到分区，请检查 DOS PBR */
	dos_type = test_block_type(buffer);                                               // 测试DOS类型【*】

	if (dos_type == DOS_PBR)
	{
		info->start = 0;
		info->size = dev_desc->lba;
		info->blksz = DOS_PART_DEFAULT_SECTOR;
		info->bootable = 0;
		strcpy((char *)info->type, "loongson");                                       // 字符串拷贝【*】信息类型
#if (CONFIG_PARTITION_UUIDS)
		info->uuid[0] = 0;
#endif
		return 0;
	}

	return -1;
}

void part_print_dos(struct blk_desc *dev_desc)
{
//  printf("Part\tStart Sector\tNum Sectors\tUUID\t\tType\r\n");
	print_partition_extended(dev_desc, 0, 0, 1, 0);                        // 输出诸多类型的信息【*】
}

int part_get_info_dos(struct blk_desc *dev_desc, int part,
		              struct disk_partition *info)
{
    int ret = part_get_info_extended(dev_desc, 0, 0, 1, part, info, 0);     // 获取诸多类型的信息【*】
    if (ret == 0)
        part_print_dos(dev_desc);                                           // 输出dos部分的信息【*】
	return ret;
}

int is_valid_dos_buf(void *buf)
{
	return test_block_type(buf) == DOS_MBR ? 0 : -1;
}

int write_mbr_partition(struct blk_desc *dev_desc, void *buf)
{
	if (is_valid_dos_buf(buf))
		return -1;

	/* write MBR */
	if (blk_dwrite(dev_desc, 0, 1, buf) != 1)
	{
		printf("%s: failed writing '%s' (1 blks at 0x0)\n", __func__, "MBR");
		return 1;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// 测试
//-------------------------------------------------------------------------------------------------

#if 1

// extern int fat_register_device(struct blk_desc *dev_desc, int part_no);

extern int fat_set_blk_dev(struct blk_desc *dev_desc, struct disk_partition *info);
extern int fat_exists(const char *filename);

void dos_test(struct blk_desc *desc)
{
    struct disk_partition mypart;
    char fn[] = "dog1.txt"; // "MAX811-MAX812.pdf"; //
    
    printf("mbr size=%i\r\n", sizeof(legacy_mbr_t));
    printf("partition size=%i\r\n", sizeof(struct partition));

    if (part_get_info_dos(desc, 1, &mypart) == 0)                       // 获取dos部分的信息【*】
    {
        if (fat_set_blk_dev(desc, &mypart) == 0)                        // 设置块存储器设备【*】
        {
            if (fat_exists(fn))                                         // 判断是否存在文件【*】
            {
                int count;
                char buf[512];
                printf("found file: %s\r\n", fn);
                count = file_fat_read(fn, buf, 512);                    // 将文件中的信息读入缓存区buf【*】
                printf("%s\r\n", buf);
            }
            else
            {
                printf("not found: %s\r\n", fn);
            }
        }
        else
        {
            printf("mass storage is not dos format\r\n");               // 大量存储不是dos格式
        }
    }
    printf("\r\n");
}

#endif // #if 1



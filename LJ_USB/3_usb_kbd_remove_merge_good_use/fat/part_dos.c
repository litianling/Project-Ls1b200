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

/* ��С�˸�ʽ�� char[4] ת��Ϊ������ʽ���� */
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
	lbaint_t lba_start = ext_part_sector + le32_to_int (p->start4);    // ��*��
	lbaint_t lba_size  = le32_to_int (p->size4);                       // ��*��

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
	} /*����û��DOSǩ�� */

	p = (struct dos_partition *)&buffer[DOS_PART_TBL_OFFSET];

	/* �������ָʾ���Ƿ���Ч�����������. */
	for (slot = 0; slot < 4; ++slot, ++p)
	{
		if (p->boot_ind != 0 && p->boot_ind != 0x80)
			break;
		if (p->sys_ind)
			++part_count;
	}

	/*
	 * �����������Ч��Ϊ�գ��������Ƿ��� DOS PBR
	 */
	if (slot != 4 || !part_count)
	{
		if (!strncmp((char *)&buffer[DOS_PBR_FSTYPE_OFFSET], "FAT", 3) ||
		    !strncmp((char *)&buffer[DOS_PBR32_FSTYPE_OFFSET], "FAT32", 5))
			return DOS_PBR; /* This is a DOS PBR and not an MBR */
	}

	if (slot == 4)
	{
    	return DOS_MBR;	/* ����һ�� DOS MBR */
    }

	/* ��Ȳ��� DOS MBR Ҳ���� DOS PBR */
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

/*  ��ӡ������չ��������صķ��� */
static void print_partition_extended(struct blk_desc *dev_desc,
				                     lbaint_t ext_part_sector,
				                     lbaint_t relative,
				                     int part_num, unsigned int disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i;

	/* �������ݹ鼶�� */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\r\n");
		return;
    }

	if (blk_dread(dev_desc, ext_part_sector, 1, (unsigned long *)buffer) != 1)     // ����洢����*��
	{
		printf("** Can't read partition table on %d:" LBAFU " **\r\n",
			   dev_desc->devnum, ext_part_sector);
		return;
	}

	i=test_block_type(buffer);                                                     // ���������͡�*��
	if (i != DOS_MBR)
	{
		printf("bad MBR sector signature 0x%02x%02x\r\n",
			   buffer[DOS_PART_MAGIC_OFFSET],
			   buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return;
	}

	if (!ext_part_sector)
		disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);                   // charתint��*��

	/* ��ӡ������/�߼����� */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++)
	{
		/* fdisk ����ʾ���� MBR �е���չ����  */
		if ((pt->sys_ind != 0) &&
		    (ext_part_sector == 0 || !is_extended (pt->sys_ind)) )
		{
			print_one_part(pt, ext_part_sector, part_num, disksig);                // �����ʾ��Ϣ��*��
		}

		/* ���� engr fdisk part# ������� */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) )
		{
			part_num++;
		}
	}

	/* ��ӡ��չ���� */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++)
	{
		if (is_extended (pt->sys_ind))                                           // �ǲ����������*��
		{
			lbaint_t lba_start = le32_to_int (pt->start4) + relative;            // charתint��*��

			print_partition_extended(dev_desc, lba_start,                        // �ݹ����----��ӡ������չ��������صķ�����*��
									 ext_part_sector == 0  ? lba_start : relative,
									 part_num, disksig);
		}
	}

	return;
}

/*  ��ӡ������չ��������صķ��� */
static int part_get_info_extended(struct blk_desc *dev_desc,
				                  lbaint_t ext_part_sector,
                                  lbaint_t relative,
								  int part_num,
                                  int which_part,
								  struct disk_partition *info,
                                  unsigned disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);                  // �����ڴ桾*��
	dos_partition_t *pt;                                                               // ĳ������ʼ��������Ϣ
	int i;
	int dos_type;

	/* �������ݹ鼶��--���Ƕ�׵�dos���� */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\n");
		return -1;
    }

    // ��ȡ������
	if (blk_dread(dev_desc, ext_part_sector, 1, (unsigned long *)buffer) != 1)         // ����洢�����������Ϣ��*��
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

	/* ��ӡ������/�߼����� */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);                           // ǿ����������ת�����������д洢�������ƫ������
	for (i = 0; i < 4; i++, pt++)
	{
		/*
		 * fdisk ����ʾ���� MBR �е���չ����
		 */
		if (((pt->boot_ind & ~0x80) == 0) &&
		    (pt->sys_ind != 0) &&
		    (part_num == which_part) &&
		    (ext_part_sector == 0 || is_extended(pt->sys_ind) == 0))                   // �ǲ�����չ�ķ�����*��
		{
			info->blksz = DOS_PART_DEFAULT_SECTOR;
			info->start = (lbaint_t)(ext_part_sector + le32_to_int(pt->start4));       // ��С�˸�ʽ�� char[4] ת��Ϊ������ʽ������*��
			info->size  = (lbaint_t)le32_to_int(pt->size4);                            // ͬ�ϡ�*��
			part_set_generic_name(dev_desc, part_num, (char *)info->name);             // ����ͨ�����ơ�*��
			/* sprintf(info->type, "%d, pt->sys_ind); */
			strcpy((char *)info->type, "loongson");                                    // �ַ���������*��
			info->bootable = get_bootable(pt);                                         // ����0��*������
#if (CONFIG_PARTITION_UUIDS)
			sprintf(info->uuid, "%08x-%02x", disksig, part_num);
#endif
			// info->sys_ind = pt->sys_ind;
			return 0;
		}

		/* ���� engr fdisk part# �������! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)))                         // ͬ�ϡ�*��������Ϊ������չ����
		{
			part_num++;
		}
	}

	/* ������չ���� */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);                          
	for (i = 0; i < 4; i++, pt++)
	{
		if (is_extended (pt->sys_ind))                                                // ͬ�ϡ�*��
		{
			lbaint_t lba_start = le32_to_int (pt->start4) + relative;                 // ͬ�ϡ�*��

			return part_get_info_extended(dev_desc, lba_start,                        // �ݹ����----��ӡ������չ��������صķ�����*��
										  ext_part_sector == 0 ? lba_start : relative,
										  part_num, which_part, info, disksig);
		}
	}

	/* ���δ�ҵ����������� DOS PBR */
	dos_type = test_block_type(buffer);                                               // ����DOS���͡�*��

	if (dos_type == DOS_PBR)
	{
		info->start = 0;
		info->size = dev_desc->lba;
		info->blksz = DOS_PART_DEFAULT_SECTOR;
		info->bootable = 0;
		strcpy((char *)info->type, "loongson");                                       // �ַ���������*����Ϣ����
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
	print_partition_extended(dev_desc, 0, 0, 1, 0);                        // ���������͵���Ϣ��*��
}

int part_get_info_dos(struct blk_desc *dev_desc, int part,
		              struct disk_partition *info)
{
    int ret = part_get_info_extended(dev_desc, 0, 0, 1, part, info, 0);     // ��ȡ������͵���Ϣ��*��
    if (ret == 0)
        part_print_dos(dev_desc);                                           // ���dos���ֵ���Ϣ��*��
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
// ����
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

    if (part_get_info_dos(desc, 1, &mypart) == 0)                       // ��ȡdos���ֵ���Ϣ��*��
    {
        if (fat_set_blk_dev(desc, &mypart) == 0)                        // ���ÿ�洢���豸��*��
        {
            if (fat_exists(fn))                                         // �ж��Ƿ�����ļ���*��
            {
                int count;
                char buf[512];
                printf("found file: %s\r\n", fn);
                count = file_fat_read(fn, buf, 512);                    // ���ļ��е���Ϣ���뻺����buf��*��
                printf("%s\r\n", buf);
            }
            else
            {
                printf("not found: %s\r\n", fn);
            }
        }
        else
        {
            printf("mass storage is not dos format\r\n");               // �����洢����dos��ʽ
        }
    }
    printf("\r\n");
}

#endif // #if 1



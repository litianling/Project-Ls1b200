/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _PART_H
#define _PART_H

#define MAX_PATH           260

#define LOG2(x) (((x & 0xaaaaaaaa) ? 1 : 0) + ((x & 0xcccccccc) ? 2 : 0) + \
		 ((x & 0xf0f0f0f0) ? 4 : 0) + ((x & 0xff00ff00) ? 8 : 0) + \
		 ((x & 0xffff0000) ? 16 : 0))

/* Interface types: */
#define IF_TYPE_UNKNOWN		0
#define IF_TYPE_IDE			1
#define IF_TYPE_SCSI		2
#define IF_TYPE_ATAPI		3
#define IF_TYPE_USB			4
#define IF_TYPE_DOC			5
#define IF_TYPE_MMC			6
#define IF_TYPE_SD			7
#define IF_TYPE_SATA		8

/* Part types */
#define PART_TYPE_UNKNOWN	0x00
#define PART_TYPE_MAC		0x01
#define PART_TYPE_DOS		0x02
#define PART_TYPE_ISO		0x03
#define PART_TYPE_AMIGA		0x04
#define PART_TYPE_EFI		0x05

/* device types */
#define DEV_TYPE_UNKNOWN	0xFF	/* not connected */
#define DEV_TYPE_HARDDISK	0x00	/* harddisk */
#define DEV_TYPE_TAPE		0x01	/* Tape */
#define DEV_TYPE_CDROM		0x05	/* CD-ROM */
#define DEV_TYPE_OPDISK		0x07	/* optical disk */

typedef struct disk_partition
{
	unsigned int	start;		/* # 分区中的第一个块 */
	unsigned int	size;		/* 分区中的块数 */
	unsigned int	blksz;		/* 块大小（以字节为单位） */
	unsigned char	name[32];	/* 分区名字 */
	unsigned char	type[32];	/* 字符串类型描述 */
	int				bootable;	/* 设置了活动/可引导标志 */
#ifdef CONFIG_PARTITION_UUIDS
	char	        uuid[37];	/* filesystem UUID as string, if exists	*/
#endif
} disk_partition_t;

/*
 * functions
 */
/*
extern int part_get_info(blk_desc_t *dev_desc, int part, disk_partition_t *info);
extern void part_init(blk_desc_t *dev_desc);
extern void dev_print(blk_desc_t *dev_desc);
extern void part_set_generic_name(const blk_desc_t *dev_desc, int part_num, char *name);
 */
 
#endif /* _PART_H */

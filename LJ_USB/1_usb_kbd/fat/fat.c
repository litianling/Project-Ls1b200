// SPDX-License-Identifier: GPL-2.0+
/*
 * fat.c
 *
 * R/O (V)FAT 12/16/32 filesystem implementation by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to uboot
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#include "usb_cfg.h"        // ARCH_DMA_MINALIGN

#include "linux_kernel.h"
#include "blk.h"
#include "part.h"
#include "memalign.h"
#include "fs.h"

#include "fat.h"

#if 0
#define debug(format, arg...) printf("DEBUG: " format "\n", ## arg)
#else
#define debug(format, arg...) do {} while (0)
#endif /* DEBUG */

#define cpu_to_le32
#define cpu_to_le16

#define assert(x)

//-------------------------------------------------------------------------------------------------

/*
 * Convert a string to lowercase.  Converts at most 'len' characters,
 * 'len' may be larger than the length of 'str' if 'str' is NULL
 * terminated.
 */
static void downcase(char *str, size_t len)
{
	while (*str != '\0' && len--)
	{
		*str = tolower(*str);
		str++;
	}
}

static struct blk_desc *cur_dev;
static struct disk_partition cur_part_info;

#define DOS_BOOT_MAGIC_OFFSET	0x1fe
#define DOS_FS_TYPE_OFFSET		0x36
#define DOS_FS32_TYPE_OFFSET	0x52

/*
 * cur_dev->blksz      == 0x20000
 * cur_part_info.blksz == 512
 */
static int disk_read(uint32_t block, uint32_t nr_blocks, void *buf)
{
	unsigned long ret;

	if (!cur_dev)
		return -1;

	ret = blk_dread(cur_dev, cur_part_info.start + block, nr_blocks, buf);

	if (ret != nr_blocks)
		return -1;

#if 0
// 测试
    printf("read from sector[%i], sectors=%i\r\n", cur_part_info.start + block, nr_blocks);
    extern void print_hex(char *p, int size);
    print_hex(buf, nr_blocks*512);
#endif

	return ret;
}

int fat_set_blk_dev(struct blk_desc *dev_desc, struct disk_partition *info)
{
#if 1

	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);

	cur_dev = dev_desc;
	cur_part_info = *info;

	/* Make sure it has a valid FAT header */
	if (disk_read(0, 1, buffer) != 1)
	{
		cur_dev = NULL;
		return -1;
	}

	/* Check if it's actually a DOS volume */
	if (memcmp(buffer + DOS_BOOT_MAGIC_OFFSET, "\x55\xAA", 2))
	{
		cur_dev = NULL;
		return -1;
	}

	/* Check for FAT12/FAT16/FAT32 filesystem */
	if (!memcmp(buffer + DOS_FS_TYPE_OFFSET, "FAT", 3))
		return 0;
	if (!memcmp(buffer + DOS_FS32_TYPE_OFFSET, "FAT32", 5))
		return 0;

	cur_dev = NULL;
	return -1;

#else

    unsigned char *buffer;
    buffer = (unsigned char *)aligned_malloc(dev_desc->blksz, ARCH_DMA_MINALIGN);

	cur_dev = dev_desc;
	cur_part_info = *info;

	/* Make sure it has a valid FAT header */
	if (disk_read(0, 1, buffer) != 1)
	{
	    aligned_free(buffer);
		cur_dev = NULL;
		return -1;
	}

	/* Check if it's actually a DOS volume */
	if (memcmp(buffer + DOS_BOOT_MAGIC_OFFSET, "\x55\xAA", 2))
	{
	    aligned_free(buffer);
		cur_dev = NULL;
		return -1;
	}

	/* Check for FAT12/FAT16/FAT32 filesystem */
	if (!memcmp(buffer + DOS_FS_TYPE_OFFSET, "FAT", 3))
	{
	    aligned_free(buffer);
    	return 0;
    }
    
	if (!memcmp(buffer + DOS_FS32_TYPE_OFFSET, "FAT32", 5))
	{
        aligned_free(buffer);
    	return 0;
    }

    aligned_free(buffer);
	cur_dev = NULL;
	return -1;
#endif
}

int fat_register_device(struct blk_desc *dev_desc, int part_no)
{
	struct disk_partition info;

	/* First close any currently found FAT filesystem */
	cur_dev = NULL;

	/* Read the partition table, if present */
	if (part_get_info(dev_desc, part_no, &info))
	{
		if (part_no != 0)
		{
			printf("** Partition %d not valid on device %d **\n",
					part_no, dev_desc->devnum);
			return -1;
		}

		info.start = 0;
		info.size = dev_desc->lba;
		info.blksz = dev_desc->blksz;
		info.name[0] = 0;
		info.type[0] = 0;
		info.bootable = 0;
#if (CONFIG_PARTITION_UUIDS)
		info.uuid[0] = 0;
#endif
	}

	return fat_set_blk_dev(dev_desc, &info);
}

/*
 * Extract zero terminated short name from a directory entry.
 */
static void get_name(dir_entry_t *dirent, char *s_name)
{
	char *ptr;

	memcpy(s_name, dirent->name, 8);
	s_name[8] = '\0';
	ptr = s_name;
	while (*ptr && *ptr != ' ')
		ptr++;
	if (dirent->lcase & CASE_LOWER_BASE)
		downcase(s_name, (unsigned)(ptr - s_name));
	if (dirent->ext[0] && dirent->ext[0] != ' ')
	{
		*ptr++ = '.';
		memcpy(ptr, dirent->ext, 3);
		if (dirent->lcase & CASE_LOWER_EXT)
			downcase(ptr, 3);
		ptr[3] = '\0';
		while (*ptr && *ptr != ' ')
			ptr++;
	}

	*ptr = '\0';
	if (*s_name == DELETED_FLAG)
		*s_name = '\0';
	else if (*s_name == aRING)
		*s_name = DELETED_FLAG;
}

static int flush_dirty_fat_buffer(fsdata_t *mydata);

/*
 * Get the entry at index 'entry' in a FAT (12/16/32) table.
 * On failure 0x00 is returned.
 */
static uint32_t get_fatent(fsdata_t *mydata, uint32_t entry)
{
	uint32_t bufnum;
	uint32_t offset, off8;
	uint32_t ret = 0x00;

	if (CHECK_CLUST(entry, mydata->fatsize))
	{
		printf("Error: Invalid FAT entry: 0x%08x\n", (int)entry);
		return ret;
	}

	switch (mydata->fatsize)
	{
		case 32:
			bufnum = entry / FAT32BUFSIZE;
			offset = entry - bufnum * FAT32BUFSIZE;
			break;
		case 16:
			bufnum = entry / FAT16BUFSIZE;
			offset = entry - bufnum * FAT16BUFSIZE;
			break;
		case 12:
			bufnum = entry / FAT12BUFSIZE;
			offset = entry - bufnum * FAT12BUFSIZE;
			break;

		default:
			/* Unsupported FAT size */
			return ret;
	}

	debug("FAT%d: entry: 0x%08x = %d, offset: 0x%04x = %d\n",
	       mydata->fatsize, entry, entry, offset, offset);

	/* Read a new block of FAT entries into the cache. */
	if (bufnum != mydata->fatbufnum)
	{
		uint32_t getsize = FATBUFBLOCKS;
		uint8_t *bufptr = mydata->fatbuf;
		uint32_t fatlength = mydata->fatlength;
		uint32_t startblock = bufnum * FATBUFBLOCKS;

		/* Cap length if fatlength is not a multiple of FATBUFBLOCKS */
		if (startblock + getsize > fatlength)
			getsize = fatlength - startblock;

		startblock += mydata->fat_sect;	/* Offset from start of disk */

		/* Write back the fatbuf to the disk */
		if (flush_dirty_fat_buffer(mydata) < 0)
			return -1;

		if (disk_read(startblock, getsize, bufptr) < 0)
		{
			debug("Error reading FAT blocks\n");
			return ret;
		}
		mydata->fatbufnum = bufnum;
	}

	/* Get the actual entry from the table */
	switch (mydata->fatsize)
	{
		case 32:
			ret = FAT2CPU32(((uint32_t *) mydata->fatbuf)[offset]);
			break;
		case 16:
			ret = FAT2CPU16(((uint16_t *) mydata->fatbuf)[offset]);
			break;
		case 12:
			off8 = (offset * 3) / 2;
			/* fatbut + off8 may be unaligned, read in byte granularity */
			ret = mydata->fatbuf[off8] + (mydata->fatbuf[off8 + 1] << 8);

			if (offset & 0x1)
				ret >>= 4;
			ret &= 0xfff;
	}

	debug("FAT%d: ret: 0x%08x, entry: 0x%08x, offset: 0x%04x\n",
	       mydata->fatsize, ret, entry, offset);

	return ret;
}

/*
 * Read at most 'size' bytes from the specified cluster into 'buffer'.
 * Return 0 on success, -1 otherwise.
 */
static int
get_cluster(fsdata_t *mydata, uint32_t clustnum, uint8_t *buffer, unsigned long size)
{
	uint32_t idx = 0;
	uint32_t startsect;
	int ret;

	if (clustnum > 0)
	{
		startsect = clust_to_sect(mydata, clustnum);
	}
	else
	{
		startsect = mydata->rootdir_sect;
	}

	debug("gc - clustnum: %d, startsect: %d\n", clustnum, startsect);

	if ((unsigned long)buffer & (ARCH_DMA_MINALIGN - 1))
	{
		ALLOC_CACHE_ALIGN_BUFFER(uint8_t, tmpbuf, mydata->sect_size);

		debug("FAT: Misaligned buffer address (%p)\n", buffer);

		while (size >= mydata->sect_size)
		{
			ret = disk_read(startsect++, 1, tmpbuf);
			if (ret != 1)
			{
				debug("Error reading data (got %d)\n", ret);
				return -1;
			}

			memcpy(buffer, tmpbuf, mydata->sect_size);
			buffer += mydata->sect_size;
			size -= mydata->sect_size;
		}
	}
	else
	{
		idx = size / mydata->sect_size;
		ret = disk_read(startsect, idx, buffer);
		if (ret != idx)
		{
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}

		startsect += idx;
		idx *= mydata->sect_size;
		buffer += idx;
		size -= idx;
	}

	if (size)
	{
		ALLOC_CACHE_ALIGN_BUFFER(uint8_t, tmpbuf, mydata->sect_size);

		ret = disk_read(startsect, 1, tmpbuf);
		if (ret != 1)
		{
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}

		memcpy(buffer, tmpbuf, size);
	}

	return 0;
}

/**
 * get_contents() - read from file
 *
 * Read at most 'maxsize' bytes from 'pos' in the file associated with 'dentptr'
 * into 'buffer'. Update the number of bytes read in *gotsize or return -1 on
 * fatal errors.
 *
 * @mydata:	file system description
 * @dentprt:	directory entry pointer
 * @pos:	position from where to read
 * @buffer:	buffer into which to read
 * @maxsize:	maximum number of bytes to read
 * @gotsize:	number of bytes actually read
 * Return:	-1 on error, otherwise 0
 */
static int get_contents(fsdata_t *mydata, dir_entry_t *dentptr, loff_t pos,
			            uint8_t *buffer, loff_t maxsize, loff_t *gotsize)
{
	loff_t filesize = FAT2CPU32(dentptr->size);
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;
	uint32_t curclust = START(dentptr);
	uint32_t endclust, newclust;
	loff_t actsize;

	*gotsize = 0;
	debug("Filesize: %llu bytes\n", filesize);

	if (pos >= filesize)
	{
		debug("Read position past EOF: %llu\n", pos);
		return 0;
	}

	if (maxsize > 0 && filesize > pos + maxsize)
		filesize = pos + maxsize;

	debug("%llu bytes\n", filesize);

	actsize = bytesperclust;

	/* go to cluster at pos */
	while (actsize <= pos)
	{
		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize))
		{
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}
		actsize += bytesperclust;
	}

	/* actsize > pos */
	actsize -= bytesperclust;
	filesize -= actsize;
	pos -= actsize;

	/* align to beginning of next cluster if any */
	if (pos)
	{
		uint8_t *tmp_buffer;

		actsize = min(filesize, (loff_t)bytesperclust);
		tmp_buffer = malloc_cache_aligned(actsize);
		if (!tmp_buffer)
		{
			debug("Error: allocating buffer\n");
			return -1;
		}

		if (get_cluster(mydata, curclust, tmp_buffer, actsize) != 0)
		{
			printf("Error reading cluster\n");
			aligned_free(tmp_buffer);
			return -1;
		}

		filesize -= actsize;
		actsize -= pos;
		memcpy(buffer, tmp_buffer + pos, actsize);
		aligned_free(tmp_buffer);
		*gotsize += actsize;
		if (!filesize)
			return 0;
		buffer += actsize;

		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize))
		{
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}
	}

	actsize = bytesperclust;
	endclust = curclust;

	do
	{
		/* search for consecutive clusters */
		while (actsize < filesize)
		{
			newclust = get_fatent(mydata, endclust);
			if ((newclust - 1) != endclust)
				goto getit;
			if (CHECK_CLUST(newclust, mydata->fatsize))
			{
				debug("curclust: 0x%x\n", newclust);
				printf("Invalid FAT entry\n");
				return -1;
			}
			endclust = newclust;
			actsize += bytesperclust;
		}

		/* get remaining bytes */
		actsize = filesize;
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0)
		{
			printf("Error reading cluster\n");
			return -1;
		}

		*gotsize += actsize;
		return 0;
getit:
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0)
		{
			printf("Error reading cluster\n");
			return -1;
		}

		*gotsize += (int)actsize;
		filesize -= actsize;
		buffer += actsize;

		curclust = get_fatent(mydata, endclust);
		if (CHECK_CLUST(curclust, mydata->fatsize))
		{
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}
		actsize = bytesperclust;
		endclust = curclust;
	} while (1);
}

/*
 * Extract the file name information from 'slotptr' into 'l_name',
 * starting at l_name[*idx].
 * Return 1 if terminator (zero byte) is found, 0 otherwise.
 */
static int slot2str(dir_slot_t *slotptr, char *l_name, int *idx)
{
	int j;

	for (j = 0; j <= 8; j += 2)
	{
		l_name[*idx] = slotptr->name0_4[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}

	for (j = 0; j <= 10; j += 2)
	{
		l_name[*idx] = slotptr->name5_10[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}

	for (j = 0; j <= 2; j += 2)
	{
		l_name[*idx] = slotptr->name11_12[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}

	return 0;
}

/* Calculate short name checksum */
static uint8_t mkcksum(const char name[8], const char ext[3])
{
	int i;

	uint8_t ret = 0;

	for (i = 0; i < 8; i++)
		ret = (((ret & 1) << 7) | ((ret & 0xfe) >> 1)) + name[i];
	for (i = 0; i < 3; i++)
		ret = (((ret & 1) << 7) | ((ret & 0xfe) >> 1)) + ext[i];

	return ret;
}

/*
 * Read boot sector and volume info from a FAT filesystem
 */
static int read_bootsectandvi(boot_sector_t *bs, volume_info_t *volinfo, int *fatsize)
{
	uint8_t *block;
	volume_info_t *vistart;
	int ret = 0;

	if (cur_dev == NULL)
	{
		debug("Error: no device selected\n");
		return -1;
	}

	block = malloc_cache_aligned(cur_dev->blksz);
	if (block == NULL)
	{
		debug("Error: allocating block\n");
		return -1;
	}

	if (disk_read(0, 1, block) < 0)
	{
		debug("Error: reading block\n");
		goto fail;
	}

	memcpy(bs, block, sizeof(boot_sector_t));
	bs->reserved = FAT2CPU16(bs->reserved);
	bs->fat_length = FAT2CPU16(bs->fat_length);
	bs->secs_track = FAT2CPU16(bs->secs_track);
	bs->heads = FAT2CPU16(bs->heads);
	bs->total_sect = FAT2CPU32(bs->total_sect);

	/* FAT32 entries */
	if (bs->fat_length == 0)
	{
		/* Assume FAT32 */
		bs->fat32_length = FAT2CPU32(bs->fat32_length);
		bs->flags = FAT2CPU16(bs->flags);
		bs->root_cluster = FAT2CPU32(bs->root_cluster);
		bs->info_sector = FAT2CPU16(bs->info_sector);
		bs->backup_boot = FAT2CPU16(bs->backup_boot);
		vistart = (volume_info_t *)(block + sizeof(boot_sector_t));
		*fatsize = 32;
	}
	else
	{
		vistart = (volume_info_t *)&(bs->fat32_length);
		*fatsize = 0;
	}

	memcpy(volinfo, vistart, sizeof(volume_info_t));

	if (*fatsize == 32)
	{
		if (strncmp(FAT32_SIGN, vistart->fs_type, SIGNLEN) == 0)
			goto exit;
	}
	else
	{
		if (strncmp(FAT12_SIGN, vistart->fs_type, SIGNLEN) == 0)
		{
			*fatsize = 12;
			goto exit;
		}

		if (strncmp(FAT16_SIGN, vistart->fs_type, SIGNLEN) == 0)
		{
			*fatsize = 16;
			goto exit;
		}
	}

	debug("Error: broken fs_type sign\n");
fail:
	ret = -1;
exit:
	aligned_free(block);
	return ret;
}

static int get_fs_info(fsdata_t *mydata)
{
	boot_sector_t bs;
	volume_info_t volinfo;
	int ret;

	ret = read_bootsectandvi(&bs, &volinfo, &mydata->fatsize);
	if (ret)
	{
		debug("Error: reading boot sector\n");
		return ret;
	}

	if (mydata->fatsize == 32)
	{
		mydata->fatlength = bs.fat32_length;
		mydata->total_sect = bs.total_sect;
	}
	else
	{
		mydata->fatlength = bs.fat_length;
		mydata->total_sect = (bs.sectors[1] << 8) + bs.sectors[0];
		if (!mydata->total_sect)
			mydata->total_sect = bs.total_sect;
	}

	if (!mydata->total_sect) /* unlikely */
		mydata->total_sect = (unsigned)cur_part_info.size;

	mydata->fats = bs.fats;
	mydata->fat_sect = bs.reserved;

	mydata->rootdir_sect = mydata->fat_sect + mydata->fatlength * bs.fats;

	mydata->sect_size = (bs.sector_size[1] << 8) + bs.sector_size[0];
	mydata->clust_size = bs.cluster_size;
	if (mydata->sect_size != cur_part_info.blksz)  /* 这两个是一致的 */
	{
		printf("Error: FAT sector size mismatch (fs=%hu, dev=%lu)\n",
			    mydata->sect_size, cur_part_info.blksz);
		return -1;
	}

	if (mydata->clust_size == 0)
	{
		printf("Error: FAT cluster size not set\n");
		return -1;
	}

	if ((unsigned int)mydata->clust_size * mydata->sect_size > MAX_CLUSTSIZE)
	{
		printf("Error: FAT cluster size too big (cs=%u, max=%u)\n",
		       (unsigned int)mydata->clust_size * mydata->sect_size,
		       MAX_CLUSTSIZE);
		return -1;
	}

	if (mydata->fatsize == 32)
	{
		mydata->data_begin = mydata->rootdir_sect - (mydata->clust_size * 2);
		mydata->root_cluster = bs.root_cluster;
	}
	else
	{
		mydata->rootdir_size = ((bs.dir_entries[1]  * (int)256 + bs.dir_entries[0]) *
					             sizeof(dir_entry_t)) / mydata->sect_size;
		mydata->data_begin = mydata->rootdir_sect +
					         mydata->rootdir_size - (mydata->clust_size * 2);

		/*
		 * The root directory is not cluster-aligned and may be on a
		 * "negative" cluster, this will be handled specially in
		 * next_cluster().
		 */
		mydata->root_cluster = 0;
	}

	mydata->fatbufnum = -1;
	mydata->fat_dirty = 0;
	mydata->fatbuf = malloc_cache_aligned(FATBUFSIZE);
	if (mydata->fatbuf == NULL)
	{
		debug("Error: allocating memory\n");
		return -1;
	}

	debug("FAT%d, fat_sect: %d, fatlength: %d\n",
	       mydata->fatsize, mydata->fat_sect, mydata->fatlength);
	debug("Rootdir begins at cluster: %d, sector: %d, offset: %x\n"
	       "Data begins at: %d\n",
	       mydata->root_cluster,
	       mydata->rootdir_sect,
	       mydata->rootdir_sect * mydata->sect_size, mydata->data_begin);
	debug("Sector size: %d, cluster size: %d\n", mydata->sect_size,
	       mydata->clust_size);

    // fat_print_info(mydata); // 测试

	return 0;
}

/*
 * Directory iterator, to simplify filesystem traversal
 *
 * Implements an iterator pattern to traverse directory tables,
 * transparently handling directory tables split across multiple
 * clusters, and the difference between FAT12/FAT16 root directory
 * (contiguous) and subdirectories + FAT32 root (chained).
 *
 * Rough usage:
 *
 *   for (fat_itr_root(&itr, fsdata); fat_itr_next(&itr); ) {
 *      // to traverse down to a subdirectory pointed to by
 *      // current iterator position:
 *      fat_itr_child(&itr, &itr);
 *   }
 *
 * For more complete example, see fat_itr_resolve()
 */

typedef struct
{
	fsdata_t   *fsdata;        /* filesystem parameters */
	unsigned    start_clust;   /* first cluster */
	unsigned    clust;         /* current cluster */
	unsigned    next_clust;    /* next cluster if remaining == 0 */
	int         last_cluster;  /* set once we've read last cluster */
	int         is_root;       /* is iterator at root directory */
	int         remaining;     /* remaining dent's in current cluster */

	/* current iterator position values: */
	dir_entry_t *dent;         /* current directory entry */
	char        l_name[VFAT_MAXLEN_BYTES];    /* long (vfat) name */
	char        s_name[14];    /* short 8.3 name */
	char       *name;          /* l_name if there is one, else s_name */

	/* storage for current cluster in memory: */
	unsigned char block[MAX_CLUSTSIZE] __attribute__((aligned(ARCH_DMA_MINALIGN))); // __aligned(ARCH_DMA_MINALIGN);
} fat_itr_t;

static int fat_itr_isdir(fat_itr_t *itr);

/**
 * fat_itr_root() - initialize an iterator to start at the root
 * directory
 *
 * @itr: iterator to initialize
 * @fsdata: filesystem data for the partition
 * @return 0 on success, else -errno
 */
static int fat_itr_root(fat_itr_t *itr, fsdata_t *fsdata)
{
	if (get_fs_info(fsdata))
		return -ENXIO;

	itr->fsdata = fsdata;
	itr->start_clust = 0;
	itr->clust = fsdata->root_cluster;
	itr->next_clust = fsdata->root_cluster;
	itr->dent = NULL;
	itr->remaining = 0;
	itr->last_cluster = 0;
	itr->is_root = 1;

	return 0;
}

/**
 * fat_itr_child() - initialize an iterator to descend into a sub-
 * directory
 *
 * Initializes 'itr' to iterate the contents of the directory at
 * the current cursor position of 'parent'.  It is an error to
 * call this if the current cursor of 'parent' is pointing at a
 * regular file.
 *
 * Note that 'itr' and 'parent' can be the same pointer if you do
 * not need to preserve 'parent' after this call, which is useful
 * for traversing directory structure to resolve a file/directory.
 *
 * @itr: iterator to initialize
 * @parent: the iterator pointing at a directory entry in the
 *    parent directory of the directory to iterate
 */
static void fat_itr_child(fat_itr_t *itr, fat_itr_t *parent)
{
	fsdata_t *mydata = parent->fsdata;  /* for silly macros */
	unsigned clustnum = START(parent->dent);

//	assert(fat_itr_isdir(parent));

	itr->fsdata = parent->fsdata;
	itr->start_clust = clustnum;
	if (clustnum > 0)
	{
		itr->clust = clustnum;
		itr->next_clust = clustnum;
		itr->is_root = 0;
	}
	else
	{
		itr->clust = parent->fsdata->root_cluster;
		itr->next_clust = parent->fsdata->root_cluster;
		itr->is_root = 1;
	}

	itr->dent = NULL;
	itr->remaining = 0;
	itr->last_cluster = 0;
}

static void *next_cluster(fat_itr_t *itr, unsigned *nbytes)
{
	// fsdata_t *mydata = itr->fsdata;  /* for silly macros */
	int ret;
	uint32_t sect;
	uint32_t read_size;

	/* have we reached the end? */
	if (itr->last_cluster)
		return NULL;

	if (itr->is_root && itr->fsdata->fatsize != 32)
	{
		/*
		 * The root directory is located before the data area and
		 * cannot be indexed using the regular unsigned cluster
		 * numbers (it may start at a "negative" cluster or not at a
		 * cluster boundary at all), so consider itr->next_clust to be
		 * a offset in cluster-sized units from the start of rootdir.
		 */
		unsigned sect_offset = itr->next_clust * itr->fsdata->clust_size;
		unsigned remaining_sects = itr->fsdata->rootdir_size - sect_offset;
		sect = itr->fsdata->rootdir_sect + sect_offset;
		/* do not read past the end of rootdir */
		read_size = min_t(uint32_t, itr->fsdata->clust_size, remaining_sects);
	}
	else
	{
		sect = clust_to_sect(itr->fsdata, itr->next_clust);
		read_size = itr->fsdata->clust_size;
	}

	debug("FAT read(sect=%d), clust_size=%d, read_size=%u, DIRENTSPERBLOCK=%zd\n",
	      sect, itr->fsdata->clust_size, read_size, DIRENTSPERBLOCK);

	/*
	 * NOTE: do_fat_read_at() had complicated logic to deal w/
	 * vfat names that span multiple clusters in the fat16 case,
	 * which get_dentfromdir() probably also needed (and was
	 * missing).  And not entirely sure what fat32 didn't have
	 * the same issue..  We solve that by only caring about one
	 * dent at a time and iteratively constructing the vfat long
	 * name.
	 */
	ret = disk_read(sect, read_size, itr->block);
	if (ret < 0)
	{
		debug("Error: reading block\n");
		return NULL;
	}

	*nbytes = read_size * itr->fsdata->sect_size;
	itr->clust = itr->next_clust;
	if (itr->is_root && itr->fsdata->fatsize != 32)
	{
		itr->next_clust++;
		if (itr->next_clust * itr->fsdata->clust_size >= itr->fsdata->rootdir_size)
		{
			debug("nextclust: 0x%x\n", itr->next_clust);
			itr->last_cluster = 1;
		}
	}
	else
	{
		itr->next_clust = get_fatent(itr->fsdata, itr->next_clust);
		if (CHECK_CLUST(itr->next_clust, itr->fsdata->fatsize))
		{
			debug("nextclust: 0x%x\n", itr->next_clust);
			itr->last_cluster = 1;
		}
	}

	return itr->block;
}

static dir_entry_t *next_dent(fat_itr_t *itr)
{
	if (itr->remaining == 0)
	{
		unsigned nbytes;
		struct dir_entry *dent = next_cluster(itr, &nbytes);

		/* have we reached the last cluster? */
		if (!dent)
		{
			/* a sign for no more entries left */
			itr->dent = NULL;
			return NULL;
		}

		itr->remaining = nbytes / sizeof(dir_entry_t) - 1;
		itr->dent = dent;
	}
	else
	{
		itr->remaining--;
		itr->dent++;
	}

	/* have we reached the last valid entry? */
	if (itr->dent->name[0] == 0)
		return NULL;

	return itr->dent;
}

static dir_entry_t *extract_vfat_name(fat_itr_t *itr)
{
	struct dir_entry *dent = itr->dent;
	int seqn = itr->dent->name[0] & ~LAST_LONG_ENTRY_MASK;
	unsigned char chksum, alias_checksum = ((dir_slot_t *)dent)->alias_checksum;
	int n = 0;

	while (seqn--)
	{
		char buf[13];
		int idx = 0;

		slot2str((dir_slot_t *)dent, buf, &idx);

		if (n + idx >= sizeof(itr->l_name))
			return NULL;

		/* shift accumulated long-name up and copy new part in: */
		memmove(itr->l_name + idx, itr->l_name, n);
		memcpy(itr->l_name, buf, idx);
		n += idx;

		dent = next_dent(itr);
		if (!dent)
			return NULL;
	}

	/*
	 * We are now at the short file name entry.
	 * If it is marked as deleted, just skip it.
	 */
	if (dent->name[0] == DELETED_FLAG ||
	    dent->name[0] == aRING)
		return NULL;

	itr->l_name[n] = '\0';

	chksum = mkcksum(dent->name, dent->ext);

	/* checksum mismatch could mean deleted file, etc.. skip it: */
	if (chksum != alias_checksum)
	{
		debug("** chksum=%x, alias_checksum=%x, l_name=%s, s_name=%8s.%3s\n",
		      chksum, alias_checksum, itr->l_name, dent->name, dent->ext);
		return NULL;
	}

	return dent;
}

/**
 * fat_itr_next() - step to the next entry in a directory
 *
 * Must be called once on a new iterator before the cursor is valid.
 *
 * @itr: the iterator to iterate
 * @return boolean, 1 if success or 0 if no more entries in the
 *    current directory
 */
static int fat_itr_next(fat_itr_t *itr)
{
	dir_entry_t *dent;

	itr->name = NULL;

	/*
	 * One logical directory entry consist of following slots:
	 *				name[0]	Attributes
	 *   dent[N - N]: LFN[N - 1]	N|0x40	ATTR_VFAT
	 *   ...
	 *   dent[N - 2]: LFN[1]	2	ATTR_VFAT
	 *   dent[N - 1]: LFN[0]	1	ATTR_VFAT
	 *   dent[N]:     SFN			ATTR_ARCH
	 */

	while (1)
	{
		dent = next_dent(itr);
		if (!dent)
			return 0;

		if (dent->name[0] == DELETED_FLAG ||
		    dent->name[0] == aRING)
			continue;

		if (dent->attr & ATTR_VOLUME)
		{
			if ((dent->attr & ATTR_VFAT) == ATTR_VFAT &&
			    (dent->name[0] & LAST_LONG_ENTRY_MASK)) {
				/* long file name */
				dent = extract_vfat_name(itr);
				/*
				 * If succeeded, dent has a valid short file
				 * name entry for the current entry.
				 * If failed, itr points to a current bogus
				 * entry. So after fetching a next one,
				 * it may have a short file name entry
				 * for this bogus entry so that we can still
				 * check for a short name.
				 */
				if (!dent)
					continue;
				itr->name = itr->l_name;
				break;
			}
			else
			{
				/* Volume label or VFAT entry, skip */
				continue;
			}
		}

		/* short file name */
		break;
	}

	get_name(dent, itr->s_name);
	if (!itr->name)
		itr->name = itr->s_name;

	return 1;
}

/**
 * fat_itr_isdir() - is current cursor position pointing to a directory
 *
 * @itr: the iterator
 * @return true if cursor is at a directory
 */
static int fat_itr_isdir(fat_itr_t *itr)
{
	return !!(itr->dent->attr & ATTR_DIR);
}

/*
 * Helpers:
 */

#define TYPE_FILE 0x1
#define TYPE_DIR  0x2
#define TYPE_ANY  (TYPE_FILE | TYPE_DIR)

/**
 * fat_itr_resolve() - traverse directory structure to resolve the
 * requested path.
 *
 * Traverse directory structure to the requested path.  If the specified
 * path is to a directory, this will descend into the directory and
 * leave it iterator at the start of the directory.  If the path is to a
 * file, it will leave the iterator in the parent directory with current
 * cursor at file's entry in the directory.
 *
 * @itr: iterator initialized to root
 * @path: the requested path
 * @type: bitmask of allowable file types
 * @return 0 on success or -errno
 */
static int fat_itr_resolve(fat_itr_t *itr, const char *path, unsigned type)
{
	const char *next;

	/* chomp any extra leading slashes: */
	while (path[0] && ISDIRDELIM(path[0]))
		path++;

	/* are we at the end? */
	if (strlen(path) == 0)
	{
		if (!(type & TYPE_DIR))
			return -ENOENT;
		return 0;
	}

	/* find length of next path entry: */
	next = path;
	while (next[0] && !ISDIRDELIM(next[0]))
		next++;

	if (itr->is_root)
	{
		/* root dir doesn't have "." nor ".." */
		if ((((next - path) == 1) && !strncmp(path, ".", 1)) ||
		    (((next - path) == 2) && !strncmp(path, "..", 2)))
		{
			/* point back to itself */
			itr->clust = itr->fsdata->root_cluster;
			itr->next_clust = itr->fsdata->root_cluster;
			itr->dent = NULL;
			itr->remaining = 0;
			itr->last_cluster = 0;

			if (next[0] == 0)
			{
				if (type & TYPE_DIR)
					return 0;
				else
					return -ENOENT;
			}

			return fat_itr_resolve(itr, next, type);
		}
	}

	while (fat_itr_next(itr))
	{
		int match = 0;
		unsigned n = max(strlen(itr->name), (size_t)(next - path));

		/* check both long and short name: */
		if (!strncasecmp(path, itr->name, n))
			match = 1;
		else if (itr->name != itr->s_name &&
			 !strncasecmp(path, itr->s_name, n))
			match = 1;

		if (!match)
			continue;

		if (fat_itr_isdir(itr))
		{
			/* recurse into directory: */
			fat_itr_child(itr, itr);
			return fat_itr_resolve(itr, next, type);
		}
		else if (next[0])
		{
			/*
			 * If next is not empty then we have a case
			 * like: /path/to/realfile/nonsense
			 */
			debug("bad trailing path: %s\n", next);
			return -ENOENT;
		}
		else if (!(type & TYPE_FILE))
		{
			return -ENOTDIR;
		}
		else
		{
			return 0;
		}
	}

	return -ENOENT;
}

int file_fat_detectfs(void)
{
	boot_sector_t bs;
	volume_info_t volinfo;
	int fatsize;
	char vol_label[12];

	if (cur_dev == NULL)
	{
		printf("No current device\n");
		return 1;
	}

#if defined(CONFIG_IDE) || \
    defined(CONFIG_SATA) || \
    defined(CONFIG_SCSI) || \
    defined(CONFIG_CMD_USB) || \
    defined(CONFIG_MMC)
	printf("Interface:  ");
	switch (cur_dev->if_type)
	{
		case IF_TYPE_IDE:
			printf("IDE");
			break;
		case IF_TYPE_SATA:
			printf("SATA");
			break;
		case IF_TYPE_SCSI:
			printf("SCSI");
			break;
		case IF_TYPE_ATAPI:
			printf("ATAPI");
			break;
		case IF_TYPE_USB:
			printf("USB");
			break;
		case IF_TYPE_DOC:
			printf("DOC");
			break;
		case IF_TYPE_MMC:
			printf("MMC");
			break;
		default:
			printf("Unknown");
	}

	printf("\n  Device %d: ", cur_dev->devnum);
	dev_print(cur_dev);
#endif

	if (read_bootsectandvi(&bs, &volinfo, &fatsize))
	{
		printf("\nNo valid FAT fs found\n");
		return 1;
	}

	memcpy(vol_label, volinfo.volume_label, 11);
	vol_label[11] = '\0';
	volinfo.fs_type[5] = '\0';

	printf("Filesystem: %s \"%s\"\n", volinfo.fs_type, vol_label);

	return 0;
}

int fat_exists(const char *filename)
{
	fsdata_t fsdata;
	fat_itr_t *itr;
	int ret;

	itr = malloc_cache_aligned(sizeof(fat_itr_t));
	if (!itr)
		return 0;
	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto out;

	ret = fat_itr_resolve(itr, filename, TYPE_ANY);

	aligned_free(fsdata.fatbuf);

out:
	aligned_free(itr);
	return ret == 0;
}

int fat_size(const char *filename, loff_t *size)
{
	fsdata_t fsdata;
	fat_itr_t *itr;
	int ret;

	itr = malloc_cache_aligned(sizeof(fat_itr_t));
	if (!itr)
		return -ENOMEM;
	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto out_free_itr;

	ret = fat_itr_resolve(itr, filename, TYPE_FILE);
	if (ret)
	{
		/*
		 * Directories don't have size, but fs_size() is not
		 * expected to fail if passed a directory path:
		 */
		aligned_free(fsdata.fatbuf);
		ret = fat_itr_root(itr, &fsdata);
		if (ret)
			goto out_free_itr;
		ret = fat_itr_resolve(itr, filename, TYPE_DIR);
		if (!ret)
			*size = 0;
		goto out_free_both;
	}

	*size = FAT2CPU32(itr->dent->size);
out_free_both:
	aligned_free(fsdata.fatbuf);
out_free_itr:
	aligned_free(itr);
	return ret;
}

int file_fat_read_at(const char *filename, loff_t pos, void *buffer,
		             loff_t maxsize, loff_t *actread)
{
	fsdata_t fsdata;
	fat_itr_t *itr;
	int ret;

	itr = malloc_cache_aligned(sizeof(fat_itr_t));
	if (!itr)
		return -ENOMEM;
	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto out_free_itr;

	ret = fat_itr_resolve(itr, filename, TYPE_FILE);
	if (ret)
		goto out_free_both;

	debug("reading %s at pos %llu\n", filename, pos);

	/* For saving default max clustersize memory allocated to malloc pool */
	dir_entry_t *dentptr = itr->dent;

	ret = get_contents(&fsdata, dentptr, pos, buffer, maxsize, actread);

out_free_both:
	aligned_free(fsdata.fatbuf);
out_free_itr:
	aligned_free(itr);
	return ret;
}

int file_fat_read(const char *filename, void *buffer, int maxsize)
{
	loff_t actread;
	int ret;

	ret =  file_fat_read_at(filename, 0, buffer, maxsize, &actread);
	if (ret)
		return ret;
	else
		return actread;
}

int fat_read_file(const char *filename, void *buf, loff_t offset, loff_t len,
		          loff_t *actread)
{
	int ret;

	ret = file_fat_read_at(filename, offset, buf, len, actread);
	if (ret)
		printf("** Unable to read file %s **\n", filename);

	return ret;
}

typedef struct
{
	struct fs_dir_stream parent;
	struct fs_dirent dirent;
	fsdata_t fsdata;
	fat_itr_t itr;
} fat_dir_t;

int fat_opendir(const char *filename, struct fs_dir_stream **dirsp)
{
	fat_dir_t *dir;
	int ret;

	dir = malloc_cache_aligned(sizeof(*dir));
	if (!dir)
		return -ENOMEM;
	memset(dir, 0, sizeof(*dir));

	ret = fat_itr_root(&dir->itr, &dir->fsdata);
	if (ret)
		goto fail_free_dir;

	ret = fat_itr_resolve(&dir->itr, filename, TYPE_DIR);
	if (ret)
		goto fail_free_both;

	*dirsp = (struct fs_dir_stream *)dir;
	return 0;

fail_free_both:
	aligned_free(dir->fsdata.fatbuf);
fail_free_dir:
	aligned_free(dir);
	return ret;
}

int fat_readdir(struct fs_dir_stream *dirs, struct fs_dirent **dentp)
{
	fat_dir_t *dir = (fat_dir_t *)dirs;
	struct fs_dirent *dent = &dir->dirent;

	if (!fat_itr_next(&dir->itr))
		return -ENOENT;

	memset(dent, 0, sizeof(*dent));
	strcpy(dent->name, dir->itr.name);

	if (fat_itr_isdir(&dir->itr))
	{
		dent->type = FS_DT_DIR;
	}
	else
	{
		dent->type = FS_DT_REG;
		dent->size = FAT2CPU32(dir->itr.dent->size);
	}

	*dentp = dent;

	return 0;
}

void fat_closedir(struct fs_dir_stream *dirs)
{
	fat_dir_t *dir = (fat_dir_t *)dirs;
	aligned_free(dir->fsdata.fatbuf);
	aligned_free(dir);
}

void fat_close(void)
{
	//
}

//-------------------------------------------------------------------------------------------------
// fat_write.c
//-------------------------------------------------------------------------------------------------

static void uppercase(char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		*str = toupper(*str);
		str++;
	}
}

static int total_sector;

static int disk_write(uint32_t block, uint32_t nr_blocks, void *buf)
{
	unsigned long ret;

	if (!cur_dev)
		return -1;

	if (cur_part_info.start + block + nr_blocks >
		cur_part_info.start + total_sector)
	{
		printf("error: overflow occurs\n");
		return -1;
	}

	ret = blk_dwrite(cur_dev, cur_part_info.start + block, nr_blocks, buf);
	if (nr_blocks && ret == 0)
		return -1;

	return ret;
}

/**
 * set_name() - set short name in directory entry
 *
 * @dirent:	directory entry
 * @filename:	long file name
 */
static void set_name(dir_entry_t *dirent, const char *filename)
{
	char s_name[VFAT_MAXLEN_BYTES];
	char *period;
	int period_location, len, i, ext_num;

	if (filename == NULL)
		return;

	len = strlen(filename);
	if (len == 0)
		return;

	strncpy(s_name, filename, VFAT_MAXLEN_BYTES - 1);
	s_name[VFAT_MAXLEN_BYTES - 1] = '\0';
	uppercase(s_name, len);

	period = strchr(s_name, '.');
	if (period == NULL)
	{
		period_location = len;
		ext_num = 0;
	}
	else
	{
		period_location = period - s_name;
		ext_num = len - period_location - 1;
	}

	/* Pad spaces when the length of file name is shorter than eight */
	if (period_location < 8)
	{
		memcpy(dirent->name, s_name, period_location);
		for (i = period_location; i < 8; i++)
			dirent->name[i] = ' ';
	}
	else if (period_location == 8)
	{
		memcpy(dirent->name, s_name, period_location);
	}
	else
	{
		memcpy(dirent->name, s_name, 6);
		/*
		 * TODO: Translating two long names with the same first six
		 *       characters to the same short name is utterly wrong.
		 *       Short names must be unique.
		 */
		dirent->name[6] = '~';
		dirent->name[7] = '1';
	}

	if (ext_num < 3)
	{
		memcpy(dirent->ext, s_name + period_location + 1, ext_num);
		for (i = ext_num; i < 3; i++)
			dirent->ext[i] = ' ';
	}
	else
	{
		memcpy(dirent->ext, s_name + period_location + 1, 3);
	}

	debug("name : %s\n", dirent->name);
	debug("ext : %s\n", dirent->ext);
}

/*
 * Write fat buffer into block device
 */
static int flush_dirty_fat_buffer(fsdata_t *mydata)
{
	int getsize = FATBUFBLOCKS;
	uint32_t fatlength = mydata->fatlength;
	uint8_t *bufptr = mydata->fatbuf;
	uint32_t startblock = mydata->fatbufnum * FATBUFBLOCKS;

	debug("debug: evicting %d, dirty: %d\n", mydata->fatbufnum,
	      (int)mydata->fat_dirty);

	if ((!mydata->fat_dirty) || (mydata->fatbufnum == -1))
		return 0;

	/* Cap length if fatlength is not a multiple of FATBUFBLOCKS */
	if (startblock + getsize > fatlength)
		getsize = fatlength - startblock;

	startblock += mydata->fat_sect;

	/* Write FAT buf */
	if (disk_write(startblock, getsize, bufptr) < 0)
	{
		debug("error: writing FAT blocks\n");
		return -1;
	}

	if (mydata->fats == 2)
	{
		/* Update corresponding second FAT blocks */
		startblock += mydata->fatlength;
		if (disk_write(startblock, getsize, bufptr) < 0)
		{
			debug("error: writing second FAT blocks\n");
			return -1;
		}
	}
	mydata->fat_dirty = 0;

	return 0;
}

/*
 * Set the file name information from 'name' into 'slotptr',
 */
static int str2slot(dir_slot_t *slotptr, const char *name, int *idx)
{
	int j, end_idx = 0;

	for (j = 0; j <= 8; j += 2)
	{
		if (name[*idx] == 0x00)
		{
			slotptr->name0_4[j] = 0;
			slotptr->name0_4[j + 1] = 0;
			end_idx++;
			goto name0_4;
		}

		slotptr->name0_4[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}

	for (j = 0; j <= 10; j += 2)
	{
		if (name[*idx] == 0x00)
		{
			slotptr->name5_10[j] = 0;
			slotptr->name5_10[j + 1] = 0;
			end_idx++;
			goto name5_10;
		}

		slotptr->name5_10[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}

	for (j = 0; j <= 2; j += 2)
	{
		if (name[*idx] == 0x00)
		{
			slotptr->name11_12[j] = 0;
			slotptr->name11_12[j + 1] = 0;
			end_idx++;
			goto name11_12;
		}

		slotptr->name11_12[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}

	if (name[*idx] == 0x00)
		return 1;

	return 0;

/* Not used characters are filled with 0xff 0xff */
name0_4:
	for (; end_idx < 5; end_idx++)
	{
		slotptr->name0_4[end_idx * 2] = 0xff;
		slotptr->name0_4[end_idx * 2 + 1] = 0xff;
	}
	end_idx = 5;

name5_10:
	end_idx -= 5;
	for (; end_idx < 6; end_idx++)
	{
		slotptr->name5_10[end_idx * 2] = 0xff;
		slotptr->name5_10[end_idx * 2 + 1] = 0xff;
	}
	end_idx = 11;

name11_12:
	end_idx -= 11;
	for (; end_idx < 2; end_idx++)
	{
		slotptr->name11_12[end_idx * 2] = 0xff;
		slotptr->name11_12[end_idx * 2 + 1] = 0xff;
	}

	return 1;
}

static int new_dir_table(fat_itr_t *itr);
static int flush_dir(fat_itr_t *itr);

/*
 * Fill dir_slot entries with appropriate name, id, and attr
 * 'itr' will point to a next entry
 */
static int fill_dir_slot(fat_itr_t *itr, const char *l_name)
{
	uint8_t temp_dir_slot_buffer[MAX_LFN_SLOT * sizeof(dir_slot_t)];
	dir_slot_t *slotptr = (dir_slot_t *)temp_dir_slot_buffer;
	uint8_t counter = 0, checksum;
	int idx = 0, ret;

	/* Get short file name checksum value */
	checksum = mkcksum(itr->dent->name, itr->dent->ext);

	do
	{
		memset(slotptr, 0x00, sizeof(dir_slot_t));
		ret = str2slot(slotptr, l_name, &idx);
		slotptr->id = ++counter;
		slotptr->attr = ATTR_VFAT;
		slotptr->alias_checksum = checksum;
		slotptr++;
	} while (ret == 0);

	slotptr--;
	slotptr->id |= LAST_LONG_ENTRY_MASK;

	while (counter >= 1)
	{
		memcpy(itr->dent, slotptr, sizeof(dir_slot_t));
		slotptr--;
		counter--;

		if (itr->remaining == 0)
			flush_dir(itr);

		/* allocate a cluster for more entries */
		if (!fat_itr_next(itr))
			if (!itr->dent &&
			   (!itr->is_root || itr->fsdata->fatsize == 32) && new_dir_table(itr))
				return -1;
	}

	return 0;
}

/*
 * Set the entry at index 'entry' in a FAT (12/16/32) table.
 */
static int set_fatent_value(fsdata_t *mydata, uint32_t entry, uint32_t entry_value)
{
	uint32_t bufnum, offset, off16;
	uint16_t val1, val2;

	switch (mydata->fatsize)
	{
		case 32:
			bufnum = entry / FAT32BUFSIZE;
			offset = entry - bufnum * FAT32BUFSIZE;
			break;
		case 16:
			bufnum = entry / FAT16BUFSIZE;
			offset = entry - bufnum * FAT16BUFSIZE;
			break;
		case 12:
			bufnum = entry / FAT12BUFSIZE;
			offset = entry - bufnum * FAT12BUFSIZE;
			break;
		default:
			/* Unsupported FAT size */
			return -1;
	}

	/* Read a new block of FAT entries into the cache. */
	if (bufnum != mydata->fatbufnum)
	{
		int getsize = FATBUFBLOCKS;
		uint8_t *bufptr = mydata->fatbuf;
		uint32_t fatlength = mydata->fatlength;
		uint32_t startblock = bufnum * FATBUFBLOCKS;

		/* Cap length if fatlength is not a multiple of FATBUFBLOCKS */
		if (startblock + getsize > fatlength)
			getsize = fatlength - startblock;

		if (flush_dirty_fat_buffer(mydata) < 0)
			return -1;

		startblock += mydata->fat_sect;

		if (disk_read(startblock, getsize, bufptr) < 0)
		{
			debug("Error reading FAT blocks\n");
			return -1;
		}

		mydata->fatbufnum = bufnum;
	}

	/* Mark as dirty */
	mydata->fat_dirty = 1;

	/* Set the actual entry */
	switch (mydata->fatsize)
	{
		case 32:
			((uint32_t *) mydata->fatbuf)[offset] = cpu_to_le32(entry_value);
			break;
		case 16:
			((uint16_t *) mydata->fatbuf)[offset] = cpu_to_le16(entry_value);
			break;
		case 12:
			off16 = (offset * 3) / 4;

			switch (offset & 0x3)
			{
				case 0:
					val1 = cpu_to_le16(entry_value) & 0xfff;
					((uint16_t *)mydata->fatbuf)[off16] &= ~0xfff;
					((uint16_t *)mydata->fatbuf)[off16] |= val1;
					break;
				case 1:
					val1 = cpu_to_le16(entry_value) & 0xf;
					val2 = (cpu_to_le16(entry_value) >> 4) & 0xff;

					((uint16_t *)mydata->fatbuf)[off16] &= ~0xf000;
					((uint16_t *)mydata->fatbuf)[off16] |= (val1 << 12);

					((uint16_t *)mydata->fatbuf)[off16 + 1] &= ~0xff;
					((uint16_t *)mydata->fatbuf)[off16 + 1] |= val2;
					break;
				case 2:
					val1 = cpu_to_le16(entry_value) & 0xff;
					val2 = (cpu_to_le16(entry_value) >> 8) & 0xf;

					((uint16_t *)mydata->fatbuf)[off16] &= ~0xff00;
					((uint16_t *)mydata->fatbuf)[off16] |= (val1 << 8);

					((uint16_t *)mydata->fatbuf)[off16 + 1] &= ~0xf;
					((uint16_t *)mydata->fatbuf)[off16 + 1] |= val2;
					break;
				case 3:
					val1 = cpu_to_le16(entry_value) & 0xfff;
					((uint16_t *)mydata->fatbuf)[off16] &= ~0xfff0;
					((uint16_t *)mydata->fatbuf)[off16] |= (val1 << 4);
					break;
				default:
					break;
			}

			break;

		default:
			return -1;
	}

	return 0;
}

/*
 * Determine the next free cluster after 'entry' in a FAT (12/16/32) table
 * and link it to 'entry'. EOC marker is not set on returned entry.
 */
static uint32_t determine_fatent(fsdata_t *mydata, uint32_t entry)
{
	uint32_t next_fat, next_entry = entry + 1;

	while (1)
	{
		next_fat = get_fatent(mydata, next_entry);
		if (next_fat == 0)
		{
			/* found free entry, link to entry */
			set_fatent_value(mydata, entry, next_entry);
			break;
		}

		next_entry++;
	}

	debug("FAT%d: entry: %08x, entry_value: %04x\n",
	       mydata->fatsize, entry, next_entry);

	return next_entry;
}

/**
 * set_sectors() - write data to sectors
 *
 * Write 'size' bytes from 'buffer' into the specified sector.
 *
 * @mydata:	data to be written
 * @startsect:	sector to be written to
 * @buffer:	data to be written
 * @size:	bytes to be written (but not more than the size of a cluster)
 * Return:	0 on success, -1 otherwise
 */
static int set_sectors(fsdata_t *mydata, uint32_t startsect, uint8_t *buffer, uint32_t size)
{
	uint32_t nsects = 0;
	int ret;

	debug("startsect: %d\n", startsect);

	if ((unsigned long)buffer & (ARCH_DMA_MINALIGN - 1))
	{
		ALLOC_CACHE_ALIGN_BUFFER(uint8_t, tmpbuf, mydata->sect_size);

		debug("FAT: Misaligned buffer address (%p)\n", buffer);

		while (size >= mydata->sect_size)
		{
			memcpy(tmpbuf, buffer, mydata->sect_size);
			ret = disk_write(startsect++, 1, tmpbuf);
			if (ret != 1)
			{
				debug("Error writing data (got %d)\n", ret);
				return -1;
			}

			buffer += mydata->sect_size;
			size -= mydata->sect_size;
		}
	}
	else if (size >= mydata->sect_size)
	{
		nsects = size / mydata->sect_size;
		ret = disk_write(startsect, nsects, buffer);
		if (ret != nsects)
		{
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}

		startsect += nsects;
		buffer += nsects * mydata->sect_size;
		size -= nsects * mydata->sect_size;
	}

	if (size)
	{
		ALLOC_CACHE_ALIGN_BUFFER(uint8_t, tmpbuf, mydata->sect_size);
		/* Do not leak content of stack */
		memset(tmpbuf, 0, mydata->sect_size);
		memcpy(tmpbuf, buffer, size);
		ret = disk_write(startsect, 1, tmpbuf);
		if (ret != 1)
		{
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}
	}

	return 0;
}

/**
 * set_cluster() - write data to cluster
 *
 * Write 'size' bytes from 'buffer' into the specified cluster.
 *
 * @mydata:	data to be written
 * @clustnum:	cluster to be written to
 * @buffer:	data to be written
 * @size:	bytes to be written (but not more than the size of a cluster)
 * Return:	0 on success, -1 otherwise
 */
static int set_cluster(fsdata_t *mydata, uint32_t clustnum, uint8_t *buffer, uint32_t size)
{
	return set_sectors(mydata, clust_to_sect(mydata, clustnum), buffer, size);
}

static int flush_dir(fat_itr_t *itr)
{
	fsdata_t *mydata = itr->fsdata;
	uint32_t startsect, sect_offset, nsects;

	if (!itr->is_root || mydata->fatsize == 32)
		return set_cluster(mydata, itr->clust, itr->block,
				           mydata->clust_size * mydata->sect_size);

	sect_offset = itr->clust * mydata->clust_size;
	startsect = mydata->rootdir_sect + sect_offset;
	/* do not write past the end of rootdir */
	nsects = min_t(uint32_t, mydata->clust_size,
		           mydata->rootdir_size - sect_offset);

	return set_sectors(mydata, startsect, itr->block, nsects * mydata->sect_size);
}

static uint8_t tmpbuf_cluster[MAX_CLUSTSIZE] __attribute__((aligned(ARCH_DMA_MINALIGN)));

/*
 * Read and modify data on existing and consecutive cluster blocks
 */
static int get_set_cluster(fsdata_t *mydata, uint32_t clustnum, loff_t pos,
                           uint8_t *buffer, loff_t size, loff_t *gotsize)
{
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;
	uint32_t startsect;
	loff_t wsize;
	int clustcount, i, ret;

	*gotsize = 0;
	if (!size)
		return 0;

	assert(pos < bytesperclust);
	startsect = clust_to_sect(mydata, clustnum);

	debug("clustnum: %d, startsect: %d, pos: %lld\n",
	      clustnum, startsect, pos);

	/* partial write at beginning */
	if (pos)
	{
		wsize = min(bytesperclust - pos, size);
		ret = disk_read(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size)
		{
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}

		memcpy(tmpbuf_cluster + pos, buffer, wsize);
		ret = disk_write(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size)
		{
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}

		size -= wsize;
		buffer += wsize;
		*gotsize += wsize;

		startsect += mydata->clust_size;

		if (!size)
			return 0;
	}

	/* full-cluster write */
	if (size >= bytesperclust)
	{
		clustcount = size / bytesperclust;  // XXX lldiv(size, bytesperclust);

		if (!((unsigned long)buffer & (ARCH_DMA_MINALIGN - 1)))
		{
			wsize = clustcount * bytesperclust;
			ret = disk_write(startsect,
					         clustcount * mydata->clust_size,
							 buffer);
			if (ret != clustcount * mydata->clust_size)
			{
				debug("Error writing data (got %d)\n", ret);
				return -1;
			}

			size -= wsize;
			buffer += wsize;
			*gotsize += wsize;

			startsect += clustcount * mydata->clust_size;
		}
		else
		{
			for (i = 0; i < clustcount; i++)
			{
				memcpy(tmpbuf_cluster, buffer, bytesperclust);
				ret = disk_write(startsect,
						         mydata->clust_size,
						         tmpbuf_cluster);
				if (ret != mydata->clust_size)
				{
					debug("Error writing data (got %d)\n", ret);
					return -1;
				}

				size -= bytesperclust;
				buffer += bytesperclust;
				*gotsize += bytesperclust;

				startsect += mydata->clust_size;
			}
		}
	}

	/* partial write at end */
	if (size)
	{
		wsize = size;
		ret = disk_read(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size)
		{
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}

		memcpy(tmpbuf_cluster, buffer, wsize);
		ret = disk_write(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size)
		{
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}

		size -= wsize;
		buffer += wsize;
		*gotsize += wsize;
	}

	assert(!size);

	return 0;
}

/*
 * Find the first empty cluster
 */
static int find_empty_cluster(fsdata_t *mydata)
{
	uint32_t fat_val, entry = 3;

	while (1)
	{
		fat_val = get_fatent(mydata, entry);
		if (fat_val == 0)
			break;
		entry++;
	}

	return entry;
}

/*
 * Allocate a cluster for additional directory entries
 */
static int new_dir_table(fat_itr_t *itr)
{
	fsdata_t *mydata = itr->fsdata;
	int dir_newclust = 0;
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;

	dir_newclust = find_empty_cluster(mydata);
	set_fatent_value(mydata, itr->clust, dir_newclust);
	if (mydata->fatsize == 32)
		set_fatent_value(mydata, dir_newclust, 0xffffff8);
	else if (mydata->fatsize == 16)
		set_fatent_value(mydata, dir_newclust, 0xfff8);
	else if (mydata->fatsize == 12)
		set_fatent_value(mydata, dir_newclust, 0xff8);

	itr->clust = dir_newclust;
	itr->next_clust = dir_newclust;

	if (flush_dirty_fat_buffer(mydata) < 0)
		return -1;

	memset(itr->block, 0x00, bytesperclust);

	itr->dent = (dir_entry_t *)itr->block;
	itr->last_cluster = 1;
	itr->remaining = bytesperclust / sizeof(dir_entry_t) - 1;

	return 0;
}

/*
 * Set empty cluster from 'entry' to the end of a file
 */
static int clear_fatent(fsdata_t *mydata, uint32_t entry)
{
	uint32_t fat_val;

	while (!CHECK_CLUST(entry, mydata->fatsize))
	{
		fat_val = get_fatent(mydata, entry);
		if (fat_val != 0)
			set_fatent_value(mydata, entry, 0);
		else
			break;

		entry = fat_val;
	}

	/* Flush fat buffer */
	if (flush_dirty_fat_buffer(mydata) < 0)
		return -1;

	return 0;
}

/*
 * Set start cluster in directory entry
 */
static void set_start_cluster(const fsdata_t *mydata, dir_entry_t *dentptr,
			                  uint32_t start_cluster)
{
	if (mydata->fatsize == 32)
		dentptr->starthi = cpu_to_le16((start_cluster & 0xffff0000) >> 16);
	dentptr->start = cpu_to_le16(start_cluster & 0xffff);
}

/*
 * Check whether adding a file makes the file system to
 * exceed the size of the block device
 * Return -1 when overflow occurs, otherwise return 0
 */
static int check_overflow(fsdata_t *mydata, uint32_t clustnum, loff_t size)
{
	uint32_t startsect, sect_num, offset;

	if (clustnum > 0)
		startsect = clust_to_sect(mydata, clustnum);
	else
		startsect = mydata->rootdir_sect;

	sect_num = div_u64_rem(size, mydata->sect_size, &offset);

	if (offset != 0)
		sect_num++;

	if (startsect + sect_num > total_sector)
		return -1;
	return 0;
}

/*
 * Write at most 'maxsize' bytes from 'buffer' into
 * the file associated with 'dentptr'
 * Update the number of bytes written in *gotsize and return 0
 * or return -1 on fatal errors.
 */
static int set_contents(fsdata_t *mydata, dir_entry_t *dentptr, loff_t pos,
                        uint8_t *buffer, loff_t maxsize, loff_t *gotsize)
{
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;
	uint32_t curclust = START(dentptr);
	uint32_t endclust = 0, newclust = 0;
	uint64_t cur_pos, filesize;
	loff_t offset, actsize, wsize;

	*gotsize = 0;
	filesize = pos + maxsize;

	debug("%llu bytes\n", filesize);

	if (!filesize)
	{
		if (!curclust)
			return 0;
		if (!CHECK_CLUST(curclust, mydata->fatsize) ||
		    IS_LAST_CLUST(curclust, mydata->fatsize))
		{
			clear_fatent(mydata, curclust);
			set_start_cluster(mydata, dentptr, 0);
			return 0;
		}
		debug("curclust: 0x%x\n", curclust);
		debug("Invalid FAT entry\n");
		return -1;
	}

	if (!curclust)
	{
		assert(pos == 0);
		goto set_clusters;
	}

	/* go to cluster at pos */
	cur_pos = bytesperclust;
	while (1)
	{
		if (pos <= cur_pos)
			break;
		if (IS_LAST_CLUST(curclust, mydata->fatsize))
			break;

		newclust = get_fatent(mydata, curclust);
		if (!IS_LAST_CLUST(newclust, mydata->fatsize) &&
		    CHECK_CLUST(newclust, mydata->fatsize))
		{
			debug("curclust: 0x%x\n", curclust);
			debug("Invalid FAT entry\n");
			return -1;
		}

		cur_pos += bytesperclust;
		curclust = newclust;
	}

	if (IS_LAST_CLUST(curclust, mydata->fatsize))
	{
		assert(pos == cur_pos);
		goto set_clusters;
	}

	assert(pos < cur_pos);
	cur_pos -= bytesperclust;

	/* overwrite */
	assert(IS_LAST_CLUST(curclust, mydata->fatsize) ||
	       !CHECK_CLUST(curclust, mydata->fatsize));

	while (1)
	{
		/* search for allocated consecutive clusters */
		actsize = bytesperclust;
		endclust = curclust;
		while (1)
		{
			if (filesize <= (cur_pos + actsize))
				break;

			newclust = get_fatent(mydata, endclust);

			if (newclust != endclust + 1)
				break;
			if (IS_LAST_CLUST(newclust, mydata->fatsize))
				break;
			if (CHECK_CLUST(newclust, mydata->fatsize))
			{
				debug("curclust: 0x%x\n", curclust);
				debug("Invalid FAT entry\n");
				return -1;
			}

			actsize += bytesperclust;
			endclust = newclust;
		}

		/* overwrite to <curclust..endclust> */
		if (pos < cur_pos)
			offset = 0;
		else
			offset = pos - cur_pos;
		wsize = min_t(unsigned long long, actsize, filesize - cur_pos);
		wsize -= offset;

		if (get_set_cluster(mydata, curclust, offset, buffer, wsize, &actsize))
		{
			printf("Error get-and-setting cluster\n");
			return -1;
		}

		buffer += wsize;
		*gotsize += wsize;
		cur_pos += offset + wsize;

		if (filesize <= cur_pos)
			break;

		if (IS_LAST_CLUST(newclust, mydata->fatsize))
			/* no more clusters */
			break;

		curclust = newclust;
	}

	if (filesize <= cur_pos)
	{
		/* no more write */
		newclust = get_fatent(mydata, endclust);
		if (!IS_LAST_CLUST(newclust, mydata->fatsize))
		{
			/* truncate the rest */
			clear_fatent(mydata, newclust);

			/* Mark end of file in FAT */
			if (mydata->fatsize == 12)
				newclust = 0xfff;
			else if (mydata->fatsize == 16)
				newclust = 0xffff;
			else if (mydata->fatsize == 32)
				newclust = 0xfffffff;
			set_fatent_value(mydata, endclust, newclust);
		}

		return 0;
	}

	curclust = endclust;
	filesize -= cur_pos;
	assert(!do_div(cur_pos, bytesperclust));

set_clusters:
	/* allocate and write */
	assert(!pos);

	/* Assure that curclust is valid */
	if (!curclust)
	{
		curclust = find_empty_cluster(mydata);
		set_start_cluster(mydata, dentptr, curclust);
	}
	else
	{
		newclust = get_fatent(mydata, curclust);

		if (IS_LAST_CLUST(newclust, mydata->fatsize))
		{
			newclust = determine_fatent(mydata, curclust);
			set_fatent_value(mydata, curclust, newclust);
			curclust = newclust;
		}
		else
		{
			debug("error: something wrong\n");
			return -1;
		}
	}

	/* TODO: already partially written */
	if (check_overflow(mydata, curclust, filesize)) {
		printf("Error: no space left: %llu\n", filesize);
		return -1;
	}

	actsize = bytesperclust;
	endclust = curclust;
	do
	{
		/* search for consecutive clusters */
		while (actsize < filesize)
		{
			newclust = determine_fatent(mydata, endclust);

			if ((newclust - 1) != endclust)
				/* write to <curclust..endclust> */
				goto getit;

			if (CHECK_CLUST(newclust, mydata->fatsize))
			{
				debug("newclust: 0x%x\n", newclust);
				debug("Invalid FAT entry\n");
				return 0;
			}
			endclust = newclust;
			actsize += bytesperclust;
		}

		/* set remaining bytes */
		actsize = filesize;
		if (set_cluster(mydata, curclust, buffer, (uint32_t)actsize) != 0)
		{
			debug("error: writing cluster\n");
			return -1;
		}
		*gotsize += actsize;

		/* Mark end of file in FAT */
		if (mydata->fatsize == 12)
			newclust = 0xfff;
		else if (mydata->fatsize == 16)
			newclust = 0xffff;
		else if (mydata->fatsize == 32)
			newclust = 0xfffffff;
		set_fatent_value(mydata, endclust, newclust);

		return 0;

getit:
		if (set_cluster(mydata, curclust, buffer, (uint32_t)actsize) != 0)
		{
			debug("error: writing cluster\n");
			return -1;
		}
		*gotsize += actsize;
		filesize -= actsize;
		buffer += actsize;

		if (CHECK_CLUST(newclust, mydata->fatsize))
		{
			debug("newclust: 0x%x\n", newclust);
			debug("Invalid FAT entry\n");
			return 0;
		}
		actsize = bytesperclust;
		curclust = endclust = newclust;
	} while (1);

	return 0;
}

/*
 * Fill dir_entry
 */
static void fill_dentry(fsdata_t *mydata, dir_entry_t *dentptr, const char *filename,
						uint32_t start_cluster, uint32_t size, uint8_t attr)
{
	set_start_cluster(mydata, dentptr, start_cluster);
	dentptr->size = cpu_to_le32(size);

	dentptr->attr = attr;

	set_name(dentptr, filename);
}

/*
 * Find a directory entry based on filename or start cluster number
 * If the directory entry is not found,
 * the new position for writing a directory entry will be returned
 */
static dir_entry_t *find_directory_entry(fat_itr_t *itr, char *filename)
{
	int match = 0;

	while (fat_itr_next(itr))
	{
		/* check both long and short name: */
		if (!strcasecmp(filename, itr->name))
			match = 1;
		else if (itr->name != itr->s_name &&
			 !strcasecmp(filename, itr->s_name))
			match = 1;

		if (!match)
			continue;

		if (itr->dent->name[0] == '\0')
			return NULL;
		else
			return itr->dent;
	}

	/* allocate a cluster for more entries */
	if (!itr->dent &&
	   (!itr->is_root || itr->fsdata->fatsize == 32) &&
	    new_dir_table(itr))
		/* indicate that allocating dent failed */
		itr->dent = NULL;

	return NULL;
}

static int split_filename(char *filename, char **dirname, char **basename)
{
	char *p, *last_slash, *last_slash_cont;

again:
	p = filename;
	last_slash = NULL;
	last_slash_cont = NULL;
	while (*p)
	{
		if (ISDIRDELIM(*p))
		{
			last_slash = p;
			last_slash_cont = p;
			/* continuous slashes */
			while (ISDIRDELIM(*p))
				last_slash_cont = p++;
			if (!*p)
				break;
		}
		p++;
	}

	if (last_slash)
	{
		if (last_slash_cont == (filename + strlen(filename) - 1))
		{
			/* remove trailing slashes */
			*last_slash = '\0';
			goto again;
		}

		if (last_slash == filename)
		{
			/* avoid ""(null) directory */
			*dirname = "/";
		}
		else
		{
			*last_slash = '\0';
			*dirname = filename;
		}

		*last_slash_cont = '\0';
		*basename = last_slash_cont + 1;
	}
	else
	{
		*dirname = "/"; /* root by default */
		*basename = filename;
	}

	return 0;
}

/**
 * normalize_longname() - check long file name and convert to lower case
 *
 * We assume here that the FAT file system is using an 8bit code page.
 * Linux typically uses CP437, EDK2 assumes CP1250.
 *
 * @l_filename:	preallocated buffer receiving the normalized name
 * @filename:	filename to normalize
 * Return:	0 on success, -1 on failure
 */
static int normalize_longname(char *l_filename, const char *filename)
{
	const char *p, illegal[] = "<>:\"/\\|?*";

	if (strlen(filename) >= VFAT_MAXLEN_BYTES)
		return -1;

	for (p = filename; *p; ++p)
	{
		if ((unsigned char)*p < 0x20)
			return -1;
		if (strchr(illegal, *p))
			return -1;
	}

	strcpy(l_filename, filename);
	downcase(l_filename, VFAT_MAXLEN_BYTES);

	return 0;
}

int file_fat_write_at(const char *filename, loff_t pos, void *buffer,
		              loff_t size, loff_t *actwrite)
{
	dir_entry_t *retdent;
	fsdata_t datablock = { .fatbuf = NULL, };
	fsdata_t *mydata = &datablock;
	fat_itr_t *itr = NULL;
	int ret = -1;
	char filename_copy[MAX_PATH], *parent, *basename;
	char l_filename[VFAT_MAXLEN_BYTES];

	debug("writing %s\n", filename);

/*
	filename_copy = strdup(filename);
	if (!filename_copy)
		return -ENOMEM;
 */
    strncpy(filename_copy, filename, MAX_PATH-1);
	split_filename(filename_copy, &parent, &basename);
	if (!strlen(basename))
	{
		ret = -EINVAL;
		goto exit;
	}

	filename = basename;
	if (normalize_longname(l_filename, filename))
	{
		printf("FAT: illegal filename (%s)\n", filename);
		ret = -EINVAL;
		goto exit;
	}

	itr = malloc_cache_aligned(sizeof(fat_itr_t));
	if (!itr)
	{
		ret = -ENOMEM;
		goto exit;
	}

	ret = fat_itr_root(itr, &datablock);
	if (ret)
		goto exit;

	total_sector = datablock.total_sect;

	ret = fat_itr_resolve(itr, parent, TYPE_DIR);
	if (ret) {
		printf("%s: doesn't exist (%d)\n", parent, ret);
		goto exit;
	}

	retdent = find_directory_entry(itr, l_filename);

	if (retdent)
	{
		if (fat_itr_isdir(itr))
		{
			ret = -EISDIR;
			goto exit;
		}

		/* A file exists */
		if (pos == -1)
			/* Append to the end */
			pos = FAT2CPU32(retdent->size);
		if (pos > retdent->size)
		{
			/* No hole allowed */
			ret = -EINVAL;
			goto exit;
		}

		/* Update file size in a directory entry */
		retdent->size = cpu_to_le32(pos + size);
	}
	else
	{
		/* Create a new file */

		if (itr->is_root)
		{
			/* root dir cannot have "." or ".." */
			if (!strcmp(l_filename, ".") ||
			    !strcmp(l_filename, ".."))
			{
				ret = -EINVAL;
				goto exit;
			}
		}

		if (!itr->dent)
		{
			printf("Error: allocating new dir entry\n");
			ret = -EIO;
			goto exit;
		}

		if (pos)
		{
			/* No hole allowed */
			ret = -EINVAL;
			goto exit;
		}

		memset(itr->dent, 0, sizeof(*itr->dent));

		/* Calculate checksum for short name */
		set_name(itr->dent, filename);

		/* Set long name entries */
		if (fill_dir_slot(itr, filename))
		{
			ret = -EIO;
			goto exit;
		}

		/* Set short name entry */
		fill_dentry(itr->fsdata, itr->dent, filename, 0, size, 0x20);

		retdent = itr->dent;
	}

	ret = set_contents(mydata, retdent, pos, buffer, size, actwrite);
	if (ret < 0)
	{
		printf("Error: writing contents\n");
		ret = -EIO;
		goto exit;
	}
	debug("attempt to write 0x%llx bytes\n", *actwrite);

	/* Flush fat buffer */
	ret = flush_dirty_fat_buffer(mydata);
	if (ret)
	{
		printf("Error: flush fat buffer\n");
		ret = -EIO;
		goto exit;
	}

	/* Write directory table to device */
	ret = flush_dir(itr);
	if (ret)
	{
		printf("Error: writing directory entry\n");
		ret = -EIO;
	}

exit:
	aligned_free(mydata->fatbuf);
	aligned_free(itr);
	return ret;
}

int file_fat_write(const char *filename, void *buffer, loff_t offset,
		           loff_t maxsize, loff_t *actwrite)
{
	return file_fat_write_at(filename, offset, buffer, maxsize, actwrite);
}

static int fat_dir_entries(fat_itr_t *itr)
{
	fat_itr_t *dirs;
	fsdata_t fsdata = { .fatbuf = NULL, }, *mydata = &fsdata;
						/* for FATBUFSIZE */
	int count;

	dirs = malloc_cache_aligned(sizeof(fat_itr_t));
	if (!dirs)
	{
		debug("Error: allocating memory\n");
		count = -ENOMEM;
		goto exit;
	}

	/* duplicate fsdata */
	fat_itr_child(dirs, itr);
	fsdata = *dirs->fsdata;

	/* allocate local fat buffer */
	fsdata.fatbuf = malloc_cache_aligned(FATBUFSIZE);
	if (!fsdata.fatbuf)
	{
		debug("Error: allocating memory\n");
		count = -ENOMEM;
		goto exit;
	}

	fsdata.fatbufnum = -1;
	dirs->fsdata = &fsdata;

	for (count = 0; fat_itr_next(dirs); count++)
		;

exit:
	aligned_free(fsdata.fatbuf);
	aligned_free(dirs);
	return count;
}

static int delete_dentry(fat_itr_t *itr)
{
	fsdata_t *mydata = itr->fsdata;
	dir_entry_t *dentptr = itr->dent;

	/* free cluster blocks */
	clear_fatent(mydata, START(dentptr));
	if (flush_dirty_fat_buffer(mydata) < 0)
	{
		printf("Error: flush fat buffer\n");
		return -EIO;
	}

	/*
	 * update a directory entry
	 * TODO:
	 *  - long file name support
	 *  - find and mark the "new" first invalid entry as name[0]=0x00
	 */
	memset(dentptr, 0, sizeof(*dentptr));
	dentptr->name[0] = 0xe5;

	if (flush_dir(itr))
	{
		printf("error: writing directory entry\n");
		return -EIO;
	}

	return 0;
}

int fat_unlink(const char *filename)
{
	fsdata_t fsdata = { .fatbuf = NULL, };
	fat_itr_t *itr = NULL;
	int n_entries, ret;
	char filename_copy[MAX_PATH], *dirname, *basename;

/*
	filename_copy = strdup(filename);
	if (!filename_copy)
	{
		printf("Error: allocating memory\n");
		ret = -ENOMEM;
		goto exit;
	}
 */
    strncpy(filename_copy, filename, MAX_PATH - 1);
	split_filename(filename_copy, &dirname, &basename);

	if (!strcmp(dirname, "/") && !strcmp(basename, ""))
	{
		printf("Error: cannot remove root\n");
		ret = -EINVAL;
		goto exit;
	}

	itr = malloc_cache_aligned(sizeof(fat_itr_t));
	if (!itr)
	{
		printf("Error: allocating memory\n");
		ret = -ENOMEM;
		goto exit;
	}

	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto exit;

	total_sector = fsdata.total_sect;

	ret = fat_itr_resolve(itr, dirname, TYPE_DIR);
	if (ret)
	{
		printf("%s: doesn't exist (%d)\n", dirname, ret);
		ret = -ENOENT;
		goto exit;
	}

	if (!find_directory_entry(itr, basename))
	{
		printf("%s: doesn't exist\n", basename);
		ret = -ENOENT;
		goto exit;
	}

	if (fat_itr_isdir(itr))
	{
		n_entries = fat_dir_entries(itr);
		if (n_entries < 0)
		{
			ret = n_entries;
			goto exit;
		}

		if (n_entries > 2)
		{
			printf("Error: directory is not empty: %d\n",
			       n_entries);
			ret = -EINVAL;
			goto exit;
		}
	}

	ret = delete_dentry(itr);

exit:
	aligned_free(fsdata.fatbuf);
	aligned_free(itr);

	return ret;
}

int fat_mkdir(const char *new_dirname)
{
	dir_entry_t *retdent;
	fsdata_t datablock = { .fatbuf = NULL, };
	fsdata_t *mydata = &datablock;
	fat_itr_t *itr = NULL;
	char dirname_copy[MAX_PATH], *parent, *dirname;
	char l_dirname[VFAT_MAXLEN_BYTES];
	int ret = -1;
	loff_t actwrite;
	unsigned int bytesperclust;
	dir_entry_t *dotdent = NULL;

/*
	dirname_copy = strdup(new_dirname);
	if (!dirname_copy)
		goto exit;
 */
    strncpy(dirname_copy, new_dirname, MAX_PATH);
    
	split_filename(dirname_copy, &parent, &dirname);
	if (!strlen(dirname))
	{
		ret = -EINVAL;
		goto exit;
	}

	if (normalize_longname(l_dirname, dirname))
	{
		printf("FAT: illegal filename (%s)\n", dirname);
		ret = -EINVAL;
		goto exit;
	}

	itr = malloc_cache_aligned(sizeof(fat_itr_t));
	if (!itr)
	{
		ret = -ENOMEM;
		goto exit;
	}

	ret = fat_itr_root(itr, &datablock);
	if (ret)
		goto exit;

	total_sector = datablock.total_sect;

	ret = fat_itr_resolve(itr, parent, TYPE_DIR);
	if (ret)
	{
		printf("%s: doesn't exist (%d)\n", parent, ret);
		goto exit;
	}

	retdent = find_directory_entry(itr, l_dirname);

	if (retdent)
	{
		printf("%s: already exists\n", l_dirname);
		ret = -EEXIST;
		goto exit;
	}
	else
	{
		if (itr->is_root)
		{
			/* root dir cannot have "." or ".." */
			if (!strcmp(l_dirname, ".") ||
			    !strcmp(l_dirname, ".."))
			{
				ret = -EINVAL;
				goto exit;
			}
		}

		if (!itr->dent)
		{
			printf("Error: allocating new dir entry\n");
			ret = -EIO;
			goto exit;
		}

		memset(itr->dent, 0, sizeof(*itr->dent));

		/* Set short name to set alias checksum field in dir_slot */
		set_name(itr->dent, dirname);
		fill_dir_slot(itr, dirname);

		/* Set attribute as archive for regular file */
		fill_dentry(itr->fsdata, itr->dent, dirname, 0, 0, ATTR_DIR | ATTR_ARCH);

		retdent = itr->dent;
	}

	/* Default entries */
	bytesperclust = mydata->clust_size * mydata->sect_size;
	dotdent = malloc_cache_aligned(bytesperclust);
	if (!dotdent)
	{
		ret = -ENOMEM;
		goto exit;
	}
	memset(dotdent, 0, bytesperclust);

	memcpy(dotdent[0].name, ".       ", 8);
	memcpy(dotdent[0].ext, "   ", 3);
	dotdent[0].attr = ATTR_DIR | ATTR_ARCH;

	memcpy(dotdent[1].name, "..      ", 8);
	memcpy(dotdent[1].ext, "   ", 3);
	dotdent[1].attr = ATTR_DIR | ATTR_ARCH;
	set_start_cluster(mydata, &dotdent[1], itr->start_clust);

	ret = set_contents(mydata, retdent, 0, (uint8_t *)dotdent,
			           bytesperclust, &actwrite);
	if (ret < 0)
	{
		printf("Error: writing contents\n");
		goto exit;
	}
	/* Write twice for "." */
	set_start_cluster(mydata, &dotdent[0], START(retdent));
	ret = set_contents(mydata, retdent, 0, (uint8_t *)dotdent,
			           bytesperclust, &actwrite);
	if (ret < 0)
	{
		printf("Error: writing contents\n");
		goto exit;
	}

	/* Flush fat buffer */
	ret = flush_dirty_fat_buffer(mydata);
	if (ret)
	{
		printf("Error: flush fat buffer\n");
		goto exit;
	}

	/* Write directory table to device */
	ret = flush_dir(itr);
	if (ret)
		printf("Error: writing directory entry\n");

exit:
	aligned_free(mydata->fatbuf);
	aligned_free(itr);
	aligned_free(dotdent);
	return ret;
}

//-------------------------------------------------------------------------------------------------
// 打印 fat 信息
//-------------------------------------------------------------------------------------------------

void fat_print_info(fsdata_t *fsdata)
{
    printf("\r\nFAT INFO:\r\n");
    printf("  Current FAT buffer:  0x%08X\r\n", (int)fsdata->fatbuf);
    printf("  Size of FAT in bits: %i\r\n", fsdata->fatsize);
    printf("  Length of FAT in sectors: %i\r\n", (int)fsdata->fatlength);
    printf("  Starting sector of the FAT: %i\r\n", (int)fsdata->fat_sect);
    printf("  Set if fatbuf has been modified: %i\r\n", (int)fsdata->fat_dirty);
    printf("  Start sector of root directory: %i\r\n", (int)fsdata->rootdir_sect);
    printf("  Size of sectors in bytes: %i\r\n", (int)fsdata->sect_size);
    printf("  Size of clusters in sectors: %i\r\n", (int)fsdata->clust_size);
    printf("  The sector of the first cluster, can be negative: %i\r\n", (int)fsdata->data_begin);
    printf("  Used by get_fatent, init to -1: %i\r\n", (int)fsdata->fatbufnum);
    printf("  Size of root dir for non-FAT32: %i\r\n", (int)fsdata->rootdir_size);
    printf("  First cluster of root dir for FAT32: %i\r\n", (int)fsdata->root_cluster);
    printf("  Number of sectors: %i\r\n", (int)fsdata->total_sect);
    printf("  Number of FATs: %i\r\n", (int)fsdata->fats);
    printf("\r\n");
}

/*
 * @@ End
 */


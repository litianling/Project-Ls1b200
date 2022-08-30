/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * R/O (V)FAT 12/16/32 filesystem implementation by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to u-boot
 */

#ifndef _FAT_H_
#define _FAT_H_

//#include <asm/byteorder.h>
//#include <fs.h>

//typedef unsigned int loff_t;

struct disk_partition;

/* Maximum Long File Name length supported here is 128 UTF-16 code units
 */
#define VFAT_MAXLEN_BYTES	256 	/* Maximum LFN buffer in bytes */
#define VFAT_MAXSEQ			9   	/* Up to 9 of 13 2-byte UTF-16 entries */
#define PREFETCH_BLOCKS		2

#define MAX_CLUSTSIZE		32768   // TODO FAT32 max?

#define DIRENTSPERBLOCK		(mydata->sect_size / sizeof(dir_entry))
#define DIRENTSPERCLUST		((mydata->clust_size * mydata->sect_size) / sizeof(dir_entry))

#define FATBUFBLOCKS        16
#define FATBUFSIZE			(mydata->sect_size * FATBUFBLOCKS)
#define FAT12BUFSIZE		((FATBUFSIZE*2)/3)
#define FAT16BUFSIZE		(FATBUFSIZE/2)
#define FAT32BUFSIZE		(FATBUFSIZE/4)

/* Maximum number of entry for long file name according to spec
 */
#define MAX_LFN_SLOT		20

/* Filesystem identifiers */
#define FAT12_SIGN			"FAT12   "
#define FAT16_SIGN			"FAT16   "
#define FAT32_SIGN			"FAT32   "
#define SIGNLEN				8

/* File attributes */
#define ATTR_RO				1
#define ATTR_HIDDEN			2
#define ATTR_SYS			4
#define ATTR_VOLUME			8
#define ATTR_DIR			16
#define ATTR_ARCH			32

#define ATTR_VFAT			(ATTR_RO | ATTR_HIDDEN | ATTR_SYS | ATTR_VOLUME)

#define DELETED_FLAG		((char)0xe5) /* Marks deleted files when in name[0] */
#define aRING				0x05	     /* Used as special character in name[0] */

/*
 * Indicates that the entry is the last long entry in a set of long
 * dir entries
 */
#define LAST_LONG_ENTRY_MASK	0x40

#define ISDIRDELIM(c)		((c) == '/' || (c) == '\\')

#define FSTYPE_NONE			(-1)

/*
#if defined(__linux__) && defined(__KERNEL__)
#define FAT2CPU16			le16_to_cpu
#define FAT2CPU32			le32_to_cpu
#else
#if __LITTLE_ENDIAN
 */
#define FAT2CPU16(x)		(x)
#define FAT2CPU32(x)		(x)
/*
#else
#define FAT2CPU16(x)		((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
#define FAT2CPU32(x)		((((x) & 0x000000ff) << 24)  |	\
			 	 	 	 	 (((x) & 0x0000ff00) << 8)  |	\
							 (((x) & 0x00ff0000) >> 8)  |	\
							 (((x) & 0xff000000) >> 24))
#endif
#endif
 */
 
#define START(dent)					(FAT2CPU16((dent)->start) + (mydata->fatsize != 32 ? \
									0 : (FAT2CPU16((dent)->starthi) << 16)))
#define IS_LAST_CLUST(x, fatsize) 	((x) >= ((fatsize) != 32 ? \
									((fatsize) != 16 ? 0xff8 : 0xfff8) : 0xffffff8))
#define CHECK_CLUST(x, fatsize) 	((x) <= 1 || (x) >= ((fatsize) != 32 ? \
									((fatsize) != 16 ? 0xff0 : 0xfff0) : 0xffffff0))

typedef struct boot_sector
{
	uint8_t	 ignored[3];		/* Bootstrap code */
	char	 system_id[8];		/* Name of fs */
	uint8_t	 sector_size[2];	/* Bytes/sector */
	uint8_t	 cluster_size;		/* Sectors/cluster */
	uint16_t reserved;			/* Number of reserved sectors */
	uint8_t	 fats;				/* Number of FATs */
	uint8_t	 dir_entries[2];	/* Number of root directory entries */
	uint8_t	 sectors[2];		/* Number of sectors */
	uint8_t	 media;				/* Media code */
	uint16_t fat_length;		/* Sectors/FAT */
	uint16_t secs_track;		/* Sectors/track */
	uint16_t heads;				/* Number of heads */
	uint32_t hidden;			/* Number of hidden sectors */
	uint32_t total_sect;		/* Number of sectors (if sectors == 0) */

	/* FAT32 only */
	uint32_t fat32_length;		/* Sectors/FAT */
	uint16_t flags;				/* Bit 8: fat mirroring, low 4: active fat */
	uint8_t	 version[2];		/* Filesystem version */
	uint32_t root_cluster;		/* First cluster in root directory */
	uint16_t info_sector;		/* Filesystem info sector */
	uint16_t backup_boot;		/* Backup boot sector */
	uint16_t reserved2[6];		/* Unused */
} boot_sector_t;

typedef struct volume_info
{
	uint8_t drive_number;		/* BIOS drive number */
	uint8_t reserved;			/* Unused */
	uint8_t ext_boot_sign;		/* 0x29 if fields below exist (DOS 3.3+) */
	uint8_t volume_id[4];		/* Volume ID number */
	char    volume_label[11];	/* Volume label */
	char    fs_type[8];			/* Typically FAT12, FAT16, or FAT32 */
	/* Boot code comes next, all but 2 bytes to fill up sector */
	/* Boot sign comes last, 2 bytes */
} volume_info_t;

/* see dir_entry::lcase: */
#define CASE_LOWER_BASE		8	/* base (name) is lower case */
#define CASE_LOWER_EXT		16	/* extension is lower case */

typedef struct dir_entry
{
	char	 name[8],ext[3];	/* Name and extension */
	uint8_t  attr;				/* Attribute bits */
	uint8_t  lcase;				/* Case for name and ext (CASE_LOWER_x) */
	uint8_t	 ctime_ms;			/* Creation time, milliseconds */
	uint16_t ctime;				/* Creation time */
	uint16_t cdate;				/* Creation date */
	uint16_t adate;				/* Last access date */
	uint16_t starthi;			/* High 16 bits of cluster in FAT32 */
	uint16_t time,date,start;	/* Time, date and first cluster */
	uint32_t size;				/* File size in bytes */
} dir_entry_t;

typedef struct dir_slot
{
	uint8_t	 id;				/* Sequence number for slot */
	uint8_t	 name0_4[10];		/* First 5 characters in name */
	uint8_t	 attr;				/* Attribute byte */
	uint8_t  reserved;			/* Unused */
	uint8_t	 alias_checksum;	/* Checksum for 8.3 alias */
	uint8_t	 name5_10[12];		/* 6 more characters in name */
	uint16_t start;				/* Unused */
	uint8_t	 name11_12[4];		/* Last 2 characters in name */
} dir_slot_t;

/*
 * Private filesystem parameters
 *
 * Note: FAT buffer has to be 32 bit aligned
 * (see FAT32 accesses)
 */
typedef struct
{
	uint8_t	*fatbuf;			/* Current FAT buffer */
	int	     fatsize;			/* Size of FAT in bits */
	uint32_t fatlength;			/* Length of FAT in sectors */
	uint16_t fat_sect;			/* Starting sector of the FAT */
	uint8_t	 fat_dirty;      	/* Set if fatbuf has been modified */
	uint32_t rootdir_sect;		/* Start sector of root directory */
	uint16_t sect_size;			/* Size of sectors in bytes */
	uint16_t clust_size;		/* Size of clusters in sectors */
	int	     data_begin;		/* The sector of the first cluster, can be negative */
	int	     fatbufnum;			/* Used by get_fatent, init to -1 */
	int	     rootdir_size;		/* Size of root dir for non-FAT32 */
	uint32_t root_cluster;		/* First cluster of root dir for FAT32 */
	unsigned total_sect;		/* Number of sectors */
	int	     fats;				/* Number of FATs */
} fsdata_t;

static inline unsigned clust_to_sect(fsdata_t *fsdata, unsigned clust)
{
	return fsdata->data_begin + clust * fsdata->clust_size;
}

static inline unsigned sect_to_clust(fsdata_t *fsdata, int sect)
{
	return (sect - fsdata->data_begin) / fsdata->clust_size;
}

int file_fat_detectfs(void);
int fat_exists(const char *filename);
int fat_size(const char *filename, loff_t *size);
int file_fat_read_at(const char *filename, loff_t pos, void *buffer,
		             loff_t maxsize, loff_t *actread);
int file_fat_read(const char *filename, void *buffer, int maxsize);
int fat_set_blk_dev(struct blk_desc *rbdd, struct disk_partition *info);
int fat_register_device(struct blk_desc *dev_desc, int part_no);

int file_fat_write(const char *filename, void *buf, loff_t offset, loff_t len,
		           loff_t *actwrite);
int fat_read_file(const char *filename, void *buf, loff_t offset, loff_t len,
		          loff_t *actread);
int fat_opendir(const char *filename, struct fs_dir_stream **dirsp);
int fat_readdir(struct fs_dir_stream *dirs, struct fs_dirent **dentp);
void fat_closedir(struct fs_dir_stream *dirs);
int fat_unlink(const char *filename);
int fat_mkdir(const char *dirname);
void fat_close(void);

void fat_print_info(fsdata_t *fsdata);

//-------------------------------------------------------------------------------------------------
// 64 λ����������
//-------------------------------------------------------------------------------------------------

static inline uint64_t div_u64_rem(uint64_t dividend, uint32_t divisor, uint32_t *remainder)
{
    uint64_t ret;
    
    ret = dividend / divisor;
    *remainder = dividend - (ret * divisor);

    return ret;
}

#endif /* _FAT_H_ */



/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 */
#ifndef _FS_H
#define _FS_H

//#include <common.h>

typedef unsigned long long loff_t;

struct cmd_tbl;

#define FS_TYPE_ANY		0
#define FS_TYPE_FAT		1
#define FS_TYPE_EXT		2
#define FS_TYPE_SANDBOX	3
#define FS_TYPE_UBIFS	4
#define FS_TYPE_BTRFS	5

struct blk_desc;


/*
 * Directory entry types, matches the subset of DT_x in posix readdir()
 * which apply to u-boot.
 */
#define FS_DT_DIR  4         /* directory */
#define FS_DT_REG  8         /* regular file */
#define FS_DT_LNK  10        /* symbolic link */

/*
 * A directory entry, returned by fs_readdir().  Returns information
 * about the file/directory at the current directory entry position.
 */
struct fs_dirent
{
	unsigned type;       /* one of FS_DT_x (not a mask) */
	loff_t size;         /* size in bytes */
	char name[256];
};

/* Note: fs_dir_stream should be treated as opaque to the user of fs layer */
struct fs_dir_stream
{
	/* private to fs. layer: */
	struct blk_desc *desc;
	int part;
};



#endif /* _FS_H */

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "blk.h"
#include "part.h"

//-------------------------------------------------------------------------------------------------
// functions
//-------------------------------------------------------------------------------------------------

int part_get_info(struct blk_desc *dev_desc, int part, struct disk_partition *info)
{
    (void *)dev_desc;
    (int)part;
    (void *)info;
    return -1;
}

void part_init(struct blk_desc *dev_desc)
{
    (void *)dev_desc;
    return;
}

void dev_print(struct blk_desc *dev_desc)
{
    (void *)dev_desc;
    return;
}

void part_set_generic_name(const struct blk_desc *dev_desc, int part_num, char *name)
{
	char *devtype;

	switch (dev_desc->if_type)
	{
		case IF_TYPE_IDE:
		case IF_TYPE_SATA:
		case IF_TYPE_ATAPI:
			devtype = "hd";
			break;
		case IF_TYPE_SCSI:
			devtype = "sd";
			break;
		case IF_TYPE_USB:
			devtype = "usbd";
			break;
		case IF_TYPE_DOC:
			devtype = "docd";
			break;
		case IF_TYPE_MMC:
		case IF_TYPE_SD:
			devtype = "mmcsd";
			break;
		default:
			devtype = "xx";
			break;
	}
	sprintf(name, "%s%c%d", devtype, 'a' + dev_desc->devnum, part_num);
}

/*
 * @@ End
 */


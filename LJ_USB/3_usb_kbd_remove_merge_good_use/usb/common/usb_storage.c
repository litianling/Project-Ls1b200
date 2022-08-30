// SPDX-License-Identifier: GPL-2.0+
/*
 * Most of this source has been derived from the Linux USB
 * project:
 *   (c) 1999-2002 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *   (c) 2000 David L. Brown, Jr. (usb-storage@davidb.org)
 *   (c) 1999 Michael Gee (michael@linuxspecific.com)
 *   (c) 2000 Yggdrasil Computing, Inc.
 *
 *
 * Adapted for U-Boot:
 *   (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 * Driver model conversion:
 *   (C) Copyright 2015 Google, Inc
 *
 * For BBB support (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 * BBB support based on /sys/dev/usb/umass.c from
 * FreeBSD.
 */

/* Note:
 * Currently only the CBI transport protocoll has been implemented, and it
 * is only tested with a TEAC USB Floppy. Other Massstorages with CBI or CB
 * transport protocoll may work as well.
 */
/*
 * New Note:
 * Support for USB Mass Storage Devices (BBB) has been added. It has
 * only been tested with USB memory sticks.
 */

#include "usb_cfg.h"

#if BSP_USE_MASS

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "bsp.h"
#include "../libc/lwmem.h"
//#include "linux_kernel.h"
#include "linux.h"
#include "part.h"       //\\ 一些定义和blk.h 有冲突
#include "usb.h"
#include "scsi.h"
#include "blk.h"
#include "memalign.h"

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE
#define BLK    0        //\\ 块设备?
#define DEBUG(format, arg...) do {} while (0)

/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
static const unsigned char us_direction[256/8] =
{
    0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
    0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

static struct scsi_cmd usb_ccb __attribute__((aligned(ARCH_DMA_MINALIGN)));

static unsigned int CBWTag;

static int usb_max_devs; /* 最高可用 USB 设备的数量 */

#if (!BLK)
static struct blk_desc  usb_dev_desc[USB_MAX_STOR_DEV];
#endif

struct us_data;

typedef int (*trans_cmnd)(struct scsi_cmd *cb, struct us_data *data);
typedef int (*trans_reset)(struct us_data *data);

struct us_data
{
    struct usb_device *pusb_dev;            /* 这个USB设备 */

    unsigned int       flags;               /* from filter initially */
#define USB_READY   (1 << 0)

    unsigned char      ifnum;               /* 接口数量 */
    unsigned char      ep_in;               /* 输入端点 */
    unsigned char      ep_out;              /* 输出端点 */
    unsigned char      ep_int;              /* 中断端点 */
    unsigned char      subclass;            /* 子类别 */
    unsigned char      protocol;            /* 协议 */
    unsigned char      attention_done;      /* force attn on first cmd */
    unsigned short     ip_data;             /* 中断数据 */
    int                action;              /* 将要执行 */
    int                ip_wanted;           /* needed */
    int               *irq_handle;          /* 用于USB中断请求 */
    unsigned int       irqpipe;             /* 释放中断请求的管道 */
    unsigned char      irqmaxp;             /* 用于中断请求管道的最大数据包 */
    unsigned char      irqinterval;         /* IRQ 管道的间隔 */
    struct scsi_cmd   *srb;                 /* 当前服务请求 */
    trans_reset        transport_reset;     /* 重置程序 */
    trans_cmnd         transport;           /* 传输程序 */
    unsigned short     max_xfer_blk;        /* 最大传输数据块 */
};

#if (!BLK)
static struct us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD     0
#define USB_STOR_TRANSPORT_FAILED  -1
#define USB_STOR_TRANSPORT_ERROR   -2

int usb_stor_get_info(struct usb_device *dev,
                      struct us_data *us,
                      struct blk_desc *dev_desc);

int usb_storage_probe(struct usb_device *dev,
                      unsigned int ifnum,
                      struct us_data *ss);

#if (CONFIG_BLK_ENABLED)
static unsigned long usb_stor_read(struct udevice *dev,
                                   lbaint_t blknr,
                                   lbaint_t blkcnt,
                                   void *buffer);

static unsigned long usb_stor_write(struct udevice *dev,
                                    lbaint_t blknr,
                                    lbaint_t blkcnt,
                                    const void *buffer);

#else

static unsigned long usb_stor_read(struct blk_desc *block_dev,
                                   lbaint_t blknr,
                                   lbaint_t blkcnt,
                                   void *buffer);

static unsigned long usb_stor_write(struct blk_desc *block_dev,
                                    lbaint_t blknr,
                                    lbaint_t blkcnt,
                                    const void *buffer);

#endif

void uhci_show_temp_int_td(void);

static void usb_show_progress(void)
{
    DEBUG(".");
}

//-------------------------------------------------------------------------------------------------

struct blk_desc* get_usb_msc_blk_dev(int index)
{
    if ((index >= 0) && (index <= USB_MAX_STOR_DEV))    // 如果设备号在最大序号范围内
        return &usb_dev_desc[index];                    // 返回指向第index个存储器的指针
    return NULL;                                        // 否则返回为空
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
int usb_stor_info(void)
{
    int count = 0;
#if (CONFIG_BLK_ENABLED)
    struct udevice *dev;

    for (blk_first_device(IF_TYPE_USB, &dev);
         dev;
         blk_next_device(&dev))
    {
        struct blk_desc *desc = dev_get_uclass_platdata(dev);

        printf("  Device %d: ", desc->devnum);
        dev_print(desc);
        count++;
    }

#else
    int i;

    if (usb_max_devs > 0)
    {
        for (i = 0; i < usb_max_devs; i++)
        {
            printf("  Device %d: ", i);
            dev_print(&usb_dev_desc[i]);
        }
        return 0;
    }

#endif

    if (!count)
    {
        printf("No storage devices, perhaps not 'usb start'ed..?\n");
        return 1;
    }

    return 0;
}

static unsigned int usb_get_max_lun(struct us_data *us)
{
    int len;
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, result, 1); // 给result分配缓存，对其缓存区type\neme\size

    len = usb_control_msg(us->pusb_dev,
                          usb_rcvctrlpipe(us->pusb_dev, 0),
                          US_BBB_GET_MAX_LUN,
                          USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
                          0,
                          us->ifnum,
                          result,
                          sizeof(char),
                          USB_CNTL_TIMEOUT * 5);

    DEBUG("Get Max LUN -> len = %i, result = %i\n", len, (int) *result);
    return (len > 0) ? *result : 0;
}

/**********************************************************
 *                  USB存储器设备探测
 **********************************************************/
static int usb_stor_probe_device(struct usb_device *udev)
{
    int lun, max_lun;       // 小型计算机系统接口逻辑单元号---->一个磁盘空间被划分为若干个小单元以供主机使用

#if (CONFIG_BLK_ENABLED)
    struct us_data *data;
    int ret;
#else
    int start;

    if (udev == NULL)
        return -ENOENT; /* 没有更多可用的设备 返回-2 */
#endif

    DEBUG("\n\nProbing for storage\n");
#if (CONFIG_BLK_ENABLED)
    /*
     * We store the us_data in the mass storage device's platdata. It
     * is shared by all LUNs (block devices) attached to this mass storage
     * device.
     */
    data = dev_get_platdata(udev->dev);
    if (!usb_storage_probe(udev, 0, data))
        return 0;

    max_lun = usb_get_max_lun(data);
    for (lun = 0; lun <= max_lun; lun++)
    {
        struct blk_desc *blkdev;
        struct udevice *dev;
        char str[10];

        snprintf(str, sizeof(str), "lun%d", lun);
        ret = blk_create_devicef(udev->dev,
                                 "usb_storage_blk",
                                 str,
                                 IF_TYPE_USB,
                                 usb_max_devs,
                                 512,
                                 0,
                                 &dev);

        if (ret)
        {
            DEBUG("Cannot bind driver\n");
            return ret;
        }

        blkdev = dev_get_uclass_platdata(dev);
        blkdev->target = 0xff;
        blkdev->lun = lun;

        ret = usb_stor_get_info(udev, data, blkdev);
        if (ret == 1)
        {
            usb_max_devs++;
            DEBUG("%s: Found device %p\n", __func__, udev);
        }
        else
        {
            DEBUG("usb_stor_get_info: Invalid device\n");
            ret = device_unbind(dev);
            if (ret)
                return ret;
        }
    }

#else
    /* 如果我们达到最大值，我们甚至没有空间进行探测=>USB大容量存储器达到上限 */
    if (usb_max_devs == USB_MAX_STOR_DEV)
    {
        printf("max USB Storage Device reached: %d stopping\n", usb_max_devs);
        return -ENOSPC;  // 返回-28
    }

    if (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))   // 探测是否是USB设备【*】不是大容量设备返回0，是则配置usb_stor返回1
        return 0;                                               // 结构体数组usb_stor，存储us_data，最大USB_MAX_DEVS

    /* 好的，这是一个存储设备。 迭代其 LUN 并填充 usb_dev_desc */
    start = usb_max_devs;
    max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);         // 从usb_stor获取当前设备最大逻辑单元号【*】
    for (lun = 0;lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;lun++)  // lun是逻辑接口号,插入两个U盘max_lun均为0，都只执行一次
    {
        struct blk_desc *blkdev;

        blkdev = &usb_dev_desc[usb_max_devs];                   // 块存储器设备&分配空间
        memset(blkdev, '\0', sizeof(struct blk_desc));
        blkdev->if_type = IF_TYPE_USB;                          // 接口类型
        blkdev->devnum = usb_max_devs;                          // 设备号
        blkdev->part_type = PART_TYPE_UNKNOWN;                  // 分区类型
        blkdev->target = 0xff;                                  // 目标（小型计算机接口）ID
        blkdev->type = DEV_TYPE_UNKNOWN;                        // 设备类型
        blkdev->block_read = usb_stor_read;                     // 指向操作---->大容量存储器设备读取【*】
        blkdev->block_write = usb_stor_write;                   // 指向操作---->大容量存储器设备写入【*】
        blkdev->lun = lun;                                      // 目标逻辑单元号
        blkdev->priv = udev;                                    // 当前设备（usb_device类型）

        if (usb_stor_get_info(udev,&usb_stor[start],&usb_dev_desc[usb_max_devs]) == 1)  // 获取信息【*】根据udev与usb_stor，填写usb_dev_desc
        {
            DEBUG("partype: %d\n", blkdev->part_type);
            part_init(blkdev);                                  // 好像没啥用【*】
            DEBUG("partype: %d\n", blkdev->part_type);
            usb_max_devs++;
            DEBUG("%s: Found device %p\n", __func__, udev);
        }
    }

#endif
    return 0;
}

// USB最大可用连接数归零
void usb_stor_reset(void)
{
    usb_max_devs = 0;
}

/*******************************************************************************
 * 如果 mode = 1 返回当前设备，则扫描 USB 并向用户报告设备信息；如果不是，则返回 -1
 *******************************************************************************/
int usb_stor_scan(int mode)
{
    if (mode == 1)
        printf("scanning usb for storage devices... ");

#if (!DM_USB)
    unsigned char i;                    // i是开发板USB设备接口号
    usb_disable_asynch(1);              // 禁用异步传输【*】
    usb_stor_reset();                   // 最高可用USB设备的数量（存储设备数）usb_max_devs清零
    for (i = 0; i < USB_MAX_DEVICE; i++)// USB_MAX_DEVICE=32
    {
        struct usb_device *dev;         // 定义USB设备结构指针
        dev = usb_get_dev_index(i);     // 遍历接口获取设备指针【*】
        DEBUG("i=%d\n", i);             
        if (usb_stor_probe_device(dev)) // USB存储器设备探测【*】发生错误（如探测到最大数量的存储器）时返回负值结束
            break;                      // 不是存储器返回0继续执行，是存储器进行操作并返回0继续执行
    }
    usb_disable_asynch(0);              // 启用异步传输【*】
#endif

    printf("%d Storage Device(s) found\n", usb_max_devs);
    if (usb_max_devs > 0)               // 显示找到的存储设备数，有连接返回0，无连接返回1
        return 0;
    return -1;
}

static int usb_stor_irq(struct usb_device *dev)
{
    struct us_data *us;
    us = (struct us_data *)dev->privptr;

    if (us->ip_wanted)
        us->ip_wanted = 0;
    return 0;
}

#ifdef DEBUG

static void usb_show_srb(struct scsi_cmd *pccb)
{
    int i;
    printf("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
    for (i = 0; i < 12; i++)
        printf("%02X ", pccb->cmd[i]);
    printf("\n");
}

static void display_int_status(unsigned long tmp)
{
    printf("Status: %s %s %s %s %s %s %s\n",
           (tmp & USB_ST_ACTIVE)     ? "Active"         : "",
           (tmp & USB_ST_STALLED)    ? "Stalled"        : "",
           (tmp & USB_ST_BUF_ERR)    ? "Buffer Error"   : "",
           (tmp & USB_ST_BABBLE_DET) ? "Babble Det"     : "",
           (tmp & USB_ST_NAK_REC)    ? "NAKed"          : "",
           (tmp & USB_ST_CRC_ERR)    ? "CRC Error"      : "",
           (tmp & USB_ST_BIT_ERR)    ? "Bitstuff Error" : "" );
}
#endif

/***********************************************************************
 * 数据传输实例
 ***********************************************************************/

static int us_one_transfer(struct us_data *us,
                           int pipe,
                           char *buf,
                           int length)
{
    int max_size;
    int this_xfer;
    int result;
    int partial;
    int maxtry;
    int stat;

    /* 确定这些传输的最大数据包大小 */
    max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;              // 返回最大数据包大小，取决于管道方向和配置值【*】

    /* 虽然我们还有数据要传输 */
    while (length)
    {
        /* 计算这将是多长时间 - 最大值或余数 */
        this_xfer = length > max_size ? max_size : length;
        length -= this_xfer;

        /* 设置重试计数器 */
        maxtry = 10;

        /* 设置传输循环 */
        do
        {
            /* 传输数据 */
            DEBUG("Bulk xfer 0x%lx(%d) try #%d\n",
                  (unsigned long)/*map_to_sysmem*/(buf), this_xfer, 11 - maxtry);

            result = usb_bulk_msg(us->pusb_dev,                     // USB块存储器信息【*】
                                  pipe,
                                  buf,
                                  this_xfer,
                                  &partial,
                                  USB_CNTL_TIMEOUT * 5 );

            DEBUG("bulk_msg returned %d xferred %d/%d\n",
                   result, partial, this_xfer);

            if (us->pusb_dev->status != 0)
            {
                /* 如果我们停下来，我们需要在继续之前清除它 */
#ifdef DEBUG
                display_int_status(us->pusb_dev->status);           // 显示状态【*】
#endif
                if (us->pusb_dev->status & USB_ST_STALLED)          // 如果状态传输ST被挂起
                {
                    DEBUG("stalled ->clearing endpoint halt for pipe 0x%x\n", pipe);
                    stat = us->pusb_dev->status;
                    usb_clear_halt(us->pusb_dev, pipe);             // 清除停止【*】
                    us->pusb_dev->status = stat;
                    if (this_xfer == partial)
                    {
                        DEBUG("bulk transferred with error %lX, but data ok\n",
                              us->pusb_dev->status);
                        return 0;
                    }
                    else
                        return result;
                }

                if (us->pusb_dev->status & USB_ST_NAK_REC)          // 如果从设备拒绝
                {
                    DEBUG("Device NAKed bulk_msg\n");
                    return result;
                }

                DEBUG("bulk transferred with error");
                if (this_xfer == partial)
                {
                    DEBUG(" %ld, but data ok\n", us->pusb_dev->status);
                    return 0;
                }

                /* 如果我们的尝试计数器达到 0，则退出 */
                DEBUG(" %ld, data %d\n", us->pusb_dev->status, partial);
                if (!maxtry--)
                    return result;
            }

            /* 更新以显示传输了哪些数据 */
            this_xfer -= partial;
            buf += partial;
            /* 继续，直到此传输完成 */
        } while (this_xfer);
    }

    /* 如果我们到达这里，我们就完成了并且成功了 */
    return 0;
}

static int usb_stor_BBB_reset(struct us_data *us)
{
    int result;
    unsigned int pipe;

    /*
     * Reset recovery (5.3.4 in Universal Serial Bus Mass Storage Class)
     *
     * For Reset Recovery the host shall issue in the following order:
     * a) a Bulk-Only Mass Storage Reset
     * b) a Clear Feature HALT to the Bulk-In endpoint
     * c) a Clear Feature HALT to the Bulk-Out endpoint
     *
     * This is done in 3 steps.
     *
     * If the reset doesn't succeed, the device should be port reset.
     *
     * This comment stolen from FreeBSD's /sys/dev/usb/umass.c.
     */
    DEBUG("BBB_reset\n");
    result = usb_control_msg(us->pusb_dev,      // 控制信息【*】
                             usb_sndctrlpipe(us->pusb_dev, 0),
                             US_BBB_RESET,
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                             0,
                             us->ifnum,
                             NULL,
                             0,
                             USB_CNTL_TIMEOUT * 5 );

    if ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED))    // 被挂起
    {
        DEBUG("RESET:stall\n");
        return -1;
    }

    /* 长时间等待复位完成 */
    DELAY_MS(15); // DELAY_MS(150);
    DEBUG("BBB_reset result %d: status %lX reset\n",
           result, us->pusb_dev->status);
    pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);    // 块存储器接收输入管道【*】
    result = usb_clear_halt(us->pusb_dev, pipe);        // 清除停止【*】

    /* 长时间等待复位完成 */
    DELAY_MS(15); // DELAY_MS(150);
    DEBUG("BBB_reset result %d: status %lX clearing IN endpoint\n",
           result, us->pusb_dev->status);

    /* 长时间等待复位完成 */
    pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);   // 块存储器接收输出管道【*】
    result = usb_clear_halt(us->pusb_dev, pipe);        // 清除停止【*】

    DELAY_MS(15); // DELAY_MS(150);
    DEBUG("BBB_reset result %d: status %lX clearing OUT endpoint\n",
           result, us->pusb_dev->status);
    DEBUG("BBB_reset done\n");

    return 0;
}

/* FIXME: 这个重置功能并没有真正重置端口。 实际上它可能应该在这里做它正在做的事情，并在物理上重置端口 */
static int usb_stor_CB_reset(struct us_data *us)
{
    unsigned char cmd[12];
    int result;

    DEBUG("CB_reset\n");
    memset(cmd, 0xff, sizeof(cmd));         // 将命令清除【*】
    cmd[0] = SCSI_SEND_DIAG;
    cmd[1] = 4;
    result = usb_control_msg(us->pusb_dev,  // USB控制信息【*】
                             usb_sndctrlpipe(us->pusb_dev, 0),
                             US_CBI_ADSC,
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                             0,
                             us->ifnum,
                             cmd,
                             sizeof(cmd),
                             USB_CNTL_TIMEOUT * 5 );

    /* 较长时间的等待重置完成 */
    DELAY_MS(150); // DELAY_MS(1500);
    DEBUG("CB_reset result %d: status %lX clearing endpoint halt\n",
           result, us->pusb_dev->status);
    usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));     // 消除停止【*】块存储器接收管道【*】
    usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));    // 同上【*】

    DEBUG("CB_reset done\n");
    return 0;
}

/* 为 BBB 设备设置命令。 请注意，实际的 小型存储器接口 命令被复制到 cbw.CBWCDB 中。*/
static int usb_stor_BBB_comdat(struct scsi_cmd *srb, struct us_data *us)
{
    int result;
    int actlen;
    int dir_in;
    unsigned int pipe;
    ALLOC_CACHE_ALIGN_BUFFER(struct umass_bbb_cbw, cbw, 1);             // 分配存储器空间【*】

    dir_in = US_DIRECTION(srb->cmd[0]);                                 // 方向【*】

#ifdef BBB_COMDAT_TRACE
    printf("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
            dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
            srb->pdata);
    if (srb->cmdlen)
    {
        for (result = 0; result < srb->cmdlen; result++)
            printf("cmd[%d] %#x ", result, srb->cmd[result]);
        printf("\n");
    }
#endif

    /* 完整性检查 */
    if (!(srb->cmdlen <= CBWCDBLENGTH))
    {
        DEBUG("usb_stor_BBB_comdat:cmdlen too large\n");
        return -1;
    }

    /* 总是向端点输出 */
    pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);                   // 块存储器发送管道【*】

    cbw->dCBWSignature          = CBWSIGNATURE;                         // 签名
    cbw->dCBWTag                = CBWTag++;                             // 标签
    cbw->dCBWDataTransferLength = srb->datalen;                         // 发送数据长度
    cbw->bCBWFlags              = (dir_in ? CBWFLAGS_IN : CBWFLAGS_OUT);// 标志
    cbw->bCBWLUN                = srb->lun;                             // 逻辑单元号
    cbw->bCDBLength             = srb->cmdlen;                          // 命令长度
    /* 将命令数据复制到 CBW 命令数据缓冲区 */
    /* DST SRC LEN!!! */

    memcpy(cbw->CBWCDB, srb->cmd, srb->cmdlen);                         // 拷贝命令【*】
    result = usb_bulk_msg(us->pusb_dev,                                 // 块存储器信息【*】
                          pipe,
                          cbw,
                          UMASS_BBB_CBW_SIZE,
                          &actlen,
                          USB_CNTL_TIMEOUT * 5 );

    if (result < 0)
        DEBUG("usb_stor_BBB_comdat:usb_bulk_msg error\n");

    return result;
}

/* FIXME: 我们还需要一个 CBI_command 来设置完成中断，并等待它 */
static int usb_stor_CB_comdat(struct scsi_cmd *srb, struct us_data *us)
{
    int result = 0;
    int dir_in, retry;
    unsigned int pipe;
    unsigned long status;

    retry = 5;
    dir_in = US_DIRECTION(srb->cmd[0]);                                     // 方向【*】

    if (dir_in)
        pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);                    // 块存储器发送管道【*】
    else
        pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);                   // 块存储器接收管道【*】

    while (retry--)
    {
        DEBUG("CBI gets a command: Try %d\n", 5 - retry);

#ifdef DEBUG
        usb_show_srb(srb);
#endif

        /* 让我们通过控制管道发送命令 */
        result = usb_control_msg(us->pusb_dev,                              // 控制信息准备传输【*】
                                 usb_sndctrlpipe(us->pusb_dev , 0),
                                 US_CBI_ADSC,
                                 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                                 0,
                                 us->ifnum,
                                 srb->cmd,
                                 srb->cmdlen,
                                 USB_CNTL_TIMEOUT * 5 );

        DEBUG("CB_transport: control msg returned %d, status %lX\n",
               result, us->pusb_dev->status);

        /* 检查命令的返回码 */
        if (result < 0)
        {
            if (us->pusb_dev->status & USB_ST_STALLED)                      // 如果数据传输被挂起了
            {
                status = us->pusb_dev->status;                              
                DEBUG(" stall during command found, clear pipe\n");
                usb_clear_halt(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0)); // USB清除停止【*】USB发送控制管道【*】
                us->pusb_dev->status = status;                              // 更新设备状态
            }

            DEBUG(" error during command %02X Stat = %lX\n", srb->cmd[0],
                   us->pusb_dev->status);
            return result;
        }

        /* 传输此命令的数据有效负载（如果存在）*/
        DEBUG("CB_transport: control msg returned %d, direction is %s to go 0x%lx\n",
               result, dir_in ? "IN" : "OUT", srb->datalen);

        if (srb->datalen)
        {
            result = us_one_transfer(us, pipe, (char *)srb->pdata, srb->datalen);   // USB一次传输【*】
            DEBUG("CBI attempted to transfer data, result is %d status %lX, len %d\n",
                   result, us->pusb_dev->status, us->pusb_dev->act_len);
            if (!(us->pusb_dev->status & USB_ST_NAK_REC))
                break;
        } /* if (srb->datalen) */
        else
            break;
    }
    /* 返回结果 */

    return result;
}


static int usb_stor_CBI_get_status(struct scsi_cmd *srb, struct us_data *us)
{
    int timeout;
    us->ip_wanted = 1;
    usb_int_msg(us->pusb_dev,                                                       // 初始化消息【*】
                us->irqpipe,
                (void *)&us->ip_data,
                us->irqmaxp,
                us->irqinterval,
                false);

    timeout = 1000;
    while (timeout--)                                                               // 延时等待上一步成功
    {
        if (us->ip_wanted == 0)
            break;
        DELAY_MS(10);
    }

    if (us->ip_wanted)                                                              // 在 CBI 上没有中断
    {
        printf("    Did not get interrupt on CBI\n");
        us->ip_wanted = 0;
        return USB_STOR_TRANSPORT_ERROR;
    }

    DEBUG("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
           us->ip_data, us->pusb_dev->irq_act_len, us->pusb_dev->irq_status);
    /* UFI 给了我们 ASC 和 ASCQ，就像请求感 */
    if (us->subclass == US_SC_UFI)
    {
        if (srb->cmd[0] == SCSI_REQ_SENSE || srb->cmd[0] == SCSI_INQUIRY)
            return USB_STOR_TRANSPORT_GOOD; /* Good */
        else if (us->ip_data)
            return USB_STOR_TRANSPORT_FAILED;
        else
            return USB_STOR_TRANSPORT_GOOD;
    }

    /* 否则，我们通常解释数据 */
    switch (us->ip_data)
    {
        case 0x0001: return USB_STOR_TRANSPORT_GOOD;
        case 0x0002: return USB_STOR_TRANSPORT_FAILED;
        default:     return USB_STOR_TRANSPORT_ERROR;
    }

    return USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY     5
#define USB_TRANSPORT_NOT_READY_RETRY   10

/* 清除端点上的停顿 - 特别适用于 BBB 设备 */
static int usb_stor_BBB_clear_endpt_stall(struct us_data *us, unsigned char endpt)
{
    /* ENDPOINT_HALT = 0, so set value to 0 */
    return usb_control_msg(us->pusb_dev,        // USB控制信息【*】
                           usb_sndctrlpipe(us->pusb_dev, 0),
                           USB_REQ_CLEAR_FEATURE,
                           USB_RECIP_ENDPOINT,
                           0,
                           endpt,
                           NULL,
                           0,
                           USB_CNTL_TIMEOUT * 5 );
}

static int usb_stor_BBB_transport(struct scsi_cmd *srb, struct us_data *us)
{
    int result, retry;
    int dir_in;
    int actlen, data_actlen;
    unsigned int pipe, pipein, pipeout;
    ALLOC_CACHE_ALIGN_BUFFER(struct umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
    unsigned char *ptr;
    int index;
#endif

    dir_in = US_DIRECTION(srb->cmd[0]);                                     // 方向【*】

    /* 命令阶段 */
    DEBUG("COMMAND phase\n");
    result = usb_stor_BBB_comdat(srb, us);                                  // BBB块存储器设备命令【*】
    if (result < 0)
    {
        DEBUG("failed to send CBW status %ld\n", us->pusb_dev->status);
        usb_stor_BBB_reset(us);                                             // 块存储器BBB复位【*】
        return USB_STOR_TRANSPORT_FAILED;
    }

    if (!(us->flags & USB_READY))
        DELAY_MS(5);

    pipein  = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);                     // 块存储器接收管道【*】
    pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);                    // 块存储器发送管道【*】

    /* 数据阶段 + 错误处理 */
    data_actlen = 0;

    /* 没有数据，立即进入状态阶段 */
    if (srb->datalen == 0)
        goto st;

    DEBUG("DATA phase\n");
    if (dir_in)
        pipe = pipein;
    else
        pipe = pipeout;

    result = usb_bulk_msg(us->pusb_dev,                                     // USB块存储器信息【*】
                          pipe,
                          srb->pdata,
                          srb->datalen,
                          &data_actlen,
                          USB_CNTL_TIMEOUT * 5 );

    /* 在 DATA 阶段对 挂起STALL 的特殊处理 */
    if ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG("DATA:stall\n");
        /* 清除端点上的 STALL */
        result = usb_stor_BBB_clear_endpt_stall(us, dir_in ? us->ep_in : us->ep_out);   // 清除BBB设备端点上的停顿【*】
        if (result >= 0)
            /* 继续进入状态阶段 */
            goto st;
    }

    if (result < 0)
    {
        DEBUG("usb_bulk_msg error status %ld\n", us->pusb_dev->status);
        usb_stor_BBB_reset(us);                                             // 块存储器BBB复位【*】
        return USB_STOR_TRANSPORT_FAILED;
    }

#ifdef BBB_XPORT_TRACE
    for (index = 0; index < data_actlen; index++)
        printf("pdata[%d] %#x ", index, srb->pdata[index]);
    printf("\n");
#endif

    /* 状态阶段 + 错误处理 */
st:
    retry = 0;

again:
    DEBUG("STATUS phase\n");
    result = usb_bulk_msg(us->pusb_dev,                                     // USB块存储器信息【*】
                          pipein,
                          csw,
                          UMASS_BBB_CSW_SIZE,
                          &actlen,
                          USB_CNTL_TIMEOUT*5);

    /* 状态阶段对 STALL 的特殊处理 */
    if ((result < 0) && (retry < 1) && (us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG("STATUS:stall\n");
        /* 清除端点上的 STALL */
        result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);             // 清除BBB设备端点上的停顿【*】
        if (result >= 0 && (retry++ < 1))
            /* 重试 */
            goto again;
    }

    if (result < 0)
    {
        DEBUG("usb_bulk_msg error status %ld\n", us->pusb_dev->status);
        usb_stor_BBB_reset(us);                                             // 块存储器BBB复位【*】
        return USB_STOR_TRANSPORT_FAILED;
    }

#ifdef BBB_XPORT_TRACE
    ptr = (unsigned char *)csw;
    for (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
        printf("ptr[%d] %#x ", index, ptr[index]);
    printf("\n");
#endif

    /* 误用管道获取残留物 */
    pipe = csw->dCSWDataResidue;
    if (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
        pipe = srb->datalen - data_actlen;

    if (CSWSIGNATURE != csw->dCSWSignature)
    {
        DEBUG("!CSWSIGNATURE\n");
        usb_stor_BBB_reset(us);                                             // 块存储器BBB复位【*】
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if ((CBWTag - 1) != csw->dCSWTag)
    {
        DEBUG("!Tag\n");
        usb_stor_BBB_reset(us);                                             // 块存储器BBB复位【*】
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (csw->bCSWStatus > CSWSTATUS_PHASE)
    {
        DEBUG(">PHASE\n");
        usb_stor_BBB_reset(us);                                             // 块存储器BBB复位【*】
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (csw->bCSWStatus == CSWSTATUS_PHASE)
    {
        DEBUG("=PHASE\n");
        usb_stor_BBB_reset(us);
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (data_actlen > srb->datalen)
    {
        DEBUG("transferred %dB instead of %ldB\n", data_actlen, srb->datalen);
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (csw->bCSWStatus == CSWSTATUS_FAILED)
    {
        DEBUG("FAILED\n");
        return USB_STOR_TRANSPORT_FAILED;
    }

    return result;
}

static int usb_stor_CB_transport(struct scsi_cmd *srb, struct us_data *us)
{
    int result, status;
    struct scsi_cmd *psrb;
    struct scsi_cmd reqsrb;
    int retry, notready;

    psrb = &reqsrb;
    status = USB_STOR_TRANSPORT_GOOD;
    retry = 0;
    notready = 0;

    /* 发出命令 */
do_retry:
    result = usb_stor_CB_comdat(srb, us);                                               // 发出存储器CB的命令【*】
    DEBUG("command / Data returned %d, status %lX\n", result, us->pusb_dev->status);

    /* 如果这是一个 CBI 协议，获取中断请求 IRQ */
    if (us->protocol == US_PR_CBI)
    {
        status = usb_stor_CBI_get_status(srb, us);                                      // 获取状态【*】
        /* 如果状态是错误，报告它 */
        if (status == USB_STOR_TRANSPORT_ERROR)
        {
            DEBUG(" USB CBI Command Error\n");
            return status;
        }

        srb->sense_buf[12] = (unsigned char)(us->ip_data >> 8);
        srb->sense_buf[13] = (unsigned char)(us->ip_data & 0xff);
        if (!us->ip_data)                                                               // 中断数据不是空
        {
            /* 如果状态好，请报告 */
            if (status == USB_STOR_TRANSPORT_GOOD)
            {
                DEBUG(" USB CBI Command Good\n");
                return status;
            }
        }
    }

    /* 我们必须发出自动请求吗? */
    /* 在这里我们必须检查结果 */
    if ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG("ERROR %lX\n", us->pusb_dev->status);
        us->transport_reset(us);                                                        // 传输复位【*】
        return USB_STOR_TRANSPORT_ERROR;
    }

    if ((us->protocol == US_PR_CBI) &&
       ((srb->cmd[0] == SCSI_REQ_SENSE) || (srb->cmd[0] == SCSI_INQUIRY)))
    {
        /* 请求感知后不发出自动请求 */
        DEBUG("No auto request and good\n");
        return USB_STOR_TRANSPORT_GOOD;
    }

    /* 发出 request_sense */
    memset(&psrb->cmd[0], 0, 12);                                                       // 将命令清零【*】
    psrb->cmd[0]  = SCSI_REQ_SENSE;
    psrb->cmd[1]  = srb->lun << 5;
    psrb->cmd[4]  = 18;
    psrb->datalen = 18;                                                                 // 数据长度
    psrb->pdata   = &srb->sense_buf[0];                                                 // 数据区
    psrb->cmdlen  = 12;                                                                 // 命令长度

    /* 发出命令 */
    result = usb_stor_CB_comdat(psrb, us);                                              // 发出CB存储器命令【*】同上
    DEBUG("auto request returned %d\n", result);

    /* 如果这是一个 CBI 协议，获取 IRQ */
    if (us->protocol == US_PR_CBI)
        status = usb_stor_CBI_get_status(psrb, us);                                     // 获取存储器状态【*】同上

    if ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG(" AUTO REQUEST ERROR %ld\n", us->pusb_dev->status);
        return USB_STOR_TRANSPORT_ERROR;
    }

    DEBUG("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
           srb->sense_buf[0], srb->sense_buf[2],
           srb->sense_buf[12], srb->sense_buf[13]);

    /* 检查自动请求结果 */
    if ((srb->sense_buf[2]  == 0) &&
        (srb->sense_buf[12] == 0) &&
        (srb->sense_buf[13] == 0))
    {
        /* 好吧，没意义 */
        return USB_STOR_TRANSPORT_GOOD;
    }

    /* 检查自动请求结果 */
    switch (srb->sense_buf[2])
    {
        case 0x01:
            return USB_STOR_TRANSPORT_GOOD;
            break;

        case 0x02:
            /* 没准备好 */
            if (notready++ > USB_TRANSPORT_NOT_READY_RETRY)
            {
                printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
                       " 0x%02X (NOT READY)\n", srb->cmd[0],
                srb->sense_buf[0], srb->sense_buf[2],
                srb->sense_buf[12], srb->sense_buf[13]);
                return USB_STOR_TRANSPORT_FAILED;
            }
            else
            {
                DELAY_MS(100);
                goto do_retry;
            }
            break;

        default:
            if (retry++ > USB_TRANSPORT_UNKNOWN_RETRY)
            {
                printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
                       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
                srb->sense_buf[2], srb->sense_buf[12],
                srb->sense_buf[13]);
                return USB_STOR_TRANSPORT_FAILED;
            }
            else
                goto do_retry;
            break;
    }

    return USB_STOR_TRANSPORT_FAILED;
}

static void usb_stor_set_max_xfer_blk(struct usb_device *udev,
                                      struct us_data *us)
{
    /*
     * Limit the total size of a transfer to 120 KB.
     *
     * Some devices are known to choke with anything larger. It seems like
     * the problem stems from the fact that original IDE controllers had
     * only an 8-bit register to hold the number of sectors in one transfer
     * and even those couldn't handle a full 256 sectors.
     *
     * Because we want to make sure we interoperate with as many devices as
     * possible, we will maintain a 240 sector transfer size limit for USB
     * Mass Storage devices.
     *
     * Tests show that other operating have similar limits with Microsoft
     * Windows 7 limiting transfers to 128 sectors for both USB2 and USB3
     * and Apple Mac OS X 10.11 limiting transfers to 256 sectors for USB2
     * and 2048 for USB3 devices.
     */
    unsigned short blk = 240;

#if (DM_USB)
    size_t size;
    int ret;

    ret = usb_get_max_xfer_size(udev, (size_t *)&size);
    if ((ret >= 0) && (size < blk * 512))
        blk = size / 512;
#endif

    us->max_xfer_blk = blk;
}

static int usb_inquiry(struct scsi_cmd *srb, struct us_data *ss)
{
    int retry, i;
    retry = 5;

    do
    {
        memset(&srb->cmd[0], 0, 12);
        srb->cmd[0] = SCSI_INQUIRY;
        srb->cmd[1] = srb->lun << 5;
        srb->cmd[4] = 36;
        srb->datalen = 36;
        srb->cmdlen = 12;
        i = ss->transport(srb, ss);         // 发送指令【*】
        DEBUG("inquiry returns %d\n", i);
        if (i == 0)
            break;
    } while (--retry);

    if (!retry)
    {
        printf("error in inquiry\n");
        return -1;
    }
    return 0;
}

static int usb_request_sense(struct scsi_cmd *srb, struct us_data *ss)
{
    char *ptr;

    ptr = (char *)srb->pdata;
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0]  = SCSI_REQ_SENSE;
    srb->cmd[1]  = srb->lun << 5;
    srb->cmd[4]  = 18;
    srb->datalen = 18;
    srb->pdata   = &srb->sense_buf[0];
    srb->cmdlen  = 12;
    ss->transport(srb, ss);
    DEBUG("Request Sense returned %02X %02X %02X\n",
          srb->sense_buf[2], srb->sense_buf[12], srb->sense_buf[13]);
    srb->pdata = (unsigned char *)ptr;
    return 0;
}

static int usb_test_unit_ready(struct scsi_cmd *srb, struct us_data *ss)
{
    int retries = 10;

    do
    {
        memset(&srb->cmd[0], 0, 12);
        srb->cmd[0]  = SCSI_TST_U_RDY;
        srb->cmd[1]  = srb->lun << 5;
        srb->datalen = 0;
        srb->cmdlen  = 12;
        if (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)      // 发送指令【*】
        {
            ss->flags |= USB_READY;
            return 0;
        }

        usb_request_sense(srb, ss);                                 // 【*】
        // 检查密钥代码限定符，如果它匹配“未就绪 - 介质不存在”（感知密钥等于 0x2 且 ASC 为 0x3a）立即返回，因为介质不存在不会改变，除非有用户操作。
        if ((srb->sense_buf[2] == 0x02) &&
            (srb->sense_buf[12] == 0x3a))
            return -1;

        DELAY_MS(100);
    } while (retries--);

    return -1;
}

static int usb_read_capacity(struct scsi_cmd *srb, struct us_data *ss)
{
    int retry;
    retry = 3;   /* XXX retries */

    do
    {
        memset(&srb->cmd[0], 0, 12);
        srb->cmd[0]  = SCSI_RD_CAPAC;
        srb->cmd[1]  = srb->lun << 5;
        srb->datalen = 8;
        srb->cmdlen  = 12;
        if (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
            return 0;
    } while (retry--);

    return -1;
}

static int usb_read_10(struct scsi_cmd *srb,
                       struct us_data *ss,
                       unsigned long start,
                       unsigned short blocks)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_READ10;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[3] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[4] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[5] = ((unsigned char) (start)) & 0xff;
    srb->cmd[7] = ((unsigned char) (blocks >> 8)) & 0xff;
    srb->cmd[8] = (unsigned char) blocks & 0xff;
    srb->cmdlen = 12;
    DEBUG("read10: start %lx blocks %x\n", start, blocks);
    return ss->transport(srb, ss);
}

static int usb_write_10(struct scsi_cmd *srb,
                        struct us_data *ss,
                        unsigned long start,
                        unsigned short blocks)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_WRITE10;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[3] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[4] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[5] = ((unsigned char) (start)) & 0xff;
    srb->cmd[7] = ((unsigned char) (blocks >> 8)) & 0xff;
    srb->cmd[8] = (unsigned char) blocks & 0xff;
    srb->cmdlen = 12;
    DEBUG("write10: start %lx blocks %x\n", start, blocks);
    return ss->transport(srb, ss);
}

#ifdef CONFIG_USB_BIN_FIXUP
/*
 * Some USB storage devices queried for SCSI identification data respond with
 * binary strings, which if output to the console freeze the terminal. The
 * workaround is to modify the vendor and product strings read from such
 * device with proper values (as reported by 'usb info').
 *
 * Vendor and product length limits are taken from the definition of
 * struct blk_desc in include/part.h.
 */
static void usb_bin_fixup(struct usb_device_descriptor descriptor,
                          unsigned char vendor[],
                          unsigned char product[])
{
    const unsigned char max_vendor_len  = 40;
    const unsigned char max_product_len = 20;

    if (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a)
    {
        strncpy((char *)vendor, "SMSC", max_vendor_len);
        strncpy((char *)product, "Flash Media Cntrller", max_product_len);
    }
}
#endif /* CONFIG_USB_BIN_FIXUP */

#if (CONFIG_BLK_ENABLED)
static unsigned long usb_stor_read(struct udevice *dev,
                                   lbaint_t blknr,
                                   lbaint_t blkcnt,
                                   void *buffer)
#else
static unsigned long usb_stor_read(struct blk_desc *block_dev,
                                   lbaint_t blknr,
                                   lbaint_t blkcnt,
                                   void *buffer)
#endif
{
    lbaint_t start, blks;
    uintptr_t buf_addr;
    unsigned short smallblks;
    struct usb_device *udev;
    struct us_data *ss;
    int retry;
    struct scsi_cmd *srb = &usb_ccb;
#if (CONFIG_BLK_ENABLED)
    struct blk_desc *block_dev;
#endif

    if (blkcnt == 0)
        return 0;

    /* 设置设备 */
#if (CONFIG_BLK_ENABLED)
    block_dev = dev_get_uclass_platdata(dev);
    udev = dev_get_parent_priv(dev_get_parent(dev));
    DEBUG("\nusb_read: udev %d\n", block_dev->devnum);
#else
    DEBUG("\nusb_read: udev %d\n", block_dev->devnum);
    udev = usb_dev_desc[block_dev->devnum].priv;            // 从usb_dev_desc中找出对应的设备usb_device
    if (!udev)
    {
        DEBUG("%s: No device\n", __func__);
        return 0;
    }
#endif
    ss = (struct us_data *)udev->privptr;

    usb_disable_asynch(1);                                  // 禁止异步传输【*】
    usb_lock_async(udev, 1);                                // USB锁异步【*】啥都没执行
    srb->lun = block_dev->lun;                              // 逻辑单元号
    buf_addr = (uintptr_t)buffer;                           // 缓存区地址
    start = blknr;                                          // 块存储器数量
    blks = blkcnt;                                          // 块存储器大小

    DEBUG("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
           block_dev->devnum, start, blks, buf_addr);

    do
    {
        /* XXX need some comment here */
        retry = 2;                                          // 尝试次数2
        srb->pdata = (unsigned char *)buf_addr;             // 声明数据区
        if (blks > ss->max_xfer_blk)                        // 块存储器大小太大就拆分成小的
            smallblks = ss->max_xfer_blk;
        else
            smallblks = (unsigned short)blks;

retry_it:
        if (smallblks == ss->max_xfer_blk)
            usb_show_progress();                            // 进程显示【*】没什么实际用处
        srb->datalen = block_dev->blksz * smallblks;        // 拆分后数据长度
        srb->pdata = (unsigned char *)buf_addr;             // 拆分后数据缓存区起始地址
        if (usb_read_10(srb, ss, start, smallblks))         // 开始读大容量存储器【*】配置完一系列命令后调用transport发出去
        {
            DEBUG("Read ERROR\n");
            ss->flags &= ~USB_READY;                        // 标志：USB未就绪
            usb_request_sense(srb, ss);                     // 同上【*】
            if (retry--)
                goto retry_it;

            blkcnt -= blks;     /* 返回：错误 - blks */
            break;
        }

        start += smallblks;                                 // 开始位置后移
        blks -= smallblks;                                  // 未读取区域减小
        buf_addr += srb->datalen;                           // 数据缓存区指针后移
    } while (blks != 0);                                    // 没读完继续读取剩下的小块

    DEBUG("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
           start, smallblks, buf_addr);

    usb_lock_async(udev, 0);                                // USB解锁异步【*】
    usb_disable_asynch(0);                                  // 使能异步传输
    if (blkcnt >= ss->max_xfer_blk)
        DEBUG("\n");

    return blkcnt;
}

#if (CONFIG_BLK_ENABLED)
static unsigned long usb_stor_write(struct udevice *dev,
                                    lbaint_t blknr,
                                    lbaint_t blkcnt,
                                    const void *buffer)
#else
static unsigned long usb_stor_write(struct blk_desc *block_dev,
                                    lbaint_t blknr,
                                    lbaint_t blkcnt,
                                    const void *buffer)
#endif
{
    lbaint_t start, blks;
    uintptr_t buf_addr;
    unsigned short smallblks;
    struct usb_device *udev;
    struct us_data *ss;
    int retry;
    struct scsi_cmd *srb = &usb_ccb;
#if (CONFIG_BLK_ENABLED)
    struct blk_desc *block_dev;
#endif

    if (blkcnt == 0)
        return 0;

    /* 设置设备 */
#if (CONFIG_BLK_ENABLED)
    block_dev = dev_get_uclass_platdata(dev);
    udev = dev_get_parent_priv(dev_get_parent(dev));
    DEBUG("\nusb_read: udev %d\n", block_dev->devnum);
#else
    DEBUG("\nusb_read: udev %d\n", block_dev->devnum);
    udev = usb_dev_desc[block_dev->devnum].priv;
    if (!udev)
    {
        DEBUG("%s: No device\n", __func__);
        return 0;
    }
#endif
    ss = (struct us_data *)udev->privptr;

    usb_disable_asynch(1);                                  // 禁止异步传输【*】
    usb_lock_async(udev, 1);                                // USB锁异步【*】

    srb->lun = block_dev->lun;                              // 逻辑单元号
    buf_addr = (uintptr_t)buffer;                           // 缓存区地址
    start = blknr;                                          // 块存储器数量
    blks = blkcnt;                                          // 块存储器大小

    DEBUG("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
           block_dev->devnum, start, blks, buf_addr);

    do
    {
        /* 如果写入失败重试最大重试次数，否则返回成功写入的块数。 */
        retry = 2;                                          // 重复次数2
        srb->pdata = (unsigned char *)buf_addr;             // 声明数据区
        if (blks > ss->max_xfer_blk)                        // 块存储器大小太大就拆分成小的
            smallblks = ss->max_xfer_blk;
        else
            smallblks = (unsigned short) blks;

retry_it:
        if (smallblks == ss->max_xfer_blk)
            usb_show_progress();                            // 进程显示【*】没什么实际用处
        srb->datalen = block_dev->blksz * smallblks;        // 数据长度
        srb->pdata = (unsigned char *)buf_addr;             // 数据区地址
        if (usb_write_10(srb, ss, start, smallblks))        // 开始写大容量存储器【*】配置完一系列命令后调用transport发出去
        {
            DEBUG("Write ERROR\n");
            ss->flags &= ~USB_READY;                        // 标志：USB未就绪
            usb_request_sense(srb, ss);                     // 同上【*】
            if (retry--)
                goto retry_it;

            blkcnt -= blks;     /* 返回：错误 - blks */
            break;
        }

        start += smallblks;                                 // 开始位置后移
        blks -= smallblks;                                  // 未读取区域减小
        buf_addr += srb->datalen;                           // 数据缓存区指针后移
    } while (blks != 0);                                    // 没读完继续读取剩下的小块

    DEBUG("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
           start, smallblks, buf_addr);

    usb_lock_async(udev, 0);                                // USB解锁异步【*】
    usb_disable_asynch(0);                                  // 使能异步传输【*】
    if (blkcnt >= ss->max_xfer_blk)
        DEBUG("\n");

    return blkcnt;
}

/* 探测新设备是否实际上是存储设备 */
int usb_storage_probe(struct usb_device *dev,
                      unsigned int ifnum,
                      struct us_data *ss)
{
    struct usb_interface *iface;                // USB接口
    int i;
    struct usb_endpoint_descriptor *ep_desc;    // USB端点描述符
    unsigned int flags = 0;

    /* 现在让我们检查一下设备  */
    iface = &dev->config.if_desc[ifnum];

    if (dev->descriptor.bDeviceClass != 0 ||
        iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
        iface->desc.bInterfaceSubClass < US_SC_MIN ||
        iface->desc.bInterfaceSubClass > US_SC_MAX)
    {
        DEBUG("Not mass storage\n");
        /* 如果它不是大容量存储，我们就不会再进一步了 */
        return 0;
    }

    memset(ss, 0, sizeof(struct us_data));                          // 分配存储空间【*】

    /* 对于这个端点, 他就是大容量存储器 */
    DEBUG("\n\nUSB Mass Storage device detected\n");

    /* 使用一些有用的信息初始化 us_data 结构 */
    ss->flags          = flags;                                     // 标志
    ss->ifnum          = ifnum;                                     // 接口数量
    ss->pusb_dev       = dev;                                       // 当前设备 从us_data即可找到usb_dev设备
    ss->attention_done = 0;                                         // 关注的第一条指令
    ss->subclass       = iface->desc.bInterfaceSubClass;            // 子类别
    ss->protocol       = iface->desc.bInterfaceProtocol;            // 协议

    /* 根据协议设置处理程序指针 */
    DEBUG("Transport: ");
    switch (ss->protocol)
    {
        case US_PR_CB:                                              // 协议支持的传输模式有：Control/Bulk w/o interrupt
            DEBUG("Control/Bulk\n");
            ss->transport = usb_stor_CB_transport;                  // 控制/批量（CB）传输程序【*】1
            ss->transport_reset = usb_stor_CB_reset;                // 控制/批量（CB）重置程序【*】
            break;

        case US_PR_CBI:                                             // 协议支持的传输模式有：Control/Bulk/Interrupt
            DEBUG("Control/Bulk/Interrupt\n");
            ss->transport = usb_stor_CB_transport;                  // 同上【*】
            ss->transport_reset = usb_stor_CB_reset;                // 同上【*】
            break;

        case US_PR_BULK:                                            // 协议支持的传输模式有：只有Bulk
            DEBUG("Bulk/Bulk/Bulk\n");
            ss->transport = usb_stor_BBB_transport;                 // 批量传输程序【*】
            ss->transport_reset = usb_stor_BBB_reset;               // 批量重置程序【*】
            break;

        default:
            printf("USB Storage Transport unknown / not yet implemented\n");
            return 0;
    }

    /*
     * 我们预计至少有 2 个端点 - 输入和输出（批量）。
     * 一个中断端点是可选的（对于 CBI 协议是必需的）。
     * 我们将忽略任何其他人。
     */
    for (i = 0; i < iface->desc.bNumEndpoints; i++)
    {
        ep_desc = &iface->ep_desc[i];
        /* 它是一个批量传输端点吗? */
        if ((ep_desc->bmAttributes &
             USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
        {
            if (ep_desc->bEndpointAddress & USB_DIR_IN)
                ss->ep_in = ep_desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            else
                ss->ep_out = ep_desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
        }

        /* 它是一个中断传输端点吗? */
        if ((ep_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT)
        {
            ss->ep_int = ep_desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            ss->irqinterval = ep_desc->bInterval;
        }
    }

    DEBUG("Endpoints In %d Out %d Int %d\n", ss->ep_in, ss->ep_out, ss->ep_int);

    /* 做一些基本的完整性检查，如果我们发现问题就结束 */
    if (usb_set_interface(dev, iface->desc.bInterfaceNumber, 0) ||
        !ss->ep_in || !ss->ep_out ||
        (ss->protocol == US_PR_CBI && ss->ep_int == 0))
    {
        DEBUG("Problems with device\n");
        return 0;
    }

    /* 设置类特定的东西 */
    /* 我们只处理某些协议。 目前，只有这些。
     * SFF8070 接受 u-boot 中使用的请求
     */
    if (ss->subclass != US_SC_UFI &&
        ss->subclass != US_SC_SCSI &&
        ss->subclass != US_SC_8070)
    {
        printf("Sorry, protocol %d not yet supported.\n", ss->subclass);
        return 0;
    }

    if (ss->ep_int)
    {
        /* 我们找到了一个中断端点，准备 irq 管道设置 IRQ 管道和处理程序 */
        ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;        // 中断间隔
        ss->irqpipe     = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);             // 中断管道
        ss->irqmaxp     = usb_maxpacket(dev, ss->irqpipe);                      // 中断最大值
        dev->irq_handle = usb_stor_irq;                                         // 中断句柄【*】
    }

    /* 设置每个主机控制器设置的最大传输大小 */
    usb_stor_set_max_xfer_blk(dev, ss);                                         // 设置最大传输大小为240【*】
    dev->privptr = (void *)ss;
    return 1;
}

int usb_stor_get_info(struct usb_device *dev,       
                      struct us_data *ss,
                      struct blk_desc *dev_desc)
{
    unsigned char perq, modi;
    ALLOC_CACHE_ALIGN_BUFFER(unsigned int, cap, 2);                 // 申请缓存区空间【*】
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, usb_stor_buf, 36);
    unsigned int capacity, blksz;                                   // 容量，块存储器大小
    struct scsi_cmd *pccb = &usb_ccb;                               // 小型计算机接口命令

    pccb->pdata = usb_stor_buf;                                     // 数据区为新建的缓存区
    dev_desc->target = dev->devnum;                                 // 目标设备的设备号
    pccb->lun = dev_desc->lun;                                      // 逻辑单元号
    DEBUG(" address %d\n", dev_desc->target);

    if (usb_inquiry(pccb, ss))                                      // USB查询【*】向设备发送五次查询指令，如果有一次收到答复则成功
    {
        DEBUG("%s: usb_inquiry() failed\n", __func__);
        return -1;
    }

    perq = usb_stor_buf[0];
    modi = usb_stor_buf[1];

    /*
     * 跳过未知设备（0x1f）和附件服务设备（0x0d），它们不会响应 test_unit_ready 。
     */
    if (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d))
    {
        DEBUG("%s: unknown/unsupported device\n", __func__);
        return 0;
    }

    if ((modi & 0x80) == 0x80)
    {
        /* 驱动器是可移动的 */
        dev_desc->removable = 1;
    }

    memcpy(dev_desc->vendor,   (const void *)&usb_stor_buf[8],  8); // 拷贝制造商
    memcpy(dev_desc->product,  (const void *)&usb_stor_buf[16], 16);// 拷贝产品号
    memcpy(dev_desc->revision, (const void *)&usb_stor_buf[32], 4); // 拷贝序列号

    dev_desc->vendor[8]   = 0;                                      // 清除原有数据
    dev_desc->product[16] = 0;
    dev_desc->revision[4] = 0;

#ifdef CONFIG_USB_BIN_FIXUP
    usb_bin_fixup(dev->descriptor, (unsigned char *)dev_desc->vendor,
                  (unsigned char *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */

    DEBUG("ISO Vers %X, Response Data %X\n", usb_stor_buf[2], usb_stor_buf[3]);
    if (usb_test_unit_ready(pccb, ss))                              // 判断USB测试单元是否准备就绪【*】重复发10次请求确认连接（有一次连接就行）
    {
        printf("Device NOT ready\n"
               "   Request Sense returned %02X %02X %02X\n",
                pccb->sense_buf[2], pccb->sense_buf[12], pccb->sense_buf[13]);

        if (dev_desc->removable == 1)
            dev_desc->type = perq;
        return 0;
    }

    pccb->pdata = (unsigned char *)cap;
    memset(pccb->pdata, 0, 8);                                      // 将此数据区清零
    if (usb_read_capacity(pccb, ss) != 0)                           // USB读取容量【*】重复三次发送命令
    {
        printf("READ_CAP ERROR\n");
        ss->flags &= ~USB_READY;
        cap[0] = 2880;
        cap[1] = 0x200;
    }

    DEBUG("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);

#ifdef __MIPSEL

	cap[0] = ((unsigned long)(
		    (((unsigned long)(cap[0]) & (unsigned long)0x000000ffUL) << 24) |
	        (((unsigned long)(cap[0]) & (unsigned long)0x0000ff00UL) <<  8) |
		    (((unsigned long)(cap[0]) & (unsigned long)0x00ff0000UL) >>  8) |
		    (((unsigned long)(cap[0]) & (unsigned long)0xff000000UL) >> 24) ));
	cap[1] = ((unsigned long)(
		    (((unsigned long)(cap[1]) & (unsigned long)0x000000ffUL) << 24) |
		    (((unsigned long)(cap[1]) & (unsigned long)0x0000ff00UL) <<  8) |
		    (((unsigned long)(cap[1]) & (unsigned long)0x00ff0000UL) >>  8) |
		    (((unsigned long)(cap[1]) & (unsigned long)0xff000000UL) >> 24) ));

#endif

    capacity = cap[0] + 1;
    blksz    = cap[1];

    DEBUG("Capacity = 0x%08x, blocksz = 0x%08x\n", capacity, blksz);
    dev_desc->lba       = capacity;                                 // 块存储器数量
    dev_desc->blksz     = blksz;                                    // 块存储器大小
    dev_desc->log2blksz = LOG2(dev_desc->blksz);                    // 块存储器大小取对数【*】方便计算
    dev_desc->type      = perq;                                     // 设备类型
    DEBUG(" address %d\n", dev_desc->target);

    return 1;
}

#if (DM_USB)

static int usb_mass_storage_probe(struct udevice *dev)
{
    struct usb_device *udev = dev_get_parent_priv(dev);
    int ret;

    usb_disable_asynch(1); /* asynch transfer not allowed */
    ret = usb_stor_probe_device(udev);
    usb_disable_asynch(0); /* asynch transfer allowed */

    return ret;
}

static const struct udevice_id usb_mass_storage_ids[] =
{
    { .compatible = "usb-mass-storage" },
    { }
};

U_BOOT_DRIVER(usb_mass_storage) =
{
    .name     = "usb_mass_storage",
    .id       = UCLASS_MASS_STORAGE,
    .of_match = usb_mass_storage_ids,
    .probe    = usb_mass_storage_probe,
#if (CONFIG_BLK_ENABLED)
    .platdata_auto_alloc_size = sizeof(struct us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) =
{
    .id   = UCLASS_MASS_STORAGE,
    .name = "usb_mass_storage",
};

static const struct usb_device_id mass_storage_id_table[] =
{
    {
        .match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
        .bInterfaceClass = USB_CLASS_MASS_STORAGE
    },
    { }     /* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);

#endif

#if (CONFIG_BLK_ENABLED)

static const struct blk_ops usb_storage_ops =
{
    .read  = usb_stor_read,
    .write = usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) =
{
    .name = "usb_storage_blk",
    .id   = UCLASS_BLK,
    .ops  = &usb_storage_ops,
};

#endif

#endif



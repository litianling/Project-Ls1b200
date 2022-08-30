// SPDX-License-Identifier: GPL-2.0+
/*
 * URB OHCI HCD (Host Controller Driver) for USB on the AT91RM9200 and PCI bus.
 *
 * Interrupt support is added. Now, it has been tested
 * on ULI1575 chip and works well with USB keyboard.
 *
 * (C) Copyright 2007
 * Zhang Wei, Freescale Semiconductor, Inc. <wei.zhang@freescale.com>
 *
 * (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 * Note: Much of this code has been derived from Linux 2.4
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2002 David Brownell
 *
 * Modified for the MP2USB by (C) Copyright 2005 Eric Benard
 * ebenard@eukrea.com - based on s3c24x0's driver
 */
/*
 * IMPORTANT NOTES
 * 1 - Read doc/README.generic_usb_ohci
 * 2 - this driver is intended for use with USB Mass Storage Devices
 *     (BBB) and USB keyboard. There is NO support for Isochronous pipes!
 * 2 - when running on a PQFP208 AT91RM9200, define CONFIG_AT91C_PQFP_UHPBUG
 *     to activate workaround for bug #41 or this driver will NOT work!
 */

#include "usb_cfg.h"

#if BSP_USE_OHCI

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "cpu.h"
#include "bsp.h"
#include "../libc/lwmem.h"
//#include "linux_kernel.h"
#include "linux.h"
#include "memalign.h"
#include "usb.h"
#include "ohci.h"

#if defined(LS1B)
#include "ls1b.h"
#elif defined(LS1C)
#include "ls1c.h"
#else
#error "No Loongson SoC defined!"
#endif

#define OHCI_MEM_POOL   0

/*
 * Loongson 1B: NO
 */
#define OHCI_USE_NPS        /* force NoPowerSwitching mode */

/* For initializing controller (mask in an HCFS mode too) */
#define OHCI_CONTROL_INIT \
    (OHCI_CTRL_CBSR & 0x3) | OHCI_CTRL_IE | OHCI_CTRL_PLE

#undef OHCI_VERBOSE_DEBUG   /* not always helpful */
#undef DEBUG
#undef SHOW_INFO
#undef OHCI_FILL_TRACE


#define DBG(format, arg...) do {} while (0)
#define ERR(format, arg...) printf("ERROR: " format "\n", ## arg)

#ifdef SHOW_INFO
#define INFO(format, arg...) printf("INFO: " format "\n", ## arg)
#else
#define INFO(format, arg...) do {} while (0)
#endif

#if (DM_USB)
/*
 * The various ohci_DELAY_MS(1) calls in the code seem unnecessary. We keep
 * them around when building for older boards not yet converted to the dm
 * just in case (to avoid regressions), for dm this turns them into nops.
 */
#define ohci_mdelay(x)
#else
#define ohci_mdelay(x)   DELAY_MS(x)
#endif

#if (!DM_USB)
/* 全局变量 ohci_t */
static ohci_t g_OHCI;

/* 必须分配256字节 */
struct ohci_hcca g_HCCA[1];
#endif

/* mapping of the OHCI CC status to error codes */
static int cc_to_error[16] =
{
    /* No  Error  */    0,
    /* CRC Error  */    USB_ST_CRC_ERR,
    /* Bit Stuff  */    USB_ST_BIT_ERR,
    /* Data Togg  */    USB_ST_CRC_ERR,
    /* Stall      */    USB_ST_STALLED,
    /* DevNotResp */    -1,
    /* PIDCheck   */    USB_ST_BIT_ERR,
    /* UnExpPID   */    USB_ST_BIT_ERR,
    /* DataOver   */    USB_ST_BUF_ERR,
    /* DataUnder  */    USB_ST_BUF_ERR,
    /* reservd    */    -1,
    /* reservd    */    -1,
    /* BufferOver */    USB_ST_BUF_ERR,
    /* BuffUnder  */    USB_ST_BUF_ERR,
    /* Not Access */    -1,
    /* Not Access */    -1
};

static const char *cc_to_string[16] =
{
    "No Error",
    "CRC: Last data packet from endpoint contained a CRC error.",
    "BITSTUFFING: Last data packet from endpoint contained a bit " \
            "stuffing violation",
    "DATATOGGLEMISMATCH: Last packet from endpoint had data toggle PID\n" \
            "that did not match the expected value.",
    "STALL: TD was moved to the Done Queue because the endpoint returned" \
            " a STALL PID",
    "DEVICENOTRESPONDING: Device did not respond to token (IN) or did\n" \
            "not provide a handshake (OUT)",
    "PIDCHECKFAILURE: Check bits on PID from endpoint failed on data PID\n"\
            "(IN) or handshake (OUT)",
    "UNEXPECTEDPID: Receive PID was not valid when encountered or PID\n" \
            "value is not defined.",
    "DATAOVERRUN: The amount of data returned by the endpoint exceeded\n" \
            "either the size of the maximum data packet allowed\n" \
            "from the endpoint (found in MaximumPacketSize field\n" \
            "of ED) or the remaining buffer size.",
    "DATAUNDERRUN: The endpoint returned less than MaximumPacketSize\n" \
            "and that amount was not sufficient to fill the\n" \
            "specified buffer",
    "reserved1",
    "reserved2",
    "BUFFEROVERRUN: During an IN, HC received data from endpoint faster\n" \
            "than it could be written to system memory",
    "BUFFERUNDERRUN: During an OUT, HC could not retrieve data from\n" \
            "system memory fast enough to keep up with data USB " \
            "data rate.",
    "NOT ACCESSED: This code is set by software before the TD is placed" \
            "on a list to be processed by the HC.(1)",
    "NOT ACCESSED: This code is set by software before the TD is placed" \
            "on a list to be processed by the HC.(2)",
};

static inline unsigned int roothub_a(struct ohci *hc)
{
    return ohci_readl(&hc->regs->roothub.a);
}

static inline unsigned int roothub_b(struct ohci *hc)
{
    return ohci_readl(&hc->regs->roothub.b);
}

static inline unsigned int roothub_status(struct ohci *hc)
{
    return ohci_readl(&hc->regs->roothub.status);
}

static inline unsigned int roothub_portstatus(struct ohci *hc, int i)
{
    return ohci_readl(&hc->regs->roothub.portstatus[i]);
}

/* forward declaration */
static int hc_interrupt(ohci_t *ohci);
static void td_submit_job(ohci_t *ohci,
                          struct usb_device *dev,
                          unsigned long pipe,
                          void *buffer,
                          int transfer_len,
                          struct devrequest *setup,
                          urb_priv_t *urb,
                          int interval);
static int ep_link(ohci_t * ohci, ed_t * ed);
static int ep_unlink(ohci_t * ohci, ed_t * ed);
static ed_t *ep_add_ed(ohci_dev_t *ohci_dev,
                       struct usb_device *usb_dev,
                       unsigned long pipe,
                       int interval,
                       int load);

//-------------------------------------------------------------------------------------------------

/* TDs ... */
static struct td *td_alloc(ohci_dev_t *ohci_dev, struct usb_device *usb_dev)
{
    int i;
    struct td *td;

    td = NULL;
    for (i = 0; i < NUM_TD; i++)
    {
        if (ohci_dev->tds[i].usb_dev == NULL)
        {
            td = &ohci_dev->tds[i];
            td->usb_dev = usb_dev;
            break;
        }
    }

    return td;
}

static inline void ed_free(struct ed *ed)
{
    ed->usb_dev = NULL;
}

//-------------------------------------------------------------------------------------------------
// URB support functions
//-------------------------------------------------------------------------------------------------

#if (OHCI_MEM_POOL)
/*
 * 使用 urb_priv_t 缓冲区管理 urb
 */
 
#define URB_POOL_MAX    16

static urb_priv_t urb_pool[URB_POOL_MAX];

static void ohci_urb_pool_init(void)
{
    int i;
    for (i=0; i<URB_POOL_MAX; i++)
        urb_pool[i].idle = 1;
}

static urb_priv_t *ohci_urb_pool_alloc(void)
{
    int i;
    for (i=0; i<URB_POOL_MAX; i++)
    {
        if (urb_pool[i].idle)
        {
            memset(&urb_pool[i], 0, sizeof(urb_priv_t));
            return &urb_pool[i];
        }
    }

    return NULL;
}

static void ohci_urb_pool_free(urb_priv_t *urb)
{
    urb->idle = 1;
}
#endif // #if (OHCI_MEM_POOL)

/* 释放与此 URB 关联的 HCD 私有数据 */
static void urb_free_priv(urb_priv_t *urb)
{
    int i, last;
    struct td *td;

    last = urb->length - 1;
    if (last >= 0)
    {
        for (i = 0; i <= last; i++)
        {
            td = urb->td[i];
            if (td)
            {
                td->usb_dev = NULL;         // 各项参数初始化
                urb->td[i] = NULL;
            }
        }
    }

#if (OHCI_MEM_POOL)
    ohci_urb_pool_free(urb);
#else
    FREE(urb);                              // 【*】
#endif
}

//-------------------------------------------------------------------------------------------------

#ifdef DEBUG
static int sohci_get_current_frame_number(ohci_t *ohci);

/* debug| print the main components of an URB
 * small: 0) header + data packets 1) just header */

static void pkt_print(ohci_t *ohci,
                      urb_priv_t *purb,
                      struct usb_device *dev,
                      unsigned long pipe,
                      void *buffer,
                      int transfer_len,
                      struct devrequest *setup,
                      char *str,
                      int small)
{
    DBG("%s URB:[%4x] dev:%2lu,ep:%2lu-%c,type:%s,len:%d/%d stat:%#lx",
        str,
        sohci_get_current_frame_number(ohci),
        usb_pipedevice(pipe),
        usb_pipeendpoint(pipe),
        usb_pipeout(pipe)? 'O': 'I',
        usb_pipetype(pipe) < 2 ? \
            (usb_pipeint(pipe)? "INTR": "ISOC"): \
            (usb_pipecontrol(pipe)? "CTRL": "BULK"),
        (purb ? purb->actual_length : 0),
        transfer_len, dev->status);

#ifdef OHCI_VERBOSE_DEBUG
    if (!small)
    {
        int i, len;

        if (usb_pipecontrol(pipe))
        {
            printf(__FILE__ ": cmd(8):");
            for (i = 0; i < 8 ; i++)
                printf(" %02x", ((unsigned char *) setup) [i]);
            printf("\n");
        }

        if (transfer_len > 0 && buffer)
        {
            printf(__FILE__ ": data(%d/%d):", (purb ? purb->actual_length : 0), transfer_len);
            len = usb_pipeout(pipe)? transfer_len: (purb ? purb->actual_length : 0);
            for (i = 0; i < 16 && i < len; i++)
                printf(" %02x", ((unsigned char *) buffer) [i]);
            printf("%s\n", i < len? "...": "");
        }
    }
#endif
}

/* just for debugging; prints non-empty branches of the int ed tree
 * inclusive iso eds */
void ep_print_int_eds(ohci_t *ohci, char *str)
{
    int i, j;
    unsigned int *ed_p;

    for (i = 0; i < 32; i++)
    {
        j = 5;
        ed_p = &(ohci->hcca->int_table[i]);
        if (*ed_p == 0)
            continue;
        //invalidate_dcache_ed(ed_p);
        clean_dcache_nowrite();
        printf(__FILE__ ": %s branch int %2d(%2x):", str, i, i);

        while (*ed_p != 0 && j--)
        {
            ed_t *ed = (ed_t *)ed_p;
            //invalidate_dcache_ed(ed);
            clean_dcache_nowrite();
            printf(" ed: %4x;", ed->hwINFO);
            ed_p = &ed->hwNextED;
        }

        printf("\n");
    }
}

static void ohci_dump_intr_mask(char *label, unsigned int mask)
{
    DBG("%s: 0x%08x %s%s%s%s%s%s%s%s%s",
        label,
        mask,
        (mask & OHCI_INTR_MIE)  ? " MIE" : "",
        (mask & OHCI_INTR_OC)   ? " OC" : "",
        (mask & OHCI_INTR_RHSC) ? " RHSC" : "",
        (mask & OHCI_INTR_FNO)  ? " FNO" : "",
        (mask & OHCI_INTR_UE)   ? " UE" : "",
        (mask & OHCI_INTR_RD)   ? " RD" : "",
        (mask & OHCI_INTR_SF)   ? " SF" : "",
        (mask & OHCI_INTR_WDH)  ? " WDH" : "",
        (mask & OHCI_INTR_SO)   ? " SO" : ""
        );
}

static void maybe_print_eds(char *label, unsigned int value)
{
    ed_t *edp = (ed_t *)value;

    if (value)
    {
        DBG("%s %08x", label, value);
        //invalidate_dcache_ed(edp);
        clean_dcache_nowrite();
        DBG("%08x", edp->hwINFO);
        DBG("%08x", edp->hwTailP);
        DBG("%08x", edp->hwHeadP);
        DBG("%08x", edp->hwNextED);
    }
}

static char *hcfs2string(int state)
{
    switch (state)
    {
        case OHCI_USB_RESET:    return "reset";
        case OHCI_USB_RESUME:   return "resume";
        case OHCI_USB_OPER:     return "operational";
        case OHCI_USB_SUSPEND:  return "suspend";
    }

    return "?";
}

/* dump control and status registers */
static void ohci_dump_status(ohci_t *controller)
{
    struct ohci_regs *regs = controller->regs;
    unsigned int temp;

    temp = ohci_readl(&regs->revision) & 0xff;
    if (temp != 0x10)
        DBG("spec %d.%d", (temp >> 4), (temp & 0x0f));

    temp = ohci_readl(&regs->control);
    DBG("control: 0x%08x%s%s%s HCFS=%s%s%s%s%s CBSR=%d", temp,
        (temp & OHCI_CTRL_RWE) ? " RWE" : "",
        (temp & OHCI_CTRL_RWC) ? " RWC" : "",
        (temp & OHCI_CTRL_IR)  ? " IR" : "",
        hcfs2string(temp & OHCI_CTRL_HCFS),
        (temp & OHCI_CTRL_BLE) ? " BLE" : "",
        (temp & OHCI_CTRL_CLE) ? " CLE" : "",
        (temp & OHCI_CTRL_IE)  ? " IE" : "",
        (temp & OHCI_CTRL_PLE) ? " PLE" : "",
        temp & OHCI_CTRL_CBSR );

    temp = ohci_readl(&regs->cmdstatus);
    DBG("cmdstatus: 0x%08x SOC=%d%s%s%s%s", temp,
        (temp & OHCI_SOC) >> 16,
        (temp & OHCI_OCR) ? " OCR" : "",
        (temp & OHCI_BLF) ? " BLF" : "",
        (temp & OHCI_CLF) ? " CLF" : "",
        (temp & OHCI_HCR) ? " HCR" : "" );

    ohci_dump_intr_mask("intrstatus", ohci_readl(&regs->intrstatus));
    ohci_dump_intr_mask("intrenable", ohci_readl(&regs->intrenable));

    maybe_print_eds("ed_periodcurrent", ohci_readl(&regs->ed_periodcurrent));

    maybe_print_eds("ed_controlhead", ohci_readl(&regs->ed_controlhead));
    maybe_print_eds("ed_controlcurrent", ohci_readl(&regs->ed_controlcurrent));

    maybe_print_eds("ed_bulkhead", ohci_readl(&regs->ed_bulkhead));
    maybe_print_eds("ed_bulkcurrent", ohci_readl(&regs->ed_bulkcurrent));

    maybe_print_eds("donehead", ohci_readl(&regs->donehead));
}

static void ohci_dump_roothub(ohci_t *controller, int verbose)
{
    unsigned int temp, ndp, i;

    temp = roothub_a(controller);
    ndp = (temp & RH_A_NDP);
#ifdef CONFIG_AT91C_PQFP_UHPBUG
    ndp = (ndp == 2) ? 1:0;
#endif

    if (verbose)
    {
        DBG("roothub.a: %08x POTPGT=%d%s%s%s%s%s NDP=%d", temp,
            ((temp & RH_A_POTPGT) >> 24) & 0xff,
            (temp & RH_A_NOCP) ? " NOCP" : "",
            (temp & RH_A_OCPM) ? " OCPM" : "",
            (temp & RH_A_DT) ?   " DT" : "",
            (temp & RH_A_NPS) ?  " NPS" : "",
            (temp & RH_A_PSM) ?  " PSM" : "",
            ndp );

        temp = roothub_b(controller);
        DBG("roothub.b: %08x PPCM=%04x DR=%04x",
            temp,
            (temp & RH_B_PPCM) >> 16,
            (temp & RH_B_DR) );

        temp = roothub_status(controller);
        DBG("roothub.status: %08x%s%s%s%s%s%s",
            temp,
            (temp & RH_HS_CRWE) ? " CRWE" : "",
            (temp & RH_HS_OCIC) ? " OCIC" : "",
            (temp & RH_HS_LPSC) ? " LPSC" : "",
            (temp & RH_HS_DRWE) ? " DRWE" : "",
            (temp & RH_HS_OCI)  ? " OCI" : "",
            (temp & RH_HS_LPS)  ? " LPS" : ""
            );
    }

    for (i = 0; i < ndp; i++)
    {
        temp = roothub_portstatus(controller, i);
        DBG("roothub.portstatus [%d] = 0x%08x%s%s%s%s%s%s%s%s%s%s%s%s",
            i,
            temp,
            (temp & RH_PS_PRSC) ? " PRSC" : "",
            (temp & RH_PS_OCIC) ? " OCIC" : "",
            (temp & RH_PS_PSSC) ? " PSSC" : "",
            (temp & RH_PS_PESC) ? " PESC" : "",
            (temp & RH_PS_CSC)  ? " CSC" : "",

            (temp & RH_PS_LSDA) ? " LSDA" : "",
            (temp & RH_PS_PPS)  ? " PPS" : "",
            (temp & RH_PS_PRS)  ? " PRS" : "",
            (temp & RH_PS_POCI) ? " POCI" : "",
            (temp & RH_PS_PSS)  ? " PSS" : "",

            (temp & RH_PS_PES)  ? " PES" : "",
            (temp & RH_PS_CCS)  ? " CCS" : "" );
    }
}

static void ohci_dump(ohci_t *ohci, int verbose)
{
    DBG("OHCI controller usb-%s state", ohci->slot_name);
    /* dumps some of the state we know about */
    ohci_dump_status(ohci);
    if (verbose)
        ep_print_int_eds(ohci, "hcca");
    //invalidate_dcache_hcca(ohci->hcca);
    clean_dcache_nowrite();
    DBG("hcca frame #%04x", ohci->hcca->frame_no);
    ohci_dump_roothub(ohci, 1);
}

#endif /* DEBUG */

//-------------------------------------------------------------------------------------------------
// Interface functions (URB)
//-------------------------------------------------------------------------------------------------

/* 获得转移请求 */

int sohci_submit_job(ohci_t *ohci, ohci_dev_t *ohci_dev, urb_priv_t *urb,
                     struct devrequest *setup)
{
    ed_t *ed;
    urb_priv_t *purb_priv = urb;
    int i, size = 0;
    struct usb_device *dev = urb->dev;
    unsigned long pipe = urb->pipe;
    void *buffer = urb->transfer_buffer;
    int transfer_len = urb->transfer_buffer_length;
    int interval = urb->interval;

    /* 当控制器挂起时，只允许 roothub 清理尝试，例如关闭端口 */
    if (ohci->disabled)
    {
        ERR("sohci_submit_job: EPIPE");
        return -1;
    }

    /* 我们即将在这里开始新的交易，所以将 URB 标记为未完成  */
    urb->finished = 0;

    /* 每个端点都有一个 ed，定位并填充它 */
    ed = ep_add_ed(ohci_dev, dev, pipe, interval, 1);               // 添加/重新初始化端点【*】端点的创建与初始化
    if (!ed)
    {
        ERR("sohci_submit_job: ENOMEM");
        return -1;
    }

    /* 根据管道类型设置单次 TD 的数量（大小） */                    // TD=transfer data传输数据
    switch (usb_pipetype(pipe))                                     // 获取管道类型【*】
    {
        case PIPE_BULK:         /* 批量管道每 4096 字节一个 TD */   // 一个包4096，共size个包
            size = (transfer_len - 1) / 4096 + 1;
            break;
        case PIPE_CONTROL:      /* 控制管道1 个 TD 用于设置，1 个用于 ACK，每个 4096 B 1 个 */
            size = (transfer_len == 0) ? 2 : (transfer_len - 1) / 4096 + 3;
            break;
        case PIPE_INTERRUPT:    /* 中断管道1 个 TD */
            size = 1;
            break;
    }

    ed->purb = urb;

    if (size >= (N_URB_TD - 1))
    {
        ERR("need %d TDs, only have %d", size, N_URB_TD);
        return -1;
    }

    /* 填写purb_priv */
    purb_priv->pipe = pipe;         // 管道赋值
    purb_priv->length = size;       // 长度赋值
    purb_priv->ed = ed;             // 端点赋值
    purb_priv->actual_length = 0;   // 接收长度

    /* 分配 传输数据 */
    /* 注意 td[0] 是在 ep_add_ed 中分配的 */
    for (i = 0; i < size; i++)
    {
        purb_priv->td[i] = td_alloc(ohci_dev, dev);     // 分配TD【*】在对应的ohci_device下的td[]中找一个空节点，让purb_priv->td[i]指针指向它
        if (!purb_priv->td[i])                          // 如果设备不存在
        {
            purb_priv->length = i;                      // 长度
            urb_free_priv(purb_priv);                   // 释放urb相关的私有数据【*】
            ERR("sohci_submit_job: ENOMEM");
            return -1;
        }
    }

    if (ed->state == ED_NEW || (ed->state & ED_DEL))    // 判断端点状态是不是新建的或者已经删除的
    {
        urb_free_priv(purb_priv);                       // 释放urb相关的私有数据【*】
        ERR("sohci_submit_job: EINVAL");
        return -1;
    }

    /* 如果端点还没有运行，则将 ed 链接到链中 */
    if (ed->state != ED_OPER)
        ep_link(ohci, ed);                              // 将端点ed链接到主机控制器HC中【*】

    /* 填写 TD 并将其链接到 ed */
    td_submit_job(ohci, dev, pipe, buffer, transfer_len, setup, purb_priv, interval);   // 准备转移的所有 TD 【*】传输在此处开始！！！！！！！

    return 0;
}

//-------------------------------------------------------------------------------------------------

#ifdef DEBUG
/* tell us the current USB frame number */
static int sohci_get_current_frame_number(ohci_t *ohci)
{
    //invalidate_dcache_hcca(ohci->hcca);
    clean_dcache_nowrite();
    return ohci->hcca->frame_no;
}
#endif

//-------------------------------------------------------------------------------------------------
// ED handling functions
//-------------------------------------------------------------------------------------------------

/* search for the right branch to insert an interrupt ed into the int tree
 * do some load ballancing;
 * returns the branch and
 * sets the interval to interval = 2^integer (ld (interval)) */

static int ep_int_ballance(ohci_t *ohci, int interval, int load)
{
    int i, branch = 0;

    /* search for the least loaded interrupt endpoint
     * branch of all 32 branches
     */
    for (i = 0; i < 32; i++)
        if (ohci->ohci_int_load [branch] > ohci->ohci_int_load [i])
            branch = i;

    branch = branch % interval;
    for (i = branch; i < 32; i += interval)
        ohci->ohci_int_load [i] += load;

    return branch;
}

//-------------------------------------------------------------------------------------------------

/*  2^int( ld (inter)) */

static int ep_2_n_interval(int inter)
{
    int i;
    for (i = 0; ((inter >> i) > 1) && (i < 5); i++);
    return 1 << i;
}

//-------------------------------------------------------------------------------------------------

/* the int tree is a binary tree
 * in order to process it sequentially the indexes of the branches have to
 * be mapped the mapping reverses the bits of a word of num_bits length */
static int ep_rev(int num_bits, int word)
{
    int i, wout = 0;

    for (i = 0; i < num_bits; i++)
        wout |= (((word >> i) & 1) << (num_bits - i - 1));
    return wout;
}

//-------------------------------------------------------------------------------------------------
// ED handling functions
//-------------------------------------------------------------------------------------------------

/* 通过写寄存器将 ed 链接到 HC 链之一 */

static int ep_link(ohci_t *ohci, ed_t *edi)
{
    volatile ed_t *ed = edi;
    int int_branch;
    int i;
    int inter;
    int interval;
    int load;
    unsigned int *ed_p;

    ed->state = ED_OPER;
    ed->int_interval = 0;

    switch (ed->type)
    {
        case PIPE_CONTROL:                                                              // 如果是控制管道
            ed->hwNextED = 0;
            clean_dcache((unsigned int)ed, sizeof(ed_t));
            if (ohci->ed_controltail == NULL)
            	ohci_writel(K0_TO_PHYS((unsigned int)ed), &ohci->regs->ed_controlhead);	// 物理
            else
            	ohci->ed_controltail->hwNextED = K0_TO_PHYS((unsigned int)ed); 			// 物理

            ed->ed_prev = ohci->ed_controltail;
            if (!ohci->ed_controltail && !ohci->ed_rm_list[0] && !ohci->ed_rm_list[1] && !ohci->sleeping)
            {
                ohci->hc_control |= OHCI_CTRL_CLE;
                ohci_writel(ohci->hc_control, &ohci->regs->control);
            }
            ohci->ed_controltail = edi;
            break;

        case PIPE_BULK:                                                                 // 如果是批量管道
            ed->hwNextED = 0;
            clean_dcache((unsigned int)ed, sizeof(ed_t));
            if (ohci->ed_bulktail == NULL)
            	ohci_writel(K0_TO_PHYS((unsigned int)ed), &ohci->regs->ed_bulkhead);	// 物理
            else
            	ohci->ed_bulktail->hwNextED = K0_TO_PHYS((unsigned int)ed);				// 物理

            ed->ed_prev = ohci->ed_bulktail;
            if (!ohci->ed_bulktail && !ohci->ed_rm_list[0] &&
                !ohci->ed_rm_list[1] && !ohci->sleeping)
            {
                ohci->hc_control |= OHCI_CTRL_BLE;
                ohci_writel(ohci->hc_control, &ohci->regs->control);
            }
            ohci->ed_bulktail = edi;
            break;

        case PIPE_INTERRUPT:                                                            // 如果是中断管道
            load = ed->int_load;
            interval = ep_2_n_interval(ed->int_period);
            ed->int_interval = interval;
            int_branch = ep_int_ballance(ohci, interval, load);
            ed->int_branch = int_branch;

            for (i = 0; i < ep_rev(6, interval); i += inter)
            {
                inter = 1;

                for (ed_p = &(ohci->hcca->int_table[ep_rev(5, i) + int_branch]);
                    (*ed_p != 0) && (((ed_t *)ed_p)->int_interval >= interval);
                     ed_p = &(((ed_t *)ed_p)->hwNextED))
                    inter = ep_rev(6, ((ed_t *)ed_p)->int_interval);

                ed->hwNextED = *ed_p;
                clean_dcache((unsigned int)ed, sizeof(ed_t));
                *ed_p = K0_TO_PHYS((unsigned int)ed);					// 物理
            }
            break;
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------

/* scan the periodic table to find and unlink this ED */
static void periodic_unlink(struct ohci *ohci, volatile struct ed *ed,
                            unsigned index, unsigned period)
{
    /*__maybe_unused *///unsigned long aligned_ed_p;

    for (; index < NUM_INTS; index += period)
    {
        unsigned int *ed_p = &ohci->hcca->int_table[index];

        /* ED might have been unlinked through another path */
        while (*ed_p != 0)
        {
            if (((struct ed *)/*(uintptr_t)(unsigned long)*/ed_p) == ed)
            {
                *ed_p = PHYS_TO_K0((unsigned int)ed->hwNextED);
                //aligned_ed_p = (unsigned long)ed_p;
                //aligned_ed_p &= ~(ARCH_DMA_MINALIGN - 1);
                //flush_dcache_buffer(aligned_ed_p, aligned_ed_p + ARCH_DMA_MINALIGN);
                clean_dcache((unsigned int)ed_p, sizeof(ed_t));

                break;
            }
/*
 * TODO
 */
            ed_p = &(((struct ed *)/*(uintptr_t)(unsigned long)*/ed_p)->hwNextED);
        }
    }
}

/* unlink an ed from one of the HC chains.
 * just the link to the ed is unlinked.
 * the link from the ed still points to another operational ed or 0
 * so the HC can eventually finish the processing of the unlinked ed */

static int ep_unlink(ohci_t *ohci, ed_t *edi)
{
    volatile ed_t *ed = edi;
    int i;

    ed->hwINFO |= OHCI_ED_SKIP;
    //flush_dcache_ed(ed);
    clean_dcache((unsigned int)ed, sizeof(ed_t));

    switch (ed->type)
    {
        case PIPE_CONTROL:
            if (ed->ed_prev == NULL)
            {
                if (!ed->hwNextED)
                {
                    ohci->hc_control &= ~OHCI_CTRL_CLE;
                    ohci_writel(ohci->hc_control, &ohci->regs->control);
                }

                ohci_writel(*((unsigned int *)&ed->hwNextED), &ohci->regs->ed_controlhead);
            }
            else
            {
                ed->ed_prev->hwNextED = ed->hwNextED;
                //flush_dcache_ed(ed->ed_prev);
                clean_dcache((unsigned int)ed->ed_prev, sizeof(ed_t));
            }

            if (ohci->ed_controltail == ed)
            {
                ohci->ed_controltail = ed->ed_prev;
            }
            else
            {
                //((ed_t *)(uintptr_t)*((unsigned int *)&ed->hwNextED))->ed_prev = ed->ed_prev;
				((ed_t *)PHYS_TO_K0(ed->hwNextED))->ed_prev = ed->ed_prev;
            }
            break;

        case PIPE_BULK:
            if (ed->ed_prev == NULL)
            {
                if (!ed->hwNextED)
                {
                    ohci->hc_control &= ~OHCI_CTRL_BLE;
                    ohci_writel(ohci->hc_control, &ohci->regs->control);
                }

                ohci_writel(*((unsigned int *)&ed->hwNextED), &ohci->regs->ed_bulkhead);
            }
            else
            {
                ed->ed_prev->hwNextED = ed->hwNextED;
                //flush_dcache_ed(ed->ed_prev);
                clean_dcache((unsigned int)ed->ed_prev, sizeof(ed_t));
            }

            if (ohci->ed_bulktail == ed)
            {
                ohci->ed_bulktail = ed->ed_prev;
            }
            else
            {
                //((ed_t *)(uintptr_t)*((unsigned int *)&ed->hwNextED))->ed_prev = ed->ed_prev;
				((ed_t *)PHYS_TO_K0(ed->hwNextED))->ed_prev = ed->ed_prev;
            }
            break;

        case PIPE_INTERRUPT:
            periodic_unlink(ohci, ed, 0, 1);
            for (i = ed->int_branch; i < 32; i += ed->int_interval)
                ohci->ohci_int_load[i] -= ed->int_load;
            break;
    }

    ed->state = ED_UNLINK;
    return 0;
}

//-------------------------------------------------------------------------------------------------

/* 添加/重新初始化端点； 这应该在usb_set_configuration 命令，但是 USB 堆栈有点无状态，所以如果 ed 的状态是 ED_NEW，我们在每次事务中执行它，
  然后添加一个虚拟 td，并且在所有其他情况下状态更改为 ED_UNLINK，状态保持不变 ed info 字段被设置，即使它们中的大多数不应该改变
 */
static ed_t *ep_add_ed(ohci_dev_t *ohci_dev, struct usb_device *usb_dev,
                       unsigned long pipe, int interval, int load)
{
    td_t *td;
    ed_t *ed_ret;
    volatile ed_t *ed;

    ed = ed_ret = &ohci_dev->ed[(usb_pipeendpoint(pipe) << 1) |
                                (usb_pipecontrol(pipe) ? 0 : usb_pipeout(pipe))];       // 端点选择【*】

    if ((ed->state & ED_DEL) || (ed->state & ED_URB_DEL))
    {
        ERR("ep_add_ed: pending delete");
        /* 待处理的删除请求 */
        return NULL;
    }

    if (ed->state == ED_NEW)
    {
        /* 虚拟 td; ed 的 td 列表结束 */
        td = td_alloc(ohci_dev, usb_dev);                                               // 创建TD【*】
        ed->hwTailP = K0_TO_PHYS((unsigned int)td);
        ed->hwHeadP = ed->hwTailP;
        ed->state = ED_UNLINK;                                                          // 端点状态
        ed->type = usb_pipetype(pipe);                                                  // 端点管道类型【*】
        ohci_dev->ed_cnt++;
    }

    ed->hwINFO = usb_pipedevice(pipe) |                                                 // 写端点的相关信息
                 usb_pipeendpoint(pipe) << 7 |
                 (usb_pipeisoc(pipe) ? 0x8000: 0) |
                 (usb_pipecontrol(pipe) ? 0: (usb_pipeout(pipe)? 0x800: 0x1000)) |
                 (usb_dev->speed == USB_SPEED_LOW) << 13 |
                 usb_maxpacket(usb_dev, pipe) << 16;

    if (ed->type == PIPE_INTERRUPT && ed->state == ED_UNLINK)                           // 如果端点是中断类型的，端点状态...
    {
        ed->int_period = interval;
        ed->int_load = load;
    }

    clean_dcache((unsigned int)ed, sizeof(ed_t));                   // 清除高速缓存【*】

    return ed_ret;
}

//-------------------------------------------------------------------------------------------------
// TD handling functions
//-------------------------------------------------------------------------------------------------

/* 为这个 URB 排队下一个 TD (OHCI spec 5.2.8.2) */

static void td_fill(ohci_t *ohci, unsigned int info, void *data, int len,
                    struct usb_device *dev, int index, urb_priv_t *urb_priv)
{
    volatile td_t *td, *td_pt;
#ifdef OHCI_FILL_TRACE
    int i;
#endif

    if (index > urb_priv->length)
    {
        ERR("index > length");
        return;
    }

    /* 使用这个 td */
    td_pt = urb_priv->td[index];
    td_pt->hwNextTD = 0;
    clean_dcache((unsigned int)td_pt, sizeof(td_t));        // 【*】

    /* fill the old dummy TD */
    td = urb_priv->td[index] = (td_t *)PHYS_TO_K0(urb_priv->ed->hwTailP & ~0x0F);

    td->ed = urb_priv->ed;  // 端点
    td->next_dl_td = NULL;  // 下一个为空
    td->index = index;      // 指针
    td->data = (unsigned int/*uintptr_t*/)data; // 接收数据缓存区

#ifdef OHCI_FILL_TRACE
    if (usb_pipebulk(urb_priv->pipe) && usb_pipeout(urb_priv->pipe))
    {
        for (i = 0; i < len; i++)
            printf("td->data[%d] %#2x ", i, ((unsigned char *)td->data)[i]);
        printf("\n");
    }
#endif

    if (!len)
        data = 0;

    td->hwINFO = info;
    td->hwCBP = K0_TO_PHYS((unsigned int)data);
    if (data)
    	td->hwBE  = K0_TO_PHYS((unsigned int)(data + len - 1));
    else
        td->hwBE = 0;

    td->hwNextTD = K0_TO_PHYS((unsigned int)td_pt);
    clean_dcache((unsigned int)td, sizeof(td_t));

    /* 追加到队列 */
    td->ed->hwTailP = td->hwNextTD;
    clean_dcache((unsigned int)td->ed, sizeof(ed_t));
}

//-------------------------------------------------------------------------------------------------

/* 准备传输的所有 TD */

static void td_submit_job(ohci_t *ohci,
                          struct usb_device *dev,
                          unsigned long pipe,
                          void *buffer,
                          int transfer_len,
                          struct devrequest *setup,
                          urb_priv_t *urb,
                          int interval)
{
    int data_len = transfer_len;
    void *data;
    int cnt = 0;
    unsigned int info = 0;
    unsigned int toggle = 0;

    clean_dcache((unsigned int)buffer, data_len);                       // 清除高速缓存

    /* OHCI 自己处理数据切换，我们只使用 USB 切换位进行重置 */
    if (usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe)))  // D0/D1 切换位【*】
    {
        toggle = TD_T_TOGGLE;
    }
    else
    {
        toggle = TD_T_DATA0;
        usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 1);
    }

    urb->td_cnt = 0;
    if (data_len)                                                       // 数据长度不为0，将数据存储指向bufer
        data = buffer;
    else
        data = 0;

    switch (usb_pipetype(pipe))                                         // 根据管道类型配置TD【*】
    {
        case PIPE_BULK:
            info = usb_pipeout(pipe) ? TD_CC | TD_DP_OUT : TD_CC | TD_DP_IN ;
            while (data_len > 4096)
            {
                td_fill(ohci, info | (cnt ? TD_T_TOGGLE:toggle), data, 4096, dev, cnt, urb);  // 为这个urb安排下一个TD【*】
                data += 4096; data_len -= 4096; cnt++;  // 数据指针+4096，数据长度-4096
            }

            info = usb_pipeout(pipe) ? TD_CC | TD_DP_OUT : TD_CC | TD_R | TD_DP_IN ;
            td_fill(ohci, info | (cnt ? TD_T_TOGGLE:toggle), data, data_len, dev, cnt, urb);
            cnt++;

            if (!ohci->sleeping)
            {
                /* 开始批量列表 */
                ohci_writel(OHCI_BLF, &ohci->regs->cmdstatus);                              // 写32位寄存器开始批量传输【*】
            }
            break;

        case PIPE_CONTROL:
            /* 设置阶段 */
            info = TD_CC | TD_DP_SETUP | TD_T_DATA0;
            //flush_dcache_buffer(setup, 8);
            clean_dcache((unsigned int)setup, 8);               // 同上【*】
            td_fill(ohci, info, setup, 8, dev, cnt++, urb);     // 同上【*】

            /* 可选数据阶段 */
            if (data_len > 0)
            {
                info = usb_pipeout(pipe) ? TD_CC | TD_R | TD_DP_OUT | TD_T_DATA1 :
                                           TD_CC | TD_R | TD_DP_IN | TD_T_DATA1;
                /* NOTE:  mishandles transfers >8K, some >4K */
                td_fill(ohci, info, data, data_len, dev, cnt++, urb);   // 同上【*】
            }

            /* 状态阶段e */
            info = (usb_pipeout(pipe) || data_len == 0) ? TD_CC | TD_DP_IN | TD_T_DATA1:
                                                          TD_CC | TD_DP_OUT | TD_T_DATA1;
            td_fill(ohci, info, data, 0, dev, cnt++, urb);      // 同上【*】

            if (!ohci->sleeping)
            {
                /* 启动控制列表t */
                ohci_writel(OHCI_CLF, &ohci->regs->cmdstatus);                              // 写32位寄存器开始控制传输【*】
            }
            break;

        case PIPE_INTERRUPT:
            info = usb_pipeout(urb->pipe) ? TD_CC | TD_DP_OUT | toggle:
                                            TD_CC | TD_R | TD_DP_IN | toggle;
            td_fill(ohci, info, data, data_len, dev, cnt++, urb);       // 同上【*】
            break;
    }

    if (urb->length != cnt)
        DBG("TD LENGTH %d != CNT %d", urb->length, cnt);
}

//-------------------------------------------------------------------------------------------------
// Done List handling functions
//-------------------------------------------------------------------------------------------------

/* calculate the transfer length and update the urb */

static void dl_transfer_length(td_t *td)
{
    unsigned int tdBE, tdCBP;
    urb_priv_t *lurb_priv = td->ed->purb;

    tdBE  = td->hwBE;
#if 1
// XXX Loongson PMON
    tdCBP = PHYS_TO_K0((unsigned int)td->hwCBP);
#else
    tdCBP = td->hwCBP;
#endif

    if (!(usb_pipecontrol(lurb_priv->pipe) &&
        ((td->index == 0) || (td->index == lurb_priv->length - 1))))
    {
        if (tdBE != 0)
        {
            if (td->hwCBP == 0)
                //lurb_priv->actual_length += tdBE - td->data + 1;
            	lurb_priv->actual_length += PHYS_TO_K0(tdBE) - K1_TO_K0(td->data) + 1;
            else
                //lurb_priv->actual_length += tdCBP - td->data;
            	lurb_priv->actual_length += PHYS_TO_K0(tdCBP) - K1_TO_K0(td->data);
        }
    }
}

//-------------------------------------------------------------------------------------------------

static void check_status(td_t *td_list)
{
    urb_priv_t *lurb_priv = td_list->ed->purb;
    int urb_len = lurb_priv->length;
    unsigned int *phwHeadP = &td_list->ed->hwHeadP;
    int cc;

    cc = TD_CC_GET(td_list->hwINFO);
    if (cc)
    {
        ERR(" USB-error: %s (%x)", cc_to_string[cc], cc);
        clean_dcache_nowrite((unsigned int)td_list->ed, sizeof(ed_t));
        if (*phwHeadP & 0x1)
        {
            if (lurb_priv && ((td_list->index + 1) < urb_len))
            {
                *phwHeadP = (lurb_priv->td[urb_len - 1]->hwNextTD & 0xfffffff0) |
                            (*phwHeadP & 0x2);

                lurb_priv->td_cnt += urb_len - td_list->index - 1;
            }
            else
                *phwHeadP &= 0xfffffff2;
            clean_dcache((unsigned int)td_list->ed, sizeof(ed_t));
        }
    }
}

/* 对请求的回复必须基于 FIFO，因此我们反转了反向的完成列表 */
static td_t *dl_reverse_done_list(ohci_t *ohci)
{
    uintptr_t td_list_hc;
    td_t *td_rev = NULL;
    td_t *td_list = NULL;

    td_list_hc = ohci->hcca->done_head & 0xfffffff0;
    ohci->hcca->done_head = 0;

    while (td_list_hc)
    {
		td_list = (td_t *)PHYS_TO_K0(td_list_hc);
        clean_dcache_nowrite((unsigned int)td_list, sizeof(td_t));
        check_status(td_list);
        td_list->next_dl_td = td_rev;
        td_rev = td_list;
        td_list_hc = td_list->hwNextTD & 0xfffffff0;
    }

    return td_list;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

static void finish_urb(ohci_t *ohci, urb_priv_t *urb, int status)
{
    if ((status & (ED_OPER | ED_UNLINK)) && (urb->state != URB_DEL))
        urb->finished = 1;
    else
        DBG("finish_urb: strange.., ED state %x, \n", status);
}

/*
 * Used to take back a TD from the host controller. This would normally be
 * called from within dl_done_list, however it may be called directly if the
 * HC no longer sees the TD and it has not appeared on the donelist (after
 * two frames).  This bug has been observed on ZF Micro systems.
 */
static int takeback_td(ohci_t *ohci, td_t *td_list)
{
    ed_t *ed;
    int cc;
    int stat = 0;
    /* urb_t *urb; */
    urb_priv_t *lurb_priv;
    unsigned int tdINFO, edHeadP, edTailP;

    //invalidate_dcache_td(td_list);
    clean_dcache_nowrite((unsigned int)td_list, sizeof(td_t));
    tdINFO = td_list->hwINFO;

    ed = td_list->ed;
    lurb_priv = ed->purb;

    dl_transfer_length(td_list);

    lurb_priv->td_cnt++;

    /* error code of transfer */
    cc = TD_CC_GET(tdINFO);
    if (cc)
    {
        ERR("USB-error: %s (%x)", cc_to_string[cc], cc);
        stat = cc_to_error[cc];
    }

    /* see if this done list makes for all TD's of current URB,
    * and mark the URB finished if so */
    if (lurb_priv->td_cnt == lurb_priv->length)
        finish_urb(ohci, lurb_priv, ed->state);

    DBG("dl_done_list: processing TD %x, len %x\n", lurb_priv->td_cnt, lurb_priv->length);

    if (ed->state != ED_NEW && (!usb_pipeint(lurb_priv->pipe)))
    {
        //invalidate_dcache_ed(ed);
        clean_dcache_nowrite((unsigned int)ed, sizeof(ed_t));
        edHeadP = ed->hwHeadP & 0xfffffff0;
        edTailP = ed->hwTailP;

        /* unlink eds if they are not busy */
        if ((edHeadP == edTailP) && (ed->state == ED_OPER))
            ep_unlink(ohci, ed);
    }

    return stat;
}

// delete down list? 删除已经做完的列表？
static int dl_done_list(ohci_t *ohci)
{
    int stat = 0;
    td_t *td_list = dl_reverse_done_list(ohci);                 // 【】1

    while (td_list)
    {
        td_t *td_next = td_list->next_dl_td;
        stat = takeback_td(ohci, td_list);                      // 取回来TD【】
        td_list = td_next;
    }

    return stat;
}

//-------------------------------------------------------------------------------------------------
// Virtual Root Hub
//-------------------------------------------------------------------------------------------------

#include "usbroothubdes.h"

/* Hub class-specific descriptor is constructed dynamically */

//-------------------------------------------------------------------------------------------------

#define OK(x)               len = (x); break

#ifdef DEBUG
#define WR_RH_STAT(x)       { INFO("WR:status %#8x", (x)); \
                              ohci_writel((x), &ohci->regs->roothub.status); }
#define WR_RH_PORTSTAT(x)   { INFO("WR:portstatus[%d] %#8x", wIndex-1, (x)); \
                              ohci_writel((x), &ohci->regs->roothub.portstatus[wIndex-1]); }
#else
#define WR_RH_STAT(x)       ohci_writel((x), &ohci->regs->roothub.status)
#define WR_RH_PORTSTAT(x)   ohci_writel((x), &ohci->regs->roothub.portstatus[wIndex-1])
#endif

#define RD_RH_STAT          roothub_status(ohci)
#define RD_RH_PORTSTAT      roothub_portstatus(ohci, wIndex-1)

/* request to virtual root hub */

int rh_check_port_status(ohci_t *controller)
{
    unsigned int temp, ndp, i;
    int res;

    res = -1;
    temp = roothub_a(controller);
    ndp = (temp & RH_A_NDP);
#ifdef CONFIG_AT91C_PQFP_UHPBUG
    ndp = (ndp == 2) ? 1:0;
#endif

    for (i = 0; i < ndp; i++)
    {
        temp = roothub_portstatus(controller, i);
        /* check for a device disconnect */
        if (((temp & (RH_PS_PESC | RH_PS_CSC)) ==
            (RH_PS_PESC | RH_PS_CSC)) &&
        	((temp & RH_PS_CCS) == 0))
        {
            res = i;
            break;
        }
    }

    return res;
}

static int ohci_submit_rh_msg(ohci_t *ohci,
                              struct usb_device *dev,
                              unsigned long pipe,
                              void *buffer,
                              int transfer_len,
                              struct devrequest *cmd)
{
    void *data = buffer;                // 数据指针指向缓存区
    int leni = transfer_len;            // 传输长度
    int len = 0;                        
    int stat = 0;
    unsigned short bmRType_bReq;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
    ALLOC_ALIGN_BUFFER(unsigned char, databuf, 16, sizeof(unsigned int));   // 分配存储器空间【*】（类型，变量名，大小，匹配）

#ifdef DEBUG
    pkt_print(ohci, NULL, dev, pipe, buffer, transfer_len,
              cmd, "SUB(rh)", usb_pipein(pipe));
#else
    ohci_mdelay(1);
#endif

    if (usb_pipeint(pipe))                                                  // 管道初始化【*】初始化此管道对应的中断
    {
        INFO("Root-Hub submit IRQ: NOT implemented");
        return 0;
    }

    bmRType_bReq = cmd->requesttype | (cmd->request << 8);                  // 命令：请求类型和请求
    wValue       = cmd->value;                                              // 命令：值
    wIndex       = cmd->index;                                              // 命令：指针
    wLength      = cmd->length;                                             // 命令：长度

    INFO("Root-Hub: adr: %2x cmd(%1x): %08x %04x %04x %04x",
          dev->devnum, 8, bmRType_bReq, wValue, wIndex, wLength);

    switch (bmRType_bReq)                                                   // 根据请求进行设置
    {                                                                       
    /* 请求目标:
       没有标识: 设备,
       RH_INTERFACE: 接口,
       RH_ENDPOINT: 端点,
       RH_CLASS 代表集线器类,
       RH_OTHER | RH_CLASS  代表集线器端口
    */
        case RH_GET_STATUS:
            *(unsigned short *)databuf = 1; OK(2);                          // databuf是新开辟的空间，OK【*】参数即为给len赋值
        case RH_GET_STATUS | RH_INTERFACE:
            *(unsigned short *)databuf = 0; OK(2);
        case RH_GET_STATUS | RH_ENDPOINT:
            *(unsigned short *)databuf = 0; OK(2);
        case RH_GET_STATUS | RH_CLASS:
            *(unsigned int *)databuf = RD_RH_STAT & ~(RH_HS_CRWE | RH_HS_DRWE); OK(4);
        case RH_GET_STATUS | RH_OTHER | RH_CLASS:
            *(unsigned int *)databuf = RD_RH_PORTSTAT; OK(4);

        case RH_CLEAR_FEATURE | RH_ENDPOINT:                                // 清除端点且挂起
            switch (wValue)
            {
                case (RH_ENDPOINT_STALL): OK(0);
            }
            break;
        case RH_CLEAR_FEATURE | RH_CLASS:                                   // 清除类，是不是本地电源，
            switch (wValue)                                                 // 电流过载执行WR_RH_STAT【*】写寄存器（OHCI/寄存器列表/根寄存器.状态寄存器）
            {
                case RH_C_HUB_LOCAL_POWER: OK(0);
                case (RH_C_HUB_OVER_CURRENT):                               
                    WR_RH_STAT(RH_HS_OCIC); OK(0);
            }
            break;
        case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:                        // 清除其他，根据wValue值改变控制寄存器的状态
            switch (wValue)                                                 // WR_RH_PORTSTAT【*】写寄存器（OHCI/寄存器列表/根寄存器.端点状态寄存器数组中寄存器）
            {
                case (RH_PORT_ENABLE):
                    WR_RH_PORTSTAT(RH_PS_CCS); OK(0);                       
                case (RH_PORT_SUSPEND):
                    WR_RH_PORTSTAT(RH_PS_POCI); OK(0);
                case (RH_PORT_POWER):
                    WR_RH_PORTSTAT(RH_PS_LSDA); OK(0);
                case (RH_C_PORT_CONNECTION):
                    WR_RH_PORTSTAT(RH_PS_CSC); OK(0);
                case (RH_C_PORT_ENABLE):
                    WR_RH_PORTSTAT(RH_PS_PESC); OK(0);
                case (RH_C_PORT_SUSPEND):
                    WR_RH_PORTSTAT(RH_PS_PSSC); OK(0);
                case (RH_C_PORT_OVER_CURRENT):
                    WR_RH_PORTSTAT(RH_PS_OCIC); OK(0);
                case (RH_C_PORT_RESET):
                    WR_RH_PORTSTAT(RH_PS_PRSC); OK(0);
            }
            break;
            
        case RH_SET_FEATURE | RH_OTHER | RH_CLASS:                          // 设置其他类
            switch (wValue)                                                 // 还是用原函数改端口寄存器状态【*】
            {
                case (RH_PORT_SUSPEND):
                    WR_RH_PORTSTAT(RH_PS_PSS); OK(0);
                case (RH_PORT_RESET):       /* 集线器代码有BUG */
                    if (RD_RH_PORTSTAT & RH_PS_CCS)
                        WR_RH_PORTSTAT(RH_PS_PRS); OK(0);
                case (RH_PORT_POWER):
                    WR_RH_PORTSTAT(RH_PS_PPS); OK(0);
                case (RH_PORT_ENABLE):      /* 集线器代码有BUG */
                    if (RD_RH_PORTSTAT & RH_PS_CCS)
                        WR_RH_PORTSTAT(RH_PS_PES); OK(0);
            }
            break;
        case RH_SET_ADDRESS:
            ohci->rh.devnum = wValue; OK(0);                                // 虚拟集线器设备号设置为wValue

        case RH_GET_DESCRIPTOR:                                             // 获取描述符（本质核心：根据wValue设定数据缓存区databuf及数据长度len）
            switch ((wValue & 0xff00) >> 8)
            {
                case (0x01):        /* 设备描述符 */
                    len = min_t(unsigned int, leni,                         // min_t【*】两个类型取最小。例如leni，sizeof(root_hub_dev_des), wLength三者的最小值
                          min_t(unsigned int, sizeof(root_hub_dev_des), wLength));
                    databuf = root_hub_dev_des;
                    OK(len);
                case (0x02):        /* 配置描述符 */
                    len = min_t(unsigned int, leni,
                          min_t(unsigned int, sizeof(root_hub_config_des), wLength));
                    databuf = root_hub_config_des;
                    OK(len);
                case (0x03):        /* 字符串描述符 */
                    if (wValue == 0x0300)
                    {
                        len = min_t(unsigned int, leni,
                              min_t(unsigned int, sizeof(root_hub_str_index0), wLength));
                        databuf = root_hub_str_index0;
                        OK(len);
                    }
                    if (wValue == 0x0301)
                    {
                        len = min_t(unsigned int, leni,
                              min_t(unsigned int, sizeof(root_hub_str_index1), wLength));
                        databuf = root_hub_str_index1;
                        OK(len);
                    }
                default:
                    stat = USB_ST_STALLED;// 挂起
            }
            break;

        case RH_GET_DESCRIPTOR | RH_CLASS:                                  // 获取类描述符
        {
            unsigned int temp = roothub_a(ohci);                            // 【*】

            databuf[0] = 9;     /* min length; */
            databuf[1] = 0x29;
            databuf[2] = temp & RH_A_NDP;
#ifdef CONFIG_AT91C_PQFP_UHPBUG
            databuf[2] = (databuf[2] == 2) ? 1 : 0;
#endif
            databuf[3] = 0;
            if (temp & RH_A_PSM)                /* 每个端口的电源选择? */
                databuf[3] |= 0x1;
            if (temp & RH_A_NOCP)               /* 无过流报告? */
                databuf[3] |= 0x10;
            else if (temp & RH_A_OCPM)          /* 每个端口的过流报告? */
                databuf[3] |= 0x8;
            databuf[4] = 0;
            databuf[5] = (temp & RH_A_POTPGT) >> 24;
            databuf[6] = 0;
            temp = roothub_b(ohci);             // 读寄存器【*】（OHCI/寄存器列表/根集线器.B参数）
            databuf[7] = temp & RH_B_DR;
            if (databuf[2] < 7)
            {
                databuf[8] = 0xff;
            }
            else
            {
                databuf[0] += 2;
                databuf[8]  = (temp & RH_B_DR) >> 8;
                databuf[10] = databuf[9] = 0xff;
            }
            len = min_t(unsigned int, leni,
                  min_t(unsigned int, databuf[0], wLength));
            OK(len);
        }

        case RH_GET_CONFIGURATION:                  // 获取配置
            databuf[0] = 0x01; OK(1);
        case RH_SET_CONFIGURATION:                  // 设置配置【*】
            WR_RH_STAT(0x10000); OK(0);
        default:
            DBG("unsupported root hub command");    // 不支持的根集线器命令
            stat = USB_ST_STALLED;                  // 状态设置为挂起
    }

#ifdef DEBUG
    ohci_dump_roothub(ohci, 1);
#else
    ohci_mdelay(1);
#endif

    len = min_t(int, len, leni);                    // 取最小值【*】
    if (data != databuf)
        memcpy(data, databuf, len);                 // 拷贝描述符信息
    dev->act_len = len;                             // 接收应答的长度
    dev->status = stat;                             // 更新状态信息

#ifdef DEBUG
    pkt_print(ohci, NULL, dev, pipe, buffer, transfer_len, cmd, "RET(rh)", 0 /*usb_pipein(pipe)*/);
#else
    ohci_mdelay(1);
#endif

    return stat;
}

//-------------------------------------------------------------------------------------------------

static ohci_dev_t *ohci_get_ohci_dev(ohci_t *ohci, int devnum, int intr)
{
    int i;

    if (!intr)
        return &ohci->ohci_dev;

    /* 查找OHCI之前是否与此设备建立连接 */
    for (i = 0; i < NUM_INT_DEVS; i++)
    {
        if (ohci->int_dev[i].devnum == devnum)          // 根据设备号在int_dev查找设备，返回
            return &ohci->int_dev[i];
    }

    /* 如果没有建立连接，那就找一个空余位置 */
    for (i = 0; i < NUM_INT_DEVS; i++)
    {
        if (ohci->int_dev[i].devnum == -1)              // 找个空余位置新建设备并返回
        {
            ohci->int_dev[i].devnum = devnum;
            return &ohci->int_dev[i];
        }
    }

    printf("ohci: Error out of ohci_devs for interrupt endpoints\n");       // 即找不到设备，又没有空位置返回NULL
    return NULL;
}

/* 处理提交消息的通用代码 - 用于除根集线器访问之外的所有访问。 */
static urb_priv_t *ohci_alloc_urb(struct usb_device *dev, unsigned long pipe,
                                  void *buffer, int transfer_len, int interval)
{
    urb_priv_t *urb;

#if (OHCI_MEM_POOL)
    urb = ohci_urb_pool_alloc();
#else
    urb = CALLOC(1, sizeof(urb_priv_t));                        // 空间分配【*】
#endif

    if (!urb)
    {
        printf("ohci: Error out of memory allocating urb\n");
        return NULL;
    }

    urb->dev = dev;                                             // 设备
    urb->pipe = pipe;                                           // 管道
    urb->transfer_buffer = buffer;                              // 传输缓存
    urb->transfer_buffer_length = transfer_len;                 // 传输长度
    urb->interval = interval;                                   // 间隔

    return urb;
}

static int submit_common_msg(ohci_t *ohci,
                             struct usb_device *dev,
                             unsigned long pipe,
                             void *buffer,
                             int transfer_len,
                             struct devrequest *setup,
                             int interval)
{
    int stat = 0;
    int maxsize = usb_maxpacket(dev, pipe);                             // 获取最大数据包的大小【*】取决于管道和方向
    int timeout;
    urb_priv_t *urb;
    ohci_dev_t *ohci_dev;

    urb = ohci_alloc_urb(dev, pipe, buffer, transfer_len, interval);    // OHCI配置USB请求块【*】设备，管道，存储区等
    if (!urb)
        return -ENOMEM;

#ifdef DEBUG
    urb->actual_length = 0;
    pkt_print(ohci, urb, dev, pipe, buffer, transfer_len, setup, "SUB", usb_pipein(pipe));
#else
    ohci_mdelay(1);
#endif

    if (!maxsize)
    {
        ERR("submit_common_message: pipesize for pipe %lx is zero", pipe);
        return -1;
    }

    ohci_dev = ohci_get_ohci_dev(ohci, dev->devnum, usb_pipeint(pipe)); // 获取OHCI设备【*】根据设备号查找，找不到就在空位新建，都不行返回错误代码
    if (!ohci_dev)
        return -ENOMEM;

    if (sohci_submit_job(ohci, ohci_dev, urb, setup) < 0)                // OHCI向下位机提交任务【*】（传输在它里面！！！！！）
    {
        ERR("sohci_submit_job failed");
        return -1;
    }

    ohci_mdelay(10);

    /* OHCI转储状态(ohci); */
    timeout = USB_TIMEOUT_MS(pipe);                                     // 配置不同管道的提交超时时间【*】

    /* 等待成功完成上一步 */
    for ( ; ; )
    {
        /*检查控制器工作是否结束*/
        stat = hc_interrupt(ohci);                                      // 当中断发生时【*】中断传输在此时发生
        if (stat < 0)
        {
            stat = USB_ST_CRC_ERR;
            break;
        }

        /* 注意：由于我们不是 U-Boot 中的中断驱动并且总是一次只处理一个 URB，我们不能假设事务在第一次从 hc_interrupt()
        成功返回时完成。除非设置了当前 URB 的标志，这意味着 所有进出设备的 TD 均已实际传输和处理。 如果当前 URB 未完成，
        我们需要重新迭代此循环，以便再次调用 hc_interrupt()，因为需要更多的 TD 来处理 */
        if ((stat >= 0) && (stat != 0xff) && (urb->finished))           // 正常结束
        {
            /* 0xff 是SF中断的返回值 */
            break;
        }

        if (--timeout)                                                  // 没有超时，时间--
        {
            ohci_mdelay(1);
            if (!urb->finished)
                DBG("*");

        }
        else                                                            // 否则超时
        {
            if (!usb_pipeint(pipe))
                ERR("CTL:TIMEOUT ");
            DBG("submit_common_msg: TO status %x\n", stat);
            urb->finished = 1;
            stat = USB_ST_CRC_ERR;
            break;
        }
    }

    dev->status = stat;
    dev->act_len = urb->actual_length;

    if (usb_pipein(pipe) && dev->status == 0 && dev->act_len)           // 如果设备状态为0并且返回长度不为0=>状态异常
        clean_dcache_nowrite((unsigned int)buffer, dev->act_len);       // 清除高速缓存【*】

#ifdef DEBUG
    pkt_print(ohci, urb, dev, pipe, buffer, transfer_len, setup, "RET(ctlr)", usb_pipein(pipe));
#else
    ohci_mdelay(1);
#endif

    urb_free_priv(urb);                                                 // 释放与此 URB 关联的 HCD 私有数据【*】
    return 0;
}

//-------------------------------------------------------------------------------------------------

#define MAX_INT_QUEUESIZE     8

struct int_queue
{
    int queuesize;
    int curr_urb;
    urb_priv_t *urb[MAX_INT_QUEUESIZE];
    int idle;
};

#if (OHCI_MEM_POOL)
/*
 * 用 pool 管理 queue
 */
#define QUEUE_POOL_MAX    MAX_INT_QUEUESIZE

static struct int_queue queue_pool[QUEUE_POOL_MAX];

static void ohci_queue_pool_init(void)
{
    int i;
    for (i=0; i<QUEUE_POOL_MAX; i++)
        queue_pool[i].idle = 1;
}

static struct int_queue *ohci_queue_pool_alloc(void)
{
    int i;
    for (i=0; i<QUEUE_POOL_MAX; i++)
    {
        if (queue_pool[i].idle)
        {
            memset(&queue_pool[i], 0, sizeof(struct int_queue));
            return &queue_pool[i];
        }
    }

    return NULL;
}

static void ohci_queue_pool_free(struct int_queue *queue)
{
    queue->idle = 1;
}
#endif

//-------------------------------------------------------------------------------------------------

static struct int_queue *_ohci_create_int_queue(ohci_t *ohci,
                                                struct usb_device *udev,
                                                unsigned long pipe,
                                                int queuesize,
                                                int elementsize,
                                                void *buffer,
                                                int interval)
{
    struct int_queue *queue;
    ohci_dev_t *ohci_dev;
    int i;

    if (queuesize > MAX_INT_QUEUESIZE)
        return NULL;

    ohci_dev = ohci_get_ohci_dev(ohci, udev->devnum, 1);
    if (!ohci_dev)
        return NULL;

#if (OHCI_MEM_POOL)
    queue = ohci_queue_pool_alloc();
#else
    queue = MALLOC(sizeof(*queue));
#endif

    if (!queue)
    {
        printf("ohci: Error out of memory allocating int queue\n");
        return NULL;
    }

    for (i = 0; i < queuesize; i++)
    {
        queue->urb[i] = ohci_alloc_urb(udev, pipe, buffer + i * elementsize, elementsize, interval);
        if (!queue->urb[i])
            break;

        if (sohci_submit_job(ohci, ohci_dev, queue->urb[i], NULL))
        {
            printf("ohci: Error submitting int queue job\n");
            urb_free_priv(queue->urb[i]);
            break;
        }
    }

    if (i == 0)
    {
        /* We did not succeed in submitting even 1 urb */
#if (OHCI_MEM_POOL)
        ohci_queue_pool_free(queue);
#else
        FREE(queue);
#endif
        return NULL;
    }

    queue->queuesize = i;
    queue->curr_urb = 0;

    return queue;
}

static void *_ohci_poll_int_queue(ohci_t *ohci, struct usb_device *udev,
                                  struct int_queue *queue)
{
    if (queue->curr_urb == queue->queuesize)
        return NULL; /* Queue depleted */

    if (hc_interrupt(ohci) < 0)
        return NULL;

    if (queue->urb[queue->curr_urb]->finished)
    {
        void *ret = queue->urb[queue->curr_urb]->transfer_buffer;
        queue->curr_urb++;
        return ret;
    }

    return NULL;
}

static int _ohci_destroy_int_queue(ohci_t *ohci, struct usb_device *dev,
                                   struct int_queue *queue)
{
    int i;

    for (i = 0; i < queue->queuesize; i++)
        urb_free_priv(queue->urb[i]);

#if (OHCI_MEM_POOL)
    ohci_queue_pool_free(queue);
#else
    FREE(queue);
#endif

    return 0;
}

#if (!DM_USB)

/* submit routines called from usb.c */

int submit_bulk_msg(struct usb_device *dev,
                    unsigned long pipe,
                    void *buffer,
                    int transfer_len)
{
    INFO("submit_bulk_msg");
    return submit_common_msg(&g_OHCI, dev, pipe, buffer, transfer_len, NULL, 0);
}

int submit_int_msg(struct usb_device *dev,
                   unsigned long pipe,
                   void *buffer,
                   int transfer_len,
                   int interval, bool nonblock)
{
    INFO("submit_int_msg");
    return submit_common_msg(&g_OHCI, dev, pipe, buffer, transfer_len, NULL, interval);
}

struct int_queue *create_int_queue(struct usb_device *dev,
                                   unsigned long pipe,
                                   int queuesize,
                                   int elementsize,
                                   void *buffer,
                                   int interval)
{
    return _ohci_create_int_queue(&g_OHCI, dev, pipe, queuesize, elementsize, buffer, interval);
}

void *poll_int_queue(struct usb_device *dev, struct int_queue *queue)
{
    return _ohci_poll_int_queue(&g_OHCI, dev, queue);
}

int destroy_int_queue(struct usb_device *dev, struct int_queue *queue)
{
    return _ohci_destroy_int_queue(&g_OHCI, dev, queue);
}
#endif

static int _ohci_submit_control_msg(ohci_t *ohci,
                                    struct usb_device *dev,
                                    unsigned long pipe,
                                    void *buffer,
                                    int transfer_len,
                                    struct devrequest *setup)
{
    int maxsize = usb_maxpacket(dev, pipe);     // 返回USB最大数据包【*】取决于管道pipe和其方向

    INFO("submit_control_msg");
#ifdef DEBUG
    pkt_print(ohci, NULL, dev, pipe, buffer, transfer_len, setup, "SUB", usb_pipein(pipe));
#else
    ohci_mdelay(1);
#endif
    if (!maxsize)
    {
        ERR("submit_control_message: pipesize for pipe %lx is zero", pipe);
        return -1;
    }

    if (((pipe >> 8) & 0x7f) == ohci->rh.devnum)    // 经过计算后发现是OHCI虚拟根集线器的设备号
    {
        ohci->rh.dev = dev;
        /* 根集线器 - 重定向 */
        return ohci_submit_rh_msg(ohci, dev, pipe, buffer, transfer_len, setup);    // 访问根集线器【*】
    }

    return submit_common_msg(ohci, dev, pipe, buffer, transfer_len, setup, 0);      // 访问其他公共外设【*】（传输在此下面开始！！！）
}

//-------------------------------------------------------------------------------------------------
// HC functions
//-------------------------------------------------------------------------------------------------

/* 复位HC和总线 */

static int hc_reset(ohci_t *ohci)
{
    int timeout = 30;           // 复位超时时间值
    int smm_timeout = 50;       // 接管超时时间值：此值设置在0-5s之间

    DBG("%s\n", __FUNCTION__);

    if (ohci_readl(&ohci->regs->control) & OHCI_CTRL_IR)            // 读32位寄存器（OHCI/寄存器列表/控制寄存器）【*】：如果第8位中断路由为1，执行此if
    {
        /*SMM拥有HC，请求所有权 */
        ohci_writel(OHCI_OCR, &ohci->regs->cmdstatus);              // 写32位寄存器（OHCI/寄存器列表/状态寄存器）【*】：所有权变更请求
        INFO("USB HC TakeOver from SMM");
        while (ohci_readl(&ohci->regs->control) & OHCI_CTRL_IR)     // 如果中断路由为1，一直执行此while
        {
            ohci_mdelay(10);
            if (--smm_timeout == 0)
            {
                ERR("USB HC TakeOver failed!");                     // USB_HC接管失败
                return -1;
            }
        } // 【注】如果在规定时间内接管成功，中断路由标志位会被自动清零结束while，代表接管成功
    }

    /* 禁止HC中断 */
    ohci_writel(OHCI_INTR_MIE, &ohci->regs->intrdisable);           // 写32位寄存器（OHCI/寄存器列表/中断禁止寄存器）【*】

    DBG("USB HC reset_hc usb-%s: ctrl = 0x%X ;\n",
        ohci->slot_name, ohci_readl(&ohci->regs->control));

    /* 复位USB (某些控制器需要复位) */
    ohci->hc_control = 0;                                           // hc控制寄存器的备份=0
    ohci_writel(ohci->hc_control, &ohci->regs->control);            // 写32位寄存器（OHCI/寄存器列表/控制寄存器）【*】：hc控制寄存器的备份

    /* HC 复位需要最大 10 微秒的延迟 */
    ohci_writel(OHCI_HCR,  &ohci->regs->cmdstatus);                 // 写32位寄存器（OHCI/寄存器列表/状态寄存器）【*】：主机控制复位
    while ((ohci_readl(&ohci->regs->cmdstatus) & OHCI_HCR) != 0)    // 如果主机控制复位为1，一直执行此while
    {
        if (--timeout == 0)
        {
            ERR("USB HC reset timed out!");                         // USB_HC复位失败
            return -1;
        }

        DELAY_US(1);
    }// 【注】如果在规定时间内复位成功，主机控制复位标志位会被自动清零结束while，代表复位成功

    return 0;
}

//-------------------------------------------------------------------------------------------------

/* 启动 OHCI 控制器，设置总线操作
    启用中断
    连接虚拟根集线器 */

static int hc_start(ohci_t *ohci)
{
    unsigned int mask;
    unsigned int fminterval;
    int i;

    ohci->disabled = 1;                     // OHCI挂起标志置一
    for (i = 0; i < NUM_INT_DEVS; i++)      // 遍历所有中断设备并初始化为“未连接”状态
        ohci->int_dev[i].devnum = -1;

    /* 告诉控制器控制列表和批量列表在哪里（现在列表是空的） */
    ohci_writel(0, &ohci->regs->ed_controlhead);                                // 写32位寄存器（OHCI/寄存器列表/控制列表表头）【*】：空
    ohci_writel(0, &ohci->regs->ed_bulkhead);                                   // 写32位寄存器（OHCI/寄存器列表/批量列表表头）【*】：空
    ohci_writel(K0_TO_PHYS((unsigned int)ohci->hcca), &ohci->regs->hcca);       // 写32位寄存器（OHCI/寄存器列表/hcca）【*】：hcca地址

    fminterval = 0x2edf;
    ohci_writel((fminterval * 9) / 10, &ohci->regs->periodicstart);             // 写32位寄存器（OHCI/寄存器列表/帧计数器启动周期）【*】
    fminterval |= ((((fminterval - 210) * 6) / 7) << 16);
    ohci_writel(fminterval, &ohci->regs->fminterval);                           // 写32位寄存器（OHCI/寄存器列表/帧计数器间隔）【*】
    ohci_writel(0x628, &ohci->regs->lsthresh);                                  // 写32位寄存器（OHCI/寄存器列表/帧计数器阈值）【*】

    /* 启动控制器操作 */
    ohci->hc_control = OHCI_CONTROL_INIT | OHCI_USB_OPER;                       // HC控制寄存器备份：OHCI控制寄存器初始值+OHCI的USB操作
    ohci->disabled = 0;                                                         // OHCI不挂起
    ohci_writel(ohci->hc_control, &ohci->regs->control);                        // 写32位寄存器（OHCI/寄存器列表/控制寄存器）【*】：HC控制寄存器备份

    /* 禁止所有中断 */
    mask = (OHCI_INTR_SO | OHCI_INTR_WDH | OHCI_INTR_SF | OHCI_INTR_RD |
            OHCI_INTR_UE | OHCI_INTR_FNO | OHCI_INTR_RHSC |
            OHCI_INTR_OC | OHCI_INTR_MIE);
    ohci_writel(mask, &ohci->regs->intrdisable);                                // 将所有中断禁止标志位置一

    /* 清除所有中断 */
    mask &= ~OHCI_INTR_MIE;                                                     // 将mask与主中断使能相对的位清零
    ohci_writel(mask, &ohci->regs->intrstatus);                                 // 写32位寄存器（OHCI/寄存器列表/中断状态存器）【*】：中断被静止，主中断使能位为0

    /*
     * 选择我们现在关注的中断 - 但主中断使能MIE除外
     */
    mask = OHCI_INTR_RHSC | OHCI_INTR_UE | OHCI_INTR_WDH | OHCI_INTR_SO;
    ohci_writel(mask, &ohci->regs->intrenable);                                 // 写32位寄存器（OHCI/寄存器列表/中断使能存器）【*】：打开相应中断使能

#ifdef OHCI_USE_NPS
    /* AMD-756 和某些 Mac 平台需要执行此操作 */
    ohci_writel((roothub_a(ohci) | RH_A_NPS) & ~RH_A_PSM, &ohci->regs->roothub.a);  // 写32位寄存器（OHCI/寄存器列表/根集线器.描述符a）【*】原描述符A+无电源切换
    ohci_writel(RH_HS_LPSC, &ohci->regs->roothub.status);                           // 写32位寄存器（OHCI/寄存器列表/根集线器.状态）【*】：本地电源状态变化
#endif  /* OHCI_USE_NPS */

#if 0
// XXX Loongson PMON
	/* POTPGT delay is bits 24-31, in 2 ms units. */
	ohci_mdelay((roothub_a (ohci) >> 23) & 0x1fe);
	ohci_mdelay(1000);
#endif

    /* 连接虚拟跟集线器 */
    ohci->rh.devnum = 0;                                                        // 虚拟根集线器（USB总线）的设备号为0

    return 0;
}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// 当有中断发生时
//-------------------------------------------------------------------------------------------------

static int hc_interrupt(ohci_t *ohci)
{
    struct ohci_regs *regs = ohci->regs;
    int ints;
    int stat = -1;


    if ((ohci->hcca->done_head != 0) && !(ohci->hcca->done_head & 0x01))    // OHCI与HCCA正确
    {
        ints =  OHCI_INTR_WDH;
    }
    else                                                                    // 错误
    {
        ints = ohci_readl(&regs->intrstatus);                               // 读OHCI的中断状态
        if (ints == ~(unsigned int)0)
        {
            ohci->disabled++;
            ERR("%s device removed!", ohci->slot_name);
            return -1;
        }
        else
        {
            ints &= ohci_readl(&regs->intrenable);
            if (ints == 0)
            {
                return 0xff;
            }
        }
    }

    if (ints & OHCI_INTR_RHSC)                                                      // 根集线器状态更改
        stat = 0xff;

    if (ints & OHCI_INTR_UE)                                                        // 中断发生不可恢复的错误
    {
        ohci->disabled++;
        ERR("OHCI Unrecoverable Error, controller usb-%s disabled", ohci->slot_name);
        /* e.g. due to PCI Master/Target Abort */
#ifdef DEBUG
        ohci_dump(ohci, 1);  // 不能在中断里 dump
#else
        ohci_mdelay(1);
#endif
        hc_reset(ohci);                                                             // 复位OCHI设备【*】复位HC和总线
        return -1;
    }

    if (ints & OHCI_INTR_WDH)                                                       // 如果完成了头的回写，读写相关寄存器
    {
        ohci_mdelay(1);
        ohci_writel(OHCI_INTR_WDH, &regs->intrdisable);
        (void)ohci_readl(&regs->intrdisable); /* flush */
        stat = dl_done_list(ohci);                                                  // 【X】【X】【X】
        ohci_writel(OHCI_INTR_WDH, &regs->intrenable);
        (void)ohci_readl(&regs->intrdisable); /* flush */
    }

    if (ints & OHCI_INTR_SO)                                                        // 如果调度超限
    {
        ohci_writel(OHCI_INTR_SO, &regs->intrenable);
        stat = -1;
    }

    /* FIXME:  this assumes SOF (1/ms) interrupts don't get lost... */
    if (ints & OHCI_INTR_SF)                                                        // 起始帧
    {
        unsigned int frame = ohci->hcca->frame_no & 1;
        ohci_mdelay(1);
        ohci_writel(OHCI_INTR_SF, &regs->intrdisable);
        if (ohci->ed_rm_list[frame] != NULL)
            ohci_writel(OHCI_INTR_SF, &regs->intrenable);
        stat = 0xff;
    }

    ohci_writel(ints, &regs->intrstatus);                                           // 更新中断状态【*】
    return stat;
}

//-------------------------------------------------------------------------------------------------

#if (!DM_USB)

//-------------------------------------------------------------------------------------------------

/* 释放所有内存资源.. */
static void hc_release_ohci(ohci_t *ohci)
{
    DBG("USB HC release ohci usb-%s", ohci->slot_name);

    if (!ohci->disabled)    // ohci->disabled为0释放
        hc_reset(ohci);
}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// low level initalisation routine, called from usb.c
//-------------------------------------------------------------------------------------------------

static char ohci_inited = 0;

int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
    ohci_t *ohci = &g_OHCI;     // 定义结构体指针指向全局变量结构体g_OHCI

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
    if (usb_cpu_init())                             /* cpu dependant init */
        return -1;
#endif

#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
    if (board_usb_init(index, USB_INIT_HOST))       /*  board dependant init */
        return -1;
#endif

    memset((void *)ohci, 0, sizeof(ohci_t));        // 擦除OHCI的数据

    if ((unsigned int)&g_HCCA[0] & 0xff)            // 对齐存储：&g_HCCA[0]地址最后两位如果不是0x00它和0xff与运算之后必不为0x00，发生错误
    {                                               //          主控制器通信区HCCA必须是256字节对齐的
        ERR("HCCA not aligned!!");
        return -1;
    }

    ohci->hcca = (struct ohci_hcca *)K0_TO_K1((unsigned int)&g_HCCA[0]);	/* hcca K1 Address */
    // &g_HCCA[0]是hcca_K0的地址，通过K0_TO_K1函数计算出hcca_K1的地址，并将其保存在ohci->hcca中，完成设置
    
    INFO("aligned g_hcca %p", ohci->hcca);                  // 没啥用的函数，类似于DEBUG【*】
    memset(ohci->hcca, 0, sizeof(struct ohci_hcca));        // 清除ohci->hcca中的数据

    ohci->disabled = 1;                                     // OHCI挂起
    ohci->sleeping = 0;                                     // OHCI不休眠
    ohci->irq = -1;                                         // 中断请求-1（无）

    ohci->regs = (struct ohci_regs *)CONFIG_SYS_USB_OHCI_REGS_BASE;     // 初始化OHCI寄存器列表基地址0xBFE08000

    ohci->flags = 0;                                                    // 标志着HC没出现BUG
    ohci->slot_name = CONFIG_SYS_USB_OHCI_SLOT_NAME;                    // 主机控制器接口名字（EHCI/OHCI），此时为“OHCI”

    if (hc_reset(ohci) < 0)                                             // 复位这个OHCI【*】接管HC并成功复位
    {
        hc_release_ohci(ohci);                                          // 复位失败，释放OHCI所占用的存储器资源【*】又调用reset，相当于reset两次
        ERR("can't reset usb-%s", ohci->slot_name);                     // 复位失败，输出提示信息（包含复位失败的设备名称）
#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
        /* board dependant cleanup */
        board_usb_cleanup(index, USB_INIT_HOST);
#endif

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
        /* cpu dependant cleanup */
        usb_cpu_init_fail();
#endif
        return -1;
    }

    if (hc_start(ohci) < 0)                                             // 启动这个OHCI【*】：配置OHCI相关寄存器与中断，创建虚拟根集线器
    {
        ERR("can't start usb-%s", ohci->slot_name);                     // 复位失败，输出提示信息（包含启动失败的设备名称）
        hc_release_ohci(ohci);                                          // 启动失败，释放OHCI所占用的存储器资源【*】
        /* Initialization failed */
#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
        /* board dependant cleanup */
        usb_board_stop();
#endif

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
        /* cpu dependant cleanup */
        usb_cpu_stop();
#endif
        return -1;
    }

#ifdef DEBUG
    ohci_dump(ohci, 1);
#else
    ohci_mdelay(10);                                                    // OCHI延时10ms【*】
#endif

#if (OHCI_MEM_POOL)
    ohci_urb_pool_init();
    ohci_queue_pool_init();
#endif
    ohci_inited = 1;                                                    // OCHI初始化成功标志
    return 0;
}

int usb_lowlevel_stop(int index)
{
    ohci_t *ohci = &g_OHCI;
    
    /* 这很早就被调用了——甚至在控制器初始化之前！ */
    if (!ohci_inited)
        return 0;

    /* TODO 释放任何中断等。在这里调用 hc_release_ohci() ？ */
    hc_reset(ohci);                                                     // 复位OHCI和总线【*】

#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
    /* board dependant cleanup */
    if (usb_board_stop())
        return -1;
#endif

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
    /* cpu dependant cleanup */
    if (usb_cpu_stop())
        return -1;
#endif

    /* 该驱动程序不再初始化。 它需要一个新的低级初始化（板/CPU）才能再次使用。 */
    ohci_inited = 0;
    return 0;
}

int submit_control_msg(struct usb_device *dev,
                       unsigned long pipe,
                       void *buffer,
                       int transfer_len,
                       struct devrequest *setup)
{
    return _ohci_submit_control_msg(&g_OHCI, dev, pipe, buffer, transfer_len, setup);   // 通过OHCI底层发送【*】（传输在此后开始！！！！）
}
#endif

#if (DM_USB)
static int ohci_submit_control_msg(struct udevice *dev,
                                   struct usb_device *udev,
                                   unsigned long pipe,
                                   void *buffer, int length,
                                   struct devrequest *setup)
{
    ohci_t *ohci = dev_get_priv(usb_get_bus(dev));

    return _ohci_submit_control_msg(ohci, udev, pipe, buffer, length, setup);
}

static int ohci_submit_bulk_msg(struct udevice *dev,
                                struct usb_device *udev,
                                unsigned long pipe,
                                void *buffer,
                                int length)
{
    ohci_t *ohci = dev_get_priv(usb_get_bus(dev));

    return submit_common_msg(ohci, udev, pipe, buffer, length, NULL, 0);
}

static int ohci_submit_int_msg(struct udevice *dev,
                               struct usb_device *udev,
                               unsigned long pipe,
                               void *buffer,
                               int length,
                               int interval,
                               bool nonblock)
{
    ohci_t *ohci = dev_get_priv(usb_get_bus(dev));

    return submit_common_msg(ohci, udev, pipe, buffer, length, NULL, interval);
}

static struct int_queue *ohci_create_int_queue(struct udevice *dev,
                                               struct usb_device *udev,
                                               unsigned long pipe,
                                               int queuesize,
                                               int elementsize,
                                               void *buffer,
                                               int interval)
{
    ohci_t *ohci = dev_get_priv(usb_get_bus(dev));

    return _ohci_create_int_queue(ohci, udev, pipe, queuesize, elementsize, buffer, interval);
}

static void *ohci_poll_int_queue(struct udevice *dev,
                                 struct usb_device *udev,
                                 struct int_queue *queue)
{
    ohci_t *ohci = dev_get_priv(usb_get_bus(dev));

    return _ohci_poll_int_queue(ohci, udev, queue);
}

static int ohci_destroy_int_queue(struct udevice *dev,
                                  struct usb_device *udev,
                                  struct int_queue *queue)
{
    ohci_t *ohci = dev_get_priv(usb_get_bus(dev));

    return _ohci_destroy_int_queue(ohci, udev, queue);
}

int ohci_register(struct udevice *dev, struct ohci_regs *regs)
{
    struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
    ohci_t *ohci = dev_get_priv(dev);
    unsigned int reg;

    priv->desc_before_addr = true;

    ohci->regs = regs;
    // ohci->hcca = memalign(256, sizeof(struct ohci_hcca));
    ohci->hcca = aligned_malloc(sizeof(struct ohci_hcca), 256);     /* K0 address */
    TO K1

    if (!ohci->hcca)
        return -ENOMEM;
    memset(ohci->hcca, 0, sizeof(struct ohci_hcca));
    flush_dcache_hcca(ohci->hcca);

    if (hc_reset(ohci) < 0)
        return -EIO;

    if (hc_start(ohci) < 0)
        return -EIO;

    reg = ohci_readl(&regs->revision);
    printf("USB OHCI %x.%x\n", (reg >> 4) & 0xf, reg & 0xf);

    return 0;
}

int ohci_deregister(struct udevice *dev)
{
    ohci_t *ohci = dev_get_priv(dev);

    if (hc_reset(ohci) < 0)
        return -EIO;

    FREE(ohci->hcca);

    return 0;
}

struct dm_usb_ops ohci_usb_ops =
{
    .control           = ohci_submit_control_msg,
    .bulk              = ohci_submit_bulk_msg,
    .interrupt         = ohci_submit_int_msg,
    .create_int_queue  = ohci_create_int_queue,
    .poll_int_queue    = ohci_poll_int_queue,
    .destroy_int_queue = ohci_destroy_int_queue,
};

#endif

#endif

/*
 * @@ EOF
 */


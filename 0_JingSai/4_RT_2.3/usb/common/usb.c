// SPDX-License-Identifier: GPL-2.0+
/*
 * Most of this source has been derived from the Linux USB
 * project:
 * (C) Copyright Linus Torvalds 1999
 * (C) Copyright Johannes Erdfelt 1999-2001
 * (C) Copyright Andreas Gal 1999
 * (C) Copyright Gregory P. Smith 1999
 * (C) Copyright Deti Fliegl 1999 (new USB architecture)
 * (C) Copyright Randy Dunlap 2000
 * (C) Copyright David Brownell 2000 (kernel hotplug, usb_device_id)
 * (C) Copyright Yggdrasil Computing, Inc. 2000
 *     (usb_device_id matching changes by Adam J. Richter)
 *
 * Adapted for U-Boot:
 * (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 */

/*
 * How it works:
 *
 * Since this is a bootloader, the devices will not be automatic
 * (re)configured on hotplug, but after a restart of the USB the
 * device should work.
 *
 * For each transfer (except "Interrupt") we wait for completion.
 */

#include "usb_cfg.h"

#if (BSP_USE_OTG) || (BSP_USE_EHCI) || (BSP_USE_OHCI)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>

#include "bsp.h"
#include "../libc/lwmem.h"
//#include "linux_ch9.h"
//#include "linux_kernel.h"
#include "linux.h"
#include "memalign.h"
#include "usb.h"

#define USB_BUFSIZ      512
#define debug(format, arg...) do {} while (0)

//-------------------------------------------------------------------------------------------------

static int asynch_allowed;
char usb_started; // USB状态标志（启动/停止）

#if (!DM_USB)
static struct usb_device usb_dev[USB_MAX_DEVICE];   // 结构体数组-->32个USB设备
static int dev_index = 0;                           // 设备指针-->搭配usb_dev遍历
#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT     1       // 配置USB所需要的最大控制器数量
#endif

/***************************************************************************
 *                                USB设备初始化
 ***************************************************************************/
int usb_init(void)
{
    void *ctrl = NULL;
    struct usb_device *dev;             // USB设备结构体
    int i, start_index = 0;             // 开始指针
    int controllers_initialized = 0;    // 已经初始化过的寄存器
    int ret;                            // 返回值-->用以查验错误类型

    dev_index = 0;                      // 设备指针
    asynch_allowed = 1;                 // 允许异步传输
    usb_hub_reset();                    // 【*】memset(hub_dev, 0, sizeof(hub_dev));

    /* 复位，让所有设备的状态设置为未识别（无连接） */
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        memset(&usb_dev[i], 0, sizeof(struct usb_device));
        usb_dev[i].devnum = -1;                             // 当前设备在总线上设备号为-1，代表无连接
    }

    /* 底层设备初始化 */
    for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++)  
    {
        /* 底层设备初始化 */
        printf("USB%d:   ", i);                             // 此for循环只执行一次，所以只输出USB0
        ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);   // USB底层初始化（USB0/主机端/空）【*】对OHCI做初始化+创建虚拟的根集线器
        if (ret == -ENODEV)                                 // 没有这个设备
        {
            puts("Port not available.\n");
            controllers_initialized++;
            continue;
        }
        if (ret)                                            // 其他的错误类型：不为0的其他值
        {
            puts("lowlevel init failed\n");
            continue;
        }
        
        // 底层初始化完成，现在扫描总线设备，即搜索集线器并配置它们
        controllers_initialized++;                          // 已经初始化过的控制器（此处用了OHCI）+1
        start_index = dev_index;                            // 开始指针指向当前设备指针
        // ----------------------------------------------------start_index = 0 / dev_index = 0
        printf("scanning bus %d for devices... ", i);       // 扫描总线i来查找设备
        ret = usb_alloc_new_device(ctrl, &dev);             // USB分配新设备【*】初始化设备数组（第0个设备可视为根集线器），初始化设备指针指向空白处dev_index++
        // ----------------------------------------------------start_index = 0 / dev_index = 1
        if (ret)                                            // 如果发生错误（ret！=0）跳出
            break;

        // 设备0始终存在（根集线器，所以让它分析）
        ret = usb_new_device(dev);                          // USB新设备【*】设备指针dev_index++多次
        // ----------------------------------------------------start_index = 0 / dev_index = 3 [插入一个设备，串口线算另一个？]

        if (ret)
            usb_free_device(dev->controller);               // 发生错误，USB释放设备【*】dev_index--

        if (start_index == dev_index)                       // 开始指针等于当前设备指针
        {
            puts("No USB Device found\n");                  // 没发现设备
            continue;
        }
        else
        {
            printf("%d USB Device(s) found\n", dev_index - start_index);
        }
        usb_started = 1;                                    // USB状态设置为已启动
    }

    debug("scan end\n");
    /* 如果我们找不到至少一条可以工作的总线 */
    if (controllers_initialized == 0)
        puts("USB error: all controllers failed lowlevel init\n");

    return usb_started ? 0 : -ENODEV;
}

/***************************************************************************
 *                              停止USB设备的使用
 *  停止底层部分（LowLevel Part）并取消注册 USB 设备
 ***************************************************************************/
int usb_stop(void)
{
    int i;

    if (usb_started)
    {
        asynch_allowed = 1;
        usb_started = 0;                                            // 标志着USB关闭
        usb_hub_reset();                                            // 集线器复位【*】memset(hub_dev, 0, sizeof(hub_dev));

        for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++)
        {
            if (usb_lowlevel_stop(i))                               // USB底层停止【*】
                printf("failed to stop USB controller %d\n", i);
        }
    }

    return 0;
}

/***************************************************************************
 *                         检测 USB 设备是否已插入或拔出
 ***************************************************************************/
int usb_detect_change(void)
{
    int i, j;
    int change = 0;

    for (j = 0; j < USB_MAX_DEVICE; j++)
    {
        for (i = 0; i < usb_dev[j].maxchild; i++)
        {
            struct usb_port_status status;

            if (usb_get_port_status(&usb_dev[j], i + 1, &status) < 0)
                /* USB request failed */
                continue;

            if (status.wPortChange & USB_PORT_STAT_C_CONNECTION)
                change++;
        }
    }
    return change;
}

/***************************************************************************
 *                       在控制器上锁定或解锁异步传输模式
 ***************************************************************************/
__attribute__((weak))
int usb_lock_async(struct usb_device *dev, int lock)
{
    return 0;
}

/*
 * 禁用控制消息的异步行为。返回原先设置，以便以后恢复。
 * 这用于使用对控制和批量消息的独占访问权限的数据传输。
 */
int usb_disable_asynch(int disable)
{
    int old_value = asynch_allowed;

    asynch_allowed = !disable;
    return old_value;
}

#endif /* #if (!DM_USB) */


/*-------------------------------------------------------------------
 *                             消息包装器
 ------------------------------------------------------------------*/

/********************************************************************
 * 提交中断消息。 一些驱动程序可能会实现非阻塞轮询：当
 *非阻塞为真并且设备没有响应时返回 -EAGAIN 而不是等待设备响应。
 ********************************************************************/
int usb_int_msg(struct usb_device *dev,
                unsigned long pipe,
                void *buffer,
                int transfer_len,
                int interval,
                bool nonblock)
{
    return submit_int_msg(dev, pipe, buffer, transfer_len, interval, nonblock);
}

/*
 * 提交控制消息并等待完成（至少超时 1 毫秒）如果超时为 0，我们不等待完成（用作设置和清除键盘 LED 的示例）。
 * 对于数据传输（存储传输），我们不允许使用 0 超时的控制消息，方法是预先重置标志 asynch_allowed (usb_disable_asynch(1))。
 * 如果 OK，则返回传输的长度；如果错误，则返回 -1。 传输的长度和当前状态存储在 dev->act_len 和 dev->status 中。
 */
int usb_control_msg(struct usb_device *dev,
                    unsigned int pipe,
                    unsigned char request,
                    unsigned char requesttype,
                    unsigned short value,
                    unsigned short index,
                    void *data,
                    unsigned short size,
                    int timeout)
{
    ALLOC_CACHE_ALIGN_BUFFER(struct devrequest, setup_packet, 1);
    int err;

    if ((timeout == 0) && (!asynch_allowed))
    {
        /* 不允许请求异步控制管道 */
        return -EINVAL;
    }

    /* 设置：打包命令 */
    setup_packet->requesttype = requesttype;    // 设置请求类型
    setup_packet->request = request;            // 设置请求
    setup_packet->value = value;                // 设置值
    setup_packet->index = index;                // 设置指针（地址）
    setup_packet->length = size;                // 设置包大小
    debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
          "value 0x%X index 0x%X length 0x%X\n",
           request, requesttype, value, index, size);
    dev->status = USB_ST_NOT_PROC;              // 设备状态设置为“尚未处理”

    err = submit_control_msg(dev, pipe, data, size, setup_packet);  // 提交控制信息【*】（参数：设备，管道，数据，大小，包）（传输在此后开始！！！！）
    if (err < 0)
        return err;
    if (timeout == 0)
        return (int)size;

    // 等待设备状态更新（状态更新跳出while，超时也跳出while）
    while (timeout--)
    {
        if (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
            break;
        DELAY_MS(1);
    }

    if (dev->status)    // 如果设备状态还是未被处理的1，表示超时退出，返回错误代码-1
        return -1;

    return dev->act_len;    // 正常更新返回接收到的数据包长度
}

/*-------------------------------------------------------------------
 * 提交批量消息，并等待完成。 如果确定则返回 0，如果错误则返回负数。 同步行为
 */
int usb_bulk_msg(struct usb_device *dev,
                 unsigned int pipe,
                 void *data,
                 int len,
                 int *actual_length,
                 int timeout)
{
    if (len < 0)
        return -EINVAL;

    dev->status = USB_ST_NOT_PROC; /*尚未处理 */
    if (submit_bulk_msg(dev, pipe, data, len) < 0)                      // 提交块存储器信息【*】
        return -EIO;

    while (timeout--)
    {
        if (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
            break;
        DELAY_MS(1);
    }

    *actual_length = dev->act_len;
    if (dev->status == 0)
        return 0;
    else
        return -EIO;
}


/*-------------------------------------------------------------------
 *                          最大数据包的东西
 ------------------------------------------------------------------*/

/*
 * 返回最大数据包大小，取决于管道方向和配置值
 */
int usb_maxpacket(struct usb_device *dev, unsigned long pipe)
{
    /* 取决于管道和方向 */
    if ((pipe & USB_DIR_IN) == 0)
        return dev->epmaxpacketout[((pipe>>15) & 0xf)];
    else
        return dev->epmaxpacketin[((pipe>>15) & 0xf)];
}

/*
 * 例程 usb_set_maxpacket_ep() 是从例程 usb_set_maxpacket() 的循环中提取的，
 * 因为当 GCC 4.x 的优化器内联到 1 个单个例程中时，该例程会阻塞。
 * 发生的情况是寄存器 r3 用作循环计数“i”，但稍后会被覆盖。
 * 这显然是一个编译器错误，但在这里解决它比更新编译器更容易（发生在至少几个
 * GCC 4.{1,2},x CodeSourcery 编译器上，例如 ARM 上的 2007q3、2008q1、2008q3 lite 版本）
 * 注意：在 ARMv5 上使用 GCC4.6 观察到类似的行为。
 */
static void usb_set_maxpacket_ep(struct usb_device *dev,
                                 int if_idx,
                                 int ep_idx)
{
    int b;
    struct usb_endpoint_descriptor *ep;
    unsigned short ep_wMaxPacketSize;

    ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];      // 说明这个端点是哪个接口下的哪个端点

    b = ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
    ep_wMaxPacketSize = ep->wMaxPacketSize; // get_unaligned(&ep->wMaxPacketSize);

    if ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_CONTROL)
    {
        /* Control => bidirectional */
        dev->epmaxpacketout[b] = ep_wMaxPacketSize;
        dev->epmaxpacketin[b] = ep_wMaxPacketSize;
        debug("##Control EP epmaxpacketout/in[%d] = %d\n", b, dev->epmaxpacketin[b]);
    }
    else
    {
        if ((ep->bEndpointAddress & 0x80) == 0)
        {
            /* OUT Endpoint */
            if (ep_wMaxPacketSize > dev->epmaxpacketout[b])
            {
                dev->epmaxpacketout[b] = ep_wMaxPacketSize;
                debug("##EP epmaxpacketout[%d] = %d\n", b, dev->epmaxpacketout[b]);
            }
        }
        else
        {
            /* IN Endpoint */
            if (ep_wMaxPacketSize > dev->epmaxpacketin[b])
            {
                dev->epmaxpacketin[b] = ep_wMaxPacketSize;
                debug("##EP epmaxpacketin[%d] = %d\n", b, dev->epmaxpacketin[b]);
            }
        } /* if out */
    } /* if control */
}

/*
 * 设置给定配置中所有端点的最大打包值
 */
static int usb_set_maxpacket(struct usb_device *dev)
{
    int i, ii;

    for (i = 0; i < dev->config.desc.bNumInterfaces; i++)
        for (ii = 0; ii < dev->config.if_desc[i].desc.bNumEndpoints; ii++)
            usb_set_maxpacket_ep(dev, i, ii);

    return 0;
}

/*******************************************************************************
 * 解析位于缓冲区中的配置，并填充 dev->config 结构
 * 请注意，所有小/大端交换都是自动完成的。 （wTotalLength 在读取时已被交换和清理。）
 *******************************************************************************/
static int usb_parse_config(struct usb_device *dev,
                            unsigned char *buffer,
                            int cfgno)
{
    struct usb_descriptor_header *head;
    int index, ifno, epno, curr_if_num;
    unsigned short ep_wMaxPacketSize;
    struct usb_interface *if_desc = NULL;

    ifno = -1;
    epno = -1;
    curr_if_num = -1;

    dev->configno = cfgno;
    head = (struct usb_descriptor_header *) &buffer[0];
    if (head->bDescriptorType != USB_DT_CONFIG)
    {
        printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
            head->bDescriptorType);
        return -EINVAL;
    }

    if (head->bLength != USB_DT_CONFIG_SIZE)
    {
        printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
        return -EINVAL;
    }
    memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
    dev->config.no_of_if = 0;

    index = dev->config.desc.bLength;
    /* Ok the first entry must be a configuration entry,
     * now process the others */
    head = (struct usb_descriptor_header *) &buffer[index];
    while (index + 1 < dev->config.desc.wTotalLength && head->bLength)
    {
        switch (head->bDescriptorType)
        {
            case USB_DT_INTERFACE:
                if (head->bLength != USB_DT_INTERFACE_SIZE)
                {
                    printf("ERROR: Invalid USB IF length (%d)\n", head->bLength);
                    break;
                }

                if (index + USB_DT_INTERFACE_SIZE > dev->config.desc.wTotalLength)
                {
                    puts("USB IF descriptor overflowed buffer!\n");
                    break;
                }

                if (((struct usb_interface_descriptor *)head)->bInterfaceNumber != curr_if_num)
                {
                    /* this is a new interface, copy new desc */
                    ifno = dev->config.no_of_if;
                    if (ifno >= USB_MAXINTERFACES)
                    {
                        puts("Too many USB interfaces!\n");
                        /* try to go on with what we have */
                        return -EINVAL;
                    }

                    if_desc = &dev->config.if_desc[ifno];
                    dev->config.no_of_if++;
                    memcpy( if_desc, head, USB_DT_INTERFACE_SIZE);
                    if_desc->no_of_ep = 0;
                    if_desc->num_altsetting = 1;
                    curr_if_num = if_desc->desc.bInterfaceNumber;
                }
                else
                {
                    /* found alternate setting for the interface */
                    if (ifno >= 0)
                    {
                        if_desc = &dev->config.if_desc[ifno];
                        if_desc->num_altsetting++;
                    }
                }
                break;

            case USB_DT_ENDPOINT:
                if (head->bLength != USB_DT_ENDPOINT_SIZE &&
                    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE)
                {
                    printf("ERROR: Invalid USB EP length (%d)\n", head->bLength);
                    break;
                }

                if (index + head->bLength > dev->config.desc.wTotalLength)
                {
                    puts("USB EP descriptor overflowed buffer!\n");
                    break;
                }

                if (ifno < 0)
                {
                    puts("Endpoint descriptor out of order!\n");
                    break;
                }

                epno = dev->config.if_desc[ifno].no_of_ep;
                if_desc = &dev->config.if_desc[ifno];
                if (epno >= USB_MAXENDPOINTS)
                {
                    printf("Interface %d has too many endpoints!\n",
                            if_desc->desc.bInterfaceNumber);
                    return -EINVAL;
                }

                /* found an endpoint */
                if_desc->no_of_ep++;
                memcpy(&if_desc->ep_desc[epno], head, USB_DT_ENDPOINT_SIZE);
                ep_wMaxPacketSize = dev->config.if_desc[ifno].ep_desc[epno].wMaxPacketSize;
                                 // get_unaligned(&dev->config.if_desc[ifno].ep_desc[epno].wMaxPacketSize);
                // put_unaligned(ep_wMaxPacketSize,
                //               &dev->config.if_desc[ifno].ep_desc[epno].wMaxPacketSize);
                dev->config.if_desc[ifno].ep_desc[epno].wMaxPacketSize = ep_wMaxPacketSize;
                debug("if %d, ep %d\n", ifno, epno);
                break;

            case USB_DT_SS_ENDPOINT_COMP:
                if (head->bLength != USB_DT_SS_EP_COMP_SIZE)
                {
                    printf("ERROR: Invalid USB EPC length (%d)\n", head->bLength);
                    break;
                }

                if (index + USB_DT_SS_EP_COMP_SIZE > dev->config.desc.wTotalLength)
                {
                    puts("USB EPC descriptor overflowed buffer!\n");
                    break;
                }

                if (ifno < 0 || epno < 0)
                {
                    puts("EPC descriptor out of order!\n");
                    break;
                }

                if_desc = &dev->config.if_desc[ifno];
                memcpy(&if_desc->ss_ep_comp_desc[epno], head, USB_DT_SS_EP_COMP_SIZE);
                break;

            default:
                if (head->bLength == 0)
                    return -EINVAL;

                debug("unknown Description Type : %x\n", head->bDescriptorType);

                break;

        }

        index += head->bLength;
        head = (struct usb_descriptor_header *)&buffer[index];
    }

    return 0;
}

/***********************************************************************
 *                              清除停止
 * 端点：端点号在0-3位 ;
 * 方向标志在 (1 = IN, 0 = OUT)
 **********************************************************************/
int usb_clear_halt(struct usb_device *dev, int pipe)
{
    int result;
    int endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

    result = usb_control_msg(dev,                           // 【*】
                             usb_sndctrlpipe(dev, 0),
                             USB_REQ_CLEAR_FEATURE,
                             USB_RECIP_ENDPOINT,
                             0,
                             endp,
                             NULL,
                             0,
                             USB_CNTL_TIMEOUT * 3 );

    /* 不清楚是否失败 */
    if (result < 0)
        return result;

    /* NOTE: 我们没有获得状态并验证重置是否成功，因为据报道某些设备在此检查时锁定.. */
    usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));   // 运行USB端点【*】端点管道【*】USB输出管道【*】

    /* 切换在清除时重置 */
    usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);       // USB设置切换【*】端点管道【*】USB输出管道【*】
    
    return 0;
}

/**********************************************************************
 *                              获取描述符类型
 **********************************************************************/
static int usb_get_descriptor(struct usb_device *dev,
                              unsigned char type,
                              unsigned char index,
                              void *buf, int size)
{
    return usb_control_msg(dev,
                           usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_DESCRIPTOR,
                           USB_DIR_IN,
                           (type << 8) + index,
                           0,
                           buf,
                           size,
                           USB_CNTL_TIMEOUT );
}

/**********************************************************************
 *                            获取配置cfgno的长度
 **********************************************************************/
int usb_get_configuration_len(struct usb_device *dev,
                              int cfgno)
{
    int result;
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, 9);
    struct usb_config_descriptor *config;

    config = (struct usb_config_descriptor *)&buffer[0];
    result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
    if (result < 9)
    {
        if (result < 0)
            printf("unable to get descriptor, error %lX\n", dev->status);
        else
            printf("config descriptor too short (expected %i, got %i)\n",
                    9, result);
        return -EIO;
    }

    return config->wTotalLength;
}

/**********************************************************************
 *                      获取配置cfgno并将其存储在缓冲区中
 **********************************************************************/
int usb_get_configuration_no(struct usb_device *dev,
                             int cfgno,
                             unsigned char *buffer,
                             int length)
{
    int result;
    struct usb_config_descriptor *config;

    config = (struct usb_config_descriptor *)&buffer[0];
    result = usb_get_descriptor(dev,
                                USB_DT_CONFIG,
                                cfgno,
                                buffer,
                                length);

    debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
          config->wTotalLength);

    config->wTotalLength = result; /* validated, with CPU byte order */

    return result;
}

/********************************************************************
 * 将设备的地址设置为 dev->devnum 中的值。
 * 这只能通过默认地址 (0) 寻址设备来完成
 ********************************************************************/
static int usb_set_address(struct usb_device *dev)
{
    debug("set address %d\n", dev->devnum);

    return usb_control_msg(dev,
                           usb_snddefctrl(dev),
                           USB_REQ_SET_ADDRESS,
                           0,
                           (dev->devnum),
                           0,
                           NULL,
                           0,
                           USB_CNTL_TIMEOUT);
}

/********************************************************************
 *                          设置接口号为接口
 ********************************************************************/
int usb_set_interface(struct usb_device *dev,
                      int interface,
                      int alternate)
{
    struct usb_interface *if_face = NULL;
    int ret, i;

    for (i = 0; i < dev->config.desc.bNumInterfaces; i++)               // 遍历所有接口查找接口号
    {
        if (dev->config.if_desc[i].desc.bInterfaceNumber == interface)
        {
            if_face = &dev->config.if_desc[i];
            break;
        }
    }

    if (!if_face)
    {
        printf("selecting invalid interface %d", interface);
        return -EINVAL;
    }
    /*
     * We should return now for devices with only one alternate setting.
     * According to 9.4.10 of the Universal Serial Bus Specification
     * Revision 2.0 such devices can return with a STALL. This results in
     * some USB sticks timeouting during initialization and then being
     * unusable in U-Boot.
     */
    if (if_face->num_altsetting == 1)
        return 0;

    ret = usb_control_msg(dev,
                          usb_sndctrlpipe(dev, 0),
                          USB_REQ_SET_INTERFACE,
                          USB_RECIP_INTERFACE,
                          alternate,
                          interface,
                          NULL,
                          0,
                          USB_CNTL_TIMEOUT * 5 );
    if (ret < 0)
        return ret;

    return 0;
}

/********************************************************************
 *                         将配置编号设置为配置
 ********************************************************************/
static int usb_set_configuration(struct usb_device *dev,
                                 int configuration)
{
    int res;
    debug("set configuration %d\n", configuration);

    /* set setup command */
    res = usb_control_msg(dev,
                          usb_sndctrlpipe(dev, 0),
                          USB_REQ_SET_CONFIGURATION,
                          0,
                          configuration,
                          0,
                          NULL,
                          0,
                          USB_CNTL_TIMEOUT);

    if (res == 0)
    {
        dev->toggle[0] = 0;
        dev->toggle[1] = 0;
        return 0;
    }
    else
        return -EIO;
}

/********************************************************************
 *                          将协议设置为协议
 ********************************************************************/
int usb_set_protocol(struct usb_device *dev,
                     int ifnum,
                     int protocol)
{
    return usb_control_msg(dev,
                           usb_sndctrlpipe(dev, 0),
                           USB_REQ_SET_PROTOCOL,
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           protocol,
                           ifnum,
                           NULL,
                           0,
                           USB_CNTL_TIMEOUT);
}

/********************************************************************
 *                             设置空闲
 *******************************************************************/
int usb_set_idle(struct usb_device *dev,
                 int ifnum,
                 int duration,
                 int report_id)
{
    return usb_control_msg(dev,
                           usb_sndctrlpipe(dev, 0),
                           USB_REQ_SET_IDLE,
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           (duration << 8) | report_id,
                           ifnum,
                           NULL,
                           0,
                           USB_CNTL_TIMEOUT );
}

/********************************************************************
 *                              获取报告
 *******************************************************************/
int usb_get_report(struct usb_device *dev,
                   int ifnum,
                   unsigned char type,
                   unsigned char id,
                   void *buf,
                   int size)
{
    return usb_control_msg(dev,
                           usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_REPORT,
                           USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           (type << 8) + id,
                           ifnum,
                           buf,
                           size,
                           USB_CNTL_TIMEOUT );
}

/********************************************************************
 *                            获取类描述符
 ********************************************************************/
int usb_get_class_descriptor(struct usb_device *dev,
                             int ifnum,
                             unsigned char type,
                             unsigned char id,
                             void *buf,
                             int size)
{
    return usb_control_msg(dev,
                           usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_DESCRIPTOR,
                           USB_RECIP_INTERFACE | USB_DIR_IN,
                           (type << 8) + id,
                           ifnum,
                           buf,
                           size,
                           USB_CNTL_TIMEOUT);
}

/********************************************************************
 *                      获取缓冲区中的字符串索引
 *******************************************************************/
static int usb_get_string(struct usb_device *dev,
                          unsigned short langid,
                          unsigned char index,
                          void *buf,
                          int size)
{
    int i;
    int result;

    for (i = 0; i < 3; ++i)
    {
        /* some devices are flaky */
        result = usb_control_msg(dev,
                                 usb_rcvctrlpipe(dev, 0),
                                 USB_REQ_GET_DESCRIPTOR,
                                 USB_DIR_IN,
                                 (USB_DT_STRING << 8) + index,
                                 langid,
                                 buf,
                                 size,
                                 USB_CNTL_TIMEOUT);

        if (result > 0)
            break;
    }

    return result;
}

static void usb_try_string_workarounds(unsigned char *buf,
                                       int *length)
{
    int newlength, oldlength = *length;

    for (newlength = 2; newlength + 1 < oldlength; newlength += 2)
        if (!isprint(buf[newlength]) || buf[newlength + 1])
            break;

    if (newlength > 2)
    {
        buf[0] = newlength;
        *length = newlength;
    }
}

static int usb_string_sub(struct usb_device *dev,
                          unsigned int langid,
                          unsigned int index,
                          unsigned char *buf)
{
    int rc;

    /* Try to read the string descriptor by asking for the maximum
     * possible number of bytes */
    rc = usb_get_string(dev, langid, index, buf, 255);

    /* If that failed try to read the descriptor length, then
     * ask for just that many bytes */
    if (rc < 2)
    {
        rc = usb_get_string(dev, langid, index, buf, 2);
        if (rc == 2)
            rc = usb_get_string(dev, langid, index, buf, buf[0]);
    }

    if (rc >= 2)
    {
        if (!buf[0] && !buf[1])
            usb_try_string_workarounds(buf, &rc);

        /* There might be extra junk at the end of the descriptor */
        if (buf[0] < rc)
            rc = buf[0];

        rc = rc - (rc & 1); /* force a multiple of two */
    }

    if (rc < 2)
        rc = -EINVAL;

    return rc;
}

/********************************************************************
 * 获取字符串索引并将其转换为 ascii。
 * 返回字符串长度 (> 0) 或错误 (< 0)
 *******************************************************************/
int usb_string(struct usb_device *dev,
               int index,
               char *buf,
               size_t size)
{
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, mybuf, USB_BUFSIZ);
    unsigned char *tbuf;
    int err;
    unsigned int u, idx;

    if (size <= 0 || !buf || !index)
        return -EINVAL;
    buf[0] = 0;
    tbuf = &mybuf[0];

    /* get langid for strings if it's not yet known */
    if (!dev->have_langid)
    {
        err = usb_string_sub(dev, 0, 0, tbuf);
        if (err < 0)
        {
            debug("error getting string descriptor 0 (error=%lx)\n",
                   dev->status);
            return -EIO;
        }
        else if (tbuf[0] < 4)
        {
            debug("string descriptor 0 too short\n");
            return -EIO;
        }
        else
        {
            dev->have_langid = -1;
            dev->string_langid = tbuf[2] | (tbuf[3] << 8);
            /* always use the first langid listed */
            debug("USB device number %d default language ID 0x%x\n",
                   dev->devnum, dev->string_langid);
        }
    }

    err = usb_string_sub(dev, dev->string_langid, index, tbuf);
    if (err < 0)
        return err;

    size--;     /* leave room for trailing NULL char in output buffer */
    for (idx = 0, u = 2; u < err; u += 2)
    {
        if (idx >= size)
            break;
        if (tbuf[u+1])              /* high byte */
            buf[idx++] = '?';       /* non-ASCII character */
        else
            buf[idx++] = tbuf[u];
    }

    buf[idx] = 0;
    err = idx;

    return err;
}


/********************************************************************
 * USB 设备处理：USB 设备是静态分配的 [USB_MAX_DEVICE]。
 *******************************************************************/

#if (!DM_USB)

/*******************************************************************
 *                  返回设备指针，没有返回空
 *  usb_dev是结构体数组
 *  存储USB设备结构体（struct usb_device）
 *  数组大小为  USB_MAX_DEVICE
 *
 ******************************************************************/
struct usb_device *usb_get_dev_index(int index)
{
    if (usb_dev[index].devnum == -1)
        return NULL;
    else
        return &usb_dev[index];
}

int usb_alloc_new_device(struct udevice *controller,struct usb_device **devp)
{
    int i;

    debug("New Device %d\n", dev_index);

    if (dev_index == USB_MAX_DEVICE)                // 已连接最多设备，无法再次连接
    {
        printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
        return -ENOSPC;
    }

    /* 默认地址为0，真实地址以1开头 */
    usb_dev[dev_index].devnum = dev_index + 1;      // 第dev_index个设备在USB总线上的设备号为dev_index + 1
    usb_dev[dev_index].maxchild = 0;                // 该设备集线器端口数为0
    for (i = 0; i < USB_MAXCHILDREN; i++)           // 此设备不是集线器，所以其子设备都是空
        usb_dev[dev_index].children[i] = NULL;
    usb_dev[dev_index].parent = NULL;               // 父集线器为空
    usb_dev[dev_index].controller = controller;     // 硬件控制器私有数据为空
    dev_index++;                                    // 设备指针++
    *devp = &usb_dev[dev_index - 1];                // *devp所指向的设备=修改之后的usb_dev[dev_index - 1]
    // 【注】上面一行为何这么写：本质上是没创建“设备”结构体，只是先修改已经创建的设备数组usb_dev[]，然后让设备指针指向设备数组中某一元素【大大节约内存】
    return 0;
}

/*
 * 释放新创建的设备节点。
 * 在配置新连接的设备由于某种原因失败的错误情况下调用。
 */
void usb_free_device(struct udevice *controller)
{
    dev_index--;                                                // 设备指针--
    debug("Freeing device node: %d\n", dev_index);
    memset(&usb_dev[dev_index], 0, sizeof(struct usb_device));  // 清空设备数据
    usb_dev[dev_index].devnum = -1;                             // 该设备在当前设备在USB总线上的设备号为-1
}

/*
 * XHCI 发出 Enable Slot 命令，然后分配设备上下文。
 * 为此目的提供一个弱别名函数，以便 XHCI 覆盖它并且 EHCI/OHCI 开箱即用。
 */
__attribute__((weak))
int usb_alloc_device(struct usb_device *udev)
{
    return 0;
}
#endif /* #if (!DM_USB) */

static int usb_hub_port_reset(struct usb_device *dev,
                              struct usb_device *hub)
{
    if (!hub)
        usb_reset_root_port(dev);

    return 0;
}

static int get_descriptor_len(struct usb_device *dev,
                              int len,
                              int expect_len)
{
    /*__maybe_unused*/ struct usb_device_descriptor *desc;          // 开辟设备描述符空间
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, tmpbuf, USB_BUFSIZ);
    int err;
    desc = (struct usb_device_descriptor *)tmpbuf;

    err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);     // 获取描述符【*】（参数类型为设备描述符）（返回真实接收到的长度）
    if (err < expect_len)
    {
        if (err < 0)
        {
            printf("unable to get device descriptor (error=%d)\n", err);
            return err;
        }
        else
        {
            printf("USB device descriptor short read (expected %i, got %i)\n",expect_len, err);
            return -EIO;
        }
    }

    memcpy(&dev->descriptor, tmpbuf, sizeof(dev->descriptor));      // 将设备描述符从临时缓存区tmpbuf拷贝到dev->descriptor

    return 0;
}

// 这是 Windows 的初始化序列方案，设备双重复位（Linux 使用相同的序列） 据说有些设备只能使用这样的初始化序列
static int usb_setup_descriptor(struct usb_device *dev, bool do_read)
{
    // 发送 64 字节的 GET-DEVICE-DESCRIPTOR 请求。由于描述符只有 18 个字节长，这将以一个短数据包结束。
    // 但是如果 maxpacket 大小为 8 或 16，则设备可能正在等待传输更多数据，或者继续重新传输 8 字节标头。
    if (dev->speed == USB_SPEED_LOW)            // 如果这是一个低速设备
    {
        dev->descriptor.bMaxPacketSize0 = 8;    // 设备描述符最大字节=8
        dev->maxpacketsize = PACKET_SIZE_8;     // 设备描述符最大字节的编码=0（编码规律0，1，2，3=>8，16，32，64）
    }
    else                                        // 高速设备
    {
        dev->descriptor.bMaxPacketSize0 = 64;   // 设备描述符最大字节=64
        dev->maxpacketsize = PACKET_SIZE_64;    // 设备描述符最大字节的编码=3（编码规律0，1，2，3=>8，16，32，64）
    }

    dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;    // 将此最大值写到输入端点
    dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;   // 将此最大值写到输出端点

    if (do_read && dev->speed == USB_SPEED_FULL)// 全速设备
    {
        int err;

        /*
         * 验证我们只收到了至少 8 个字节，而不是我们收到了整个描述符。 理由是：
         * - 该代码仅使用前 8 个字节中的字段，因此这就是我们现阶段需要获取的所有内容。
         * - 最小的 maxpacket 大小为 8 个字节。 在我们知道设备使用的实际 maxpacket 之前，
         *   USB 控制器可能只接受一个数据包。 因此，即使在非错误情况下，我们也只能保证接收 1 个数据包（至少 8 个字节）。
         *
         * 除了字节数之外，至少需要对 DWC2 控制器编程数据包数。 请求 64 字节数据且 maxpacket 猜测为 64（上图）会产生 1 个数据包的请求。
         */
        err = get_descriptor_len(dev, 64, 8);   // 获取描述符【*】（参数：设备，长度，希望接收到的长度）

        if (err)
            return err;
    }

    dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;    // 将此最大值写到输入端点
    dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;   // 将此最大值写到输出端点

    switch (dev->descriptor.bMaxPacketSize0)                    // 编码
    {
        case 8:
            dev->maxpacketsize  = PACKET_SIZE_8;
            break;

        case 16:
            dev->maxpacketsize = PACKET_SIZE_16;
            break;

        case 32:
            dev->maxpacketsize = PACKET_SIZE_32;
            break;

        case 64:
            dev->maxpacketsize = PACKET_SIZE_64;
            break;

        default:
            printf("%s: invalid max packet size\n", __func__);
            return -EIO;
    }

    return 0;
}

static int usb_prepare_device(struct usb_device *dev,
                              int addr,
                              bool do_read,
                              struct usb_device *parent)
{
    int err;

    /* 分配 usb 3.0 设备上下文。 USB 3.0 (xHCI) 协议首先尝试分配设备插槽和相关数据结构。 这个调用就是这样做的。*/
    err = usb_alloc_device(dev);    // 龙芯1B不支持3.0设备【*】直接返回0（不用往下看了，哈哈哈哈）
    if (err)
    {
        printf("Cannot allocate device context to get SLOT_ID\n");
        return err;
    }

    err = usb_setup_descriptor(dev, do_read);   // 设置设备描述符【*】高速和低速设备只赋值描述符长度
    if (err)
        return err;
    err = usb_hub_port_reset(dev, parent);      // USB集线器接口复位【*】啥都没执行（参数：当前设备与父设备）
    if (err)
        return err;

    dev->devnum = addr;

    err = usb_set_address(dev);                 // 设置地址【*】
    if (err < 0)
    {
        printf("\n      USB device not accepting new address (error=%lX)\n",dev->status);
        return err;
    }

    DELAY_MS(10);                               // 延时以等待设置地址结束

    // 如果我们之前没有读取设备描述符，请在设备分配地址后在此处读取。 到目前为止，这仅适用于 xHCI。
    if (!do_read)
    {
        err = usb_setup_descriptor(dev, true);  // do_read是TRUE,所以不会执行此if【*】同上
        if (err)
            return err;
    }

    return 0;
}

int usb_select_config(struct usb_device *dev)
{
    unsigned char *tmpbuf = NULL;
    int err;

    err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);  // 获取描述符大小【*】
    if (err)
        return err;

    /* 更正值 */
/*
    &dev->descriptor.bcdUSB;
    &dev->descriptor.idVendor;
    &dev->descriptor.idProduct;
    &dev->descriptor.bcdDevice;
 */
 
    /*
     * Kingston DT Ultimate 32GB USB 3.0 seems to be extremely sensitive
     * about this first Get Descriptor request. If there are any other
     * requests in the first microframe, the stick crashes. Wait about
     * one microframe duration here (1mS for USB 1.x , 125uS for USB 2.0).
     */
    DELAY_MS(1);

    /* 目前只支持一种配置 */
    err = usb_get_configuration_len(dev, 0);    // 获取配置描述符大小【*】出错返回错误代码，不出错返回大小到err
    if (err >= 0)                               // 没出错
    {
        tmpbuf = (unsigned char *)malloc_cache_aligned(err);    // 没出错err是描述符长度，根据err开辟临时缓存空间tmpbuf

        if (!tmpbuf)                                            // 临时存储空间为空，返回错误代码
            err = -ENOMEM;
        else
            err = usb_get_configuration_no(dev, 0, tmpbuf, err);// 获取配置描述符【*】存储到tempbuf中
    }
    if (err < 0)
    {
        printf("usb_new_device: Cannot read configuration, " \
               "skipping device %04x:%04x\n",
                dev->descriptor.idVendor, dev->descriptor.idProduct);
        aligned_free(tmpbuf);                                   // 如果获取配置描述符发生错误，释放临时缓存空间【*】
        return err;
    }

    usb_parse_config(dev, tmpbuf, 0);                           // 解析配置描述符【*】解析位于缓存的配置描述符将其写入dev->config
    aligned_free(tmpbuf);                                       // 释放缓存区【*】
    usb_set_maxpacket(dev);                                     // 设置所有端点最大数据包大小【*】

    /*
     * 我们在这里设置默认配置这似乎为时过早。 如果驱动程序需要不同的配置，则需要自行选择。
     */
    err = usb_set_configuration(dev, dev->config.desc.bConfigurationValue);     // 将配置编号设置为配置【*】
    if (err < 0)
    {
        printf("failed to set default configuration len %d, status %lX\n",
                dev->act_len, dev->status);
        return err;
    }

    /*
     * 等到设备处理设置配置请求。 DWC2 OTG 控制器上的 SanDisk Cruzer Pop USB 2.0 和 Kingston DT Ultimate 32GB USB 3.0 至少需要此功能。
     */
    DELAY_MS(10);

    debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
           dev->descriptor.iManufacturer, dev->descriptor.iProduct,
           dev->descriptor.iSerialNumber);

    memset(dev->mf, 0, sizeof(dev->mf));            // 设备制造商清零
    memset(dev->prod, 0, sizeof(dev->prod));        // 设备产品清零
    memset(dev->serial, 0, sizeof(dev->serial));    // 序列号清零

    if (dev->descriptor.iManufacturer)              // 写入设备制造商信息【*】
        usb_string(dev, dev->descriptor.iManufacturer, dev->mf, sizeof(dev->mf));
    if (dev->descriptor.iProduct)                   // 写入产品信息【*】
        usb_string(dev, dev->descriptor.iProduct, dev->prod, sizeof(dev->prod));
    if (dev->descriptor.iSerialNumber)              // 写入序列号【*】
        usb_string(dev, dev->descriptor.iSerialNumber, dev->serial, sizeof(dev->serial));

    debug("Manufacturer %s\n", dev->mf);
    debug("Product      %s\n", dev->prod);
    debug("SerialNumber %s\n", dev->serial);

    return 0;
}

int usb_setup_device(struct usb_device *dev,
                     bool do_read,
                     struct usb_device *parent)
{
    int addr;
    int ret;

    /* 我们还没有设置地址 */
    addr = dev->devnum;         // 地址是在总线上的设备号
    dev->devnum = 0;            // 总线上的设备号归0

    ret = usb_prepare_device(dev, addr, do_read, parent);   // 准备USB设备【*】
    if (ret)
        return ret;

    ret = usb_select_config(dev);   // USB选择配置【*】向下位机发送请求，获取设备描述符保存到缓存区，解析缓存的数据后保存到设备结构体对应位置

    return ret;
}

#if (!DM_USB)
/*
 * 当我们到达这里时，设备已经获得了一个新的设备 ID 并且处于默认状态。
 * 我们需要识别事物并让球滚动。返回 0 表示成功，!= 0 表示错误。
 */
int usb_new_device(struct usb_device *dev)
{
    bool do_read = true;
    int err;

    /* XHCI 需要发出地址设备命令来设置正确的设备上下文结构，然后才能与设备交互。
       因此，与 EHCI 不同，在为 XHCI 完成任何操作之前，get_descriptor 将失败. */
#ifdef CONFIG_USB_XHCI_HCD
    do_read = false;
#endif

    err = usb_setup_device(dev, do_read, dev->parent);  // 创建设备（设备/true/父设备）【*】
    if (err)
        return err;

    // 现在探测设备是否是集线器【*】【不是是集线器直接返回0，是集线器先配置集线器后返回（配置不出错也是返回0）】
    err = usb_hub_probe(dev, 0);        // dev_index++
    if (err < 0)
        return err;

    return 0;
}
#endif

__attribute__((weak))
int board_usb_init(int index, enum usb_init_type init)
{
    return 0;
}

__attribute__((weak))
int board_usb_cleanup(int index, enum usb_init_type init)
{
    return 0;
}

bool usb_device_has_child_on_port(struct usb_device *parent, int port)
{
#if (DM_USB)
    return false;
#else
    return parent->children[port] != NULL;
#endif
}

#if (DM_USB)

void usb_find_usb2_hub_address_port(struct usb_device *udev,
                                    unsigned char *hub_address,
                                    unsigned char *hub_port)
{
    struct udevice *parent;
    struct usb_device *uparent, *ttdev;

    /*
     * When called from usb-uclass.c: usb_scan_device() udev->dev points
     * to the parent udevice, not the actual udevice belonging to the
     * udev as the device is not instantiated yet. So when searching
     * for the first usb-2 parent start with udev->dev not
     * udev->dev->parent .
     */
    ttdev = udev;
    parent = udev->dev;
    uparent = dev_get_parent_priv(parent);

    while (uparent->speed != USB_SPEED_HIGH)
    {
        struct udevice *dev = parent;

        if (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB)
        {
            printf("Error: Cannot find high speed parent of usb-1 device\n");
            *hub_address = 0;
            *hub_port = 0;
            return;
        }

        ttdev = dev_get_parent_priv(dev);
        parent = dev->parent;
        uparent = dev_get_parent_priv(parent);
    }

    *hub_address = uparent->devnum;
    *hub_port = ttdev->portnr;
}

#else

void usb_find_usb2_hub_address_port(struct usb_device *udev,
                                    unsigned char *hub_address,
                                    unsigned char *hub_port)
{
    /* Find out the nearest parent which is high speed */
    while (udev->parent->parent != NULL)
    {
        if (udev->parent->speed != USB_SPEED_HIGH)
        {
            udev = udev->parent;
        }
        else
        {
            *hub_address = udev->parent->devnum;
            *hub_port = udev->portnr;
            return;
        }
    }

    printf("Error: Cannot find high speed parent of usb-1 device\n");
    *hub_address = 0;
    *hub_port = 0;
}

#endif

#endif // #if (BSP_USE_OTG) || (BSP_USE_EHCI) || (BSP_USE_OHCI)

/*
 * @@ EOF
 */



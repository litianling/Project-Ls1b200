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
char usb_started; // USB״̬��־������/ֹͣ��

#if (!DM_USB)
static struct usb_device usb_dev[USB_MAX_DEVICE];   // �ṹ������-->32��USB�豸
static int dev_index = 0;                           // �豸ָ��-->����usb_dev����
#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT     1       // ����USB����Ҫ��������������
#endif

/***************************************************************************
 *                                USB�豸��ʼ��
 ***************************************************************************/
int usb_init(void)
{
    void *ctrl = NULL;
    struct usb_device *dev;             // USB�豸�ṹ��
    int i, start_index = 0;             // ��ʼָ��
    int controllers_initialized = 0;    // �Ѿ���ʼ�����ļĴ���
    int ret;                            // ����ֵ-->���Բ����������

    dev_index = 0;                      // �豸ָ��
    asynch_allowed = 1;                 // �����첽����
    usb_hub_reset();                    // ��*��memset(hub_dev, 0, sizeof(hub_dev));

    /* ��λ���������豸��״̬����Ϊδʶ�������ӣ� */
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        memset(&usb_dev[i], 0, sizeof(struct usb_device));
        usb_dev[i].devnum = -1;                             // ��ǰ�豸���������豸��Ϊ-1������������
    }

    /* �ײ��豸��ʼ�� */
    for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++)  
    {
        /* �ײ��豸��ʼ�� */
        printf("USB%d:   ", i);                             // ��forѭ��ִֻ��һ�Σ�����ֻ���USB0
        ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);   // USB�ײ��ʼ����USB0/������/�գ���*����OHCI����ʼ��+��������ĸ�������
        if (ret == -ENODEV)                                 // û������豸
        {
            puts("Port not available.\n");
            controllers_initialized++;
            continue;
        }
        if (ret)                                            // �����Ĵ������ͣ���Ϊ0������ֵ
        {
            puts("lowlevel init failed\n");
            continue;
        }
        
        // �ײ��ʼ����ɣ�����ɨ�������豸������������������������
        controllers_initialized++;                          // �Ѿ���ʼ�����Ŀ��������˴�����OHCI��+1
        start_index = dev_index;                            // ��ʼָ��ָ��ǰ�豸ָ��
        // ----------------------------------------------------start_index = 0 / dev_index = 0
        printf("scanning bus %d for devices... ", i);       // ɨ������i�������豸
        ret = usb_alloc_new_device(ctrl, &dev);             // USB�������豸��*����ʼ���豸���飨��0���豸����Ϊ��������������ʼ���豸ָ��ָ��հ״�dev_index++
        // ----------------------------------------------------start_index = 0 / dev_index = 1
        if (ret)                                            // �����������ret��=0������
            break;

        // �豸0ʼ�մ��ڣ�������������������������
        ret = usb_new_device(dev);                          // USB���豸��*���豸ָ��dev_index++���
        // ----------------------------------------------------start_index = 0 / dev_index = 3 [����һ���豸������������һ����]

        if (ret)
            usb_free_device(dev->controller);               // ��������USB�ͷ��豸��*��dev_index--

        if (start_index == dev_index)                       // ��ʼָ����ڵ�ǰ�豸ָ��
        {
            puts("No USB Device found\n");                  // û�����豸
            continue;
        }
        else
        {
            printf("%d USB Device(s) found\n", dev_index - start_index);
        }
        usb_started = 1;                                    // USB״̬����Ϊ������
    }

    debug("scan end\n");
    /* ��������Ҳ�������һ�����Թ��������� */
    if (controllers_initialized == 0)
        puts("USB error: all controllers failed lowlevel init\n");

    return usb_started ? 0 : -ENODEV;
}

/***************************************************************************
 *                              ֹͣUSB�豸��ʹ��
 *  ֹͣ�ײ㲿�֣�LowLevel Part����ȡ��ע�� USB �豸
 ***************************************************************************/
int usb_stop(void)
{
    int i;

    if (usb_started)
    {
        asynch_allowed = 1;
        usb_started = 0;                                            // ��־��USB�ر�
        usb_hub_reset();                                            // ��������λ��*��memset(hub_dev, 0, sizeof(hub_dev));

        for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++)
        {
            if (usb_lowlevel_stop(i))                               // USB�ײ�ֹͣ��*��
                printf("failed to stop USB controller %d\n", i);
        }
    }

    return 0;
}

/***************************************************************************
 *                         ��� USB �豸�Ƿ��Ѳ����γ�
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
 *                       �ڿ�����������������첽����ģʽ
 ***************************************************************************/
__attribute__((weak))
int usb_lock_async(struct usb_device *dev, int lock)
{
    return 0;
}

/*
 * ���ÿ�����Ϣ���첽��Ϊ������ԭ�����ã��Ա��Ժ�ָ���
 * ������ʹ�öԿ��ƺ�������Ϣ�Ķ�ռ����Ȩ�޵����ݴ��䡣
 */
int usb_disable_asynch(int disable)
{
    int old_value = asynch_allowed;

    asynch_allowed = !disable;
    return old_value;
}

#endif /* #if (!DM_USB) */


/*-------------------------------------------------------------------
 *                             ��Ϣ��װ��
 ------------------------------------------------------------------*/

/********************************************************************
 * �ύ�ж���Ϣ�� һЩ����������ܻ�ʵ�ַ�������ѯ����
 *������Ϊ�沢���豸û����Ӧʱ���� -EAGAIN �����ǵȴ��豸��Ӧ��
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
 * �ύ������Ϣ���ȴ���ɣ����ٳ�ʱ 1 ���룩�����ʱΪ 0�����ǲ��ȴ���ɣ��������ú�������� LED ��ʾ������
 * �������ݴ��䣨�洢���䣩�����ǲ�����ʹ�� 0 ��ʱ�Ŀ�����Ϣ��������Ԥ�����ñ�־ asynch_allowed (usb_disable_asynch(1))��
 * ��� OK���򷵻ش���ĳ��ȣ���������򷵻� -1�� ����ĳ��Ⱥ͵�ǰ״̬�洢�� dev->act_len �� dev->status �С�
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
        /* �����������첽���ƹܵ� */
        return -EINVAL;
    }

    /* ���ã�������� */
    setup_packet->requesttype = requesttype;    // ������������
    setup_packet->request = request;            // ��������
    setup_packet->value = value;                // ����ֵ
    setup_packet->index = index;                // ����ָ�루��ַ��
    setup_packet->length = size;                // ���ð���С
    debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
          "value 0x%X index 0x%X length 0x%X\n",
           request, requesttype, value, index, size);
    dev->status = USB_ST_NOT_PROC;              // �豸״̬����Ϊ����δ����

    err = submit_control_msg(dev, pipe, data, size, setup_packet);  // �ύ������Ϣ��*�����������豸���ܵ������ݣ���С�������������ڴ˺�ʼ����������
    if (err < 0)
        return err;
    if (timeout == 0)
        return (int)size;

    // �ȴ��豸״̬���£�״̬��������while����ʱҲ����while��
    while (timeout--)
    {
        if (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
            break;
        DELAY_MS(1);
    }

    if (dev->status)    // ����豸״̬����δ�������1����ʾ��ʱ�˳������ش������-1
        return -1;

    return dev->act_len;    // �������·��ؽ��յ������ݰ�����
}

/*-------------------------------------------------------------------
 * �ύ������Ϣ�����ȴ���ɡ� ���ȷ���򷵻� 0����������򷵻ظ����� ͬ����Ϊ
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

    dev->status = USB_ST_NOT_PROC; /*��δ���� */
    if (submit_bulk_msg(dev, pipe, data, len) < 0)                      // �ύ��洢����Ϣ��*��
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
 *                          ������ݰ��Ķ���
 ------------------------------------------------------------------*/

/*
 * ����������ݰ���С��ȡ���ڹܵ����������ֵ
 */
int usb_maxpacket(struct usb_device *dev, unsigned long pipe)
{
    /* ȡ���ڹܵ��ͷ��� */
    if ((pipe & USB_DIR_IN) == 0)
        return dev->epmaxpacketout[((pipe>>15) & 0xf)];
    else
        return dev->epmaxpacketin[((pipe>>15) & 0xf)];
}

/*
 * ���� usb_set_maxpacket_ep() �Ǵ����� usb_set_maxpacket() ��ѭ������ȡ�ģ�
 * ��Ϊ�� GCC 4.x ���Ż��������� 1 ������������ʱ�������̻�������
 * ����������ǼĴ��� r3 ����ѭ��������i�������Ժ�ᱻ���ǡ�
 * ����Ȼ��һ�����������󣬵������������ȸ��±����������ף����������ټ���
 * GCC 4.{1,2},x CodeSourcery �������ϣ����� ARM �ϵ� 2007q3��2008q1��2008q3 lite �汾��
 * ע�⣺�� ARMv5 ��ʹ�� GCC4.6 �۲쵽���Ƶ���Ϊ��
 */
static void usb_set_maxpacket_ep(struct usb_device *dev,
                                 int if_idx,
                                 int ep_idx)
{
    int b;
    struct usb_endpoint_descriptor *ep;
    unsigned short ep_wMaxPacketSize;

    ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];      // ˵������˵����ĸ��ӿ��µ��ĸ��˵�

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
 * ���ø������������ж˵�������ֵ
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
 * ����λ�ڻ������е����ã������ dev->config �ṹ
 * ��ע�⣬����С/��˽��������Զ���ɵġ� ��wTotalLength �ڶ�ȡʱ�ѱ�������������
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
 *                              ���ֹͣ
 * �˵㣺�˵����0-3λ ;
 * �����־�� (1 = IN, 0 = OUT)
 **********************************************************************/
int usb_clear_halt(struct usb_device *dev, int pipe)
{
    int result;
    int endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

    result = usb_control_msg(dev,                           // ��*��
                             usb_sndctrlpipe(dev, 0),
                             USB_REQ_CLEAR_FEATURE,
                             USB_RECIP_ENDPOINT,
                             0,
                             endp,
                             NULL,
                             0,
                             USB_CNTL_TIMEOUT * 3 );

    /* ������Ƿ�ʧ�� */
    if (result < 0)
        return result;

    /* NOTE: ����û�л��״̬����֤�����Ƿ�ɹ�����Ϊ�ݱ���ĳЩ�豸�ڴ˼��ʱ����.. */
    usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));   // ����USB�˵㡾*���˵�ܵ���*��USB����ܵ���*��

    /* �л������ʱ���� */
    usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);       // USB�����л���*���˵�ܵ���*��USB����ܵ���*��
    
    return 0;
}

/**********************************************************************
 *                              ��ȡ����������
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
 *                            ��ȡ����cfgno�ĳ���
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
 *                      ��ȡ����cfgno������洢�ڻ�������
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
 * ���豸�ĵ�ַ����Ϊ dev->devnum �е�ֵ��
 * ��ֻ��ͨ��Ĭ�ϵ�ַ (0) Ѱַ�豸�����
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
 *                          ���ýӿں�Ϊ�ӿ�
 ********************************************************************/
int usb_set_interface(struct usb_device *dev,
                      int interface,
                      int alternate)
{
    struct usb_interface *if_face = NULL;
    int ret, i;

    for (i = 0; i < dev->config.desc.bNumInterfaces; i++)               // �������нӿڲ��ҽӿں�
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
 *                         �����ñ������Ϊ����
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
 *                          ��Э������ΪЭ��
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
 *                             ���ÿ���
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
 *                              ��ȡ����
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
 *                            ��ȡ��������
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
 *                      ��ȡ�������е��ַ�������
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
 * ��ȡ�ַ�������������ת��Ϊ ascii��
 * �����ַ������� (> 0) ����� (< 0)
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
 * USB �豸����USB �豸�Ǿ�̬����� [USB_MAX_DEVICE]��
 *******************************************************************/

#if (!DM_USB)

/*******************************************************************
 *                  �����豸ָ�룬û�з��ؿ�
 *  usb_dev�ǽṹ������
 *  �洢USB�豸�ṹ�壨struct usb_device��
 *  �����СΪ  USB_MAX_DEVICE
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

    if (dev_index == USB_MAX_DEVICE)                // ����������豸���޷��ٴ�����
    {
        printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
        return -ENOSPC;
    }

    /* Ĭ�ϵ�ַΪ0����ʵ��ַ��1��ͷ */
    usb_dev[dev_index].devnum = dev_index + 1;      // ��dev_index���豸��USB�����ϵ��豸��Ϊdev_index + 1
    usb_dev[dev_index].maxchild = 0;                // ���豸�������˿���Ϊ0
    for (i = 0; i < USB_MAXCHILDREN; i++)           // ���豸���Ǽ����������������豸���ǿ�
        usb_dev[dev_index].children[i] = NULL;
    usb_dev[dev_index].parent = NULL;               // ��������Ϊ��
    usb_dev[dev_index].controller = controller;     // Ӳ��������˽������Ϊ��
    dev_index++;                                    // �豸ָ��++
    *devp = &usb_dev[dev_index - 1];                // *devp��ָ����豸=�޸�֮���usb_dev[dev_index - 1]
    // ��ע������һ��Ϊ����ôд����������û�������豸���ṹ�壬ֻ�����޸��Ѿ��������豸����usb_dev[]��Ȼ�����豸ָ��ָ���豸������ĳһԪ�ء�����Լ�ڴ桿
    return 0;
}

/*
 * �ͷ��´������豸�ڵ㡣
 * �����������ӵ��豸����ĳ��ԭ��ʧ�ܵĴ�������µ��á�
 */
void usb_free_device(struct udevice *controller)
{
    dev_index--;                                                // �豸ָ��--
    debug("Freeing device node: %d\n", dev_index);
    memset(&usb_dev[dev_index], 0, sizeof(struct usb_device));  // ����豸����
    usb_dev[dev_index].devnum = -1;                             // ���豸�ڵ�ǰ�豸��USB�����ϵ��豸��Ϊ-1
}

/*
 * XHCI ���� Enable Slot ���Ȼ������豸�����ġ�
 * Ϊ��Ŀ���ṩһ���������������Ա� XHCI ���������� EHCI/OHCI ���伴�á�
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
    /*__maybe_unused*/ struct usb_device_descriptor *desc;          // �����豸�������ռ�
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, tmpbuf, USB_BUFSIZ);
    int err;
    desc = (struct usb_device_descriptor *)tmpbuf;

    err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);     // ��ȡ��������*������������Ϊ�豸����������������ʵ���յ��ĳ��ȣ�
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

    memcpy(&dev->descriptor, tmpbuf, sizeof(dev->descriptor));      // ���豸����������ʱ������tmpbuf������dev->descriptor

    return 0;
}

// ���� Windows �ĳ�ʼ�����з������豸˫�ظ�λ��Linux ʹ����ͬ�����У� ��˵��Щ�豸ֻ��ʹ�������ĳ�ʼ������
static int usb_setup_descriptor(struct usb_device *dev, bool do_read)
{
    // ���� 64 �ֽڵ� GET-DEVICE-DESCRIPTOR ��������������ֻ�� 18 ���ֽڳ����⽫��һ�������ݰ�������
    // ������� maxpacket ��СΪ 8 �� 16�����豸�������ڵȴ�����������ݣ����߼������´��� 8 �ֽڱ�ͷ��
    if (dev->speed == USB_SPEED_LOW)            // �������һ�������豸
    {
        dev->descriptor.bMaxPacketSize0 = 8;    // �豸����������ֽ�=8
        dev->maxpacketsize = PACKET_SIZE_8;     // �豸����������ֽڵı���=0���������0��1��2��3=>8��16��32��64��
    }
    else                                        // �����豸
    {
        dev->descriptor.bMaxPacketSize0 = 64;   // �豸����������ֽ�=64
        dev->maxpacketsize = PACKET_SIZE_64;    // �豸����������ֽڵı���=3���������0��1��2��3=>8��16��32��64��
    }

    dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;    // �������ֵд������˵�
    dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;   // �������ֵд������˵�

    if (do_read && dev->speed == USB_SPEED_FULL)// ȫ���豸
    {
        int err;

        /*
         * ��֤����ֻ�յ������� 8 ���ֽڣ������������յ��������������� �����ǣ�
         * - �ô����ʹ��ǰ 8 ���ֽ��е��ֶΣ��������������ֽ׶���Ҫ��ȡ���������ݡ�
         * - ��С�� maxpacket ��СΪ 8 ���ֽڡ� ������֪���豸ʹ�õ�ʵ�� maxpacket ֮ǰ��
         *   USB ����������ֻ����һ�����ݰ��� ��ˣ���ʹ�ڷǴ�������£�����Ҳֻ�ܱ�֤���� 1 �����ݰ������� 8 ���ֽڣ���
         *
         * �����ֽ���֮�⣬������Ҫ�� DWC2 ������������ݰ����� ���� 64 �ֽ������� maxpacket �²�Ϊ 64����ͼ������� 1 �����ݰ�������
         */
        err = get_descriptor_len(dev, 64, 8);   // ��ȡ��������*�����������豸�����ȣ�ϣ�����յ��ĳ��ȣ�

        if (err)
            return err;
    }

    dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;    // �������ֵд������˵�
    dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;   // �������ֵд������˵�

    switch (dev->descriptor.bMaxPacketSize0)                    // ����
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

    /* ���� usb 3.0 �豸�����ġ� USB 3.0 (xHCI) Э�����ȳ��Է����豸��ۺ�������ݽṹ�� ������þ����������ġ�*/
    err = usb_alloc_device(dev);    // ��о1B��֧��3.0�豸��*��ֱ�ӷ���0���������¿��ˣ�����������
    if (err)
    {
        printf("Cannot allocate device context to get SLOT_ID\n");
        return err;
    }

    err = usb_setup_descriptor(dev, do_read);   // �����豸��������*�����ٺ͵����豸ֻ��ֵ����������
    if (err)
        return err;
    err = usb_hub_port_reset(dev, parent);      // USB�������ӿڸ�λ��*��ɶ��ûִ�У���������ǰ�豸�븸�豸��
    if (err)
        return err;

    dev->devnum = addr;

    err = usb_set_address(dev);                 // ���õ�ַ��*��
    if (err < 0)
    {
        printf("\n      USB device not accepting new address (error=%lX)\n",dev->status);
        return err;
    }

    DELAY_MS(10);                               // ��ʱ�Եȴ����õ�ַ����

    // �������֮ǰû�ж�ȡ�豸�������������豸�����ַ���ڴ˴���ȡ�� ��ĿǰΪֹ����������� xHCI��
    if (!do_read)
    {
        err = usb_setup_descriptor(dev, true);  // do_read��TRUE,���Բ���ִ�д�if��*��ͬ��
        if (err)
            return err;
    }

    return 0;
}

int usb_select_config(struct usb_device *dev)
{
    unsigned char *tmpbuf = NULL;
    int err;

    err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);  // ��ȡ��������С��*��
    if (err)
        return err;

    /* ����ֵ */
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

    /* Ŀǰֻ֧��һ������ */
    err = usb_get_configuration_len(dev, 0);    // ��ȡ������������С��*�������ش�����룬�������ش�С��err
    if (err >= 0)                               // û����
    {
        tmpbuf = (unsigned char *)malloc_cache_aligned(err);    // û����err�����������ȣ�����err������ʱ����ռ�tmpbuf

        if (!tmpbuf)                                            // ��ʱ�洢�ռ�Ϊ�գ����ش������
            err = -ENOMEM;
        else
            err = usb_get_configuration_no(dev, 0, tmpbuf, err);// ��ȡ������������*���洢��tempbuf��
    }
    if (err < 0)
    {
        printf("usb_new_device: Cannot read configuration, " \
               "skipping device %04x:%04x\n",
                dev->descriptor.idVendor, dev->descriptor.idProduct);
        aligned_free(tmpbuf);                                   // �����ȡ�������������������ͷ���ʱ����ռ䡾*��
        return err;
    }

    usb_parse_config(dev, tmpbuf, 0);                           // ����������������*������λ�ڻ������������������д��dev->config
    aligned_free(tmpbuf);                                       // �ͷŻ�������*��
    usb_set_maxpacket(dev);                                     // �������ж˵�������ݰ���С��*��

    /*
     * ��������������Ĭ���������ƺ�Ϊʱ���硣 �������������Ҫ��ͬ�����ã�����Ҫ����ѡ��
     */
    err = usb_set_configuration(dev, dev->config.desc.bConfigurationValue);     // �����ñ������Ϊ���á�*��
    if (err < 0)
    {
        printf("failed to set default configuration len %d, status %lX\n",
                dev->act_len, dev->status);
        return err;
    }

    /*
     * �ȵ��豸���������������� DWC2 OTG �������ϵ� SanDisk Cruzer Pop USB 2.0 �� Kingston DT Ultimate 32GB USB 3.0 ������Ҫ�˹��ܡ�
     */
    DELAY_MS(10);

    debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
           dev->descriptor.iManufacturer, dev->descriptor.iProduct,
           dev->descriptor.iSerialNumber);

    memset(dev->mf, 0, sizeof(dev->mf));            // �豸����������
    memset(dev->prod, 0, sizeof(dev->prod));        // �豸��Ʒ����
    memset(dev->serial, 0, sizeof(dev->serial));    // ���к�����

    if (dev->descriptor.iManufacturer)              // д���豸��������Ϣ��*��
        usb_string(dev, dev->descriptor.iManufacturer, dev->mf, sizeof(dev->mf));
    if (dev->descriptor.iProduct)                   // д���Ʒ��Ϣ��*��
        usb_string(dev, dev->descriptor.iProduct, dev->prod, sizeof(dev->prod));
    if (dev->descriptor.iSerialNumber)              // д�����кš�*��
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

    /* ���ǻ�û�����õ�ַ */
    addr = dev->devnum;         // ��ַ���������ϵ��豸��
    dev->devnum = 0;            // �����ϵ��豸�Ź�0

    ret = usb_prepare_device(dev, addr, do_read, parent);   // ׼��USB�豸��*��
    if (ret)
        return ret;

    ret = usb_select_config(dev);   // USBѡ�����á�*������λ���������󣬻�ȡ�豸���������浽��������������������ݺ󱣴浽�豸�ṹ���Ӧλ��

    return ret;
}

#if (!DM_USB)
/*
 * �����ǵ�������ʱ���豸�Ѿ������һ���µ��豸 ID ���Ҵ���Ĭ��״̬��
 * ������Ҫʶ�����ﲢ������������� 0 ��ʾ�ɹ���!= 0 ��ʾ����
 */
int usb_new_device(struct usb_device *dev)
{
    bool do_read = true;
    int err;

    /* XHCI ��Ҫ������ַ�豸������������ȷ���豸�����Ľṹ��Ȼ��������豸������
       ��ˣ��� EHCI ��ͬ����Ϊ XHCI ����κβ���֮ǰ��get_descriptor ��ʧ��. */
#ifdef CONFIG_USB_XHCI_HCD
    do_read = false;
#endif

    err = usb_setup_device(dev, do_read, dev->parent);  // �����豸���豸/true/���豸����*��
    if (err)
        return err;

    // ����̽���豸�Ƿ��Ǽ�������*���������Ǽ�����ֱ�ӷ���0���Ǽ����������ü������󷵻أ����ò�����Ҳ�Ƿ���0����
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



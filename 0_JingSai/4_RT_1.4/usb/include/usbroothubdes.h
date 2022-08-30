/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * USB virtual root hub descriptors
 *
 * (C) Copyright 2014
 * Stephen Warren swarren@wwwdotorg.org
 *
 * Based on ohci-hcd.c
 */

#ifndef __USBROOTHUBDES_H__
#define __USBROOTHUBDES_H__

/*
 * Device descriptor
 */
static unsigned char root_hub_dev_des[] =
{
    0x12,       /* unsigned char   bLength; */
    0x01,       /* unsigned char   bDescriptorType; Device */
    0x10,       /* unsigned short  bcdUSB; v1.1 */
    0x01,
    0x09,       /* unsigned char   bDeviceClass; HUB_CLASSCODE */
    0x00,       /* unsigned char   bDeviceSubClass; */
    0x00,       /* unsigned char   bDeviceProtocol; */
    0x08,       /* unsigned char   bMaxPacketSize0; 8 Bytes */
    0x00,       /* unsigned short  idVendor; */
    0x00,
    0x00,       /* unsigned short  idProduct; */
    0x00,
    0x00,       /* unsigned short  bcdDevice; */
    0x00,
    0x00,       /* unsigned char   iManufacturer; */
    0x01,       /* unsigned char   iProduct; */
    0x00,       /* unsigned char   iSerialNumber; */
    0x01,       /* unsigned char   bNumConfigurations; */
};

/* Configuration descriptor */
static unsigned char root_hub_config_des[] =
{
    0x09,       /* unsigned char   bLength; */
    0x02,       /* unsigned char   bDescriptorType; Configuration */
    0x19,       /* unsigned short  wTotalLength; */
    0x00,
    0x01,       /* unsigned char   bNumInterfaces; */
    0x01,       /* unsigned char   bConfigurationValue; */
    0x00,       /* unsigned char   iConfiguration; */
    0x40,       /* unsigned char   bmAttributes;
                 *       Bit 7: Bus-powered
                 *           6: Self-powered,
                 *           5: Remote-wakwup,
                 *           4..0: resvd
                 */
    0x00,       /* unsigned char   MaxPower; */
    /* interface */
    0x09,       /* unsigned char   if_bLength; */
    0x04,       /* unsigned char   if_bDescriptorType; Interface */
    0x00,       /* unsigned char   if_bInterfaceNumber; */
    0x00,       /* unsigned char   if_bAlternateSetting; */
    0x01,       /* unsigned char   if_bNumEndpoints; */
    0x09,       /* unsigned char   if_bInterfaceClass; HUB_CLASSCODE */
    0x00,       /* unsigned char   if_bInterfaceSubClass; */
    0x00,       /* unsigned char   if_bInterfaceProtocol; */
    0x00,       /* unsigned char   if_iInterface; */
    /* endpoint */
    0x07,       /* unsigned char   ep_bLength; */
    0x05,       /* unsigned char   ep_bDescriptorType; Endpoint */
    0x81,       /* unsigned char   ep_bEndpointAddress; IN Endpoint 1 */
    0x03,       /* unsigned char   ep_bmAttributes; Interrupt */
    0x02,       /* unsigned short  ep_wMaxPacketSize; ((MAX_ROOT_PORTS + 1) / 8 */
    0x00,
    0xff,       /* unsigned char   ep_bInterval; 255 ms */
};

#ifdef WANT_USB_ROOT_HUB_HUB_DES
static unsigned char root_hub_hub_des[] =
{
    0x09,       /* unsigned char   bLength; */
    0x29,       /* unsigned char   bDescriptorType; Hub-descriptor */
    0x02,       /* unsigned char   bNbrPorts; */
    0x00,       /* unsigned short  wHubCharacteristics; */
    0x00,
    0x01,       /* unsigned char   bPwrOn2pwrGood; 2ms */
    0x00,       /* unsigned char   bHubContrCurrent; 0 mA */
    0x00,       /* unsigned char   DeviceRemovable; *** 7 Ports max *** */
    0xff,       /* unsigned char   PortPwrCtrlMask; *** 7 ports max *** */
};
#endif

static unsigned char root_hub_str_index0[] =
{
    0x04,       /* unsigned char   bLength; */
    0x03,       /* unsigned char   bDescriptorType; String-descriptor */
    0x09,       /* unsigned char   lang ID */
    0x04,       /* unsigned char   lang ID */
};

static unsigned char root_hub_str_index1[] =
{
    32,         /* unsigned char   bLength; */
    0x03,       /* unsigned char   bDescriptorType; String-descriptor */
    'U',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    '-',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'B',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'o',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'o',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    't',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    ' ',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'R',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'o',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'o',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    't',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    ' ',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'H',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'u',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
    'b',        /* unsigned char   Unicode */
    0,          /* unsigned char   Unicode */
};

#endif



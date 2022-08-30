/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 *
 * Based on linux/drivers/usbd/usbd.h
 *
 * Copyright (c) 2000, 2001, 2002 Lineo
 * Copyright (c) 2001 Hewlett Packard
 *
 * By:
 *  Stuart Lynne <sl@lineo.com>,
 *  Tom Rushworth <tbr@lineo.com>,
 *  Bruce Balden <balden@lineo.com>
 */

#ifndef __USBDCORE_H__
#define __USBDCORE_H__

#include "usbdescriptors.h"

#define MAX_URBS_QUEUED 5

#if 1
#define USBERR(fmt,args...) printf("ERROR: %s(), %d: "fmt"\n",__FUNCTION__,__LINE__,##args)
#else
#define USBERR(fmt,args...) do {} while(0)
#endif

#if 0
#define USBDBG(fmt,args...) printf("debug: %s(), %d: "fmt"\n",__FUNCTION__,__LINE__,##args)
#else
#define USBDBG(fmt,args...) do {} while(0)
#endif

#if 0
#define USBINFO(fmt,args...) printf("info: %s(), %d: "fmt"\n",__FUNCTION__,__LINE__,##args)
#else
#define USBINFO(fmt,args...) do {} while(0)
#endif

/*
 * Structure member address manipulation macros.
 * These are used by client code (code using the urb_link routines), since
 * the urb_link structure is embedded in the client data structures.
 *
 * Note: a macro offsetof equivalent to member_offset is defined in stddef.h
 *   but this is kept here for the sake of portability.
 *
 * p2surround returns a pointer to the surrounding structure given
 * type of the surrounding structure, the name memb of the structure
 * member pointed at by ptr.  For example, if you have:
 *
 *  struct foo
 *  {
 *      int x;
 *      float y;
 *      char z;
 *  } thingy;
 *
 *  char *cp = &thingy.z;
 *
 * then
 *
 *  &thingy == p2surround(struct foo, z, cp)
 *
 * Clear?
 */
#define _cv_(ptr)                 ((char*)(void*)(ptr))
#define member_offset(type,memb)  (_cv_(&(((type*)0)->memb))-(char*)0)
#define p2surround(type,memb,ptr) ((type*)(void*)(_cv_(ptr)-member_offset(type,memb)))

struct urb;

struct usb_endpoint_instance;
struct usb_interface_instance;
struct usb_configuration_instance;
struct usb_device_instance;
struct usb_bus_instance;

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE     0   /* for DeviceClass */
#define USB_CLASS_AUDIO             1
#define USB_CLASS_COMM              2
#define USB_CLASS_HID               3
#define USB_CLASS_PHYSICAL          5
#define USB_CLASS_PRINTER           7
#define USB_CLASS_MASS_STORAGE      8
#define USB_CLASS_HUB               9
#define USB_CLASS_DATA              10
#define USB_CLASS_APP_SPEC          0xfe
#define USB_CLASS_VENDOR_SPEC       0xff

/*
 * USB types
 */
#define USB_TYPE_STANDARD       (0x00 << 5)
#define USB_TYPE_CLASS          (0x01 << 5)
#define USB_TYPE_VENDOR         (0x02 << 5)
#define USB_TYPE_RESERVED       (0x03 << 5)

/*
 * USB recipients
 */
#define USB_RECIP_DEVICE        0x00
#define USB_RECIP_INTERFACE     0x01
#define USB_RECIP_ENDPOINT      0x02
#define USB_RECIP_OTHER         0x03

/*
 * USB directions
 */
#define USB_DIR_OUT             0
#define USB_DIR_IN              0x80

/*
 * Descriptor types
 */
#define USB_DT_DEVICE           0x01
#define USB_DT_CONFIG           0x02
#define USB_DT_STRING           0x03
#define USB_DT_INTERFACE        0x04
#define USB_DT_ENDPOINT         0x05

#if defined(CONFIG_USBD_HS)
#define USB_DT_QUAL             0x06
#endif

#define USB_DT_HID              (USB_TYPE_CLASS | 0x01)
#define USB_DT_REPORT           (USB_TYPE_CLASS | 0x02)
#define USB_DT_PHYSICAL         (USB_TYPE_CLASS | 0x03)
#define USB_DT_HUB              (USB_TYPE_CLASS | 0x09)

/*
 * 每个描述符类型的描述符大小
 */
#define USB_DT_DEVICE_SIZE          18
#define USB_DT_CONFIG_SIZE          9
#define USB_DT_INTERFACE_SIZE       9
#define USB_DT_ENDPOINT_SIZE        7
#define USB_DT_ENDPOINT_AUDIO_SIZE  9       /* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE      7
#define USB_DT_HID_SIZE             9

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK    0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK       0x80

#define USB_ENDPOINT_XFERTYPE_MASK  0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL   0
#define USB_ENDPOINT_XFER_ISOC      1
#define USB_ENDPOINT_XFER_BULK      2
#define USB_ENDPOINT_XFER_INT       3

/*
 * USB Packet IDs (PIDs)
 */
#define USB_PID_UNDEF_0             0xf0
#define USB_PID_OUT                 0xe1
#define USB_PID_ACK                 0xd2
#define USB_PID_DATA0               0xc3
#define USB_PID_PING                0xb4    /* USB 2.0 */
#define USB_PID_SOF                 0xa5
#define USB_PID_NYET                0x96    /* USB 2.0 */
#define USB_PID_DATA2               0x87    /* USB 2.0 */
#define USB_PID_SPLIT               0x78    /* USB 2.0 */
#define USB_PID_IN                  0x69
#define USB_PID_NAK                 0x5a
#define USB_PID_DATA1               0x4b
#define USB_PID_PREAMBLE            0x3c    /* Token mode */
#define USB_PID_ERR                 0x3c    /* USB 2.0: handshake mode */
#define USB_PID_SETUP               0x2d
#define USB_PID_STALL               0x1e
#define USB_PID_MDATA               0x0f    /* USB 2.0 */

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS          0x00
#define USB_REQ_CLEAR_FEATURE       0x01
#define USB_REQ_SET_FEATURE         0x03
#define USB_REQ_SET_ADDRESS         0x05
#define USB_REQ_GET_DESCRIPTOR      0x06
#define USB_REQ_SET_DESCRIPTOR      0x07
#define USB_REQ_GET_CONFIGURATION   0x08
#define USB_REQ_SET_CONFIGURATION   0x09
#define USB_REQ_GET_INTERFACE       0x0A
#define USB_REQ_SET_INTERFACE       0x0B
#define USB_REQ_SYNCH_FRAME         0x0C

#define USBD_DEVICE_REQUESTS(x)     (((unsigned int)x <= USB_REQ_SYNCH_FRAME) ? usbd_device_requests[x] : "UNKNOWN")

/*
 * HID requests
 */
#define USB_REQ_GET_REPORT          0x01
#define USB_REQ_GET_IDLE            0x02
#define USB_REQ_GET_PROTOCOL        0x03
#define USB_REQ_SET_REPORT          0x09
#define USB_REQ_SET_IDLE            0x0A
#define USB_REQ_SET_PROTOCOL        0x0B

/*
 * USB Spec Release number
 */
#if defined(CONFIG_USBD_HS)
#define USB_BCD_VERSION             0x0200
#else
#define USB_BCD_VERSION             0x0110
#endif

/*
 * Device Requests  (c.f Table 9-2)
 */
#define USB_REQ_DIRECTION_MASK      0x80
#define USB_REQ_TYPE_MASK           0x60
#define USB_REQ_RECIPIENT_MASK      0x1f

#define USB_REQ_DEVICE2HOST         0x80
#define USB_REQ_HOST2DEVICE         0x00

#define USB_REQ_TYPE_STANDARD       0x00
#define USB_REQ_TYPE_CLASS          0x20
#define USB_REQ_TYPE_VENDOR         0x40

#define USB_REQ_RECIPIENT_DEVICE    0x00
#define USB_REQ_RECIPIENT_INTERFACE 0x01
#define USB_REQ_RECIPIENT_ENDPOINT  0x02
#define USB_REQ_RECIPIENT_OTHER     0x03

/*
 * get status bits
 */
#define USB_STATUS_SELFPOWERED      0x01
#define USB_STATUS_REMOTEWAKEUP     0x02

#define USB_STATUS_HALT             0x01

/*
 * descriptor types
 */
#define USB_DESCRIPTOR_TYPE_DEVICE              0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION       0x02
#define USB_DESCRIPTOR_TYPE_STRING              0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE           0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT            0x05
#define USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER    0x06
#define USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION 0x07
#define USB_DESCRIPTOR_TYPE_INTERFACE_POWER     0x08
#define USB_DESCRIPTOR_TYPE_HID                 0x21
#define USB_DESCRIPTOR_TYPE_REPORT              0x22

#define USBD_DEVICE_DESCRIPTORS(x) (((unsigned int)x <= USB_DESCRIPTOR_TYPE_INTERFACE_POWER) ? \
        usbd_device_descriptors[x] : "UNKNOWN")

/*
 * standard feature selectors
 */
#define USB_ENDPOINT_HALT           0x00
#define USB_DEVICE_REMOTE_WAKEUP    0x01
#define USB_TEST_MODE               0x02

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

#endif



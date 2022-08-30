/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 *
 * Based on
 * linux/drivers/usbd/usb-function.h - USB Function
 *
 * Copyright (c) 2000, 2001, 2002 Lineo
 * Copyright (c) 2001 Hewlett Packard
 *
 * By:
 *  Stuart Lynne <sl@lineo.com>,
 *  Tom Rushworth <tbr@lineo.com>,
 *  Bruce Balden <balden@lineo.com>
 */

/* USB Descriptors - Create a complete description of all of the
 * function driver capabilities. These map directly to the USB descriptors.
 *
 * This heirarchy is created by the functions drivers and is passed to the
 * usb-device driver when the function driver is registered.
 *
 *  device
 *  configuration
 *       interface
 *      alternate
 *           class
 *           class
 *      alternate
 *           endpoint
 *           endpoint
 *       interface
 *      alternate
 *           endpoint
 *           endpoint
 *  configuration
 *       interface
 *      alternate
 *           endpoint
 *           endpoint
 *
 *
 * The configuration structures refer to the USB Configurations that will be
 * made available to a USB HOST during the enumeration process.
 *
 * The USB HOST will select a configuration and optionally an interface with
 * the usb set configuration and set interface commands.
 *
 * The selected interface (or the default interface if not specifically
 * selected) will define the list of endpoints that will be used.
 *
 * The configuration and interfaces are stored in an array that is indexed
 * by the specified configuratin or interface number minus one.
 *
 * A configuration number of zero is used to specify a return to the unconfigured
 * state.
 *
 */

#ifndef __USBDESCRIPTORS_H__
#define __USBDESCRIPTORS_H__

//#include <asm/types.h>

/*
 * communications class types
 *
 * c.f. CDC  USB Class Definitions for Communications Devices
 * c.f. WMCD USB CDC Subclass Specification for Wireless Mobile Communications Devices
 *
 */

#define CLASS_BCD_VERSION           0x0110

/* c.f. CDC 4.1 Table 14 */
#define COMMUNICATIONS_DEVICE_CLASS 0x02

/* c.f. CDC 4.2 Table 15 */
#define COMMUNICATIONS_INTERFACE_CLASS_CONTROL  0x02
#define COMMUNICATIONS_INTERFACE_CLASS_DATA     0x0A
#define COMMUNICATIONS_INTERFACE_CLASS_VENDOR   0x0FF

/* c.f. CDC 4.3 Table 16 */
#define COMMUNICATIONS_NO_SUBCLASS      0x00
#define COMMUNICATIONS_DLCM_SUBCLASS    0x01
#define COMMUNICATIONS_ACM_SUBCLASS     0x02
#define COMMUNICATIONS_TCM_SUBCLASS     0x03
#define COMMUNICATIONS_MCCM_SUBCLASS    0x04
#define COMMUNICATIONS_CCM_SUBCLASS     0x05
#define COMMUNICATIONS_ENCM_SUBCLASS    0x06
#define COMMUNICATIONS_ANCM_SUBCLASS    0x07

/* c.f. WMCD 5.1 */
#define COMMUNICATIONS_WHCM_SUBCLASS    0x08
#define COMMUNICATIONS_DMM_SUBCLASS     0x09
#define COMMUNICATIONS_MDLM_SUBCLASS    0x0a
#define COMMUNICATIONS_OBEX_SUBCLASS    0x0b

/* c.f. CDC 4.4 Table 17 */
#define COMMUNICATIONS_NO_PROTOCOL      0x00
#define COMMUNICATIONS_V25TER_PROTOCOL  0x01    /*Common AT Hayes compatible*/

/* c.f. CDC 4.5 Table 18 */
#define DATA_INTERFACE_CLASS            0x0a

/* c.f. CDC 4.6 No Table */
#define DATA_INTERFACE_SUBCLASS_NONE    0x00    /* No subclass pertinent */

/* c.f. CDC 4.7 Table 19 */
#define DATA_INTERFACE_PROTOCOL_NONE    0x00    /* No class protcol required */

/* c.f. CDC 5.2.3 Table 24 */
#define CS_INTERFACE        0x24
#define CS_ENDPOINT         0x25

/*
 * bDescriptorSubtypes
 *
 * c.f. CDC 5.2.3 Table 25
 * c.f. WMCD 5.3 Table 5.3
 */
#define USB_ST_HEADER       0x00
#define USB_ST_CMF          0x01
#define USB_ST_ACMF         0x02
#define USB_ST_DLMF         0x03
#define USB_ST_TRF          0x04
#define USB_ST_TCLF         0x05
#define USB_ST_UF           0x06
#define USB_ST_CSF          0x07
#define USB_ST_TOMF         0x08
#define USB_ST_USBTF        0x09
#define USB_ST_NCT          0x0a
#define USB_ST_PUF          0x0b
#define USB_ST_EUF          0x0c
#define USB_ST_MCMF         0x0d
#define USB_ST_CCMF         0x0e
#define USB_ST_ENF          0x0f
#define USB_ST_ATMNF        0x10

#define USB_ST_WHCM         0x11
#define USB_ST_MDLM         0x12
#define USB_ST_MDLMD        0x13
#define USB_ST_DMM          0x14
#define USB_ST_OBEX         0x15
#define USB_ST_CS           0x16
#define USB_ST_CSD          0x17
#define USB_ST_TCM          0x18

/* endpoint modifiers
 * static struct usb_endpoint_description function_default_A_1[] = {
 *
 *     {this_endpoint: 0, attributes: CONTROL,   max_size: 8,  polling_interval: 0 },
 *     {this_endpoint: 1, attributes: BULK,  max_size: 64, polling_interval: 0, direction: IN},
 *     {this_endpoint: 2, attributes: BULK,  max_size: 64, polling_interval: 0, direction: OUT},
 *     {this_endpoint: 3, attributes: INTERRUPT, max_size: 8,  polling_interval: 0},
 *
 *
 */
#define OUT         0x00
#define IN          0x80

#define CONTROL     0x00
#define ISOCHRONOUS 0x01
#define BULK        0x02
#define INTERRUPT   0x03

/* configuration modifiers
 */
#define BMATTRIBUTE_RESERVED        0x80
#define BMATTRIBUTE_SELF_POWERED    0x40

/*
 * standard usb descriptor structures
 */
struct usb_endpoint_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;   /* 0x5 */
    unsigned char  bEndpointAddress;
    unsigned char  bmAttributes;
    unsigned short wMaxPacketSize;
    unsigned char  bInterval;
} __attribute__ ((packed));

struct usb_interface_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;    /* 0x04 */
    unsigned char bInterfaceNumber;
    unsigned char bAlternateSetting;
    unsigned char bNumEndpoints;
    unsigned char bInterfaceClass;
    unsigned char bInterfaceSubClass;
    unsigned char bInterfaceProtocol;
    unsigned char iInterface;
} __attribute__ ((packed));

struct usb_configuration_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;   /* 0x2 */
    unsigned short wTotalLength;
    unsigned char  bNumInterfaces;
    unsigned char  bConfigurationValue;
    unsigned char  iConfiguration;
    unsigned char  bmAttributes;
    unsigned char  bMaxPower;
} __attribute__ ((packed));

struct usb_device_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;   /* 0x01 */
    unsigned short bcdUSB;
    unsigned char  bDeviceClass;
    unsigned char  bDeviceSubClass;
    unsigned char  bDeviceProtocol;
    unsigned char  bMaxPacketSize0;
    unsigned short idVendor;
    unsigned short idProduct;
    unsigned short bcdDevice;
    unsigned char  iManufacturer;
    unsigned char  iProduct;
    unsigned char  iSerialNumber;
    unsigned char  bNumConfigurations;
} __attribute__ ((packed));

#if defined(CONFIG_USBD_HS)
struct usb_qualifier_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;

    unsigned short bcdUSB;
    unsigned char bDeviceClass;
    unsigned char bDeviceSubClass;
    unsigned char bDeviceProtocol;
    unsigned char bMaxPacketSize0;
    unsigned char bNumConfigurations;
    unsigned char breserved;
} __attribute__ ((packed));
#endif

struct usb_string_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;   /* 0x03 */
    unsigned short wData[0];
} __attribute__ ((packed));

struct usb_generic_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype;
} __attribute__ ((packed));

/*
 * communications class descriptor structures
 *
 * c.f. CDC 5.2 Table 25c
 */

struct usb_class_function_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype;
} __attribute__ ((packed));

struct usb_class_function_descriptor_generic
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype;
    unsigned char bmCapabilities;
} __attribute__ ((packed));

struct usb_class_header_function_descriptor
{
    unsigned char  bFunctionLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;    /* 0x00 */
    unsigned short bcdCDC;
} __attribute__ ((packed));

struct usb_class_call_management_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x01 */
    unsigned char bmCapabilities;
    unsigned char bDataInterface;
} __attribute__ ((packed));

struct usb_class_abstract_control_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x02 */
    unsigned char bmCapabilities;
} __attribute__ ((packed));

struct usb_class_direct_line_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x03 */
} __attribute__ ((packed));

struct usb_class_telephone_ringer_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x04 */
    unsigned char bRingerVolSeps;
    unsigned char bNumRingerPatterns;
} __attribute__ ((packed));

struct usb_class_telephone_call_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x05 */
    unsigned char bmCapabilities;
} __attribute__ ((packed));

struct usb_class_union_function_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x06 */
    unsigned char bMasterInterface;
    /* unsigned char bSlaveInterface0[0]; */
    unsigned char bSlaveInterface0;
} __attribute__ ((packed));

struct usb_class_country_selection_descriptor
{
    unsigned char  bFunctionLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;    /* 0x07 */
    unsigned char  iCountryCodeRelDate;
    unsigned short wCountryCode0[0];
} __attribute__ ((packed));


struct usb_class_telephone_operational_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x08 */
    unsigned char bmCapabilities;
} __attribute__ ((packed));


struct usb_class_usb_terminal_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x09 */
    unsigned char bEntityId;
    unsigned char bInterfaceNo;
    unsigned char bOutInterfaceNo;
    unsigned char bmOptions;
    unsigned char bChild0[0];
} __attribute__ ((packed));

struct usb_class_network_channel_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x0a */
    unsigned char bEntityId;
    unsigned char iName;
    unsigned char bChannelIndex;
    unsigned char bPhysicalInterface;
} __attribute__ ((packed));

struct usb_class_protocol_unit_function_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x0b */
    unsigned char bEntityId;
    unsigned char bProtocol;
    unsigned char bChild0[0];
} __attribute__ ((packed));

struct usb_class_extension_unit_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x0c */
    unsigned char bEntityId;
    unsigned char bExtensionCode;
    unsigned char iName;
    unsigned char bChild0[0];
} __attribute__ ((packed));

struct usb_class_multi_channel_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x0d */
    unsigned char bmCapabilities;
} __attribute__ ((packed));

struct usb_class_capi_control_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x0e */
    unsigned char bmCapabilities;
} __attribute__ ((packed));

struct usb_class_ethernet_networking_descriptor
{
    unsigned char  bFunctionLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;    /* 0x0f */
    unsigned char  iMACAddress;
    unsigned int   bmEthernetStatistics;
    unsigned short wMaxSegmentSize;
    unsigned short wNumberMCFilters;
    unsigned char  bNumberPowerFilters;
} __attribute__ ((packed));

struct usb_class_atm_networking_descriptor
{
    unsigned char  bFunctionLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;    /* 0x10 */
    unsigned char  iEndSystermIdentifier;
    unsigned char  bmDataCapabilities;
    unsigned char  bmATMDeviceStatistics;
    unsigned short wType2MaxSegmentSize;
    unsigned short wType3MaxSegmentSize;
    unsigned short wMaxVC;
} __attribute__ ((packed));

struct usb_class_mdlm_descriptor
{
    unsigned char  bFunctionLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;    /* 0x12 */
    unsigned short bcdVersion;
    unsigned char  bGUID[16];
} __attribute__ ((packed));

struct usb_class_mdlmd_descriptor
{
    unsigned char bFunctionLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype; /* 0x13 */
    unsigned char bGuidDescriptorType;
    unsigned char bDetailData[0];

} __attribute__ ((packed));

/*
 * HID class descriptor structures
 *
 * c.f. HID 6.2.1
 */
struct usb_class_hid_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short bcdCDC;
    unsigned char  bCountryCode;
    unsigned char  bNumDescriptors;   /* 0x01 */
    unsigned char  bDescriptorType0;
    unsigned short wDescriptorLength0;
    /* optional descriptors are not supported. */
} __attribute__((packed));

struct usb_class_report_descriptor
{
    unsigned char  bLength;   /* dummy */
    unsigned char  bDescriptorType;
    unsigned short wLength;
    unsigned char  bData[0];
} __attribute__((packed));

/*
 * descriptor union structures
 */
struct usb_descriptor
{
    union
    {
        struct usb_generic_descriptor generic;
        struct usb_endpoint_descriptor endpoint;
        struct usb_interface_descriptor interface;
        struct usb_configuration_descriptor configuration;
        struct usb_device_descriptor device;
        struct usb_string_descriptor string;
    } descriptor;

} __attribute__ ((packed));

struct usb_class_descriptor
{
    union
    {
        struct usb_class_function_descriptor function;
        struct usb_class_function_descriptor_generic generic;
        struct usb_class_header_function_descriptor header_function;
        struct usb_class_call_management_descriptor call_management;
        struct usb_class_abstract_control_descriptor abstract_control;
        struct usb_class_direct_line_descriptor direct_line;
        struct usb_class_telephone_ringer_descriptor telephone_ringer;
        struct usb_class_telephone_operational_descriptor telephone_operational;
        struct usb_class_telephone_call_descriptor telephone_call;
        struct usb_class_union_function_descriptor union_function;
        struct usb_class_country_selection_descriptor country_selection;
        struct usb_class_usb_terminal_descriptor usb_terminal;
        struct usb_class_network_channel_descriptor network_channel;
        struct usb_class_extension_unit_descriptor extension_unit;
        struct usb_class_multi_channel_descriptor multi_channel;
        struct usb_class_capi_control_descriptor capi_control;
        struct usb_class_ethernet_networking_descriptor ethernet_networking;
        struct usb_class_atm_networking_descriptor atm_networking;
        struct usb_class_mdlm_descriptor mobile_direct;
        struct usb_class_mdlmd_descriptor mobile_direct_detail;
        struct usb_class_hid_descriptor hid;
    } descriptor;

} __attribute__ ((packed));

#ifdef DEBUG
static inline void print_device_descriptor(struct usb_device_descriptor *d)
{
    serial_printf("usb device descriptor \n");
    serial_printf("\tbLength %2.2x\n", d->bLength);
    serial_printf("\tbDescriptorType %2.2x\n", d->bDescriptorType);
    serial_printf("\tbcdUSB %4.4x\n", d->bcdUSB);
    serial_printf("\tbDeviceClass %2.2x\n", d->bDeviceClass);
    serial_printf("\tbDeviceSubClass %2.2x\n", d->bDeviceSubClass);
    serial_printf("\tbDeviceProtocol %2.2x\n", d->bDeviceProtocol);
    serial_printf("\tbMaxPacketSize0 %2.2x\n", d->bMaxPacketSize0);
    serial_printf("\tidVendor %4.4x\n", d->idVendor);
    serial_printf("\tidProduct %4.4x\n", d->idProduct);
    serial_printf("\tbcdDevice %4.4x\n", d->bcdDevice);
    serial_printf("\tiManufacturer %2.2x\n", d->iManufacturer);
    serial_printf("\tiProduct %2.2x\n", d->iProduct);
    serial_printf("\tiSerialNumber %2.2x\n", d->iSerialNumber);
    serial_printf("\tbNumConfigurations %2.2x\n", d->bNumConfigurations);
}

#else

/* stubs */
#define print_device_descriptor(d)

#endif /* DEBUG */

#endif


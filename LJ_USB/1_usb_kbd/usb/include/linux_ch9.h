/*
 * This file holds USB constants and structures that are needed for
 * USB device APIs.  These are used by the USB device model, which is
 * defined in chapter 9 of the USB 2.0 specification and in the
 * Wireless USB 1.0 (spread around).  Linux has several APIs in C that
 * need these:
 *
 * - the master/host side Linux-USB kernel driver API;
 * - the "usbfs" user space API; and
 * - the Linux "gadget" slave/device/peripheral side driver API.
 *
 * USB 2.0 adds an additional "On The Go" (OTG) mode, which lets systems
 * act either as a USB master/host or as a USB slave/device.  That means
 * the master and slave side APIs benefit from working well together.
 *
 * There's also "Wireless USB", using low power short range radios for
 * peripheral interconnection but otherwise building on the USB framework.
 *
 * Note all descriptors are declared '__attribute__((packed))' so that:
 *
 * [a] they never get padded, either internally (USB spec writers
 *     probably handled that) or externally;
 *
 * [b] so that accessing bigger-than-a-bytes fields will never
 *     generate bus errors on any platform, even when the location of
 *     its descriptor inside a bundle isn't "naturally aligned", and
 *
 * [c] for consistency, removing all doubt even when it appears to
 *     someone that the two other points are non-issues for that
 *     particular descriptor type.
 */

#ifndef __LINUX_USB_CH9_H
#define __LINUX_USB_CH9_H

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, or
 * (rarely) accepted by SET_DESCRIPTOR.
 *
 * Note that all multi-byte values here are encoded in little endian
 * byte order "on the wire".  Within the kernel and when exposed
 * through the Linux-USB APIs, they are not converted to cpu byte
 * order; it is the responsibility of the client code to do this.
 * The single exception is when device and configuration descriptors (but
 * not other descriptors) are read from usbfs (i.e. /proc/bus/usb/BBB/DDD);
 * in this case the fields are converted to host endianness by the kernel.
 */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIG                   0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05
#define USB_DT_DEVICE_QUALIFIER         0x06
#define USB_DT_OTHER_SPEED_CONFIG       0x07
#define USB_DT_INTERFACE_POWER          0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG                      0x09
#define USB_DT_DEBUG                    0x0a
#define USB_DT_INTERFACE_ASSOCIATION    0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY                 0x0c
#define USB_DT_KEY                      0x0d
#define USB_DT_ENCRYPTION_TYPE          0x0e
#define USB_DT_BOS                      0x0f
#define USB_DT_DEVICE_CAPABILITY        0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP   0x11
#define USB_DT_WIRE_ADAPTER             0x21
#define USB_DT_RPIPE                    0x22
#define USB_DT_CS_RADIO_CONTROL         0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE               0x24
/* From the USB 3.0 spec */
#define USB_DT_SS_ENDPOINT_COMP         0x30
/* From HID 1.11 spec */
#define USB_DT_HID_REPORT               0x22

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_DT_CS_DEVICE        (USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG        (USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING        (USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE     (USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT      (USB_TYPE_CLASS | USB_DT_ENDPOINT)

/*
 * All standard descriptors have these 2 fields at the beginning
 */
struct usb_descriptor_header
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
} __attribute__ ((packed));

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/* USB_DT_CONFIG: Configuration descriptor information.
 *
 * USB_DT_OTHER_SPEED_CONFIG is the same descriptor, except that the
 * descriptor type is different.  Highspeed-capable devices can look
 * different depending on what speed they're currently running.  Only
 * devices with a USB_DT_DEVICE_QUALIFIER have any OTHER_SPEED_CONFIG
 * descriptors.
 */
struct usb_config_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned short wTotalLength;
    unsigned char  bNumInterfaces;
    unsigned char  bConfigurationValue;
    unsigned char  iConfiguration;
    unsigned char  bmAttributes;
    unsigned char  bMaxPower;
} __attribute__ ((packed));

#define USB_DT_CONFIG_SIZE          9

/*
 * from config descriptor bmAttributes
 */
#define USB_CONFIG_ATT_ONE          (1 << 7)    /* must be set */
#define USB_CONFIG_ATT_SELFPOWER    (1 << 6)    /* self powered */
#define USB_CONFIG_ATT_WAKEUP       (1 << 5)    /* can wakeup */
#define USB_CONFIG_ATT_BATTERY      (1 << 4)    /* battery powered */

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * USB_DT_SS_ENDPOINT_COMP: SuperSpeed Endpoint Companion descriptor
 */
struct usb_ss_ep_comp_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned char  bMaxBurst;
    unsigned char  bmAttributes;
    unsigned short wBytesPerInterval;
} __attribute__ ((packed));

#define USB_DT_SS_EP_COMP_SIZE      6

/*
 * Bits 4:0 of bmAttributes if this is a bulk endpoint
 */
static inline int
usb_ss_max_streams(const struct usb_ss_ep_comp_descriptor *comp)
{
    int max_streams;

    if (!comp)
        return 0;

    max_streams = comp->bmAttributes & 0x1f;

    if (!max_streams)
        return 0;

    max_streams = 1 << max_streams;

    return max_streams;
}

/*
 * Bits 1:0 of bmAttributes if this is an isoc endpoint
 */
#define USB_SS_MULT(p)          (1 + ((p) & 0x3))

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * USB 2.0 defines three speeds, here's how Linux identifies them
 */

enum usb_device_speed
{
    USB_SPEED_UNKNOWN = 0,              /* enumerating */
    USB_SPEED_LOW, USB_SPEED_FULL,      /* usb 1.1 */
    USB_SPEED_HIGH,                     /* usb 2.0 */
    USB_SPEED_WIRELESS,                 /* wireless (usb 2.5) */
    USB_SPEED_SUPER,                    /* usb 3.0 */
};

/**
 * struct usb_string - wraps a C string and its USB id
 * @id:the (nonzero) ID for this string
 * @s:the string, in UTF-8 encoding
 *
 * If you're using usb_gadget_get_string(), use this to wrap a string
 * together with its ID.
 */
struct usb_string
{
    unsigned char id;
    const char *s;
};

#endif /* __LINUX_USB_CH9_H */



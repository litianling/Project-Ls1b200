// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */

#include "usb_cfg.h"

#if BSP_USE_KBD

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "bsp.h"
#include "../libc/lwmem.h"

#include "usb.h"
#include "memalign.h"
//#include "linux_kernel.h"
#include "linux.h"
#define debug   printk
#define LOCK    0

#ifndef USB_KBD_BOOT_REPORT_SIZE
#define USB_KBD_BOOT_REPORT_SIZE 0x20
#endif

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int overwrite_console(void);
#else
int overwrite_console(void)
{
    return 0;
}
#endif

/* ���̲����� */
#define REPEAT_RATE     40      /* 40msec -> 25cps */

#if (LOCK)
#define REPEAT_DELAY    1
#else
#define REPEAT_DELAY    10      /* 10 x REPEAT_RATE = 400msec */
#endif

#define NUM_LOCK        0x53
#define CAPS_LOCK       0x39
#define SCROLL_LOCK     0x47

/* Modifier bits */
#define LEFT_CNTR       (1 << 0)
#define LEFT_SHIFT      (1 << 1)
#define LEFT_ALT        (1 << 2)
#define LEFT_GUI        (1 << 3)
#define RIGHT_CNTR      (1 << 4)
#define RIGHT_SHIFT     (1 << 5)
#define RIGHT_ALT       (1 << 6)
#define RIGHT_GUI       (1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN  0x20
/* Device name */
//#define DEVNAME         "usbkbd"

/* Keyboard maps */
static const unsigned char usb_kbd_numkey[] =
{
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
    '\\', '#', ';', '\'', '`', ',', '.', '/'
};

static const unsigned char usb_kbd_numkey_shifted[] =
{
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
    '|', '~', ':', '"', '~', '<', '>', '?'
};

static const unsigned char usb_kbd_num_keypad[] =
{
    '/', '*', '-', '+', '\r',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '.', 0, 0, 0, '='
};

static const unsigned char usb_special_keys[] =
{
#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
    '2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#else
    'C', 'D', 'B', 'A'
#endif
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#define USB_KBD_NUMLOCK     (1 << 0)
#define USB_KBD_CAPSLOCK    (1 << 1)
#define USB_KBD_SCROLLLOCK  (1 << 2)
#define USB_KBD_CTRL        (1 << 3)

#define USB_KBD_LEDMASK     (USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

struct usb_kbd_pdata
{
    unsigned long     intpipe;
    int               intpktsize;
    int               intinterval;
    unsigned long     last_report;
    struct int_queue *intq;

    unsigned int      repeat_delay;

    unsigned int      usb_in_pointer;
    unsigned int      usb_out_pointer;
    unsigned char     usb_kbd_buffer[USB_KBD_BUFFER_LEN];

    unsigned char    *new_;
    unsigned char     old[USB_KBD_BOOT_REPORT_SIZE];

    unsigned char     flags;
};

extern int net_busy_flag;
int lock = 0;
/* The period of time between two calls of usb_kbd_testc(). */
static unsigned long kbd_testc_tms;

//-------------------------------------------------------------------------------------------------
// KEYBOARD DEVICE
//-------------------------------------------------------------------------------------------------

static struct usb_device *m_usb_kbd_dev = NULL;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/* ���ַ�������в�������������ָ��. */
static void usb_kbd_put_queue(struct usb_kbd_pdata *data, unsigned char c)
{
    if (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1)
    {
        /* ��黺��������. */
        if (data->usb_out_pointer == 0)
            return;
        data->usb_in_pointer = 0;
    }
    else
    {
        /* ��黺��������. */
        if (data->usb_in_pointer == data->usb_out_pointer - 1)
            return;
        data->usb_in_pointer++;
    }

    data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * ���� LED�� ���������� irq ������ʹ�õģ���˿�����ҵ�ĳ�ʱʱ��Ϊ 0������ζ�ţ�����ҵ���Ŷӵȴ���ҵ���.
 */
static void usb_kbd_setled(struct usb_device *dev)
{
    struct usb_interface *iface = &dev->config.if_desc[0];
    struct usb_kbd_pdata *data = dev->privptr;
    ALLOC_ALIGN_BUFFER(unsigned int, leds, 1, USB_DMA_MINALIGN);

    *leds = data->flags & USB_KBD_LEDMASK;

    usb_control_msg(dev,        // ��*��
                    usb_sndctrlpipe(dev, 0),
                    USB_REQ_SET_REPORT,
                    USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    0x200,
                    iface->desc.bInterfaceNumber,
                    leds,
                    1,
                    0);
}

#define CAPITAL_MASK    0x20

/* ��ɨ����ת��Ϊ ASCII��������У� */
static int usb_kbd_translate(struct usb_kbd_pdata *data,
                             unsigned char scancode,
                             unsigned char modifier,
                             int pressed)
{
    unsigned char keycode = 0;

    /* �����ͷ� */
    if (pressed == 0)
    {
        data->repeat_delay = 0;
        return 0;
    }

    if (pressed == 2)
    {
        data->repeat_delay++;
        if (data->repeat_delay < REPEAT_DELAY)
            return 0;
        data->repeat_delay = REPEAT_DELAY;
    }

    /* ɨ����Ϊ��ĸ */
    if ((scancode > 3) && (scancode <= 0x1d))
    {
        keycode = scancode - 4 + 'a';

        if (data->flags & USB_KBD_CAPSLOCK)
            keycode &= ~CAPITAL_MASK;

        if (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
        {
            /* ͬʱ���� CAPSLock + Shift */
            if (keycode & CAPITAL_MASK)
                keycode &= ~CAPITAL_MASK;
            else
                keycode |= CAPITAL_MASK;
        }
    }

    /* ɨ����Ϊ�������ϲ����ּ��� */
    if ((scancode > 0x1d) && (scancode < 0x39))
    {
        /* Shift ���� */
        if (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
            keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
        else
            keycode = usb_kbd_numkey[scancode - 0x1e];
    }

    /* ���ּ��� */
    if ((scancode >= 0x54) && (scancode <= 0x67))
        keycode = usb_kbd_num_keypad[scancode - 0x54];
    if (data->flags & USB_KBD_CTRL)
        keycode = scancode - 0x3;

    if (pressed == 1)
    {
        if (scancode == NUM_LOCK)
        {
            data->flags ^= USB_KBD_NUMLOCK;
            return 1;
        }
        if (scancode == CAPS_LOCK)
        {
            data->flags ^= USB_KBD_CAPSLOCK;
            return 1;
        }
        if (scancode == SCROLL_LOCK)
        {
            data->flags ^= USB_KBD_SCROLLLOCK;
            return 1;
        }
    }

    /* ������루����У� */
    if (keycode)
    {
/*
        if(keycode==13)
            debug("\n");                                            // �����س�������
        else if(keycode==8)
            debug("%c %c",keycode,keycode);                         // �����˸�ɾ��ԭ����ʾ
        else
            debug("%c", keycode);                                   // ��������
*/
        usb_kbd_put_queue(data, keycode);                           // ���ַ�������в�������������ָ�롾*��
        return 0;
    }

#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
    if (scancode < 0x3a || scancode > 0x52 ||
        scancode == 0x46 || scancode == 0x47)
        return 1;

    usb_kbd_put_queue(data, 0x1b);
    if (scancode < 0x3e)
    {
        /* F1 - F4 */
        usb_kbd_put_queue(data, 0x4f);
        usb_kbd_put_queue(data, scancode - 0x3a + 'P');
        return 0;
    }

    usb_kbd_put_queue(data, '[');

    if (scancode < 0x42)
    {
        /* F5 - F8 */
        usb_kbd_put_queue(data, '1');
        if (scancode == 0x3e)
            --scancode;
        keycode = scancode - 0x3f + '7';
    }
    else if (scancode < 0x49)
    {
        /* F9 - F12 */
        usb_kbd_put_queue(data, '2');
        if (scancode > 0x43)
            ++scancode;
        keycode = scancode - 0x42 + '0';
    }
    else
    {
        /*
         * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
         * RIGHT, LEFT, DOWN, UP
         */
        keycode = usb_special_keys[scancode - 0x49];
    }

    usb_kbd_put_queue(data, keycode);
    if (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
        usb_kbd_put_queue(data, '~');

    return 0;

#else
    /* ���ҡ��ϡ��� */
    if (scancode > 0x4e && scancode < 0x53)
    {
        usb_kbd_put_queue(data, 0x1b);                                  // ͬ�ϡ�*��
        usb_kbd_put_queue(data, '[');                                   // ͬ�ϡ�*��
        usb_kbd_put_queue(data, usb_special_keys[scancode - 0x4f]);     // ͬ�ϡ�*�������ַ����顾*��
        return 0;
    }

    return 1;

#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

/*
 * ����: addr - �ڴ�����
 *       c    - Ҫ���ҵ��ֽ�
 *       size - ����Ĵ�С
 *
 * ɨ���ַ�����ҵ�, �����ַ���һ�γ��ֵĵ�ַ, ���򷵻�1
 */
void *memscan(void *addr, int c, size_t size)
{
    int i;
    unsigned char *ptr = (unsigned char *)addr;
    
    for (i=0; i<size; i++)
    {
        if ((int)ptr[i] == c)
        {
            return (void *)(ptr + i);
        }
    }
            
    return NULL;
}

// USB���̰�������
static unsigned int usb_kbd_service_key(struct usb_device *dev,
                                        int i,
                                        int pressed)
{
    unsigned int res = 0;
    struct usb_kbd_pdata *data = dev->privptr;
    unsigned char *new_;
    unsigned char *old;

    if (pressed)
    {
        new_ = data->old;
        old  = data->new_;
    }
    else
    {
        new_ = data->new_;
        old  = data->old;
    }


    // ɨ���ַ���*������ҵ�, �����ַ���һ�γ��ֵĵ�ַ, ���򷵻�NULL
    //if ((old[i] > 3) &&(memscan(new_ + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==new_ + USB_KBD_BOOT_REPORT_SIZE))
    if ((old[i] > 3) &&(memscan(new_ + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) == NULL))
    {
        res |= usb_kbd_translate(data, old[i], data->new_[0], pressed);          // ��ɨ����ת��Ϊ ASCII��*����������У�
    }
    else if((old[i] > 3)&&(old[i] != new_[i]))
    {
        res |= usb_kbd_translate(data, old[i], data->new_[0], pressed);          // ��ɨ����ת��Ϊ ASCII��*����������У�
    }
    return res;
}

/* �жϷ������ */
static int usb_kbd_irq_worker(struct usb_device *dev)
{
    struct usb_kbd_pdata *data = dev->privptr;
    int i, res = 0;

    /* û�а�����ϼ� */
    if (data->new_[0] == 0x00)
        data->flags &= ~USB_KBD_CTRL;
    /* ��������� Ctrl */
    else if ((data->new_[0] == LEFT_CNTR) || (data->new_[0] == RIGHT_CNTR))
        data->flags |= USB_KBD_CTRL;

    /* ���ΰ��¼�� */
    //for (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++)                              // ����0x20Ϊ32
    for (i = 2; i < 8; i++)
    {
        res |= usb_kbd_service_key(dev, i, 0);                                  // USB���̰�������*����������ѯ��ASCII����ת����������У�
        res |= usb_kbd_service_key(dev, i, 1);                                  // ͬ�ϡ�*��
    }

    /* ���Ա��������� */
    if ((data->new_[2] > 3) && (data->old[2] == data->new_[2]) && !lock)
        res |= usb_kbd_translate(data, data->new_[2], data->new_[0], 2);        // ��ɨ����ת��Ϊ ASCII��*����������У�

#if (LOCK)
    if((data->new_[2]==data->old[2])&&(data->new_[2]!=0))
        lock = 1;
    else
        lock = 0;
#endif

    if (res == 1)
        usb_kbd_setled(dev);                                                    // �ж���ɡ�*��

    memcpy(data->old, data->new_, USB_KBD_BOOT_REPORT_SIZE);                    // ���ݿ�����*����new_�г���Ϊ0x20�����ݿ�����old
    return 1;
}

/* �����жϴ������ */
static int usb_kbd_irq(struct usb_device *dev)
{
    if ((dev->irq_status != 0) ||
        (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE))
    {
        debug("USB KBD: Error %lX, len %d\n", dev->irq_status, dev->irq_act_len);
        return 1;
    }

    return usb_kbd_irq_worker(dev);
}

/* �ж���ѯ */
static inline void usb_kbd_poll_for_event(struct usb_device *dev)
{
#if defined(CONFIG_SYS_USB_EVENT_POLL)
    struct usb_kbd_pdata *data = dev->privptr;

    if (usb_int_msg(dev,                                        // �ύ�жϴ�������*��
                    data->intpipe,
                    &data->new_[0],
                    data->intpktsize,
                    data->intinterval,
                    true) >= 0)
        usb_kbd_irq_worker(dev);                                // �жϷ������*��

#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
  #if defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)

    struct usb_interface *iface;
    struct usb_kbd_pdata *data = dev->privptr;
    iface = &dev->config.if_desc[0];
    usb_get_report(dev,
                   iface->desc.bInterfaceNumber,
                   1,
                   0,
                   data->new,
                   USB_KBD_BOOT_REPORT_SIZE );
    if (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE))
    {
        usb_kbd_irq_worker(dev);

  #else

    struct usb_kbd_pdata *data = dev->privptr;
    if (poll_int_queue(dev, data->intq))
    {
        usb_kbd_irq_worker(dev);
        /* We've consumed all queued int packets, create new */
        destroy_int_queue(dev, data->intq);
        data->intq = create_int_queue(dev,
                                      data->intpipe,
                                      1,
                                      USB_KBD_BOOT_REPORT_SIZE,
                                      data->new,
                                      data->intinterval);
  #endif
        data->last_report = get_timer(0);
        /* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
    }
    else if (data->last_report != -1 && get_timer(data->last_report) > REPEAT_RATE)
    {
        usb_kbd_irq_worker(dev);
        data->last_report = get_timer(0);
    }

#endif
}

/* ����һ���ַ��Ƿ��ڶ����� */
//static
int usb_kbd_testc(/*struct stdio_dev *sdev*/ void)
{
//    struct stdio_dev *dev;
//    struct usb_device *usb_kbd_dev;
    struct usb_kbd_pdata *data;
#ifdef CONFIG_CMD_NET
    /*
     * If net_busy_flag is 1, NET transfer is running,
     * then we check key-pressed every second (first check may be
     * less than 1 second) to improve TFTP booting performance.
     */
    if (net_busy_flag && (get_timer(kbd_testc_tms) < CONFIG_SYS_HZ))
        return 0;
    kbd_testc_tms = get_timer(0);
#endif
//    dev = stdio_get_by_name(sdev->name);
//    usb_kbd_dev = (struct usb_device *)dev->priv;

    if (m_usb_kbd_dev == NULL)
        return 0;
    data = m_usb_kbd_dev->privptr;
    usb_kbd_poll_for_event(m_usb_kbd_dev);                              // �ж���ѯ��*��
    return !(data->usb_in_pointer == data->usb_out_pointer);
}

/* �Ӷ����л�ȡ�ַ� */
//static
int usb_kbd_getc(/*struct stdio_dev *sdev*/ void)
{
//    struct stdio_dev *dev;
//    struct usb_device *usb_kbd_dev;
    struct usb_kbd_pdata *data;

//    dev = stdio_get_by_name(sdev->name);
//    usb_kbd_dev = (struct usb_device *)dev->priv;

    if (m_usb_kbd_dev == NULL)
        return 0;

    data = m_usb_kbd_dev->privptr;

    while (data->usb_in_pointer == data->usb_out_pointer)
    {
        usb_kbd_poll_for_event(m_usb_kbd_dev);              // �ж���ѯ��*��
    }

    if (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
        data->usb_out_pointer = 0;
    else
        data->usb_out_pointer++;

    return data->usb_kbd_buffer[data->usb_out_pointer];     // ���ݷ���

}

/* ̽�� USB �豸�ļ�������. */
static int usb_kbd_probe_dev(struct usb_device *dev, unsigned int ifnum)
{
    struct usb_interface *iface;
    struct usb_endpoint_descriptor *ep;
    struct usb_kbd_pdata *data;

    if (dev->descriptor.bNumConfigurations != 1)
        return 0;
    iface = &dev->config.if_desc[ifnum];
    if (iface->desc.bInterfaceClass != USB_CLASS_HID)
        return 0;
    if (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
        return 0;
    if (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
        return 0;
    if (iface->desc.bNumEndpoints != 1)
        return 0;

    ep = &iface->ep_desc[0];

    /* ���˵�1�Ƿ�Ϊ�ж϶˵� */
    if (!(ep->bEndpointAddress & 0x80))
        return 0;
    if ((ep->bmAttributes & 3) != 3)
        return 0;
    debug("USB KBD: found set protocol...\n");
    data = MALLOC(sizeof(struct usb_kbd_pdata));                        // �洢���ռ���䡾*��
    if (!data)
    {
        printf("USB KBD: Error allocating private data\n");
        return 0;
    }

    /* ���˽������ */
    memset(data, 0, sizeof(struct usb_kbd_pdata));                      // �洢���ռ������*��

    /* �������뻺�������벢������С�Խ��� USB DMA ���� */
//  data->new_ = memalign(USB_DMA_MINALIGN, roundup(USB_KBD_BOOT_REPORT_SIZE, USB_DMA_MINALIGN));
    data->new_ = aligned_malloc(USB_DMA_MINALIGN, roundup(USB_KBD_BOOT_REPORT_SIZE, USB_DMA_MINALIGN));     // �ռ���䡾*����*��

    /* ��˽�����ݲ��� USB �豸�ṹ */
    dev->privptr = data;
    /* ���� IRQ ������� */
    dev->irq_handle = usb_kbd_irq;

    data->intpipe = usb_rcvintpipe(dev, ep->bEndpointAddress);                                      // �������ݵ��жϹܵ���*������
    data->intpktsize = min(usb_maxpacket(dev, data->intpipe), USB_KBD_BOOT_REPORT_SIZE);            // �жϴ������ݰ���С��*��
    data->intinterval = ep->bInterval;                                                              // �жϼ��
    data->last_report = -1;                                                                         // �ϴα���

    /* �����ҵ���һ�� USB ���̣���װ��. */
    usb_set_protocol(dev, iface->desc.bInterfaceNumber, 0);                                         // ����Э�顾*��������λ���������

    debug("USB KBD: found set idle...\n");
#if !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
    usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE / 4, 0);                            // ���ÿ��С�*��������λ���������
#else
    usb_set_idle(dev, iface->desc.bInterfaceNumber, 0, 0);
#endif

    debug("USB KBD: enable interrupt pipe...\n");

#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
    data->intq = create_int_queue(dev,
                                  data->intpipe,
                                  1,
                                  USB_KBD_BOOT_REPORT_SIZE,
                                  data->new_,
                                  data->intinterval);
    if (!data->intq)
    {
#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
    if (usb_get_report(dev,
                       iface->desc.bInterfaceNumber,
                       1,
                       0,
                       data->new_,
                       USB_KBD_BOOT_REPORT_SIZE) < 0)
    {
#else
    if (usb_int_msg(dev,                                                                            // ��ʼ��USB��Ϣ��*���������Ƿ����
                    data->intpipe,
                    data->new_,
                    data->intpktsize,
                    data->intinterval,
                    false) < 0)
    {
#endif
        printf("Failed to get keyboard state from device %04x:%04x\n",
                dev->descriptor.idVendor, dev->descriptor.idProduct);

        return 0;   /* ��ֹ�����ǲ���ʹ���Ǹ��������õļ���. */
    }

    return 1;       /* �ɹ�. */
}


#if (!DM_USB)

/* �������̲����ҵ�ʱע��. */
int drv_usb_kbd_init(void)
{
    int error, i;

    debug("%s: Probing for keyboard\n", __func__);

    /* ɨ������ USB �豸 */
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        struct usb_device *dev;
        dev = usb_get_dev_index(i);                     // ��ȡ USB �豸��*�������豸���鷵���豸ָ�룬û���豸���ؿ�
        if (!dev)
            break;
        if (dev->devnum == -1)
            continue;
        if (usb_kbd_probe_dev(dev, 0) == 1)             // ����̽����̡�*���ж϶˵㡢�ܵ�����ʼ��
        {
            m_usb_kbd_dev = dev;
            return 1;
        }
        if (error && error != -ENOENT)
            return error;
    }

    /* δ�ҵ� USB ���� */
    return -1;
}


#if (DM_USB)

static int usb_kbd_probe(struct udevice *dev)
{
    struct usb_device *udev = dev_get_parent_priv(dev);

    return probe_usb_keyboard(udev);
}

static int usb_kbd_remove(struct udevice *dev)
{
    struct usb_device *udev = dev_get_parent_priv(dev);
    struct usb_kbd_pdata *data;
    struct stdio_dev *sdev;
    int ret;

    sdev = stdio_get_by_name(DEVNAME);
    if (!sdev)
    {
        ret = -ENXIO;
        goto err;
    }

    data = udev->privptr;
    if (stdio_deregister_dev(sdev, true))
    {
        ret = -EPERM;
        goto err;
    }

#if (CONSOLE_MUX)
    if (iomux_doenv(stdin, env_get("stdin")))
    {
        ret = -ENOLINK;
        goto err;
    }
#endif

#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
    destroy_int_queue(udev, data->intq);
#endif

    FREE(data->new);
    FREE(data);

    return 0;

err:
    printf("%s: warning, ret=%d", __func__, ret);
    return ret;
}

static const struct udevice_id usb_kbd_ids[] =
{
    { .compatible = "usb-keyboard" },
    { }
};

U_BOOT_DRIVER(usb_kbd) =
{
    .name   = "usb_kbd",
    .id = UCLASS_KEYBOARD,
    .of_match = usb_kbd_ids,
    .probe = usb_kbd_probe,
    .remove = usb_kbd_remove,
};

static const struct usb_device_id kbd_id_table[] =
{
    {
        .match_flags = USB_DEVICE_ID_MATCH_INT_CLASS |
                       USB_DEVICE_ID_MATCH_INT_SUBCLASS |
                       USB_DEVICE_ID_MATCH_INT_PROTOCOL,
        .bInterfaceClass = USB_CLASS_HID,
        .bInterfaceSubClass = USB_SUB_HID_BOOT,
        .bInterfaceProtocol = USB_PROT_HID_KEYBOARD,
    },
    { }     /* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_kbd, kbd_id_table);

#endif // #if 0

#endif

#endif



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

/* 键盘采样率 */
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

/* 将字符放入队列并设置输入和输出指针. */
static void usb_kbd_put_queue(struct usb_kbd_pdata *data, unsigned char c)
{
    if (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1)
    {
        /* 检查缓冲区已满. */
        if (data->usb_out_pointer == 0)
            return;
        data->usb_in_pointer = 0;
    }
    else
    {
        /* 检查缓冲区已满. */
        if (data->usb_in_pointer == data->usb_out_pointer - 1)
            return;
        data->usb_in_pointer++;
    }

    data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * 设置 LED。 由于这是在 irq 例程中使用的，因此控制作业的超时时间为 0。这意味着，该作业已排队等待作业完成.
 */
static void usb_kbd_setled(struct usb_device *dev)
{
    struct usb_interface *iface = &dev->config.if_desc[0];
    struct usb_kbd_pdata *data = dev->privptr;
    ALLOC_ALIGN_BUFFER(unsigned int, leds, 1, USB_DMA_MINALIGN);

    *leds = data->flags & USB_KBD_LEDMASK;

    usb_control_msg(dev,        // 【*】
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

/* 将扫描码转换为 ASCII（放入队列） */
static int usb_kbd_translate(struct usb_kbd_pdata *data,
                             unsigned char scancode,
                             unsigned char modifier,
                             int pressed)
{
    unsigned char keycode = 0;

    /* 按键释放 */
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

    /* 扫描码为字母 */
    if ((scancode > 3) && (scancode <= 0x1d))
    {
        keycode = scancode - 4 + 'a';

        if (data->flags & USB_KBD_CAPSLOCK)
            keycode &= ~CAPITAL_MASK;

        if (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
        {
            /* 同时按下 CAPSLock + Shift */
            if (keycode & CAPITAL_MASK)
                keycode &= ~CAPITAL_MASK;
            else
                keycode |= CAPITAL_MASK;
        }
    }

    /* 扫描码为主键盘上侧数字键盘 */
    if ((scancode > 0x1d) && (scancode < 0x39))
    {
        /* Shift 按下 */
        if (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
            keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
        else
            keycode = usb_kbd_numkey[scancode - 0x1e];
    }

    /* 数字键盘 */
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

    /* 报告键码（如果有） */
    if (keycode)
    {
/*
        if(keycode==13)
            debug("\n");                                            // 修正回车不换行
        else if(keycode==8)
            debug("%c %c",keycode,keycode);                         // 修正退格不删除原先显示
        else
            debug("%c", keycode);                                   // 正常回显
*/
        usb_kbd_put_queue(data, keycode);                           // 将字符放入队列并设置输入和输出指针【*】
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
    /* 左、右、上、下 */
    if (scancode > 0x4e && scancode < 0x53)
    {
        usb_kbd_put_queue(data, 0x1b);                                  // 同上【*】
        usb_kbd_put_queue(data, '[');                                   // 同上【*】
        usb_kbd_put_queue(data, usb_special_keys[scancode - 0x4f]);     // 同上【*】特殊字符数组【*】
        return 0;
    }

    return 1;

#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

/*
 * 参数: addr - 内存区域
 *       c    - 要查找的字节
 *       size - 区域的大小
 *
 * 扫描字符如果找到, 返回字符第一次出现的地址, 否则返回1
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

// USB键盘按键服务
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


    // 扫描字符【*】如果找到, 返回字符第一次出现的地址, 否则返回NULL
    //if ((old[i] > 3) &&(memscan(new_ + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==new_ + USB_KBD_BOOT_REPORT_SIZE))
    if ((old[i] > 3) &&(memscan(new_ + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) == NULL))
    {
        res |= usb_kbd_translate(data, old[i], data->new_[0], pressed);          // 将扫描码转换为 ASCII【*】（放入队列）
    }
    else if((old[i] > 3)&&(old[i] != new_[i]))
    {
        res |= usb_kbd_translate(data, old[i], data->new_[0], pressed);          // 将扫描码转换为 ASCII【*】（放入队列）
    }
    return res;
}

/* 中断服务程序 */
static int usb_kbd_irq_worker(struct usb_device *dev)
{
    struct usb_kbd_pdata *data = dev->privptr;
    int i, res = 0;

    /* 没有按下组合键 */
    if (data->new_[0] == 0x00)
        data->flags &= ~USB_KBD_CTRL;
    /* 按下左或右 Ctrl */
    else if ((data->new_[0] == LEFT_CNTR) || (data->new_[0] == RIGHT_CNTR))
        data->flags |= USB_KBD_CTRL;

    /* 单次按下检测 */
    //for (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++)                              // 长度0x20为32
    for (i = 2; i < 8; i++)
    {
        res |= usb_kbd_service_key(dev, i, 0);                                  // USB键盘按键服务【*】（按键查询、ASCII解码转换、放入队列）
        res |= usb_kbd_service_key(dev, i, 1);                                  // 同上【*】
    }

    /* 键仍被持续按下 */
    if ((data->new_[2] > 3) && (data->old[2] == data->new_[2]) && !lock)
        res |= usb_kbd_translate(data, data->new_[2], data->new_[0], 2);        // 将扫描码转换为 ASCII【*】（放入队列）

#if (LOCK)
    if((data->new_[2]==data->old[2])&&(data->new_[2]!=0))
        lock = 1;
    else
        lock = 0;
#endif

    if (res == 1)
        usb_kbd_setled(dev);                                                    // 中断完成【*】

    memcpy(data->old, data->new_, USB_KBD_BOOT_REPORT_SIZE);                    // 数据拷贝【*】将new_中长度为0x20的数据拷贝到old
    return 1;
}

/* 键盘中断处理程序 */
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

/* 中断轮询 */
static inline void usb_kbd_poll_for_event(struct usb_device *dev)
{
#if defined(CONFIG_SYS_USB_EVENT_POLL)
    struct usb_kbd_pdata *data = dev->privptr;

    if (usb_int_msg(dev,                                        // 提交中断传输请求【*】
                    data->intpipe,
                    &data->new_[0],
                    data->intpktsize,
                    data->intinterval,
                    true) >= 0)
        usb_kbd_irq_worker(dev);                                // 中断服务程序【*】

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

/* 测试一个字符是否在队列中 */
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
    usb_kbd_poll_for_event(m_usb_kbd_dev);                              // 中断轮询【*】
    return !(data->usb_in_pointer == data->usb_out_pointer);
}

/* 从队列中获取字符 */
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
        usb_kbd_poll_for_event(m_usb_kbd_dev);              // 中断轮询【*】
    }

    if (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
        data->usb_out_pointer = 0;
    else
        data->usb_out_pointer++;

    return data->usb_kbd_buffer[data->usb_out_pointer];     // 数据返回

}

/* 探测 USB 设备的键盘类型. */
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

    /* 检查端点1是否为中断端点 */
    if (!(ep->bEndpointAddress & 0x80))
        return 0;
    if ((ep->bmAttributes & 3) != 3)
        return 0;
    debug("USB KBD: found set protocol...\n");
    data = MALLOC(sizeof(struct usb_kbd_pdata));                        // 存储器空间分配【*】
    if (!data)
    {
        printf("USB KBD: Error allocating private data\n");
        return 0;
    }

    /* 清除私人数据 */
    memset(data, 0, sizeof(struct usb_kbd_pdata));                      // 存储器空间清除【*】

    /* 分配输入缓冲区对齐并调整大小以进行 USB DMA 对齐 */
//  data->new_ = memalign(USB_DMA_MINALIGN, roundup(USB_KBD_BOOT_REPORT_SIZE, USB_DMA_MINALIGN));
    data->new_ = aligned_malloc(USB_DMA_MINALIGN, roundup(USB_KBD_BOOT_REPORT_SIZE, USB_DMA_MINALIGN));     // 空间分配【*】【*】

    /* 将私有数据插入 USB 设备结构 */
    dev->privptr = data;
    /* 设置 IRQ 处理程序 */
    dev->irq_handle = usb_kbd_irq;

    data->intpipe = usb_rcvintpipe(dev, ep->bEndpointAddress);                                      // 接收数据的中断管道【*】计算
    data->intpktsize = min(usb_maxpacket(dev, data->intpipe), USB_KBD_BOOT_REPORT_SIZE);            // 中断传输数据包大小【*】
    data->intinterval = ep->bInterval;                                                              // 中断间隔
    data->last_report = -1;                                                                         // 上次报告

    /* 我们找到了一个 USB 键盘，安装它. */
    usb_set_protocol(dev, iface->desc.bInterfaceNumber, 0);                                         // 设置协议【*】（向下位机发送命令）

    debug("USB KBD: found set idle...\n");
#if !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
    usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE / 4, 0);                            // 设置空闲【*】（向下位机发送命令）
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
    if (usb_int_msg(dev,                                                                            // 初始化USB信息【*】检测键盘是否可用
                    data->intpipe,
                    data->new_,
                    data->intpktsize,
                    data->intinterval,
                    false) < 0)
    {
#endif
        printf("Failed to get keyboard state from device %04x:%04x\n",
                dev->descriptor.idVendor, dev->descriptor.idProduct);

        return 0;   /* 中止，我们不想使用那个不起作用的键盘. */
    }

    return 1;       /* 成功. */
}


#if (!DM_USB)

/* 搜索键盘并在找到时注册. */
int drv_usb_kbd_init(void)
{
    int error, i;

    debug("%s: Probing for keyboard\n", __func__);

    /* 扫描所有 USB 设备 */
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        struct usb_device *dev;
        dev = usb_get_dev_index(i);                     // 获取 USB 设备【*】遍历设备数组返回设备指针，没有设备返回空
        if (!dev)
            break;
        if (dev->devnum == -1)
            continue;
        if (usb_kbd_probe_dev(dev, 0) == 1)             // 尝试探测键盘【*】中断端点、管道、初始化
        {
            m_usb_kbd_dev = dev;
            return 1;
        }
        if (error && error != -ENOENT)
            return error;
    }

    /* 未找到 USB 键盘 */
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



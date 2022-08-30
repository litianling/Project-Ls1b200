/*
 * 2022
 * YuChen
 * 2018040094
 * Loongson1B MOUSE_DRIVER ,this driver is only for HID sub-class(boot) mouse.
 * project.
 */

#include "usb_cfg.h"

#if BSP_USE_MOUSE

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
#define debug_keypress

#ifndef USB_MOUSE_BOOT_REPORT_SIZE
#define USB_MOUSE_BOOT_REPORT_SIZE 0x20
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

/* mouse sampling rate */
#define REPEAT_RATE     40      /* 40msec -> 25cps */
#define REPEAT_DELAY    10      /* 10 x REPEAT_RATE = 400msec */



/* Size of the mouse buffer */
#define USB_MOUSE_BUFFER_LEN  0x20


struct usb_mouse_pdata          //鼠标管道数据
{
    unsigned long     intpipe;
    int               intpktsize;
    int               intinterval;
    unsigned long     last_report;
    struct int_queue *intq;

    unsigned int      repeat_delay;

    unsigned int      usb_in_pointer;
    unsigned int      usb_out_pointer;
    unsigned char     usb_mouse_buffer[USB_MOUSE_BUFFER_LEN];

    unsigned char    *new_;    //鼠标报表数据获取
    unsigned char     old[USB_MOUSE_BOOT_REPORT_SIZE];

    unsigned char     flags;
};

extern int net_busy_flag;

/* The period of time between two calls of usb_mouse_testc(). */
static unsigned long mouse_testc_tms;

//-------------------------------------------------------------------------------------------------
// mouse DEVICE
//-------------------------------------------------------------------------------------------------

static struct usb_device *m_usb_mouse_dev = NULL;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
static void usb_mouse_put_queue(struct usb_mouse_pdata *data, unsigned char c)
{
    if (data->usb_in_pointer == USB_MOUSE_BUFFER_LEN - 1)
    {
        /* Check for buffer full. */
        if (data->usb_out_pointer == 0)
            return;
        data->usb_in_pointer = 0;
    }
    else
    {
        /* Check for buffer full. */
        if (data->usb_in_pointer == data->usb_out_pointer - 1)
            return;
        data->usb_in_pointer++;
    }

    data->usb_mouse_buffer[data->usb_in_pointer] = c;    //注意该buffer只能1个字节1个字节地存，并行若不进行先后判定，鼠标输入地时序会容易判定出错
}

/*
 * 参数: addr - 内存区域
 *       c    - 要查找的字节
 *       size - 区域的大小
 *
 * 如果找到, 返回字符第一次出现的地址, 否则返回1
 */
void *memscan(void *addr, int c, int size)
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
static int usb_mouse_translate(struct usb_mouse_pdata *data,
                             unsigned char scancode,
                             int pressed)
{
      unsigned char keycode = 0;

    /* Key released 按键释放 */
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

        data->repeat_delay = REPEAT_DELAY;    //关键，不会return
    }
    
 
    if(scancode == 0X01)
    {
        keycode = 1;
        debug("left click");
        usb_mouse_put_queue(data, keycode);
    }
    
    if(scancode == 0X02)
    {
        keycode = 2;
        debug("right click");
        usb_mouse_put_queue(data, keycode);
    }
    
    if(scancode == 0X04)
    {
        keycode = 4;
        debug("middle click");
        usb_mouse_put_queue(data, keycode);
    }

    return 1;
 

}
static unsigned int usb_mouse_service_key(struct usb_device *dev,
                                        int i,
                                        int up)
{//此函数就是将计数标志进行清零，并执行点按翻译
    unsigned int res = 0;
    struct usb_mouse_pdata *data = dev->privptr;
    unsigned char *new_;
    unsigned char *old;

    if (up)
    {
        new_ = data->old;
        old  = data->new_;
    }
    else
    {
        new_ = data->new_;
        old  = data->old;
    }
    
    if((old[0] != 0) && memscan(new_ , old[0], 1) == NULL)     //&&&&若测试结果出不来，尝试不调用该函数，直接通过判定不相等执行
    {
         usb_mouse_translate(data, old[0], up);
    }

    return res;
}

/* Interrupt service routine */
static int usb_mouse_irq_worker(struct usb_device *dev)
{
    struct usb_mouse_pdata *data = dev->privptr;
    int i, res = 0;
    
#ifdef debug_1
    debug("report is:");
    for (i = 0;i < 3; i++)
    {
    debug("%d ", data->new_[i]);
    }
    debug("\n");
#endif

#ifdef debug_keypress

        usb_mouse_service_key(dev,0,0);                 //up=0,若有按键变化则清零延时 长按计数标志位置
        usb_mouse_service_key(dev,0,1);                 //up=1,若现态不等于次态 执行译码操作

         /* Key is still pressed */
    if ((data->new_[0] != 0) && (data->old[0] == data->new_[0]))    //普通按键的报表值从3起始  次态=现态 表示按键一直按下 old为次态 new为现态
        usb_mouse_translate(data, data->new_[0], 2);

#endif














#ifdef debug_on
    if(data->new_[0] == 0X01)
    {
        debug("left click\n");
    }

    if(data->new_[0] == 0X02)
    {
        debug("right click\n");
    }

    if(data->new_[0] == 0X04)
    {
        debug("middle click\n");
    }
    
       if(data->new_[0] == 0X03)
    {
        debug("left click & right click \n");
    }
    
    if(data->new_[0] == 0X05)
    {
        debug("middle click & left click \n");
    }
    
    if(data->new_[0] == 0X06)
    {
        debug("middle click & right click \n");
    }
    
    if(data->new_[0] == 0X07)
    {
        debug("middle click & right click & left click \n");
    }
#endif
    
    if(data->new_[1] != 0)         //X坐标下的移动
    {
        if(data->new_[1] >= 125)
        {
            debug("move_left\n");
        }
        else
        {
            debug("move_right\n");
        }
    }
    
    if(data->new_[2] != 0)        //Y坐标下的移动
    {

        if(data->new_[2] >= 125)
        {
            debug("move_up\n");
        }
        else
        {
            debug("move_down\n");
        }
    }




    memcpy(data->old, data->new_, USB_MOUSE_BOOT_REPORT_SIZE);    //解码，拷贝到缓冲存储区后才给次态赋值
    return 1;
}

/* mouse interrupt handler */
static int usb_mouse_irq(struct usb_device *dev)
{
    if ((dev->irq_status != 0) ||
        (dev->irq_act_len != USB_MOUSE_BOOT_REPORT_SIZE))
    {
        debug("USB MOUSE: Error %lX, len %d\n", dev->irq_status, dev->irq_act_len);
        return 1;
    }

    return usb_mouse_irq_worker(dev);
}

/* Interrupt polling */
static inline void usb_mouse_poll_for_event(struct usb_device *dev)
{
#if defined(CONFIG_SYS_USB_EVENT_POLL)
    struct usb_mouse_pdata *data = dev->privptr;    //这句就相当于对data置零的操作没做

    /* Submit an interrupt transfer request */
    if (usb_int_msg(dev,
                    data->intpipe,
                    &data->new_[0],
                    data->intpktsize,
                    data->intinterval,
                    true) >= 0)
        usb_mouse_irq_worker(dev);


#endif
}

/* test if a character is in the queue */
//static
int usb_mouse_testc(/*struct stdio_dev *sdev*/ void)
{
//    struct stdio_dev *dev;
//    struct usb_device *usb_mouse_dev;
    struct usb_mouse_pdata *data;



//    dev = stdio_get_by_name(sdev->name);
//    usb_mouse_dev = (struct usb_device *)dev->priv;

    if (m_usb_mouse_dev == NULL)
        return 0;

    data = m_usb_mouse_dev->privptr;
//    debug("poll_for_event");                                     //插入一个debug看看是否会进入轮询
    usb_mouse_poll_for_event(m_usb_mouse_dev);

    return !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
//static
 int usb_mouse_getc(/*struct stdio_dev *sdev*/ void)
{
//    struct stdio_dev *dev;
//    struct usb_device *usb_mouse_dev;
    struct usb_mouse_pdata *data;

//    dev = stdio_get_by_name(sdev->name);
//    usb_mouse_dev = (struct usb_device *)dev->priv;

    if (m_usb_mouse_dev == NULL)
        return 0;

    data = m_usb_mouse_dev->privptr;

    while (data->usb_in_pointer == data->usb_out_pointer)  //判定缓存区域是否已满
    {
//        WATCHDOG_RESET();
        usb_mouse_poll_for_event(m_usb_mouse_dev);
    }

    if (data->usb_out_pointer == USB_MOUSE_BUFFER_LEN - 1)
        data->usb_out_pointer = 0;
    else
        data->usb_out_pointer++;

    return data->usb_mouse_buffer[data->usb_out_pointer];

}

/* probes the USB device dev for mouse type. */
static int usb_mouse_probe_dev(struct usb_device *dev, unsigned int ifnum)
{
    struct usb_interface *iface;
    struct usb_endpoint_descriptor *ep;
    struct usb_mouse_pdata *data;

    if (dev->descriptor.bNumConfigurations != 1)
        return 0;

    iface = &dev->config.if_desc[ifnum];

    if (iface->desc.bInterfaceClass != USB_CLASS_HID)
        return 0;

    if (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
        return 0;

    if (iface->desc.bInterfaceProtocol != USB_PROT_HID_MOUSE)
        return 0;

    if (iface->desc.bNumEndpoints != 1)
        return 0;

    ep = &iface->ep_desc[0];

    /* Check if endpoint 1 is interrupt endpoint */
    if (!(ep->bEndpointAddress & 0x80))
        return 0;

    if ((ep->bmAttributes & 3) != 3)
        return 0;

    debug("USB MOUSE: found set protocol...\n");

    data = MALLOC(sizeof(struct usb_mouse_pdata));
    if (!data)
    {
        printf("USB MOUSE: Error allocating private data\n");
        return 0;
    }

    /* Clear private data */
    memset(data, 0, sizeof(struct usb_mouse_pdata));

    /*分配输入缓冲区对齐并调整大小以进行 USB DMA 对齐*/
//  data->new_ = memalign(USB_DMA_MINALIGN, roundup(USB_MOUSE_BOOT_REPORT_SIZE, USB_DMA_MINALIGN));
    data->new_ = aligned_malloc(USB_DMA_MINALIGN, roundup(USB_MOUSE_BOOT_REPORT_SIZE, USB_DMA_MINALIGN));  //&&&&&重点data_new的获取方式（报表于此获取）

    /* Insert private data into USB device structure */
    dev->privptr = data;    /*&&&&&重点现在才将void privptr指向data*/

    /* Set IRQ handler */
    dev->irq_handle = usb_mouse_irq;    /*此为调用函数的一种方式：令dev中的irq_handle函数指针指向函数usb_mouse_irq*/

    data->intpipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
    data->intpktsize = min(usb_maxpacket(dev, data->intpipe), USB_MOUSE_BOOT_REPORT_SIZE);
    data->intinterval = ep->bInterval;
    data->last_report = -1;

    /* We found a USB mouse, install it. */
    usb_set_protocol(dev, iface->desc.bInterfaceNumber, 0);

    debug("USB MOUSE: found set idle...\n");
#if !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
    usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE / 4, 0);

#endif

    debug("USB MOUSE: enable interrupt pipe...\n");

    if (usb_int_msg(dev,
                    data->intpipe,
                    data->new_,
                    data->intpktsize,
                    data->intinterval,
                    false) < 0)
    {

        printf("Failed to get mouse state from device %04x:%04x\n",
                dev->descriptor.idVendor, dev->descriptor.idProduct);

        return 0;   /* Abort, we don't want to use that non-functional mouse. */
    }

    return 1;       /* Success. */
}



#if (!DM_USB)

/* Search for mouse and register it if found. */
int drv_usb_mouse_init(void)
{
    int error, i;

    debug("%s: Probing for mouse\n", __func__);

    /* Scan all USB Devices */
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        struct usb_device *dev;

        /* Get USB device. */
        dev = usb_get_dev_index(i);
        if (!dev)
            break;

        if (dev->devnum == -1)
            continue;

#if 0
        error = probe_usb_mouse(dev);
        if (!error)
            return 1;
#else

        /* Try probing the mouse */
        if (usb_mouse_probe_dev(dev, 0) == 1)
        {
            m_usb_mouse_dev = dev;
            return 1;
        }

#endif

        if (error && error != -ENOENT)
            return error;
    }

    /* No USB mouse found */
    return -1;
}





#endif

#endif



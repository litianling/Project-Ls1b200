/*
 * 2022
 * YuChen
 * 2018040094
 * Loongson1B MOUSE_DRIVER ,this driver is only for HID sub-class(boot) mouse.
 * project.
 */
#include "usb_mouse.h"
#include "ls1x_fb.h"

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
static void usb_mouse_put_queue(struct usb_mouse_pdata *data, unsigned char c)  // 暂时不改队列
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

void Mouse_logical_coordinates_reset(void)
{
    Col = 0;
    Row = 0;
    mouse_x = Col*(CONS_FONT_WIDTH+COL_GAP);
	mouse_y = Row*(CONS_FONT_HEIGHT+ROW_GAP);
	extern int get_fg_color(void);
	fb_drawrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color()+1);    // 显示白色方框（光标）
    return;
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

        data->repeat_delay = REPEAT_DELAY;    //关键——不会return
    }
    
    if(scancode == 0X01)
    {
        keycode = 1;                                                                        // 只点击了左键
        fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());   // 显示前景色块
        fb_drawrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());   // 刷新前景色方框（光标）
        usb_mouse_put_queue(data, keycode);
    }
    
    if(scancode == 0X02)
    {
        keycode = 2;                                                                         // 只点击了右键
        fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_bg_color());    // 显示背景色块
        fb_drawrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());    // 刷新前景色方框（光标）
        usb_mouse_put_queue(data, keycode);
    }
    
    if(scancode == 0X04)                                                                     // 只点击了鼠标滚轮
    {
        keycode = 4;
        usb_mouse_put_queue(data, keycode);
    }
    return 1;
}


static unsigned int usb_mouse_service_key(struct usb_device *dev,int i,int up)
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
    //int check;
    usb_mouse_service_key(dev,0,0);                 //up=0,若有按键变化则清零延时 长按计数标志位置
    usb_mouse_service_key(dev,0,1);                 //up=1,若现态不等于次态 执行译码操作

    /* Key is still pressed 取消长按点击 */
    /*
    if ((data->new_[0] != 0) && (data->old[0] == data->new_[0]))    //普通按键的报表值从3起始  次态=现态 表示按键一直按下 old为次态 new为现态
        usb_mouse_translate(data, data->new_[0], 2);
    */

    if(data->new_[1] != 0)         //X坐标下的移动——水平移动
    {
        fb_drawrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_bg_color());     // 刷新背景色方框（去掉光标）
        if(data->new_[1] >= 127)    // 左移
        {
            if(Col > 0)
            {
                Col--;
                mouse_x = Col*(CONS_FONT_WIDTH+COL_GAP);
            }
            if(data->new_[0]==1)                                                                                 // 按下左键左移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());    // 显示前景色块
            else if(data->new_[0]==2)                                                                            // 按下右键左移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_bg_color());    // 显示黑色块 背景
        }
        else                        // 右移
        {
            if(Col < Col_max)
            {
                Col++;
                mouse_x = Col*(CONS_FONT_WIDTH+COL_GAP);
            }
            if(data->new_[0]==1)                                                                                 // 按下左键右移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());    // 显示蓝色块 前景
            else if(data->new_[0]==2)                                                                            // 按下右键右移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_bg_color());    // 显示黑色块 背景
        }
        fb_drawrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());    // 刷新前景色方框（新的光标）
    }
    
    if(data->new_[2] != 0)        //Y坐标下的移动——垂直移动
    {
        fb_drawrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_bg_color());     // 刷新背景色方框（去掉光标）
        if(data->new_[2] >= 127)    // 上移
        {
            if(Row > 0)
            {
                Row--;
                mouse_y = Row*(CONS_FONT_HEIGHT+ROW_GAP);
            }
            if(data->new_[0]==1)                                                                                 // 按下左键上移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());    // 显示前景色块
            else if(data->new_[0]==2)                                                                            // 按下右键上移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_bg_color());    // 显示背景色块
        }
        else                        // 下移
        {
            if(Row < Row_max)
            {
                Row++;
                mouse_y = Row*(CONS_FONT_HEIGHT+ROW_GAP);
            }
            if(data->new_[0]==1)                                                                                 // 按下左键下移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());    // 显示前景色块
            else if(data->new_[0]==2)                                                                            // 按下右键下移
                fb_fillrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_bg_color());    // 显示背景色块
        }
        fb_drawrect(mouse_x,mouse_y,mouse_x+CONS_FONT_WIDTH,mouse_y+CONS_FONT_HEIGHT,get_fg_color());    // 刷新前景色方框（新的光标）
    }
    data->new_[1] = 0;
    data->new_[2] = 0;
    data->new_[3] = 0;
    memcpy(data->old, data->new_, USB_MOUSE_BOOT_REPORT_SIZE);    //解码，拷贝到缓冲存储区后才给状态赋值
    return 1;
}

/* mouse interrupt handler 入口 */
static int usb_mouse_irq(struct usb_device *dev)
{
    if ((dev->irq_status != 0) || (dev->irq_act_len != USB_MOUSE_BOOT_REPORT_SIZE))
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
int usb_mouse_testc(void)
{
    struct usb_mouse_pdata *data;

    if (m_usb_mouse_dev == NULL)                            // 检查设备是否存在
        return 0;

    data = m_usb_mouse_dev->privptr;
    usb_mouse_poll_for_event(m_usb_mouse_dev);              // 请求数据轮询

    return !(data->usb_in_pointer == data->usb_out_pointer);    // 返回队列是否存在数据
}


/* gets the character from the queue */
 int usb_mouse_getc(void)   // 不判断满
{

    struct usb_mouse_pdata *data;
    if (m_usb_mouse_dev == NULL)
        return 0;
    data = m_usb_mouse_dev->privptr;

    while (data->usb_in_pointer == data->usb_out_pointer)       // 判定缓存区域是否为空
    {
        usb_mouse_poll_for_event(m_usb_mouse_dev);              // 队列为空，继续请求
    }

    if (data->usb_out_pointer == USB_MOUSE_BUFFER_LEN - 1)      // out到头了，循环回初始位置
        data->usb_out_pointer = 0;
    else
        data->usb_out_pointer++;                                // 非空非到头，直接加一准备输出

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



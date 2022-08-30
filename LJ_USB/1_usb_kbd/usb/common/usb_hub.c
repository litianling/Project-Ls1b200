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

/****************************************************************************
 * HUB "Driver"
 * Probes device for being a hub and configurate it
 */

#include "usb_cfg.h"

#if (BSP_USE_OTG) || (BSP_USE_EHCI) || (BSP_USE_OHCI)

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

#include "bsp.h"
#include "../libc/lwmem.h"

#include "linux_kernel.h"
#include "linux_list.h"
#include "memalign.h"

#include "usb.h"

#define USB_BUFSIZ              512

#define HUB_SHORT_RESET_TIME    20
#define HUB_LONG_RESET_TIME     200

#define PORT_OVERCURRENT_MAX_SCAN_COUNT 3

#if 0
#define DEBUG(format, arg...) printf("DEBUG: " format "\n", ## arg)
#else
#define DEBUG(format, arg...) do {} while (0)
#endif /* DEBUG */

//-------------------------------------------------------------------------------------------------

struct usb_device_scan
{
    struct usb_device *dev;         /* USB hub device to scan */
    struct usb_hub_device *hub;     /* USB hub struct */
    int port;                       /* USB port to scan */
    struct list_head list;
};

static LIST_HEAD(usb_scan_list);

__attribute__((weak))
void usb_hub_reset_devices(struct usb_hub_device *hub, int port)
{
    return;
}

static inline bool usb_hub_is_superspeed(struct usb_device *hdev)
{
    return hdev->descriptor.bDeviceProtocol == 3;
}

#if (DM_USB)
bool usb_hub_is_root_hub(struct udevice *hub)
{
    if (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
        return true;

    return false;
}

static int usb_set_hub_depth(struct usb_device *dev, int depth)
{
    if (depth < 0 || depth > 4)
        return -EINVAL;

    return usb_control_msg(dev,
                           usb_sndctrlpipe(dev, 0),
                           USB_REQ_SET_HUB_DEPTH,
                           USB_DIR_OUT | USB_RT_HUB,
                           depth,
                           0,
                           NULL,
                           0,
                           USB_CNTL_TIMEOUT );
}
#endif

static int usb_get_hub_descriptor(struct usb_device *dev,
                                  void *data,
                                  int size)
{
    unsigned short dtype = USB_DT_HUB;

    if (usb_hub_is_superspeed(dev))
        dtype = USB_DT_SS_HUB;

    return usb_control_msg(dev,
                           usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_DESCRIPTOR,
                           USB_DIR_IN | USB_RT_HUB,
                           dtype << 8,
                           0,
                           data,
                           size,
                           USB_CNTL_TIMEOUT );
}

static int usb_clear_port_feature(struct usb_device *dev,
                                  int port,
                                  int feature)
{
    return usb_control_msg(dev,
                           usb_sndctrlpipe(dev, 0),
                           USB_REQ_CLEAR_FEATURE,
                           USB_RT_PORT,
                           feature,
                           port,
                           NULL,
                           0,
                           USB_CNTL_TIMEOUT );
}

static int usb_set_port_feature(struct usb_device *dev,
                                int port,
                                int feature)
{
    return usb_control_msg(dev,
                           usb_sndctrlpipe(dev, 0),
                           USB_REQ_SET_FEATURE,
                           USB_RT_PORT,
                           feature,
                           port,
                           NULL,
                           0,
                           USB_CNTL_TIMEOUT );
}

static int usb_get_hub_status(struct usb_device *dev,
                              void *data)
{
    return usb_control_msg(dev,
                           usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_STATUS,
                           USB_DIR_IN | USB_RT_HUB,
                           0,
                           0,
                           data,
                           sizeof(struct usb_hub_status),
                           USB_CNTL_TIMEOUT );
}

int usb_get_port_status(struct usb_device *dev, int port, void *data)
{
    int ret;

    ret = usb_control_msg(dev,
                          usb_rcvctrlpipe(dev, 0),
                          USB_REQ_GET_STATUS,
                          USB_DIR_IN | USB_RT_PORT,
                          0,
                          port,
                          data,
                          sizeof(struct usb_port_status),
                          USB_CNTL_TIMEOUT );

#if (DM_USB)
    if (ret < 0)
        return ret;

    /*
     * Translate the USB 3.0 hub port status field into the old version
     * that U-Boot understands. Do this only when the hub is not root hub.
     * For root hub, the port status field has already been translated
     * in the host controller driver (see xhci_submit_root() in xhci.c).
     *
     * Note: this only supports driver model.
     */

    if (!usb_hub_is_root_hub(dev->dev) && usb_hub_is_superspeed(dev))
    {
        struct usb_port_status *status = (struct usb_port_status *)data;
        u16 tmp = (status->wPortStatus) & USB_SS_PORT_STAT_MASK;

        if (status->wPortStatus & USB_SS_PORT_STAT_POWER)
            tmp |= USB_PORT_STAT_POWER;
        if ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
             USB_SS_PORT_STAT_SPEED_5GBPS)
            tmp |= USB_PORT_STAT_SUPER_SPEED;

        status->wPortStatus = tmp;
    }
#endif

    return ret;
}

static void usb_hub_power_on(struct usb_hub_device *hub)
{
    int i;
    struct usb_device *dev;
    unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
#if 0
    const char *env;
#endif

    dev = hub->pusb_dev;

    DEBUG("enabling power on all ports\n");
    for (i = 0; i < dev->maxchild; i++)
    {
        usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
        DEBUG("port %d returns %lX\n", i + 1, dev->status);
    }

#ifdef CONFIG_SANDBOX
    /*
     * Don't set timeout / delay values here. This results
     * in these values still being reset to 0.
     */
    if (state_get_skip_delays())
        return;
#endif

    /*
     * Wait for power to become stable,
     * plus spec-defined max time for device to connect
     * but allow this time to be increased via env variable as some
     * devices break the spec and require longer warm-up times
     */
#if 0 // TODO
    env = env_get("usb_pgood_delay");
    if (env)
        pgood_delay = max(pgood_delay, (unsigned)/*simple_*/strtol(env, NULL, 0));
#endif

    DEBUG("pgood_delay=%dms\n", pgood_delay);

    /*
     * Do a minimum delay of the larger value of 100ms or pgood_delay
     * so that the power can stablize before the devices are queried
     */
    hub->query_delay = get_timer(0) + max(100, (int)pgood_delay);

    /*
     * Record the power-on timeout here. The max. delay (timeout)
     * will be done based on this value in the USB port loop in
     * usb_hub_configure() later.
     */
    hub->connect_timeout = hub->query_delay + 1000;
    DEBUG("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
           dev->devnum, max(100, (int)pgood_delay),
           max(100, (int)pgood_delay) + 1000);
}

#if (!DM_USB)

static struct usb_hub_device hub_dev[USB_MAX_HUB];
static int usb_hub_index;

void usb_hub_reset(void)
{
    usb_hub_index = 0;

    /* Zero out global hub_dev in case its re-used again */
    memset(hub_dev, 0, sizeof(hub_dev));
}

static struct usb_hub_device *usb_hub_allocate(void)
{
    if (usb_hub_index < USB_MAX_HUB)
        return &hub_dev[usb_hub_index++];

    printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
    return NULL;
}
#endif

#define MAX_TRIES 5

static inline const char *portspeed(int portstatus)
{
    switch (portstatus & USB_PORT_STAT_SPEED_MASK)
    {
        case USB_PORT_STAT_SUPER_SPEED:
            return "5 Gb/s";

        case USB_PORT_STAT_HIGH_SPEED:
            return "480 Mb/s";

        case USB_PORT_STAT_LOW_SPEED:
            return "1.5 Mb/s";

        default:
            return "12 Mb/s";
    }
}

/**
 * usb_hub_port_reset() - reset a port given its usb_device pointer
 *
 * Reset a hub port and see if a device is present on that port, providing
 * sufficient time for it to show itself. The port status is returned.
 *
 * @dev:    USB device to reset
 * @port:   Port number to reset (note ports are numbered from 0 here)
 * @portstat:   Returns port status
 */
static int usb_hub_port_reset(struct usb_device *dev,
                              int port,
                              unsigned short *portstat)
{
    int err, tries;

    ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
    unsigned short portstatus, portchange;
    int delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */


#if (DM_USB)
    DEBUG("%s: resetting '%s' port %d...\n", __func__, dev->dev->name, port + 1);
#else
    DEBUG("%s: resetting port %d...\n", __func__, port + 1);
#endif

    for (tries = 0; tries < MAX_TRIES; tries++)
    {
        err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
        if (err < 0)
            return err;

        DELAY_MS(delay);

        if (usb_get_port_status(dev, port + 1, portsts) < 0)
        {
            DEBUG("get_port_status failed status %lX\n", dev->status);
            return -1;
        }

        portstatus = portsts->wPortStatus;
        portchange = portsts->wPortChange;

        DEBUG("portstatus %x, change %x, %s\n", portstatus, portchange,
               portspeed(portstatus));

        DEBUG("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
              "  USB_PORT_STAT_ENABLE %d\n",
              (portchange & USB_PORT_STAT_C_CONNECTION) ? 1 : 0,
              (portstatus & USB_PORT_STAT_CONNECTION) ? 1 : 0,
              (portstatus & USB_PORT_STAT_ENABLE) ? 1 : 0);

        /*
         * Perhaps we should check for the following here:
         * - C_CONNECTION hasn't been set.
         * - CONNECTION is still set.
         *
         * Doing so would ensure that the device is still connected
         * to the bus, and hasn't been unplugged or replaced while the
         * USB bus reset was going on.
         *
         * However, if we do that, then (at least) a San Disk Ultra
         * USB 3.0 16GB device fails to reset on (at least) an NVIDIA
         * Tegra Jetson TK1 board. For some reason, the device appears
         * to briefly drop off the bus when this second bus reset is
         * executed, yet if we retry this loop, it'll eventually come
         * back after another reset or two.
         */

        if (portstatus & USB_PORT_STAT_ENABLE)
            break;

        /* Switch to long reset delay for the next round */
        delay = HUB_LONG_RESET_TIME;
    }

    if (tries == MAX_TRIES)
    {
        DEBUG("Cannot enable port %i after %i retries, disabling port.\n",
               port + 1, MAX_TRIES);
        DEBUG("Maybe the USB cable is bad?\n");
        return -1;
    }

    usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
    *portstat = portstatus;
    return 0;
}

int usb_hub_port_connect_change(struct usb_device *dev, int port)
{
    ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
    unsigned short portstatus;
    int ret, speed;

    /* Check status */
    ret = usb_get_port_status(dev, port + 1, portsts);
    if (ret < 0)
    {
        DEBUG("get_port_status failed\n");
        return ret;
    }

    portstatus = portsts->wPortStatus;
    DEBUG("portstatus %x, change %x, %s\n",
           portstatus,
           portsts->wPortChange,
           portspeed(portstatus));

    /* Clear the connection change status */
    usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

    /* Disconnect any existing devices under this port */
    if (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
         (!(portstatus & USB_PORT_STAT_ENABLE))) ||
        usb_device_has_child_on_port(dev, port))
    {
        DEBUG("usb_disconnect(&hub->children[port]);\n");
        /* Return now if nothing is connected */
        if (!(portstatus & USB_PORT_STAT_CONNECTION))
            return -ENOTCONN;
    }

    /* Reset the port */
    ret = usb_hub_port_reset(dev, port, &portstatus);
    if (ret < 0)
    {
        if (ret != -ENXIO)
            printf("cannot reset port %i!?\n", port + 1);
        return ret;
    }

    switch (portstatus & USB_PORT_STAT_SPEED_MASK)
    {
        case USB_PORT_STAT_SUPER_SPEED:
            speed = USB_SPEED_SUPER;
            break;

        case USB_PORT_STAT_HIGH_SPEED:
            speed = USB_SPEED_HIGH;
            break;

        case USB_PORT_STAT_LOW_SPEED:
            speed = USB_SPEED_LOW;
            break;

        default:
            speed = USB_SPEED_FULL;
            break;
    }

#if (DM_USB)

    struct udevice *child;
    ret = usb_scan_device(dev->dev, port + 1, speed, &child);
    
#else

    struct usb_device *usb;

    ret = usb_alloc_new_device(dev->controller, &usb);
    if (ret)
    {
        printf("cannot create new device: ret=%d", ret);
        return ret;
    }

    dev->children[port] = usb;
    usb->speed = speed;
    usb->parent = dev;
    usb->portnr = port + 1;

    /* Run it through the hoops (find a driver, etc) */
    ret = usb_new_device(usb);
    if (ret < 0)
    {
        /* Woops, disable the port */
        usb_free_device(dev->controller);
        dev->children[port] = NULL;
    }
#endif

    if (ret < 0)
    {
        DEBUG("hub: disabling port %d\n", port + 1);
        usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
    }

    return ret;
}

static int usb_scan_port(struct usb_device_scan *usb_scan)
{
    ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
    unsigned short portstatus;
    unsigned short portchange;
    struct usb_device *dev;
    struct usb_hub_device *hub;
    int ret = 0;
    int i;

    dev = usb_scan->dev;
    hub = usb_scan->hub;
    i = usb_scan->port;

    /*
     * Don't talk to the device before the query delay is expired.
     * This is needed for voltages to stabalize.
     */
#if 0
    if (get_timer(0) < hub->query_delay)
        return 0;
#endif

    ret = usb_get_port_status(dev, i + 1, portsts);
    if (ret < 0)
    {
        DEBUG("get_port_status failed\n");
        if (get_timer(0) >= hub->connect_timeout)
        {
            DEBUG("devnum=%d port=%d: timeout\n", dev->devnum, i + 1);
            /* Remove this device from scanning list */
            list_del(&usb_scan->list);
            FREE(usb_scan);
            return 0;
        }

        return 0;
    }

    portstatus = portsts->wPortStatus;
    portchange = portsts->wPortChange;
    DEBUG("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

    /*
     * No connection change happened, wait a bit more.
     *
     * For some situation, the hub reports no connection change but a
     * device is connected to the port (eg: CCS bit is set but CSC is not
     * in the PORTSC register of a root hub), ignore such case.
     */
    if (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
        !(portstatus & USB_PORT_STAT_CONNECTION))
    {
        if (get_timer(0) >= hub->connect_timeout)
        {
            DEBUG("devnum=%d port=%d: timeout\n", dev->devnum, i + 1);
            /* Remove this device from scanning list */
            list_del(&usb_scan->list);
            FREE(usb_scan);
            return 0;
        }

        return 0;
    }

    if (portchange & USB_PORT_STAT_C_RESET)
    {
        DEBUG("port %d reset change\n", i + 1);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
    }

    if ((portchange & USB_SS_PORT_STAT_C_BH_RESET) && usb_hub_is_superspeed(dev))
    {
        DEBUG("port %d BH reset change\n", i + 1);
        usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
    }

    /* A new USB device is ready at this point */
    DEBUG("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

    usb_hub_port_connect_change(dev, i);

    if (portchange & USB_PORT_STAT_C_ENABLE)
    {
        DEBUG("port %d enable change, status %x\n", i + 1, portstatus);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
        /*
         * The following hack causes a ghost device problem
         * to Faraday EHCI
         */
#ifndef CONFIG_USB_EHCI_FARADAY
        /*
         * EM interference sometimes causes bad shielded USB
         * devices to be shutdown by the hub, this hack enables
         * them again. Works at least with mouse driver
         */
        if (!(portstatus & USB_PORT_STAT_ENABLE) &&
            (portstatus & USB_PORT_STAT_CONNECTION) &&
            usb_device_has_child_on_port(dev, i))
        {
            DEBUG("already running port %i disabled by hub (EMI?), "
                  "re-enabling...\n", i + 1);
            usb_hub_port_connect_change(dev, i);
        }
#endif
    }

    if (portstatus & USB_PORT_STAT_SUSPEND)
    {
        DEBUG("port %d suspend change\n", i + 1);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
    }

    if (portchange & USB_PORT_STAT_C_OVERCURRENT)
    {
        DEBUG("port %d over-current change\n", i + 1);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_OVER_CURRENT);
        /* Only power-on this one port */
        usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
        hub->overcurrent_count[i]++;

        /*
         * If the max-scan-count is not reached, return without removing
         * the device from scan-list. This will re-issue a new scan.
         */
        if (hub->overcurrent_count[i] <= PORT_OVERCURRENT_MAX_SCAN_COUNT)
            return 0;

        /* Otherwise the device will get removed */
        printf("Port %d over-current occurred %d times\n", i + 1,
                hub->overcurrent_count[i]);
    }

    /*
     * We're done with this device, so let's remove this device from
     * scanning list
     */
    list_del(&usb_scan->list);
    FREE(usb_scan);

    return 0;
}

static int usb_device_list_scan(void)
{
    struct usb_device_scan *usb_scan;
    struct usb_device_scan *tmp;
    static int running;
    int ret = 0;

    /* Only run this loop once for each controller */
    if (running)
        return 0;

    running = 1;

    while (1)
    {
        /* We're done, once the list is empty again */
        if (list_empty(&usb_scan_list))
            goto out;

        list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list)
        {
            int ret;

            /* Scan this port */
            ret = usb_scan_port(usb_scan);
            if (ret)
                goto out;
        }
    }

out:
    /*
     * This USB controller has finished scanning all its connected
     * USB devices. Set "running" back to 0, so that other USB controllers
     * will scan their devices too.
     */
    running = 0;

    return ret;
}

static struct usb_hub_device *usb_get_hub_device(struct usb_device *dev)
{
    struct usb_hub_device *hub;

#if (!DM_USB)
    /* "allocate" Hub device */
    hub = usb_hub_allocate();
#else
    hub = dev_get_uclass_priv(dev->dev);
#endif

    return hub;
}

static int usb_hub_configure(struct usb_device *dev)
{
    int i, length;
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, USB_BUFSIZ);
    unsigned char *bitmap;
    short hubCharacteristics;
    struct usb_hub_descriptor *descriptor;
    struct usb_hub_device *hub;
    struct usb_hub_status *hubsts;
    int ret;

    hub = usb_get_hub_device(dev);
    if (hub == NULL)
        return -ENOMEM;
    hub->pusb_dev = dev;

    /* Get the the hub descriptor */
    ret = usb_get_hub_descriptor(dev, buffer, 4);
    if (ret < 0)
    {
        DEBUG("usb_hub_configure: failed to get hub descriptor, giving up %lX\n",
               dev->status);
        return ret;
    }

    descriptor = (struct usb_hub_descriptor *)buffer;

    length = min_t(int, descriptor->bLength, sizeof(struct usb_hub_descriptor));

    ret = usb_get_hub_descriptor(dev, buffer, length);
    if (ret < 0)
    {
        DEBUG("usb_hub_configure: failed to get hub descriptor 2nd giving up %lX\n",
               dev->status);
        return ret;
    }

    memcpy((unsigned char *)&hub->desc, buffer, length);
    /* adjust 16bit values */
    // put_unaligned(get_unaligned(&descriptor->wHubCharacteristics),
    //               &hub->desc.wHubCharacteristics);
    hub->desc.wHubCharacteristics = descriptor->wHubCharacteristics;
    
    /* set the bitmap */
    bitmap = (unsigned char *)&hub->desc.u.hs.DeviceRemovable[0];
    /* devices not removable by default */
    memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
    bitmap = (unsigned char *)&hub->desc.u.hs.PortPowerCtrlMask[0];
    memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

    for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
        hub->desc.u.hs.DeviceRemovable[i] = descriptor->u.hs.DeviceRemovable[i];

    for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
        hub->desc.u.hs.PortPowerCtrlMask[i] = descriptor->u.hs.PortPowerCtrlMask[i];

    dev->maxchild = descriptor->bNbrPorts;
    DEBUG("%d ports detected\n", dev->maxchild);

    hubCharacteristics = hub->desc.wHubCharacteristics;
                      // get_unaligned(&hub->desc.wHubCharacteristics);
    switch (hubCharacteristics & HUB_CHAR_LPSM)
    {
        case 0x00:
            DEBUG("ganged power switching\n");
            break;

        case 0x01:
            DEBUG("individual port power switching\n");
            break;

        case 0x02:
        case 0x03:
            DEBUG("unknown reserved power switching mode\n");
            break;
    }

    if (hubCharacteristics & HUB_CHAR_COMPOUND)
        DEBUG("part of a compound device\n");
    else
        DEBUG("standalone hub\n");

    switch (hubCharacteristics & HUB_CHAR_OCPM)
    {
        case 0x00:
            DEBUG("global over-current protection\n");
            break;

        case 0x08:
            DEBUG("individual port over-current protection\n");
            break;

        case 0x10:
        case 0x18:
            DEBUG("no over-current protection\n");
            break;
    }

    switch (dev->descriptor.bDeviceProtocol)
    {
        case USB_HUB_PR_FS:
            break;

        case USB_HUB_PR_HS_SINGLE_TT:
            DEBUG("Single TT\n");
            break;

        case USB_HUB_PR_HS_MULTI_TT:
            ret = usb_set_interface(dev, 0, 1);
            if (ret == 0)
            {
                DEBUG("TT per port\n");
                hub->tt.multi = true;
            }
            else
            {
                DEBUG("Using single TT (err %d)\n", ret);
            }
            break;

        case USB_HUB_PR_SS:
            /* USB 3.0 hubs don't have a TT */
            break;

        default:
            DEBUG("Unrecognized hub protocol %d\n", dev->descriptor.bDeviceProtocol);
            break;
    }

    /* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
    switch (hubCharacteristics & HUB_CHAR_TTTT)
    {
        case HUB_TTTT_8_BITS:
            if (dev->descriptor.bDeviceProtocol != 0)
            {
                hub->tt.think_time = 666;
                DEBUG("TT requires at most %d FS bit times (%d ns)\n",
                       8, hub->tt.think_time);
            }
            break;

        case HUB_TTTT_16_BITS:
            hub->tt.think_time = 666 * 2;
            DEBUG("TT requires at most %d FS bit times (%d ns)\n",
                   16, hub->tt.think_time);
            break;

        case HUB_TTTT_24_BITS:
            hub->tt.think_time = 666 * 3;
            DEBUG("TT requires at most %d FS bit times (%d ns)\n",
                   24, hub->tt.think_time);
            break;

        case HUB_TTTT_32_BITS:
            hub->tt.think_time = 666 * 4;
            DEBUG("TT requires at most %d FS bit times (%d ns)\n",
                   32, hub->tt.think_time);
            break;
    }

    DEBUG("power on to power good time: %dms\n", descriptor->bPwrOn2PwrGood * 2);
    DEBUG("hub controller current requirement: %dmA\n", descriptor->bHubContrCurrent);

    for (i = 0; i < dev->maxchild; i++)
        DEBUG("port %d is%s removable\n", i + 1,
               hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
               (1 << ((i + 1) % 8)) ? " not" : "");

    if (sizeof(struct usb_hub_status) > USB_BUFSIZ)
    {
        DEBUG("usb_hub_configure: failed to get Status - too long: %d\n",
               descriptor->bLength);
        return -EFBIG;
    }

    ret = usb_get_hub_status(dev, buffer);
    if (ret < 0)
    {
        DEBUG("usb_hub_configure: failed to get Status %lX\n", dev->status);
        return ret;
    }

    hubsts = (struct usb_hub_status *)buffer;

    DEBUG("get_hub_status returned status %X, change %X\n",
          hubsts->wHubStatus,
          hubsts->wHubChange);
    DEBUG("local power source is %s\n",
          (hubsts->wHubStatus & HUB_STATUS_LOCAL_POWER) ? \
          "lost (inactive)" : "good");
    DEBUG("%sover-current condition exists\n",
          (hubsts->wHubStatus & HUB_STATUS_OVERCURRENT) ? \
          "" : "no ");

#if (DM_USB)
    /*
     * Update USB host controller's internal representation of this hub
     * after the hub descriptor is fetched.
     */
    ret = usb_update_hub_device(dev);
    if (ret < 0 && ret != -ENOSYS)
    {
        DEBUG("%s: failed to update hub device for HCD (%x)\n", __func__, ret);
        return ret;
    }

    /*
     * A maximum of seven tiers are allowed in a USB topology, and the
     * root hub occupies the first tier. The last tier ends with a normal
     * USB device. USB 3.0 hubs use a 20-bit field called 'route string'
     * to route packets to the designated downstream port. The hub uses a
     * hub depth value multiplied by four as an offset into the 'route
     * string' to locate the bits it uses to determine the downstream
     * port number.
     */
    if (usb_hub_is_root_hub(dev->dev))
    {
        hub->hub_depth = -1;
    }
    else
    {
        struct udevice *hdev;
        int depth = 0;

        hdev = dev->dev->parent;
        while (!usb_hub_is_root_hub(hdev))
        {
            depth++;
            hdev = hdev->parent;
        }

        hub->hub_depth = depth;

        if (usb_hub_is_superspeed(dev))
        {
            DEBUG("set hub (%p) depth to %d\n", dev, depth);
            /*
             * This request sets the value that the hub uses to
             * determine the index into the 'route string index'
             * for this hub.
             */
            ret = usb_set_hub_depth(dev, depth);
            if (ret < 0)
            {
                DEBUG("%s: failed to set hub depth (%lX)\n", __func__, dev->status);
                return ret;
            }
        }
    }
#endif

    usb_hub_power_on(hub);

    /*
     * Reset any devices that may be in a bad state when applying
     * the power.  This is a __weak function.  Resetting of the devices
     * should occur in the board file of the device.
     */
    for (i = 0; i < dev->maxchild; i++)
        usb_hub_reset_devices(hub, i + 1);

    /*
     * Only add the connected USB devices, including potential hubs,
     * to a scanning list. This list will get scanned and devices that
     * are detected (either via port connected or via port timeout)
     * will get removed from this list. Scanning of the devices on this
     * list will continue until all devices are removed.
     */
    for (i = 0; i < dev->maxchild; i++)
    {
        struct usb_device_scan *usb_scan;

        usb_scan = CALLOC(1, sizeof(*usb_scan));
        if (!usb_scan)
        {
            printf("Can't allocate memory for USB device!\n");
            return -ENOMEM;
        }

        usb_scan->dev = dev;
        usb_scan->hub = hub;
        usb_scan->port = i;
        list_add_tail(&usb_scan->list, &usb_scan_list);
    }

    /*
     * And now call the scanning code which loops over the generated list
     */
    ret = usb_device_list_scan();
    return ret;
}

static int usb_hub_check(struct usb_device *dev, int ifnum)
{
    struct usb_interface *iface;
    struct usb_endpoint_descriptor *ep = NULL;

    iface = &dev->config.if_desc[ifnum];
    /* Is it a hub? */
    if (iface->desc.bInterfaceClass != USB_CLASS_HUB)
        goto err;
    /* Some hubs have a subclass of 1, which AFAICT according to the */
    /* specs is not defined, but it works */
    if ((iface->desc.bInterfaceSubClass != 0) &&
        (iface->desc.bInterfaceSubClass != 1))
        goto err;
    /* Multiple endpoints? What kind of mutant ninja-hub is this? */
    if (iface->desc.bNumEndpoints != 1)
        goto err;
    ep = &iface->ep_desc[0];
    /* Output endpoint? Curiousier and curiousier.. */
    if (!(ep->bEndpointAddress & USB_DIR_IN))
        goto err;
    /* If it's not an interrupt endpoint, we'd better punt! */
    if ((ep->bmAttributes & 3) != 3)
        goto err;
    /* We found a hub */
    DEBUG("USB hub found\n");
    return 0;

err:
    DEBUG("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
          iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
          iface->desc.bNumEndpoints);

    if (ep)
    {
        DEBUG("   bEndpointAddress=%#x, bmAttributes=%d",
               ep->bEndpointAddress, ep->bmAttributes);
    }

    return -ENOENT;
}

int usb_hub_probe(struct usb_device *dev, int ifnum)
{
    int ret;

    ret = usb_hub_check(dev, ifnum);
    if (ret)
        return 0;
    ret = usb_hub_configure(dev);
    return ret;
}

#if (DM_USB)

int usb_hub_scan(struct udevice *hub)
{
    struct usb_device *udev = dev_get_parent_priv(hub);

    return usb_hub_configure(udev);
}

static int usb_hub_post_probe(struct udevice *dev)
{
    DEBUG("%s\n", __func__);
    return usb_hub_scan(dev);
}

static const struct udevice_id usb_hub_ids[] =
{
    { .compatible = "usb-hub" },
    { }
};

U_BOOT_DRIVER(usb_generic_hub) =
{
    .name     = "usb_hub",
    .id       = UCLASS_USB_HUB,
    .of_match = usb_hub_ids,
    .flags    = DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) =
{
    .id              = UCLASS_USB_HUB,
    .name            = "usb_hub",
    .post_bind       = dm_scan_fdt_dev,
    .post_probe      = usb_hub_post_probe,
    .child_pre_probe = usb_child_pre_probe,
    .per_child_auto_alloc_size          = sizeof(struct usb_device),
    .per_child_platdata_auto_alloc_size = sizeof(struct usb_dev_platdata),
    .per_device_auto_alloc_size         = sizeof(struct usb_hub_device),
};

static const struct usb_device_id hub_id_table[] =
{
    {
        .match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
        .bDeviceClass = USB_CLASS_HUB
    },
    { } /* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#endif

#endif // #if (BSP_USE_OTG) || (BSP_USE_EHCI) || (BSP_USE_OHCI)

/*
 * @@ EOF
 */
 

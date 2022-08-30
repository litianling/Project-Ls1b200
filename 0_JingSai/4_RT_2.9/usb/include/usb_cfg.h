/*
 * cfg_usb.h
 *
 * created: 2020/7/13
 * authour: 
 */

#ifndef _CFG_USB_H
#define _CFG_USB_H

#define BSP_USE_USB     1

#if BSP_USE_USB

#if defined(LS1B)
#include "ls1b.h"
#define LS1x_OHCI_BASE  0xBFE08000
#define LS1x_EHCI_BASE  0xBFE00000
#elif defined(LS1C)
#include "ls1c.h"
#define LS1x_OHCI_BASE  0xBFE28000
#define LS1x_EHCI_BASE  0xBFE20000
#else
#error "No Loongson SoC defined!"
#endif

#define ARCH_DMA_MINALIGN   32

//-----------------------------------------------------------------------------
// USB HOST
//-----------------------------------------------------------------------------

#if defined(LS1C)
#define BSP_USE_OTG         1
#if BSP_USE_OTG
#define CONFIG_USB_DWC2_BUFFER_SIZE 8
#endif
#endif

#define BSP_USE_EHCI        0
#if BSP_USE_EHCI
//
#endif

#define BSP_USE_OHCI        1
#if BSP_USE_OHCI
#define CONFIG_SYS_USB_OHCI_REGS_BASE  LS1x_OHCI_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME  "ohci"
#endif

//-----------------------------------------------------------------------------
// USB Device
//-----------------------------------------------------------------------------

#define BSP_USE_MASS        1
#if (BSP_USE_MASS)
#define USB_MAX_STOR_DEV    4
#endif

#define BSP_USE_KBD         1
#if (BSP_USE_KBD)
#define CONFIG_SYS_USB_EVENT_POLL
#endif

#define BSP_USE_MOUSE         1
#if (BSP_USE_MOUSE)
#define CONFIG_SYS_USB_EVENT_POLL
#endif

/*
 * @@ End
 */
#endif // #if BSP_USE_USB

#endif // #ifndef _CFG_USB_H


/*
 * URB OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2001 David Brownell <dbrownell@users.sourceforge.net>
 *
 * usb-ohci.h
 */

#define ohci_readl(a)       READ_REG32(a)
#define ohci_writel(v, a)   WRITE_REG32(a, v)

#define ED_ALIGNMENT 16
#define TD_ALIGNMENT 32

/* functions for doing board or CPU specific setup/cleanup */
int usb_board_stop(void);

int usb_cpu_init(void);
int usb_cpu_stop(void);
int usb_cpu_init_fail(void);

/* ED States */
#define ED_NEW          0x00
#define ED_UNLINK       0x01
#define ED_OPER         0x02
#define ED_DEL          0x04
#define ED_URB_DEL      0x08

/* usb_ohci_ed */
struct ed
{
    unsigned int hwINFO;
    unsigned int hwTailP;
    unsigned int hwHeadP;
    unsigned int hwNextED;

    struct ed     *ed_prev;
    unsigned char  int_period;
    unsigned char  int_branch;
    unsigned char  int_load;
    unsigned char  int_interval;
    unsigned char  state;
    unsigned char  type;
    unsigned short last_iso;
    struct ed     *ed_rm_list;

    struct usb_device *usb_dev;
    void        *purb;
    unsigned int unused[2];
} __attribute__((aligned(ED_ALIGNMENT)));

typedef struct ed ed_t;


/* TD info field */
#define TD_CC           0xf0000000
#define TD_CC_GET(td_p) ((td_p >>28) & 0x0f)
#define TD_CC_SET(td_p, cc) (td_p) = ((td_p) & 0x0fffffff) | (((cc) & 0x0f) << 28)
#define TD_EC           0x0C000000
#define TD_T            0x03000000
#define TD_T_DATA0      0x02000000
#define TD_T_DATA1      0x03000000
#define TD_T_TOGGLE     0x00000000
#define TD_R            0x00040000
#define TD_DI           0x00E00000
#define TD_DI_SET(X) (((X) & 0x07)<< 21)
#define TD_DP           0x00180000
#define TD_DP_SETUP     0x00000000
#define TD_DP_IN        0x00100000
#define TD_DP_OUT       0x00080000

#define TD_ISO          0x00010000
#define TD_DEL          0x00020000

/* CC Codes */
#define TD_CC_NOERROR       0x00
#define TD_CC_CRC           0x01
#define TD_CC_BITSTUFFING   0x02
#define TD_CC_DATATOGGLEM   0x03
#define TD_CC_STALL         0x04
#define TD_DEVNOTRESP       0x05
#define TD_PIDCHECKFAIL     0x06
#define TD_UNEXPECTEDPID    0x07
#define TD_DATAOVERRUN      0x08
#define TD_DATAUNDERRUN     0x09
#define TD_BUFFEROVERRUN    0x0C
#define TD_BUFFERUNDERRUN   0x0D
#define TD_NOTACCESSED      0x0F

#define MAXPSW      1

struct td
{
    unsigned int hwINFO;
    unsigned int hwCBP;         /* 当前缓存区指针 */
    unsigned int hwNextTD;      /* 下一个TD的指针 */
    unsigned int hwBE;          /* 存储器缓存结束指针 */

    unsigned short hwPSW[MAXPSW];
    unsigned char  unused;
    unsigned char  index;
    struct ed     *ed;
    struct td     *next_dl_td;
    struct usb_device *usb_dev;
    int           transfer_len;
    unsigned int  data;

    unsigned int  unused2[2];
} __attribute__((aligned(TD_ALIGNMENT)));

typedef struct td td_t;

#define OHCI_ED_SKIP    (1 << 14)

/*
 * The HCCA (主控制器通信区) is a 256 byte
 * structure defined in the OHCI spec. that the host controller is
 * told the base address of.  It must be 256-byte aligned.
 */

#define NUM_INTS    32  /* part of the OHCI standard */

struct ohci_hcca
{
    unsigned int   int_table[NUM_INTS];    /* 中断 ED 表 */
    unsigned short frame_no;               /* 当前帧号 */
    unsigned short pad1;                   /* 在每个 frame_no 上设置为 0 */
    unsigned int   done_head;              /* 为中断返回的信息 */
    unsigned char  reserved_for_hc[116];
} __attribute__((aligned(256)));

/*
 * Maximum number of root hub ports.
 */
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 4

/*
 * This is the structure of the OHCI controller's memory mapped I/O
 * region.  This is Memory Mapped I/O.  You must use the ohci_readl() and
 * ohci_writel() macros defined in this file to access these!!
 */
 
 // 文档119页可查
struct ohci_regs
{
    /* OHCI控制和状态寄存器 */
    volatile unsigned int revision;
    volatile unsigned int control;
    volatile unsigned int cmdstatus;
    volatile unsigned int intrstatus;
    volatile unsigned int intrenable;
    volatile unsigned int intrdisable;
    /* 内存指针 */
    volatile unsigned int hcca;
    volatile unsigned int ed_periodcurrent;
    volatile unsigned int ed_controlhead;
    volatile unsigned int ed_controlcurrent;
    volatile unsigned int ed_bulkhead;
    volatile unsigned int ed_bulkcurrent;
    volatile unsigned int donehead;
    /* 帧计数器 */
    volatile unsigned int fminterval;
    volatile unsigned int fmremaining;
    volatile unsigned int fmnumber;
    volatile unsigned int periodicstart;
    volatile unsigned int lsthresh;
    /* 根集线器 */
    struct  ohci_roothub_regs
    {
        volatile unsigned int a;
        volatile unsigned int b;
        volatile unsigned int status;
        volatile unsigned int portstatus[CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS];
    } roothub;
} __attribute__((aligned(32)));

/* Some EHCI controls */
#define EHCI_USBCMD_OFF         0x20
#define EHCI_USBCMD_HCRESET    (1 << 1)

/* OHCI CONTROL AND STATUS REGISTER MASKS */

/*
 * HcControl (control) register masks
 */
#define OHCI_CTRL_CBSR      (3 << 0)    /* control/bulk service ratio */
#define OHCI_CTRL_PLE       (1 << 2)    /* periodic list enable */
#define OHCI_CTRL_IE        (1 << 3)    /* isochronous enable */
#define OHCI_CTRL_CLE       (1 << 4)    /* control list enable */
#define OHCI_CTRL_BLE       (1 << 5)    /* bulk list enable */
#define OHCI_CTRL_HCFS      (3 << 6)    /* host controller functional state */
#define OHCI_CTRL_IR        (1 << 8)    /* interrupt routing */
#define OHCI_CTRL_RWC       (1 << 9)    /* remote wakeup connected */
#define OHCI_CTRL_RWE       (1 << 10)   /* remote wakeup enable */

/* pre-shifted values for HCFS */
#define OHCI_USB_RESET      (0 << 6)
#define OHCI_USB_RESUME     (1 << 6)
#define OHCI_USB_OPER       (2 << 6)
#define OHCI_USB_SUSPEND    (3 << 6)

/*
 * HcCommandStatus (cmdstatus) register masks
 */
#define OHCI_HCR            (1 << 0)    /* host controller reset */
#define OHCI_CLF            (1 << 1)    /* control list filled */
#define OHCI_BLF            (1 << 2)    /* bulk list filled */
#define OHCI_OCR            (1 << 3)    /* ownership change request */
#define OHCI_SOC            (3 << 16)   /* scheduling overrun count */

/*
 * masks used with interrupt registers:
 * HcInterruptStatus (intrstatus)
 * HcInterruptEnable (intrenable)
 * HcInterruptDisable (intrdisable)
 */
#define OHCI_INTR_SO        (1 << 0)    /* scheduling overrun */
#define OHCI_INTR_WDH       (1 << 1)    /* writeback of done_head */
#define OHCI_INTR_SF        (1 << 2)    /* start frame */
#define OHCI_INTR_RD        (1 << 3)    /* resume detect */
#define OHCI_INTR_UE        (1 << 4)    /* unrecoverable error */
#define OHCI_INTR_FNO       (1 << 5)    /* frame number overflow */
#define OHCI_INTR_RHSC      (1 << 6)    /* root hub status change */
#define OHCI_INTR_OC        (1 << 30)   /* ownership change */
#define OHCI_INTR_MIE       (1 << 31)   /* master interrupt enable */

/* 虚拟根集线器 */
struct virt_root_hub
{
    int   devnum;       /* 根集线器端点地址（同设备在USB总线上的设备号） */
    void *dev;          /* was urb */
    void *int_addr;
    int   send;
    int   interval;
};

/* USB HUB CONSTANTS (not OHCI-specific; see hub.h) */

/* destination of request */
#define RH_INTERFACE            0x01
#define RH_ENDPOINT             0x02
#define RH_OTHER                0x03

#define RH_CLASS                0x20
#define RH_VENDOR               0x40

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS           0x0080
#define RH_CLEAR_FEATURE        0x0100
#define RH_SET_FEATURE          0x0300
#define RH_SET_ADDRESS          0x0500
#define RH_GET_DESCRIPTOR       0x0680
#define RH_SET_DESCRIPTOR       0x0700
#define RH_GET_CONFIGURATION    0x0880
#define RH_SET_CONFIGURATION    0x0900
#define RH_GET_STATE            0x0280
#define RH_GET_INTERFACE        0x0A80
#define RH_SET_INTERFACE        0x0B00
#define RH_SYNC_FRAME           0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP               0x2000

/* Hub port features */
#define RH_PORT_CONNECTION      0x00
#define RH_PORT_ENABLE          0x01
#define RH_PORT_SUSPEND         0x02
#define RH_PORT_OVER_CURRENT    0x03
#define RH_PORT_RESET           0x04
#define RH_PORT_POWER           0x08
#define RH_PORT_LOW_SPEED       0x09

#define RH_C_PORT_CONNECTION    0x10
#define RH_C_PORT_ENABLE        0x11
#define RH_C_PORT_SUSPEND       0x12
#define RH_C_PORT_OVER_CURRENT  0x13
#define RH_C_PORT_RESET         0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER    0x00
#define RH_C_HUB_OVER_CURRENT   0x01

#define RH_DEVICE_REMOTE_WAKEUP 0x00
#define RH_ENDPOINT_STALL       0x01

#define RH_ACK                  0x01
#define RH_REQ_ERR              -1
#define RH_NACK                 0x00

/* OHCI ROOT HUB REGISTER MASKS */

/* roothub.portstatus [i] bits */
#define RH_PS_CCS           0x00000001      /* current connect status */
#define RH_PS_PES           0x00000002      /* port enable status*/
#define RH_PS_PSS           0x00000004      /* port suspend status */
#define RH_PS_POCI          0x00000008      /* port over current indicator */
#define RH_PS_PRS           0x00000010      /* port reset status */
#define RH_PS_PPS           0x00000100      /* port power status */
#define RH_PS_LSDA          0x00000200      /* low speed device attached */
#define RH_PS_CSC           0x00010000      /* connect status change */
#define RH_PS_PESC          0x00020000      /* port enable status change */
#define RH_PS_PSSC          0x00040000      /* port suspend status change */
#define RH_PS_OCIC          0x00080000      /* over current indicator change */
#define RH_PS_PRSC          0x00100000      /* port reset status change */

/* roothub.status bits */
#define RH_HS_LPS           0x00000001      /* local power status */
#define RH_HS_OCI           0x00000002      /* over current indicator */
#define RH_HS_DRWE          0x00008000      /* device remote wakeup enable */
#define RH_HS_LPSC          0x00010000      /* local power status change */
#define RH_HS_OCIC          0x00020000      /* over current indicator change */
#define RH_HS_CRWE          0x80000000      /* clear remote wakeup enable */

/* roothub.b masks */
#define RH_B_DR             0x0000ffff      /* device removable flags */
#define RH_B_PPCM           0xffff0000      /* port power control mask */

/* roothub.a masks */
#define RH_A_NDP            (0xff << 0)     /* number of downstream ports */
#define RH_A_PSM            (1 << 8)        /* power switching mode */
#define RH_A_NPS            (1 << 9)        /* no power switching */
#define RH_A_DT             (1 << 10)       /* device type (mbz) */
#define RH_A_OCPM           (1 << 11)       /* over current protection mode */
#define RH_A_NOCP           (1 << 12)       /* no over current protection */
#define RH_A_POTPGT         (0xff << 24)    /* power on to power good time */

/* urb */
#define N_URB_TD    96

typedef struct
{
    ed_t *ed;
    unsigned short length;          /* 与此请求关联的 td 数 */
    unsigned short td_cnt;          /* 已服务的 td 数量 */
    struct usb_device *dev;
    int            state;
    unsigned long  pipe;
    void *transfer_buffer;
    int   transfer_buffer_length;
    int   interval;
    int   actual_length;
    int   finished;
    td_t *td[N_URB_TD];             /* 指向与此请求关联的所有相应 TD 的列表指针 */
    int   idle;
} urb_priv_t;

#define URB_DEL     1

#define NUM_EDS     64   // 32      /* 预分配的端点描述符的数量

#define NUM_TD      128  // 64      /* we need more TDs than EDs */

#define NUM_INT_DEVS 8              /* num of ohci_dev structs for int endpoints */

typedef struct ohci_device
{
    ed_t ed[NUM_EDS] __attribute__((aligned(ED_ALIGNMENT))); // __aligned(ED_ALIGNMENT);
    td_t tds[NUM_TD] __attribute__((aligned(TD_ALIGNMENT))); // __aligned(TD_ALIGNMENT);
    int ed_cnt;
    int devnum;
} ohci_dev_t;

/*
 * This is the full ohci controller description
 *
 * Note how the "proper" USB information is just
 * a subset of what the full implementation needs. (Linus)
 */

// OCHI 开放的主机控制器接口
typedef struct ohci
{
    /* 这为所有可能的端点分配 ED */
    struct ohci_device ohci_dev __attribute__((aligned(TD_ALIGNMENT)));              // __aligned(TD_ALIGNMENT);
    struct ohci_device int_dev[NUM_INT_DEVS] __attribute__((aligned(TD_ALIGNMENT))); // __aligned(TD_ALIGNMENT);
    struct ohci_hcca *hcca;     /* hcca */
    /*dma_addr_t hcca_dma;*/

    int irq;
    int disabled;               /* 例如：获取到UE，挂起 */
    int sleeping;
    unsigned long flags;        /* HC 出现BUG的标志 */

    struct ohci_regs *regs;     /* OHCI 寄存器 */

    int   ohci_int_load[32];    /* 32个中断向量的加载 */
    ed_t *ed_rm_list[2];        /* 列出所有将要移除的端点 */
    ed_t *ed_bulktail;          /* 批量列表的最后一个端点 */
    ed_t *ed_controltail;       /* 控制列表的最后一个端点 */
    int   intrstatus;           // 中断状态
    unsigned int hc_control;    /* hc控制寄存器的备份 */
    struct usb_device *dev[32];
    struct virt_root_hub rh;    // 虚拟根集线器

    const char *slot_name;      // 主机控制器接口名字
} ohci_t;



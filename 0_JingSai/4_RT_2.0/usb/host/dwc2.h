/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#ifndef __DWC2_H__
#define __DWC2_H__

#define ENOTSUPP    524         /* Operation is not supported */

struct dwc2_hc_regs
{
    unsigned int hcchar;        /* 0x00 */
    unsigned int hcsplt;
    unsigned int hcint;
    unsigned int hcintmsk;
    unsigned int hctsiz;        /* 0x10 */
    unsigned int hcdma;
    unsigned int reserved;
    unsigned int hcdmab;
};

struct dwc2_host_regs
{
    unsigned int hcfg;          /* 0x00 */
    unsigned int hfir;
    unsigned int hfnum;
    unsigned int _pad_0x40c;
    unsigned int hptxsts;       /* 0x10 */
    unsigned int haint;
    unsigned int haintmsk;
    unsigned int hflbaddr;
};

struct dwc2_core_regs
{
    unsigned int gotgctl;       /* 0x000 */
    unsigned int gotgint;
    unsigned int gahbcfg;
    unsigned int gusbcfg;
    unsigned int grstctl;       /* 0x010 */
    unsigned int gintsts;
    unsigned int gintmsk;
    unsigned int grxstsr;
    unsigned int grxstsp;       /* 0x020 */
    unsigned int grxfsiz;
    unsigned int gnptxfsiz;
    unsigned int gnptxsts;
    unsigned int gi2cctl;       /* 0x030 */
    unsigned int gpvndctl;
    unsigned int ggpio;
    unsigned int guid;
    unsigned int gsnpsid;       /* 0x040 */
    unsigned int ghwcfg1;
    unsigned int ghwcfg2;
    unsigned int ghwcfg3;
    unsigned int ghwcfg4;       /* 0x050 */
    unsigned int glpmcfg;
    unsigned int _pad_0x58_0x9c[42];
    unsigned int hptxfsiz;              /* 0x100 */
    unsigned int dptxfsiz_dieptxf[15];
    unsigned int _pad_0x140_0x3fc[176];
    struct dwc2_host_regs host_regs;    /* 0x400 */
    unsigned int _pad_0x420_0x43c[8];
    unsigned int hprt0;                 /* 0x440 */
    unsigned int _pad_0x444_0x4fc[47];
    struct dwc2_hc_regs hc_regs[16];    /* 0x500 */
    unsigned int _pad_0x700_0xe00[448];
    unsigned int pcgcctl;               /* 0xe00 */
};

#define DWC2_GOTGCTL_SESREQSCS                      (1 << 0)
#define DWC2_GOTGCTL_SESREQSCS_OFFSET               0
#define DWC2_GOTGCTL_SESREQ                         (1 << 1)
#define DWC2_GOTGCTL_SESREQ_OFFSET                  1
#define DWC2_GOTGCTL_HSTNEGSCS                      (1 << 8)
#define DWC2_GOTGCTL_HSTNEGSCS_OFFSET               8
#define DWC2_GOTGCTL_HNPREQ                         (1 << 9)
#define DWC2_GOTGCTL_HNPREQ_OFFSET                  9
#define DWC2_GOTGCTL_HSTSETHNPEN                    (1 << 10)
#define DWC2_GOTGCTL_HSTSETHNPEN_OFFSET             10
#define DWC2_GOTGCTL_DEVHNPEN                       (1 << 11)
#define DWC2_GOTGCTL_DEVHNPEN_OFFSET                11
#define DWC2_GOTGCTL_CONIDSTS                       (1 << 16)
#define DWC2_GOTGCTL_CONIDSTS_OFFSET                16
#define DWC2_GOTGCTL_DBNCTIME                       (1 << 17)
#define DWC2_GOTGCTL_DBNCTIME_OFFSET                17
#define DWC2_GOTGCTL_ASESVLD                        (1 << 18)
#define DWC2_GOTGCTL_ASESVLD_OFFSET                 18
#define DWC2_GOTGCTL_BSESVLD                        (1 << 19)
#define DWC2_GOTGCTL_BSESVLD_OFFSET                 19
#define DWC2_GOTGCTL_OTGVER                         (1 << 20)
#define DWC2_GOTGCTL_OTGVER_OFFSET                  20

#define DWC2_GOTGINT_SESENDDET                      (1 << 2)
#define DWC2_GOTGINT_SESENDDET_OFFSET               2
#define DWC2_GOTGINT_SESREQSUCSTSCHNG               (1 << 8)
#define DWC2_GOTGINT_SESREQSUCSTSCHNG_OFFSET        8
#define DWC2_GOTGINT_HSTNEGSUCSTSCHNG               (1 << 9)
#define DWC2_GOTGINT_HSTNEGSUCSTSCHNG_OFFSET        9
#define DWC2_GOTGINT_RESERVER10_16_MASK             (0x7F << 10)
#define DWC2_GOTGINT_RESERVER10_16_OFFSET           10
#define DWC2_GOTGINT_HSTNEGDET                      (1 << 17)
#define DWC2_GOTGINT_HSTNEGDET_OFFSET               17
#define DWC2_GOTGINT_ADEVTOUTCHNG                   (1 << 18)
#define DWC2_GOTGINT_ADEVTOUTCHNG_OFFSET            18
#define DWC2_GOTGINT_DEBDONE                        (1 << 19)
#define DWC2_GOTGINT_DEBDONE_OFFSET                 19

#define DWC2_GAHBCFG_GLBLINTRMSK                    (1 << 0)
#define DWC2_GAHBCFG_GLBLINTRMSK_OFFSET             0
#define DWC2_GAHBCFG_HBURSTLEN_SINGLE               (0 << 1)
#define DWC2_GAHBCFG_HBURSTLEN_INCR                 (1 << 1)
#define DWC2_GAHBCFG_HBURSTLEN_INCR4                (3 << 1)
#define DWC2_GAHBCFG_HBURSTLEN_INCR8                (5 << 1)
#define DWC2_GAHBCFG_HBURSTLEN_INCR16               (7 << 1)
#define DWC2_GAHBCFG_HBURSTLEN_MASK                 (0xF << 1)
#define DWC2_GAHBCFG_HBURSTLEN_OFFSET               1
#define DWC2_GAHBCFG_DMAENABLE                      (1 << 5)
#define DWC2_GAHBCFG_DMAENABLE_OFFSET               5
#define DWC2_GAHBCFG_NPTXFEMPLVL_TXFEMPLVL          (1 << 7)
#define DWC2_GAHBCFG_NPTXFEMPLVL_TXFEMPLVL_OFFSET   7
#define DWC2_GAHBCFG_PTXFEMPLVL                     (1 << 8)
#define DWC2_GAHBCFG_PTXFEMPLVL_OFFSET              8

#define DWC2_GUSBCFG_TOUTCAL_MASK                   (0x7 << 0)
#define DWC2_GUSBCFG_TOUTCAL_OFFSET                 0
#define DWC2_GUSBCFG_PHYIF                          (1 << 3)
#define DWC2_GUSBCFG_PHYIF_OFFSET                   3
#define DWC2_GUSBCFG_ULPI_UTMI_SEL                  (1 << 4)
#define DWC2_GUSBCFG_ULPI_UTMI_SEL_OFFSET           4
#define DWC2_GUSBCFG_FSINTF                         (1 << 5)
#define DWC2_GUSBCFG_FSINTF_OFFSET                  5
#define DWC2_GUSBCFG_PHYSEL                         (1 << 6)
#define DWC2_GUSBCFG_PHYSEL_OFFSET                  6
#define DWC2_GUSBCFG_DDRSEL                         (1 << 7)
#define DWC2_GUSBCFG_DDRSEL_OFFSET                  7
#define DWC2_GUSBCFG_SRPCAP                         (1 << 8)
#define DWC2_GUSBCFG_SRPCAP_OFFSET                  8
#define DWC2_GUSBCFG_HNPCAP                         (1 << 9)
#define DWC2_GUSBCFG_HNPCAP_OFFSET                  9
#define DWC2_GUSBCFG_USBTRDTIM_MASK                 (0xF << 10)
#define DWC2_GUSBCFG_USBTRDTIM_OFFSET               10
#define DWC2_GUSBCFG_NPTXFRWNDEN                    (1 << 14)
#define DWC2_GUSBCFG_NPTXFRWNDEN_OFFSET             14
#define DWC2_GUSBCFG_PHYLPWRCLKSEL                  (1 << 15)
#define DWC2_GUSBCFG_PHYLPWRCLKSEL_OFFSET           15
#define DWC2_GUSBCFG_OTGUTMIFSSEL                   (1 << 16)
#define DWC2_GUSBCFG_OTGUTMIFSSEL_OFFSET            16
#define DWC2_GUSBCFG_ULPI_FSLS                      (1 << 17)
#define DWC2_GUSBCFG_ULPI_FSLS_OFFSET               17
#define DWC2_GUSBCFG_ULPI_AUTO_RES                  (1 << 18)
#define DWC2_GUSBCFG_ULPI_AUTO_RES_OFFSET           18
#define DWC2_GUSBCFG_ULPI_CLK_SUS_M                 (1 << 19)
#define DWC2_GUSBCFG_ULPI_CLK_SUS_M_OFFSET          19
#define DWC2_GUSBCFG_ULPI_EXT_VBUS_DRV              (1 << 20)
#define DWC2_GUSBCFG_ULPI_EXT_VBUS_DRV_OFFSET       20
#define DWC2_GUSBCFG_ULPI_INT_VBUS_INDICATOR        (1 << 21)
#define DWC2_GUSBCFG_ULPI_INT_VBUS_INDICATOR_OFFSET 21
#define DWC2_GUSBCFG_TERM_SEL_DL_PULSE              (1 << 22)
#define DWC2_GUSBCFG_TERM_SEL_DL_PULSE_OFFSET       22
#define DWC2_GUSBCFG_INDICATOR_PASSTHROUGH          (1 << 24)
#define DWC2_GUSBCFG_INDICATOR_PASSTHROUGH_OFFSET   24
#define DWC2_GUSBCFG_IC_USB_CAP                     (1 << 26)
#define DWC2_GUSBCFG_IC_USB_CAP_OFFSET              26
#define DWC2_GUSBCFG_IC_TRAFFIC_PULL_REMOVE         (1 << 27)
#define DWC2_GUSBCFG_IC_TRAFFIC_PULL_REMOVE_OFFSET  27
#define DWC2_GUSBCFG_TX_END_DELAY                   (1 << 28)
#define DWC2_GUSBCFG_TX_END_DELAY_OFFSET            28
#define DWC2_GUSBCFG_FORCEHOSTMODE                  (1 << 29)
#define DWC2_GUSBCFG_FORCEHOSTMODE_OFFSET           29
#define DWC2_GUSBCFG_FORCEDEVMODE                   (1 << 30)
#define DWC2_GUSBCFG_FORCEDEVMODE_OFFSET            30

#define DWC2_GLPMCTL_LPM_CAP_EN                     (1 << 0)
#define DWC2_GLPMCTL_LPM_CAP_EN_OFFSET              0
#define DWC2_GLPMCTL_APPL_RESP                      (1 << 1)
#define DWC2_GLPMCTL_APPL_RESP_OFFSET               1
#define DWC2_GLPMCTL_HIRD_MASK                      (0xF << 2)
#define DWC2_GLPMCTL_HIRD_OFFSET                    2
#define DWC2_GLPMCTL_REM_WKUP_EN                    (1 << 6)
#define DWC2_GLPMCTL_REM_WKUP_EN_OFFSET             6
#define DWC2_GLPMCTL_EN_UTMI_SLEEP                  (1 << 7)
#define DWC2_GLPMCTL_EN_UTMI_SLEEP_OFFSET           7
#define DWC2_GLPMCTL_HIRD_THRES_MASK                (0x1F << 8)
#define DWC2_GLPMCTL_HIRD_THRES_OFFSET              8
#define DWC2_GLPMCTL_LPM_RESP_MASK                  (0x3 << 13)
#define DWC2_GLPMCTL_LPM_RESP_OFFSET                13
#define DWC2_GLPMCTL_PRT_SLEEP_STS                  (1 << 15)
#define DWC2_GLPMCTL_PRT_SLEEP_STS_OFFSET           15
#define DWC2_GLPMCTL_SLEEP_STATE_RESUMEOK           (1 << 16)
#define DWC2_GLPMCTL_SLEEP_STATE_RESUMEOK_OFFSET    16
#define DWC2_GLPMCTL_LPM_CHAN_INDEX_MASK            (0xF << 17)
#define DWC2_GLPMCTL_LPM_CHAN_INDEX_OFFSET          17
#define DWC2_GLPMCTL_RETRY_COUNT_MASK               (0x7 << 21)
#define DWC2_GLPMCTL_RETRY_COUNT_OFFSET             21
#define DWC2_GLPMCTL_SEND_LPM                       (1 << 24)
#define DWC2_GLPMCTL_SEND_LPM_OFFSET                24
#define DWC2_GLPMCTL_RETRY_COUNT_STS_MASK           (0x7 << 25)
#define DWC2_GLPMCTL_RETRY_COUNT_STS_OFFSET         25
#define DWC2_GLPMCTL_HSIC_CONNECT                   (1 << 30)
#define DWC2_GLPMCTL_HSIC_CONNECT_OFFSET            30
#define DWC2_GLPMCTL_INV_SEL_HSIC                   (1 << 31)
#define DWC2_GLPMCTL_INV_SEL_HSIC_OFFSET            31

#define DWC2_GRSTCTL_CSFTRST                        (1 << 0)
#define DWC2_GRSTCTL_CSFTRST_OFFSET                 0
#define DWC2_GRSTCTL_HSFTRST                        (1 << 1)
#define DWC2_GRSTCTL_HSFTRST_OFFSET                 1
#define DWC2_GRSTCTL_HSTFRM                         (1 << 2)
#define DWC2_GRSTCTL_HSTFRM_OFFSET                  2
#define DWC2_GRSTCTL_INTKNQFLSH                     (1 << 3)
#define DWC2_GRSTCTL_INTKNQFLSH_OFFSET              3
#define DWC2_GRSTCTL_RXFFLSH                        (1 << 4)
#define DWC2_GRSTCTL_RXFFLSH_OFFSET                 4
#define DWC2_GRSTCTL_TXFFLSH                        (1 << 5)
#define DWC2_GRSTCTL_TXFFLSH_OFFSET                 5
#define DWC2_GRSTCTL_TXFNUM_MASK                    (0x1F << 6)
#define DWC2_GRSTCTL_TXFNUM_OFFSET                  6
#define DWC2_GRSTCTL_DMAREQ                         (1 << 30)
#define DWC2_GRSTCTL_DMAREQ_OFFSET                  30
#define DWC2_GRSTCTL_AHBIDLE                        (1 << 31)
#define DWC2_GRSTCTL_AHBIDLE_OFFSET                 31

#define DWC2_GINTMSK_MODEMISMATCH                   (1 << 1)
#define DWC2_GINTMSK_MODEMISMATCH_OFFSET            1
#define DWC2_GINTMSK_OTGINTR                        (1 << 2)
#define DWC2_GINTMSK_OTGINTR_OFFSET                 2
#define DWC2_GINTMSK_SOFINTR                        (1 << 3)
#define DWC2_GINTMSK_SOFINTR_OFFSET                 3
#define DWC2_GINTMSK_RXSTSQLVL                      (1 << 4)
#define DWC2_GINTMSK_RXSTSQLVL_OFFSET               4
#define DWC2_GINTMSK_NPTXFEMPTY                     (1 << 5)
#define DWC2_GINTMSK_NPTXFEMPTY_OFFSET              5
#define DWC2_GINTMSK_GINNAKEFF                      (1 << 6)
#define DWC2_GINTMSK_GINNAKEFF_OFFSET               6
#define DWC2_GINTMSK_GOUTNAKEFF                     (1 << 7)
#define DWC2_GINTMSK_GOUTNAKEFF_OFFSET              7
#define DWC2_GINTMSK_I2CINTR                        (1 << 9)
#define DWC2_GINTMSK_I2CINTR_OFFSET                 9
#define DWC2_GINTMSK_ERLYSUSPEND                    (1 << 10)
#define DWC2_GINTMSK_ERLYSUSPEND_OFFSET             10
#define DWC2_GINTMSK_USBSUSPEND                     (1 << 11)
#define DWC2_GINTMSK_USBSUSPEND_OFFSET              11
#define DWC2_GINTMSK_USBRESET                       (1 << 12)
#define DWC2_GINTMSK_USBRESET_OFFSET                12
#define DWC2_GINTMSK_ENUMDONE                       (1 << 13)
#define DWC2_GINTMSK_ENUMDONE_OFFSET                13
#define DWC2_GINTMSK_ISOOUTDROP                     (1 << 14)
#define DWC2_GINTMSK_ISOOUTDROP_OFFSET              14
#define DWC2_GINTMSK_EOPFRAME                       (1 << 15)
#define DWC2_GINTMSK_EOPFRAME_OFFSET                15
#define DWC2_GINTMSK_EPMISMATCH                     (1 << 17)
#define DWC2_GINTMSK_EPMISMATCH_OFFSET              17
#define DWC2_GINTMSK_INEPINTR                       (1 << 18)
#define DWC2_GINTMSK_INEPINTR_OFFSET                18
#define DWC2_GINTMSK_OUTEPINTR                      (1 << 19)
#define DWC2_GINTMSK_OUTEPINTR_OFFSET               19
#define DWC2_GINTMSK_INCOMPLISOIN                   (1 << 20)
#define DWC2_GINTMSK_INCOMPLISOIN_OFFSET            20
#define DWC2_GINTMSK_INCOMPLISOOUT                  (1 << 21)
#define DWC2_GINTMSK_INCOMPLISOOUT_OFFSET           21
#define DWC2_GINTMSK_PORTINTR                       (1 << 24)
#define DWC2_GINTMSK_PORTINTR_OFFSET                24
#define DWC2_GINTMSK_HCINTR                         (1 << 25)
#define DWC2_GINTMSK_HCINTR_OFFSET                  25
#define DWC2_GINTMSK_PTXFEMPTY                      (1 << 26)
#define DWC2_GINTMSK_PTXFEMPTY_OFFSET               26
#define DWC2_GINTMSK_LPMTRANRCVD                    (1 << 27)
#define DWC2_GINTMSK_LPMTRANRCVD_OFFSET             27
#define DWC2_GINTMSK_CONIDSTSCHNG                   (1 << 28)
#define DWC2_GINTMSK_CONIDSTSCHNG_OFFSET            28
#define DWC2_GINTMSK_DISCONNECT                     (1 << 29)
#define DWC2_GINTMSK_DISCONNECT_OFFSET              29
#define DWC2_GINTMSK_SESSREQINTR                    (1 << 30)
#define DWC2_GINTMSK_SESSREQINTR_OFFSET             30
#define DWC2_GINTMSK_WKUPINTR                       (1 << 31)
#define DWC2_GINTMSK_WKUPINTR_OFFSET                31

#define DWC2_GINTSTS_CURMODE_DEVICE                 (0 << 0)
#define DWC2_GINTSTS_CURMODE_HOST                   (1 << 0)
#define DWC2_GINTSTS_CURMODE                        (1 << 0)
#define DWC2_GINTSTS_CURMODE_OFFSET                 0
#define DWC2_GINTSTS_MODEMISMATCH                   (1 << 1)
#define DWC2_GINTSTS_MODEMISMATCH_OFFSET            1
#define DWC2_GINTSTS_OTGINTR                        (1 << 2)
#define DWC2_GINTSTS_OTGINTR_OFFSET                 2
#define DWC2_GINTSTS_SOFINTR                        (1 << 3)
#define DWC2_GINTSTS_SOFINTR_OFFSET                 3
#define DWC2_GINTSTS_RXSTSQLVL                      (1 << 4)
#define DWC2_GINTSTS_RXSTSQLVL_OFFSET               4
#define DWC2_GINTSTS_NPTXFEMPTY                     (1 << 5)
#define DWC2_GINTSTS_NPTXFEMPTY_OFFSET              5
#define DWC2_GINTSTS_GINNAKEFF                      (1 << 6)
#define DWC2_GINTSTS_GINNAKEFF_OFFSET               6
#define DWC2_GINTSTS_GOUTNAKEFF                     (1 << 7)
#define DWC2_GINTSTS_GOUTNAKEFF_OFFSET              7
#define DWC2_GINTSTS_I2CINTR                        (1 << 9)
#define DWC2_GINTSTS_I2CINTR_OFFSET                 9
#define DWC2_GINTSTS_ERLYSUSPEND                    (1 << 10)
#define DWC2_GINTSTS_ERLYSUSPEND_OFFSET             10
#define DWC2_GINTSTS_USBSUSPEND                     (1 << 11)
#define DWC2_GINTSTS_USBSUSPEND_OFFSET              11
#define DWC2_GINTSTS_USBRESET                       (1 << 12)
#define DWC2_GINTSTS_USBRESET_OFFSET                12
#define DWC2_GINTSTS_ENUMDONE                       (1 << 13)
#define DWC2_GINTSTS_ENUMDONE_OFFSET                13
#define DWC2_GINTSTS_ISOOUTDROP                     (1 << 14)
#define DWC2_GINTSTS_ISOOUTDROP_OFFSET              14
#define DWC2_GINTSTS_EOPFRAME                       (1 << 15)
#define DWC2_GINTSTS_EOPFRAME_OFFSET                15
#define DWC2_GINTSTS_INTOKENRX                      (1 << 16)
#define DWC2_GINTSTS_INTOKENRX_OFFSET               16
#define DWC2_GINTSTS_EPMISMATCH                     (1 << 17)
#define DWC2_GINTSTS_EPMISMATCH_OFFSET              17
#define DWC2_GINTSTS_INEPINT                        (1 << 18)
#define DWC2_GINTSTS_INEPINT_OFFSET                 18
#define DWC2_GINTSTS_OUTEPINTR                      (1 << 19)
#define DWC2_GINTSTS_OUTEPINTR_OFFSET               19
#define DWC2_GINTSTS_INCOMPLISOIN                   (1 << 20)
#define DWC2_GINTSTS_INCOMPLISOIN_OFFSET            20
#define DWC2_GINTSTS_INCOMPLISOOUT                  (1 << 21)
#define DWC2_GINTSTS_INCOMPLISOOUT_OFFSET           21
#define DWC2_GINTSTS_PORTINTR                       (1 << 24)
#define DWC2_GINTSTS_PORTINTR_OFFSET                24
#define DWC2_GINTSTS_HCINTR                         (1 << 25)
#define DWC2_GINTSTS_HCINTR_OFFSET                  25
#define DWC2_GINTSTS_PTXFEMPTY                      (1 << 26)
#define DWC2_GINTSTS_PTXFEMPTY_OFFSET               26
#define DWC2_GINTSTS_LPMTRANRCVD                    (1 << 27)
#define DWC2_GINTSTS_LPMTRANRCVD_OFFSET             27
#define DWC2_GINTSTS_CONIDSTSCHNG                   (1 << 28)
#define DWC2_GINTSTS_CONIDSTSCHNG_OFFSET            28
#define DWC2_GINTSTS_DISCONNECT                     (1 << 29)
#define DWC2_GINTSTS_DISCONNECT_OFFSET              29
#define DWC2_GINTSTS_SESSREQINTR                    (1 << 30)
#define DWC2_GINTSTS_SESSREQINTR_OFFSET             30
#define DWC2_GINTSTS_WKUPINTR                       (1 << 31)
#define DWC2_GINTSTS_WKUPINTR_OFFSET                31

#define DWC2_GRXSTS_EPNUM_MASK                      (0xF << 0)
#define DWC2_GRXSTS_EPNUM_OFFSET                    0
#define DWC2_GRXSTS_BCNT_MASK                       (0x7FF << 4)
#define DWC2_GRXSTS_BCNT_OFFSET                     4
#define DWC2_GRXSTS_DPID_MASK                       (0x3 << 15)
#define DWC2_GRXSTS_DPID_OFFSET                     15
#define DWC2_GRXSTS_PKTSTS_MASK                     (0xF << 17)
#define DWC2_GRXSTS_PKTSTS_OFFSET                   17
#define DWC2_GRXSTS_FN_MASK                         (0xF << 21)
#define DWC2_GRXSTS_FN_OFFSET                       21

#define DWC2_FIFOSIZE_STARTADDR_MASK                (0xFFFF << 0)
#define DWC2_FIFOSIZE_STARTADDR_OFFSET              0
#define DWC2_FIFOSIZE_DEPTH_MASK                    (0xFFFF << 16)
#define DWC2_FIFOSIZE_DEPTH_OFFSET                  16

#define DWC2_GNPTXSTS_NPTXFSPCAVAIL_MASK            (0xFFFF << 0)
#define DWC2_GNPTXSTS_NPTXFSPCAVAIL_OFFSET          0
#define DWC2_GNPTXSTS_NPTXQSPCAVAIL_MASK            (0xFF << 16)
#define DWC2_GNPTXSTS_NPTXQSPCAVAIL_OFFSET          16
#define DWC2_GNPTXSTS_NPTXQTOP_TERMINATE            (1 << 24)
#define DWC2_GNPTXSTS_NPTXQTOP_TERMINATE_OFFSET     24
#define DWC2_GNPTXSTS_NPTXQTOP_TOKEN_MASK           (0x3 << 25)
#define DWC2_GNPTXSTS_NPTXQTOP_TOKEN_OFFSET         25
#define DWC2_GNPTXSTS_NPTXQTOP_CHNEP_MASK           (0xF << 27)
#define DWC2_GNPTXSTS_NPTXQTOP_CHNEP_OFFSET         27

#define DWC2_DTXFSTS_TXFSPCAVAIL_MASK               (0xFFFF << 0)
#define DWC2_DTXFSTS_TXFSPCAVAIL_OFFSET             0

#define DWC2_GI2CCTL_RWDATA_MASK                    (0xFF << 0)
#define DWC2_GI2CCTL_RWDATA_OFFSET                  0
#define DWC2_GI2CCTL_REGADDR_MASK                   (0xFF << 8)
#define DWC2_GI2CCTL_REGADDR_OFFSET                 8
#define DWC2_GI2CCTL_ADDR_MASK                      (0x7F << 16)
#define DWC2_GI2CCTL_ADDR_OFFSET                    16
#define DWC2_GI2CCTL_I2CEN                          (1 << 23)
#define DWC2_GI2CCTL_I2CEN_OFFSET                   23
#define DWC2_GI2CCTL_ACK                            (1 << 24)
#define DWC2_GI2CCTL_ACK_OFFSET                     24
#define DWC2_GI2CCTL_I2CSUSPCTL                     (1 << 25)
#define DWC2_GI2CCTL_I2CSUSPCTL_OFFSET              25
#define DWC2_GI2CCTL_I2CDEVADDR_MASK                (0x3 << 26)
#define DWC2_GI2CCTL_I2CDEVADDR_OFFSET              26
#define DWC2_GI2CCTL_RW                             (1 << 30)
#define DWC2_GI2CCTL_RW_OFFSET                      30
#define DWC2_GI2CCTL_BSYDNE                         (1 << 31)
#define DWC2_GI2CCTL_BSYDNE_OFFSET                  31

#define DWC2_HWCFG1_EP_DIR0_MASK                    (0x3 << 0)
#define DWC2_HWCFG1_EP_DIR0_OFFSET                  0
#define DWC2_HWCFG1_EP_DIR1_MASK                    (0x3 << 2)
#define DWC2_HWCFG1_EP_DIR1_OFFSET                  2
#define DWC2_HWCFG1_EP_DIR2_MASK                    (0x3 << 4)
#define DWC2_HWCFG1_EP_DIR2_OFFSET                  4
#define DWC2_HWCFG1_EP_DIR3_MASK                    (0x3 << 6)
#define DWC2_HWCFG1_EP_DIR3_OFFSET                  6
#define DWC2_HWCFG1_EP_DIR4_MASK                    (0x3 << 8)
#define DWC2_HWCFG1_EP_DIR4_OFFSET                  8
#define DWC2_HWCFG1_EP_DIR5_MASK                    (0x3 << 10)
#define DWC2_HWCFG1_EP_DIR5_OFFSET                  10
#define DWC2_HWCFG1_EP_DIR6_MASK                    (0x3 << 12)
#define DWC2_HWCFG1_EP_DIR6_OFFSET                  12
#define DWC2_HWCFG1_EP_DIR7_MASK                    (0x3 << 14)
#define DWC2_HWCFG1_EP_DIR7_OFFSET                  14
#define DWC2_HWCFG1_EP_DIR8_MASK                    (0x3 << 16)
#define DWC2_HWCFG1_EP_DIR8_OFFSET                  16
#define DWC2_HWCFG1_EP_DIR9_MASK                    (0x3 << 18)
#define DWC2_HWCFG1_EP_DIR9_OFFSET                  18
#define DWC2_HWCFG1_EP_DIR10_MASK                   (0x3 << 20)
#define DWC2_HWCFG1_EP_DIR10_OFFSET                 20
#define DWC2_HWCFG1_EP_DIR11_MASK                   (0x3 << 22)
#define DWC2_HWCFG1_EP_DIR11_OFFSET                 22
#define DWC2_HWCFG1_EP_DIR12_MASK                   (0x3 << 24)
#define DWC2_HWCFG1_EP_DIR12_OFFSET                 24
#define DWC2_HWCFG1_EP_DIR13_MASK                   (0x3 << 26)
#define DWC2_HWCFG1_EP_DIR13_OFFSET                 26
#define DWC2_HWCFG1_EP_DIR14_MASK                   (0x3 << 28)
#define DWC2_HWCFG1_EP_DIR14_OFFSET                 28
#define DWC2_HWCFG1_EP_DIR15_MASK                   (0x3 << 30)
#define DWC2_HWCFG1_EP_DIR15_OFFSET                 30

#define DWC2_HWCFG2_OP_MODE_MASK                    (0x7 << 0)
#define DWC2_HWCFG2_OP_MODE_OFFSET                  0
#define DWC2_HWCFG2_ARCHITECTURE_SLAVE_ONLY         (0x0 << 3)
#define DWC2_HWCFG2_ARCHITECTURE_EXT_DMA            (0x1 << 3)
#define DWC2_HWCFG2_ARCHITECTURE_INT_DMA            (0x2 << 3)
#define DWC2_HWCFG2_ARCHITECTURE_MASK               (0x3 << 3)
#define DWC2_HWCFG2_ARCHITECTURE_OFFSET             3
#define DWC2_HWCFG2_POINT2POINT                     (1 << 5)
#define DWC2_HWCFG2_POINT2POINT_OFFSET              5
#define DWC2_HWCFG2_HS_PHY_TYPE_MASK                (0x3 << 6)
#define DWC2_HWCFG2_HS_PHY_TYPE_OFFSET              6
#define DWC2_HWCFG2_FS_PHY_TYPE_MASK                (0x3 << 8)
#define DWC2_HWCFG2_FS_PHY_TYPE_OFFSET              8
#define DWC2_HWCFG2_NUM_DEV_EP_MASK                 (0xF << 10)
#define DWC2_HWCFG2_NUM_DEV_EP_OFFSET               10
#define DWC2_HWCFG2_NUM_HOST_CHAN_MASK              (0xF << 14)
#define DWC2_HWCFG2_NUM_HOST_CHAN_OFFSET            14
#define DWC2_HWCFG2_PERIO_EP_SUPPORTED              (1 << 18)
#define DWC2_HWCFG2_PERIO_EP_SUPPORTED_OFFSET       18
#define DWC2_HWCFG2_DYNAMIC_FIFO                    (1 << 19)
#define DWC2_HWCFG2_DYNAMIC_FIFO_OFFSET             19
#define DWC2_HWCFG2_MULTI_PROC_INT                  (1 << 20)
#define DWC2_HWCFG2_MULTI_PROC_INT_OFFSET           20
#define DWC2_HWCFG2_NONPERIO_TX_Q_DEPTH_MASK        (0x3 << 22)
#define DWC2_HWCFG2_NONPERIO_TX_Q_DEPTH_OFFSET      22
#define DWC2_HWCFG2_HOST_PERIO_TX_Q_DEPTH_MASK      (0x3 << 24)
#define DWC2_HWCFG2_HOST_PERIO_TX_Q_DEPTH_OFFSET    24
#define DWC2_HWCFG2_DEV_TOKEN_Q_DEPTH_MASK          (0x1F << 26)
#define DWC2_HWCFG2_DEV_TOKEN_Q_DEPTH_OFFSET        26

#define DWC2_HWCFG3_XFER_SIZE_CNTR_WIDTH_MASK       (0xF << 0)
#define DWC2_HWCFG3_XFER_SIZE_CNTR_WIDTH_OFFSET     0
#define DWC2_HWCFG3_PACKET_SIZE_CNTR_WIDTH_MASK     (0x7 << 4)
#define DWC2_HWCFG3_PACKET_SIZE_CNTR_WIDTH_OFFSET   4
#define DWC2_HWCFG3_OTG_FUNC                        (1 << 7)
#define DWC2_HWCFG3_OTG_FUNC_OFFSET                 7
#define DWC2_HWCFG3_I2C                             (1 << 8)
#define DWC2_HWCFG3_I2C_OFFSET                      8
#define DWC2_HWCFG3_VENDOR_CTRL_IF                  (1 << 9)
#define DWC2_HWCFG3_VENDOR_CTRL_IF_OFFSET           9
#define DWC2_HWCFG3_OPTIONAL_FEATURES               (1 << 10)
#define DWC2_HWCFG3_OPTIONAL_FEATURES_OFFSET        10
#define DWC2_HWCFG3_SYNCH_RESET_TYPE                (1 << 11)
#define DWC2_HWCFG3_SYNCH_RESET_TYPE_OFFSET         11
#define DWC2_HWCFG3_OTG_ENABLE_IC_USB               (1 << 12)
#define DWC2_HWCFG3_OTG_ENABLE_IC_USB_OFFSET        12
#define DWC2_HWCFG3_OTG_ENABLE_HSIC                 (1 << 13)
#define DWC2_HWCFG3_OTG_ENABLE_HSIC_OFFSET          13
#define DWC2_HWCFG3_OTG_LPM_EN                      (1 << 15)
#define DWC2_HWCFG3_OTG_LPM_EN_OFFSET               15
#define DWC2_HWCFG3_DFIFO_DEPTH_MASK                (0xFFFF << 16)
#define DWC2_HWCFG3_DFIFO_DEPTH_OFFSET              16

#define DWC2_HWCFG4_NUM_DEV_PERIO_IN_EP_MASK        (0xF << 0)
#define DWC2_HWCFG4_NUM_DEV_PERIO_IN_EP_OFFSET      0
#define DWC2_HWCFG4_POWER_OPTIMIZ                   (1 << 4)
#define DWC2_HWCFG4_POWER_OPTIMIZ_OFFSET            4
#define DWC2_HWCFG4_MIN_AHB_FREQ_MASK               (0x1FF << 5)
#define DWC2_HWCFG4_MIN_AHB_FREQ_OFFSET             5
#define DWC2_HWCFG4_UTMI_PHY_DATA_WIDTH_MASK        (0x3 << 14)
#define DWC2_HWCFG4_UTMI_PHY_DATA_WIDTH_OFFSET      14
#define DWC2_HWCFG4_NUM_DEV_MODE_CTRL_EP_MASK       (0xF << 16)
#define DWC2_HWCFG4_NUM_DEV_MODE_CTRL_EP_OFFSET     16
#define DWC2_HWCFG4_IDDIG_FILT_EN                   (1 << 20)
#define DWC2_HWCFG4_IDDIG_FILT_EN_OFFSET            20
#define DWC2_HWCFG4_VBUS_VALID_FILT_EN              (1 << 21)
#define DWC2_HWCFG4_VBUS_VALID_FILT_EN_OFFSET       21
#define DWC2_HWCFG4_A_VALID_FILT_EN                 (1 << 22)
#define DWC2_HWCFG4_A_VALID_FILT_EN_OFFSET          22
#define DWC2_HWCFG4_B_VALID_FILT_EN                 (1 << 23)
#define DWC2_HWCFG4_B_VALID_FILT_EN_OFFSET          23
#define DWC2_HWCFG4_SESSION_END_FILT_EN             (1 << 24)
#define DWC2_HWCFG4_SESSION_END_FILT_EN_OFFSET      24
#define DWC2_HWCFG4_DED_FIFO_EN                     (1 << 25)
#define DWC2_HWCFG4_DED_FIFO_EN_OFFSET              25
#define DWC2_HWCFG4_NUM_IN_EPS_MASK                 (0xF << 26)
#define DWC2_HWCFG4_NUM_IN_EPS_OFFSET               26
#define DWC2_HWCFG4_DESC_DMA                        (1 << 30)
#define DWC2_HWCFG4_DESC_DMA_OFFSET                 30
#define DWC2_HWCFG4_DESC_DMA_DYN                    (1 << 31)
#define DWC2_HWCFG4_DESC_DMA_DYN_OFFSET             31

#define DWC2_HCFG_FSLSPCLKSEL_30_60_MHZ             0
#define DWC2_HCFG_FSLSPCLKSEL_48_MHZ                1
#define DWC2_HCFG_FSLSPCLKSEL_6_MHZ                 2
#define DWC2_HCFG_FSLSPCLKSEL_MASK                  (0x3 << 0)
#define DWC2_HCFG_FSLSPCLKSEL_OFFSET                0
#define DWC2_HCFG_FSLSSUPP                          (1 << 2)
#define DWC2_HCFG_FSLSSUPP_OFFSET                   2
#define DWC2_HCFG_DESCDMA                           (1 << 23)
#define DWC2_HCFG_DESCDMA_OFFSET                    23
#define DWC2_HCFG_FRLISTEN_MASK                     (0x3 << 24)
#define DWC2_HCFG_FRLISTEN_OFFSET                   24
#define DWC2_HCFG_PERSCHEDENA                       (1 << 26)
#define DWC2_HCFG_PERSCHEDENA_OFFSET                26
#define DWC2_HCFG_PERSCHEDSTAT                      (1 << 27)
#define DWC2_HCFG_PERSCHEDSTAT_OFFSET               27

#define DWC2_HFIR_FRINT_MASK                        (0xFFFF << 0)
#define DWC2_HFIR_FRINT_OFFSET                      0

#define DWC2_HFNUM_FRNUM_MASK                       (0xFFFF << 0)
#define DWC2_HFNUM_FRNUM_OFFSET                     0
#define DWC2_HFNUM_FRREM_MASK                       (0xFFFF << 16)
#define DWC2_HFNUM_FRREM_OFFSET                     16
#define DWC2_HFNUM_MAX_FRNUM                        0x3FFF

#define DWC2_HPTXSTS_PTXFSPCAVAIL_MASK              (0xFFFF << 0)
#define DWC2_HPTXSTS_PTXFSPCAVAIL_OFFSET            0
#define DWC2_HPTXSTS_PTXQSPCAVAIL_MASK              (0xFF << 16)
#define DWC2_HPTXSTS_PTXQSPCAVAIL_OFFSET            16
#define DWC2_HPTXSTS_PTXQTOP_TERMINATE              (1 << 24)
#define DWC2_HPTXSTS_PTXQTOP_TERMINATE_OFFSET       24
#define DWC2_HPTXSTS_PTXQTOP_TOKEN_MASK             (0x3 << 25)
#define DWC2_HPTXSTS_PTXQTOP_TOKEN_OFFSET           25
#define DWC2_HPTXSTS_PTXQTOP_CHNUM_MASK             (0xF << 27)
#define DWC2_HPTXSTS_PTXQTOP_CHNUM_OFFSET           27
#define DWC2_HPTXSTS_PTXQTOP_ODD                    (1 << 31)
#define DWC2_HPTXSTS_PTXQTOP_ODD_OFFSET             31

#define DWC2_HPRT0_PRTCONNSTS                       (1 << 0)
#define DWC2_HPRT0_PRTCONNSTS_OFFSET                0
#define DWC2_HPRT0_PRTCONNDET                       (1 << 1)
#define DWC2_HPRT0_PRTCONNDET_OFFSET                1
#define DWC2_HPRT0_PRTENA                           (1 << 2)
#define DWC2_HPRT0_PRTENA_OFFSET                    2
#define DWC2_HPRT0_PRTENCHNG                        (1 << 3)
#define DWC2_HPRT0_PRTENCHNG_OFFSET                 3
#define DWC2_HPRT0_PRTOVRCURRACT                    (1 << 4)
#define DWC2_HPRT0_PRTOVRCURRACT_OFFSET             4
#define DWC2_HPRT0_PRTOVRCURRCHNG                   (1 << 5)
#define DWC2_HPRT0_PRTOVRCURRCHNG_OFFSET            5
#define DWC2_HPRT0_PRTRES                           (1 << 6)
#define DWC2_HPRT0_PRTRES_OFFSET                    6
#define DWC2_HPRT0_PRTSUSP                          (1 << 7)
#define DWC2_HPRT0_PRTSUSP_OFFSET                   7
#define DWC2_HPRT0_PRTRST                           (1 << 8)
#define DWC2_HPRT0_PRTRST_OFFSET                    8
#define DWC2_HPRT0_PRTLNSTS_MASK                    (0x3 << 10)
#define DWC2_HPRT0_PRTLNSTS_OFFSET                  10
#define DWC2_HPRT0_PRTPWR                           (1 << 12)
#define DWC2_HPRT0_PRTPWR_OFFSET                    12
#define DWC2_HPRT0_PRTTSTCTL_MASK                   (0xF << 13)
#define DWC2_HPRT0_PRTTSTCTL_OFFSET                 13
#define DWC2_HPRT0_PRTSPD_HIGH                      (0 << 17)
#define DWC2_HPRT0_PRTSPD_FULL                      (1 << 17)
#define DWC2_HPRT0_PRTSPD_LOW                       (2 << 17)
#define DWC2_HPRT0_PRTSPD_MASK                      (0x3 << 17)
#define DWC2_HPRT0_PRTSPD_OFFSET                    17

#define DWC2_HAINT_CH0                              (1 << 0)
#define DWC2_HAINT_CH0_OFFSET                       0
#define DWC2_HAINT_CH1                              (1 << 1)
#define DWC2_HAINT_CH1_OFFSET                       1
#define DWC2_HAINT_CH2                              (1 << 2)
#define DWC2_HAINT_CH2_OFFSET                       2
#define DWC2_HAINT_CH3                              (1 << 3)
#define DWC2_HAINT_CH3_OFFSET                       3
#define DWC2_HAINT_CH4                              (1 << 4)
#define DWC2_HAINT_CH4_OFFSET                       4
#define DWC2_HAINT_CH5                              (1 << 5)
#define DWC2_HAINT_CH5_OFFSET                       5
#define DWC2_HAINT_CH6                              (1 << 6)
#define DWC2_HAINT_CH6_OFFSET                       6
#define DWC2_HAINT_CH7                              (1 << 7)
#define DWC2_HAINT_CH7_OFFSET                       7
#define DWC2_HAINT_CH8                              (1 << 8)
#define DWC2_HAINT_CH8_OFFSET                       8
#define DWC2_HAINT_CH9                              (1 << 9)
#define DWC2_HAINT_CH9_OFFSET                       9
#define DWC2_HAINT_CH10                             (1 << 10)
#define DWC2_HAINT_CH10_OFFSET                      10
#define DWC2_HAINT_CH11                             (1 << 11)
#define DWC2_HAINT_CH11_OFFSET                      11
#define DWC2_HAINT_CH12                             (1 << 12)
#define DWC2_HAINT_CH12_OFFSET                      12
#define DWC2_HAINT_CH13                             (1 << 13)
#define DWC2_HAINT_CH13_OFFSET                      13
#define DWC2_HAINT_CH14                             (1 << 14)
#define DWC2_HAINT_CH14_OFFSET                      14
#define DWC2_HAINT_CH15                             (1 << 15)
#define DWC2_HAINT_CH15_OFFSET                      15
#define DWC2_HAINT_CHINT_MASK                       0xffff
#define DWC2_HAINT_CHINT_OFFSET                     0

#define DWC2_HAINTMSK_CH0                           (1 << 0)
#define DWC2_HAINTMSK_CH0_OFFSET                    0
#define DWC2_HAINTMSK_CH1                           (1 << 1)
#define DWC2_HAINTMSK_CH1_OFFSET                    1
#define DWC2_HAINTMSK_CH2                           (1 << 2)
#define DWC2_HAINTMSK_CH2_OFFSET                    2
#define DWC2_HAINTMSK_CH3                           (1 << 3)
#define DWC2_HAINTMSK_CH3_OFFSET                    3
#define DWC2_HAINTMSK_CH4                           (1 << 4)
#define DWC2_HAINTMSK_CH4_OFFSET                    4
#define DWC2_HAINTMSK_CH5                           (1 << 5)
#define DWC2_HAINTMSK_CH5_OFFSET                    5
#define DWC2_HAINTMSK_CH6                           (1 << 6)
#define DWC2_HAINTMSK_CH6_OFFSET                    6
#define DWC2_HAINTMSK_CH7                           (1 << 7)
#define DWC2_HAINTMSK_CH7_OFFSET                    7
#define DWC2_HAINTMSK_CH8                           (1 << 8)
#define DWC2_HAINTMSK_CH8_OFFSET                    8
#define DWC2_HAINTMSK_CH9                           (1 << 9)
#define DWC2_HAINTMSK_CH9_OFFSET                    9
#define DWC2_HAINTMSK_CH10                          (1 << 10)
#define DWC2_HAINTMSK_CH10_OFFSET                   10
#define DWC2_HAINTMSK_CH11                          (1 << 11)
#define DWC2_HAINTMSK_CH11_OFFSET                   11
#define DWC2_HAINTMSK_CH12                          (1 << 12)
#define DWC2_HAINTMSK_CH12_OFFSET                   12
#define DWC2_HAINTMSK_CH13                          (1 << 13)
#define DWC2_HAINTMSK_CH13_OFFSET                   13
#define DWC2_HAINTMSK_CH14                          (1 << 14)
#define DWC2_HAINTMSK_CH14_OFFSET                   14
#define DWC2_HAINTMSK_CH15                          (1 << 15)
#define DWC2_HAINTMSK_CH15_OFFSET                   15
#define DWC2_HAINTMSK_CHINT_MASK                    0xffff
#define DWC2_HAINTMSK_CHINT_OFFSET                  0

#define DWC2_HCCHAR_MPS_MASK                        (0x7FF << 0)
#define DWC2_HCCHAR_MPS_OFFSET                      0
#define DWC2_HCCHAR_EPNUM_MASK                      (0xF << 11)
#define DWC2_HCCHAR_EPNUM_OFFSET                    11
#define DWC2_HCCHAR_EPDIR                           (1 << 15)
#define DWC2_HCCHAR_EPDIR_OFFSET                    15
#define DWC2_HCCHAR_LSPDDEV                         (1 << 17)
#define DWC2_HCCHAR_LSPDDEV_OFFSET                  17
#define DWC2_HCCHAR_EPTYPE_CONTROL                  0
#define DWC2_HCCHAR_EPTYPE_ISOC                     1
#define DWC2_HCCHAR_EPTYPE_BULK                     2
#define DWC2_HCCHAR_EPTYPE_INTR                     3
#define DWC2_HCCHAR_EPTYPE_MASK                     (0x3 << 18)
#define DWC2_HCCHAR_EPTYPE_OFFSET                   18
#define DWC2_HCCHAR_MULTICNT_MASK                   (0x3 << 20)
#define DWC2_HCCHAR_MULTICNT_OFFSET                 20
#define DWC2_HCCHAR_DEVADDR_MASK                    (0x7F << 22)
#define DWC2_HCCHAR_DEVADDR_OFFSET                  22
#define DWC2_HCCHAR_ODDFRM                          (1 << 29)
#define DWC2_HCCHAR_ODDFRM_OFFSET                   29
#define DWC2_HCCHAR_CHDIS                           (1 << 30)
#define DWC2_HCCHAR_CHDIS_OFFSET                    30
#define DWC2_HCCHAR_CHEN                            (1 << 31)
#define DWC2_HCCHAR_CHEN_OFFSET                     31

#define DWC2_HCSPLT_PRTADDR_MASK                    (0x7F << 0)
#define DWC2_HCSPLT_PRTADDR_OFFSET                  0
#define DWC2_HCSPLT_HUBADDR_MASK                    (0x7F << 7)
#define DWC2_HCSPLT_HUBADDR_OFFSET                  7
#define DWC2_HCSPLT_XACTPOS_MASK                    (0x3 << 14)
#define DWC2_HCSPLT_XACTPOS_OFFSET                  14
#define DWC2_HCSPLT_COMPSPLT                        (1 << 16)
#define DWC2_HCSPLT_COMPSPLT_OFFSET                 16
#define DWC2_HCSPLT_SPLTENA                         (1 << 31)
#define DWC2_HCSPLT_SPLTENA_OFFSET                  31

#define DWC2_HCINT_XFERCOMP                         (1 << 0)
#define DWC2_HCINT_XFERCOMP_OFFSET                  0
#define DWC2_HCINT_CHHLTD                           (1 << 1)
#define DWC2_HCINT_CHHLTD_OFFSET                    1
#define DWC2_HCINT_AHBERR                           (1 << 2)
#define DWC2_HCINT_AHBERR_OFFSET                    2
#define DWC2_HCINT_STALL                            (1 << 3)
#define DWC2_HCINT_STALL_OFFSET                     3
#define DWC2_HCINT_NAK                              (1 << 4)
#define DWC2_HCINT_NAK_OFFSET                       4
#define DWC2_HCINT_ACK                              (1 << 5)
#define DWC2_HCINT_ACK_OFFSET                       5
#define DWC2_HCINT_NYET                             (1 << 6)
#define DWC2_HCINT_NYET_OFFSET                      6
#define DWC2_HCINT_XACTERR                          (1 << 7)
#define DWC2_HCINT_XACTERR_OFFSET                   7
#define DWC2_HCINT_BBLERR                           (1 << 8)
#define DWC2_HCINT_BBLERR_OFFSET                    8
#define DWC2_HCINT_FRMOVRUN                         (1 << 9)
#define DWC2_HCINT_FRMOVRUN_OFFSET                  9
#define DWC2_HCINT_DATATGLERR                       (1 << 10)
#define DWC2_HCINT_DATATGLERR_OFFSET                10
#define DWC2_HCINT_BNA                              (1 << 11)
#define DWC2_HCINT_BNA_OFFSET                       11
#define DWC2_HCINT_XCS_XACT                         (1 << 12)
#define DWC2_HCINT_XCS_XACT_OFFSET                  12
#define DWC2_HCINT_FRM_LIST_ROLL                    (1 << 13)
#define DWC2_HCINT_FRM_LIST_ROLL_OFFSET             13

#define DWC2_HCINTMSK_XFERCOMPL                     (1 << 0)
#define DWC2_HCINTMSK_XFERCOMPL_OFFSET              0
#define DWC2_HCINTMSK_CHHLTD                        (1 << 1)
#define DWC2_HCINTMSK_CHHLTD_OFFSET                 1
#define DWC2_HCINTMSK_AHBERR                        (1 << 2)
#define DWC2_HCINTMSK_AHBERR_OFFSET                 2
#define DWC2_HCINTMSK_STALL                         (1 << 3)
#define DWC2_HCINTMSK_STALL_OFFSET                  3
#define DWC2_HCINTMSK_NAK                           (1 << 4)
#define DWC2_HCINTMSK_NAK_OFFSET                    4
#define DWC2_HCINTMSK_ACK                           (1 << 5)
#define DWC2_HCINTMSK_ACK_OFFSET                    5
#define DWC2_HCINTMSK_NYET                          (1 << 6)
#define DWC2_HCINTMSK_NYET_OFFSET                   6
#define DWC2_HCINTMSK_XACTERR                       (1 << 7)
#define DWC2_HCINTMSK_XACTERR_OFFSET                7
#define DWC2_HCINTMSK_BBLERR                        (1 << 8)
#define DWC2_HCINTMSK_BBLERR_OFFSET                 8
#define DWC2_HCINTMSK_FRMOVRUN                      (1 << 9)
#define DWC2_HCINTMSK_FRMOVRUN_OFFSET               9
#define DWC2_HCINTMSK_DATATGLERR                    (1 << 10)
#define DWC2_HCINTMSK_DATATGLERR_OFFSET             10
#define DWC2_HCINTMSK_BNA                           (1 << 11)
#define DWC2_HCINTMSK_BNA_OFFSET                    11
#define DWC2_HCINTMSK_XCS_XACT                      (1 << 12)
#define DWC2_HCINTMSK_XCS_XACT_OFFSET               12
#define DWC2_HCINTMSK_FRM_LIST_ROLL                 (1 << 13)
#define DWC2_HCINTMSK_FRM_LIST_ROLL_OFFSET          13

#define DWC2_HCTSIZ_XFERSIZE_MASK                   0x7ffff
#define DWC2_HCTSIZ_XFERSIZE_OFFSET                 0
#define DWC2_HCTSIZ_SCHINFO_MASK                    0xff
#define DWC2_HCTSIZ_SCHINFO_OFFSET                  0
#define DWC2_HCTSIZ_NTD_MASK                        (0xff << 8)
#define DWC2_HCTSIZ_NTD_OFFSET                      8
#define DWC2_HCTSIZ_PKTCNT_MASK                     (0x3ff << 19)
#define DWC2_HCTSIZ_PKTCNT_OFFSET                   19
#define DWC2_HCTSIZ_PID_MASK                        (0x3 << 29)
#define DWC2_HCTSIZ_PID_OFFSET                      29
#define DWC2_HCTSIZ_DOPNG                           (1 << 31)
#define DWC2_HCTSIZ_DOPNG_OFFSET                    31

#define DWC2_HCDMA_CTD_MASK                         (0xFF << 3)
#define DWC2_HCDMA_CTD_OFFSET                       3
#define DWC2_HCDMA_DMA_ADDR_MASK                    (0x1FFFFF << 11)
#define DWC2_HCDMA_DMA_ADDR_OFFSET                  11

#define DWC2_PCGCCTL_STOPPCLK                       (1 << 0)
#define DWC2_PCGCCTL_STOPPCLK_OFFSET                0
#define DWC2_PCGCCTL_GATEHCLK                       (1 << 1)
#define DWC2_PCGCCTL_GATEHCLK_OFFSET                1
#define DWC2_PCGCCTL_PWRCLMP                        (1 << 2)
#define DWC2_PCGCCTL_PWRCLMP_OFFSET                 2
#define DWC2_PCGCCTL_RSTPDWNMODULE                  (1 << 3)
#define DWC2_PCGCCTL_RSTPDWNMODULE_OFFSET           3
#define DWC2_PCGCCTL_PHYSUSPENDED                   (1 << 4)
#define DWC2_PCGCCTL_PHYSUSPENDED_OFFSET            4
#define DWC2_PCGCCTL_ENBL_SLEEP_GATING              (1 << 5)
#define DWC2_PCGCCTL_ENBL_SLEEP_GATING_OFFSET       5
#define DWC2_PCGCCTL_PHY_IN_SLEEP                   (1 << 6)
#define DWC2_PCGCCTL_PHY_IN_SLEEP_OFFSET            6
#define DWC2_PCGCCTL_DEEP_SLEEP                     (1 << 7)
#define DWC2_PCGCCTL_DEEP_SLEEP_OFFSET              7

#define DWC2_SNPSID_DEVID_VER_2xx                   (0x4f542 << 12)
#define DWC2_SNPSID_DEVID_VER_3xx                   (0x4f543 << 12)
#define DWC2_SNPSID_DEVID_MASK                      (0xfffff << 12)
#define DWC2_SNPSID_DEVID_OFFSET                    12

/* Host controller specific */
#define DWC2_HC_PID_DATA0       0
#define DWC2_HC_PID_DATA2       1
#define DWC2_HC_PID_DATA1       2
#define DWC2_HC_PID_MDATA       3
#define DWC2_HC_PID_SETUP       3

/* roothub.a masks */
#define RH_A_NDP                (0xff << 0)     /* number of downstream ports */
#define RH_A_PSM                (1 << 8)        /* power switching mode */
#define RH_A_NPS                (1 << 9)        /* no power switching */
#define RH_A_DT                 (1 << 10)       /* device type (mbz) */
#define RH_A_OCPM               (1 << 11)       /* over current protection mode */
#define RH_A_NOCP               (1 << 12)       /* no over current protection */
#define RH_A_POTPGT             (0xff << 24)    /* power on to power good time */

/* roothub.b masks */
#define RH_B_DR                 0x0000ffff      /* device removable flags */
#define RH_B_PPCM               0xffff0000      /* port power control mask */

/* Default driver configuration */
#define CONFIG_DWC2_DMA_ENABLE
#define CONFIG_DWC2_DMA_BURST_SIZE           32     /* DMA burst len */
#undef CONFIG_DWC2_DFLT_SPEED_FULL                  /* Do not force DWC2 to FS */
#define CONFIG_DWC2_ENABLE_DYNAMIC_FIFO             /* Runtime FIFO size detect */
#define CONFIG_DWC2_MAX_CHANNELS             16     /* Max # of EPs */
#define CONFIG_DWC2_HOST_RX_FIFO_SIZE        (516 + CONFIG_DWC2_MAX_CHANNELS)
#define CONFIG_DWC2_HOST_NPERIO_TX_FIFO_SIZE 0x100  /* nPeriodic TX FIFO */
#define CONFIG_DWC2_HOST_PERIO_TX_FIFO_SIZE  0x200  /* Periodic TX FIFO */
#define CONFIG_DWC2_MAX_TRANSFER_SIZE        65535
#define CONFIG_DWC2_MAX_PACKET_COUNT         511

#define DWC2_PHY_TYPE_FS            0
#define DWC2_PHY_TYPE_UTMI          1
#define DWC2_PHY_TYPE_ULPI          2
#define CONFIG_DWC2_PHY_TYPE        DWC2_PHY_TYPE_UTMI  /* PHY type */
#ifndef CONFIG_DWC2_UTMI_WIDTH
#define CONFIG_DWC2_UTMI_WIDTH      8           /* UTMI bus width (8/16) */
#endif

#undef CONFIG_DWC2_PHY_ULPI_DDR                 /* ULPI PHY uses DDR mode */
#define CONFIG_DWC2_PHY_ULPI_EXT_VBUS           /* ULPI PHY controls VBUS */
#undef CONFIG_DWC2_I2C_ENABLE                   /* Enable I2C */
#undef CONFIG_DWC2_ULPI_FS_LS                   /* ULPI is FS/LS */
#undef CONFIG_DWC2_TS_DLINE                     /* External DLine pulsing */
#undef CONFIG_DWC2_THR_CTL                      /* Threshold control */
#define CONFIG_DWC2_TX_THR_LENGTH   64
#undef CONFIG_DWC2_IC_USB_CAP                   /* IC Cap */

#endif  /* __DWC2_H__ */


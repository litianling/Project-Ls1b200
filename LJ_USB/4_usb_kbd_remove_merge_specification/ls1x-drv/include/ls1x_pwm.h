/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_pwm.h
 *
 * created: 2020/6/27
 *  author: Bian
 */

/******************************************************************************
 * PWM ����ģʽ:
 *
 * 1. PULSE ���������Ļ��ߵ��ε������ƽ.
 *
 * 2. TIMER ���������Ļ��ߵ��εĶ�ʱ��, ��ʱ����ʱ�����ж�.
 *          a. API ��װ�Զ����ж�����Ӧ��ʱ���ж�;
 *          b. API ʹ�ûص���������Ӧ��ʱ���ж�;
 *          c. API ��RTOS��ʹ�� event ����׽��ʱ���ж�.
 *
 *    TIMER �ж���Ӧa/b/c֮һ, ���ȼ�: a->b->c
 *
 *    TODO  ��������ʱ��ʱ������, ��Щ��ֵ����ʹ��!!!
 *
 ******************************************************************************/

#ifndef _LS1X_PWM_H
#define _LS1X_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls1x_io.h"

//-----------------------------------------------------------------------------

#ifdef BSP_USE_PWM0
extern void *devPWM0;
#endif
#ifdef BSP_USE_PWM1
extern void *devPWM1;
#endif
#ifdef BSP_USE_PWM2
extern void *devPWM2;
#endif
#ifdef BSP_USE_PWM3
extern void *devPWM3;
#endif

//-----------------------------------------------------------------------------

/*
 * PWM ����ģʽ
 */
#define PWM_SINGLE_PULSE    0x01    // single pulse
#define PWM_SINGLE_TIMER    0x02    // single timer, interval = hi_ns
#define PWM_CONTINUE_PULSE  0x04    // continue pulse
#define PWM_CONTINUE_TIMER  0x08    // continue timer, interval = hi_ns

/*
 * PWM Timer �жϴ����ص�����
 */
typedef void (*pwmtimer_callback_t)(void *pwm, int *stopit);

/*
 * PWM open �� arg ����
 */
typedef struct pwm_cfg
{
    unsigned int hi_ns;             /* high level nano seconds,  ��ʱ��ģʽ���� hi_ns */
    unsigned int lo_ns;             /* low  level nano seconds,  ��ʱ��ģʽû�� lo_ns */
    int          mode;              /* pulse or timer, see above ��ʱ������ģʽ */

    irq_handler_t       isr;        /* User defined interrupt handler */
    pwmtimer_callback_t cb;         /* called by interrupt handler */
#if BSP_USE_OS
    void               *event;      /* RTOS event created by user */
#endif
} pwm_cfg_t;

//-----------------------------------------------------------------------------
// driver operator
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_pwm_drv_ops;

#define ls1x_pwm_init(pwm, arg)         ls1x_pwm_drv_ops->init_entry(pwm, arg)
#define ls1x_pwm_open(pwm, arg)         ls1x_pwm_drv_ops->open_entry(pwm, arg)
#define ls1x_pwm_close(pwm, arg)        ls1x_pwm_drv_ops->close_entry(pwm, arg)

#else

int LS1x_PWM_initialize(void *dev, void *arg);
int LS1x_PWM_open(void *dev, void *arg);
int LS1x_PWM_close(void *dev, void *arg);

#define ls1x_pwm_init(pwm, arg)         LS1x_PWM_initialize(pwm, arg)   // pwm_cfg_t *
#define ls1x_pwm_open(pwm, arg)         LS1x_PWM_open(pwm, arg)         // pwm_cfg_t *
#define ls1x_pwm_close(pwm, arg)        LS1x_PWM_close(pwm, arg)

#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

int ls1x_pwm_pulse_start(void *pwm, pwm_cfg_t *cfg);
int ls1x_pwm_pulse_stop(void *pwm);

int ls1x_pwm_timer_start(void *pwm, pwm_cfg_t *cfg);
int ls1x_pwm_timer_stop(void *pwm);

#if defined(OS_RTTHREAD)
const char *ls1x_pwm_get_device_name(void *pwm);
#endif

#ifdef __cplusplus
}
#endif

#endif // _LS1X_PWM_H

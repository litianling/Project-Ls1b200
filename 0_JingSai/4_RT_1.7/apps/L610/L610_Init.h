/*******************************************************************************
**                       @ Copyright 2014 - 2019                              **
**                           www.diysoon.com                                  **
**                                                                            **
*******************************************************************************/

#ifndef L610_INIT_H
#define L610_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

void l610_open_module(void);
void l610_wait_open(void);
void l610_init_pre(void);

void l610_init(void);
void l610_at_check(void);
void l610_deactive_pdp(void);
void l610_csq_check(void);
void l610_ati_check(void);
void l610_cpin_check(void);
void l610_gprs_check(void);
void l610_eps_check(void);
void l610_hex_set(void);

#ifdef __cplusplus
}
#endif

 #endif /*L610_Init_H */

/*******************************************************************************
**                       @ Copyright 2014 - 2020                              **
**                           www.diysoon.com                                  **
**                                                                            **
*******************************************************************************/

#ifndef L610_TCP_H
#define L610_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

int send_at_command(const char *buf);
int send_at_command_TXcloud(const char *buf);
int wait_for_reply(const char *verstr, const char *endstr, int waitms);
int wait_for_reply_TXcloud(const char *verstr, const char *endstr, int waitms);

void l610_ip_allo(void);
void l610_check_socket(void);
void l610_creat_tcp(void);
void l610_send_data(char *data);
void l610_end_tcp(void);
void l610_ip_release(void);

#ifdef __cplusplus
}
#endif

#endif



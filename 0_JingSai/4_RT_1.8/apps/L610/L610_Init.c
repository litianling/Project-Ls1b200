
/******************************************************************************
 *                       @ Copyright 2014 - 2020                              *
 *                           www.diysoon.com                                  *
 ******************************************************************************/

#include <string.h>

#include "bsp.h"
#include "ns16550.h"

#include "L610_Init.h"
#include "L610_TCP.h"

//-----------------------------------------------------------------------------

/**
 * 模块开机函数
 * 模块上电后默认是关机状态，需要控制IO口输出电平控制模块开机
 */
void l610_open_module(void)
{
    //
}

/**
 * 等待模块开机指示
 */
void l610_wait_open(void)
{
    //
}

/**
 * 部分L610的阿里云软件版本在上电后会自动拨号，为避免影响部分应用场景，
 * 可在前置初始化中关闭拨号
 */
void l610_init_pre(void)
{	
	l610_at_check();       // 检测模块AT指令状态，回复OK可以正常发送AT指令，结束后清除buffer。     AT
	l610_csq_check();      // 检查模块信号值，如果信号值太差可能会导致联网失败                     AT+CSQ
//	l610_csq_check();
//	l610_csq_check();
	l610_ati_check();      // 查询模块固件版本号，便于问题分析，本步骤不强制                       ATI
	l610_cpin_check();     // 检查SIM卡状态，如果SIM卡状态不正常，模块就无法正常建立无线通信       AT+CPIN?
	l610_gprs_check();     // 查询GPRS服务是否可用,检查模块网络是否正常                            AT+CGREG?
	l610_eps_check();      // 查询EPS服务是否可用,检查模块网络是否正常                             AT+CEREG?
	l610_deactive_pdp();   // 部分L610的阿里云软件版本在上电后会自动拨号，可以在上电后发AT+MIPCALL=0断开拨号。
                           // 简单发2次，不做执行结果判断。                                        AT+MIPCALL=0
}

/**
 * 模块初始化函数，通过AT指令对模块各个部分进行初始化，
 * 初始化结束后模块可以通过TCP协议向服务器发送数据。
 * 最后是请求运营商分配IP
 */
void l610_init(void)
{	
	l610_hex_set();        // 设置当模块收到服务器的数据时，上报数据的的格式。默认 0，掉电保存。 AT+GTSET="IPRFMT",0
	delay_ms(10);
	l610_ip_allo();        // 请求运营商分配IP AT+mipcall =
	delay_ms(10);
	l610_csq_check();      // 检查模块信号值，如果信号值太差可能会导致联网失败 AT+CSQ
	delay_ms(10);
}

/**
 * 检测模块AT指令状态，回复OK可以正常发送AT指令，结束后
 * 清除buffer。
 * 对应AT指令：AT
 */
void l610_at_check(void)
{
    int tmo = 0;
    send_at_command("AT\r\n");
    while ((wait_for_reply(NULL, "OK", 0) <= 0) && (tmo++ < 5))
        send_at_command("AT\r\n");
}

/**
 * 部分L610的阿里云软件版本在上电后会自动拨号，可以在上电后
 * 发AT+MIPCALL=0断开拨号。
 * 简单发2次，不做执行结果判断。
 * 本函数仅用于调试用途。
 */
void l610_deactive_pdp(void)
{
	send_at_command("AT+MIPCALL=0\r\n");
	wait_for_reply(NULL, "OK", 0); 
}

/**
 * 检查模块信号值，如果信号值太差可能会导致联网失败
 * 对应AT指令：AT+CSQ?
 */
void l610_csq_check(void)
{
    send_at_command("AT+CSQ\r\n");     
	wait_for_reply("+CSQ:", "OK", 0); 
}

/**
 * 查询模块固件版本号，便于问题分析，本步骤不强制
 * 对应AT指令：ATI
 */
void l610_ati_check(void)
{
	send_at_command("ATI\r\n");        // 检查模块的版本号
	wait_for_reply(NULL, "OK", 0);
}

/**
 * 检查SIM卡状态，如果SIM卡状态不正常，模块就无法正常建立无线通信
 * 对应AT指令 ：AT+CPIN?
 */
void l610_cpin_check(void)
{
    int tmo = 0;
	send_at_command("AT+CPIN?\r\n");       // 检查SIM卡是否在位,卡的缺口朝外放置
	while ((wait_for_reply("+CPIN: READY", "OK", 0) <= 0) && (tmo++ < 5))
        send_at_command("AT+CPIN?\r\n");
}

/**
 * 查询GPRS服务是否可用,检查模块网络是否正常
 * 对应AT指令：AT+CGREG?
 */
void l610_gprs_check(void)
{
    int tmo = 0;
	send_at_command("AT+CGREG?\r\n");      // 查看是否注册网络
	while ((wait_for_reply("+CGREG: 0", "OK", 0) <= 0) && (tmo++ < 5))
	    send_at_command("AT+CGREG?\r\n");
}

/**
 * 查询EPS服务是否可用,检查模块网络是否正常
 * 对应AT指令：AT+CEREG?
 */
void l610_eps_check(void)
{
    int tmo = 0;
	send_at_command("AT+CEREG?\r\n");      // 查看是否注册GSM网络
	while ((wait_for_reply("+CEREG: 0", "OK", 0) <= 0) && (tmo++ < 5))
	    send_at_command("AT+CGREG?\r\n");
}

/**
 * 设置当模块收到服务器的数据时，上报数据的的格式。默认 0，掉电保存。
 * 对应AT指令：AT+GTSET="IPRFMT",0
 */
void l610_hex_set(void)
{
    int tmo = 0;
	send_at_command("AT+GTSET=\"IPRFMT\",0\r\n");
	while ((wait_for_reply(NULL, "OK", 0) <= 0) && tmo++ < 5)
	    send_at_command("AT+GTSET=\"IPRFMT\",0\r\n");
}



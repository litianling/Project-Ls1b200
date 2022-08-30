
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
 * ģ�鿪������
 * ģ���ϵ��Ĭ���ǹػ�״̬����Ҫ����IO�������ƽ����ģ�鿪��
 */
void l610_open_module(void)
{
    //
}

/**
 * �ȴ�ģ�鿪��ָʾ
 */
void l610_wait_open(void)
{
    //
}

/**
 * ����L610�İ���������汾���ϵ����Զ����ţ�Ϊ����Ӱ�첿��Ӧ�ó�����
 * ����ǰ�ó�ʼ���йرղ���
 */
void l610_init_pre(void)
{	
	l610_at_check();       // ���ģ��ATָ��״̬���ظ�OK������������ATָ����������buffer��     AT
	l610_csq_check();      // ���ģ���ź�ֵ������ź�ֵ̫����ܻᵼ������ʧ��                     AT+CSQ
//	l610_csq_check();
//	l610_csq_check();
	l610_ati_check();      // ��ѯģ��̼��汾�ţ�������������������費ǿ��                       ATI
	l610_cpin_check();     // ���SIM��״̬�����SIM��״̬��������ģ����޷�������������ͨ��       AT+CPIN?
	l610_gprs_check();     // ��ѯGPRS�����Ƿ����,���ģ�������Ƿ�����                            AT+CGREG?
	l610_eps_check();      // ��ѯEPS�����Ƿ����,���ģ�������Ƿ�����                             AT+CEREG?
	l610_deactive_pdp();   // ����L610�İ���������汾���ϵ����Զ����ţ��������ϵ��AT+MIPCALL=0�Ͽ����š�
                           // �򵥷�2�Σ�����ִ�н���жϡ�                                        AT+MIPCALL=0
}

/**
 * ģ���ʼ��������ͨ��ATָ���ģ��������ֽ��г�ʼ����
 * ��ʼ��������ģ�����ͨ��TCPЭ����������������ݡ�
 * �����������Ӫ�̷���IP
 */
void l610_init(void)
{	
	l610_hex_set();        // ���õ�ģ���յ�������������ʱ���ϱ����ݵĵĸ�ʽ��Ĭ�� 0�����籣�档 AT+GTSET="IPRFMT",0
	delay_ms(10);
	l610_ip_allo();        // ������Ӫ�̷���IP AT+mipcall =
	delay_ms(10);
	l610_csq_check();      // ���ģ���ź�ֵ������ź�ֵ̫����ܻᵼ������ʧ�� AT+CSQ
	delay_ms(10);
}

/**
 * ���ģ��ATָ��״̬���ظ�OK������������ATָ�������
 * ���buffer��
 * ��ӦATָ�AT
 */
void l610_at_check(void)
{
    int tmo = 0;
    send_at_command("AT\r\n");
    while ((wait_for_reply(NULL, "OK", 0) <= 0) && (tmo++ < 5))
        send_at_command("AT\r\n");
}

/**
 * ����L610�İ���������汾���ϵ����Զ����ţ��������ϵ��
 * ��AT+MIPCALL=0�Ͽ����š�
 * �򵥷�2�Σ�����ִ�н���жϡ�
 * �����������ڵ�����;��
 */
void l610_deactive_pdp(void)
{
	send_at_command("AT+MIPCALL=0\r\n");
	wait_for_reply(NULL, "OK", 0); 
}

/**
 * ���ģ���ź�ֵ������ź�ֵ̫����ܻᵼ������ʧ��
 * ��ӦATָ�AT+CSQ?
 */
void l610_csq_check(void)
{
    send_at_command("AT+CSQ\r\n");     
	wait_for_reply("+CSQ:", "OK", 0); 
}

/**
 * ��ѯģ��̼��汾�ţ�������������������費ǿ��
 * ��ӦATָ�ATI
 */
void l610_ati_check(void)
{
	send_at_command("ATI\r\n");        // ���ģ��İ汾��
	wait_for_reply(NULL, "OK", 0);
}

/**
 * ���SIM��״̬�����SIM��״̬��������ģ����޷�������������ͨ��
 * ��ӦATָ�� ��AT+CPIN?
 */
void l610_cpin_check(void)
{
    int tmo = 0;
	send_at_command("AT+CPIN?\r\n");       // ���SIM���Ƿ���λ,����ȱ�ڳ������
	while ((wait_for_reply("+CPIN: READY", "OK", 0) <= 0) && (tmo++ < 5))
        send_at_command("AT+CPIN?\r\n");
}

/**
 * ��ѯGPRS�����Ƿ����,���ģ�������Ƿ�����
 * ��ӦATָ�AT+CGREG?
 */
void l610_gprs_check(void)
{
    int tmo = 0;
	send_at_command("AT+CGREG?\r\n");      // �鿴�Ƿ�ע������
	while ((wait_for_reply("+CGREG: 0", "OK", 0) <= 0) && (tmo++ < 5))
	    send_at_command("AT+CGREG?\r\n");
}

/**
 * ��ѯEPS�����Ƿ����,���ģ�������Ƿ�����
 * ��ӦATָ�AT+CEREG?
 */
void l610_eps_check(void)
{
    int tmo = 0;
	send_at_command("AT+CEREG?\r\n");      // �鿴�Ƿ�ע��GSM����
	while ((wait_for_reply("+CEREG: 0", "OK", 0) <= 0) && (tmo++ < 5))
	    send_at_command("AT+CGREG?\r\n");
}

/**
 * ���õ�ģ���յ�������������ʱ���ϱ����ݵĵĸ�ʽ��Ĭ�� 0�����籣�档
 * ��ӦATָ�AT+GTSET="IPRFMT",0
 */
void l610_hex_set(void)
{
    int tmo = 0;
	send_at_command("AT+GTSET=\"IPRFMT\",0\r\n");
	while ((wait_for_reply(NULL, "OK", 0) <= 0) && tmo++ < 5)
	    send_at_command("AT+GTSET=\"IPRFMT\",0\r\n");
}



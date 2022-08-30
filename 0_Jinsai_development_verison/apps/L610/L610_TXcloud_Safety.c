/*
 * L610_TXcloud_Safety.c
 *
 * created: 2022/5/28
 *  author: 
 */
 
#include <rtthread.h>
#include "ns16550.h"
#include "L610_include/TCP.h"
#include "ls1b_gpio.h"
#include "bsp.h"

#define  getch  usb_kbd_getc
#define  cmd_max_len 200
#define  SIMPLE_SHOW            1
#define  SIMPLE_DECODE          1

#define THREAD_PRIORITY         20
#define THREAD_STACK_SIZE       1024*4
#define THREAD_TIMESLICE        500
static rt_thread_t L610_TXcloud_safety1 = RT_NULL;

/********************************************************************************
    �豸����
********************************************************************************/

int SG180_angle = 0;
int buzzer_mode = 0;
int air_quality_old = 1;
int air_quality_new = 1;
int human_active_old = 0;
int human_active_new = 0;
double temperature_old = 0;
double temperature_new = 0;
int DQ_1 = 0;                   // ���ŵ�ƽ
int fg_1=1;        			    // �¶�������־

void init_sensor(void);
void Init_DS18B20_1(void);
unsigned char ReadOneChar_1(void);
void WriteOneChar_1(unsigned char dat);
double ReadTemperature_1(void);
int wait_for_reply_cloud_safety(const char *verstr, const char *endstr, int waitms);
int receive_SG180(char *tmp_scan);
int receive_BUZZER(char *tmp_scan);
/********************************************************************************
    �̺߳���
********************************************************************************/

void L610_TXcloud_safety_entry(void)
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    fb_cons_clear();

    init_sensor();                            // ��������ʼ��
    
    send_at_command("AT+MIPCALL=1\r\n");      // ����
    wait_for_reply(NULL, "OK", 0);
    delay_ms(100);

    send_at_command("AT+TCDEVINFOSET=1,\"YDTZU7FCM1\",\"Safety_Device_001\",\"y6Mokxr0cL454UjUogIFEg==\"\r\n");      // ����ƽ̨�豸��Ϣ����ƷID/�豸����/��Կ��
    wait_for_reply(NULL, "OK", 0);
    delay_ms(100);

    send_at_command("AT+TCMQTTCONN=1,20000,240,1,1\r\n");                                               // �������Ӳ���������
    wait_for_reply(NULL, "OK", 0);
    delay_ms(100);

    send_at_command("AT+TCMQTTSUB=\"$thing/down/property/YDTZU7FCM1/Safety_Device_001\",1\r\n");       // �����ϱ��������Ա�ǩ
    wait_for_reply(NULL, "OK", 0);
    delay_ms(1000);


    char cmd_air_quality[250];
    char cmd_human_active[250];
    char cmd_temperature[250];

#if (SIMPLE_SHOW)
    // ��ʼ��������������
    char a=92;
    snprintf(cmd_air_quality, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"AIR%c\":1}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a);
    send_at_command_TXcloud(cmd_air_quality);
    wait_for_reply_TXcloud(NULL, "OK", 0);
    delay_ms(100);
    printk("TX:Air quality is 1.\r\n");
    
    while(!(gpio_read(TOUCH)))
    {
        air_quality_new = gpio_read(Air_Q);                                                             // ���»�ȡ��������Ϣ
        human_active_new = gpio_read(MAN_S);
        temperature_new = ReadTemperature_1();

        if(air_quality_new != air_quality_old)                                                          // �Աȴ��������ݣ������仯���ϱ�
        {
            snprintf(cmd_air_quality, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"AIR%c\":%d}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,air_quality_new);
            send_at_command_TXcloud(cmd_air_quality);                                                   // ��������
            wait_for_reply_TXcloud(NULL, "OK", 1000);                                                   // L610ȷ��
            printk("TX:Air quality is %d.\r\n",air_quality_new);
            air_quality_old = air_quality_new;
        }

        if(human_active_new != human_active_old)
        {
            snprintf(cmd_human_active, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"MAN%c\":%d}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,human_active_new);
            send_at_command_TXcloud(cmd_human_active);
            wait_for_reply_TXcloud(NULL, "OK", 1000);
            printk("TX:Human active is %d.\r\n",human_active_new);
            human_active_old = human_active_new;
        }

        if(temperature_new != temperature_old)
        {
            snprintf(cmd_temperature, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"TEM%c\":%.2f}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,temperature_new);
            send_at_command_TXcloud(cmd_temperature);
            wait_for_reply_TXcloud(NULL, "OK", 1000);
            printk("TX:Temperature is %.2f.\r\n",temperature_new);
            temperature_old = temperature_new;
        }
        
        wait_for_reply_cloud_safety(NULL, NULL, 1000);                                                  // ����ȷ�����������
        delay_ms(10);
    }
#else
    // ��ʼ��������������
    snprintf(cmd_air_quality, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"AIR%c\":1}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a);
    send_at_command(cmd_air_quality);
    wait_for_reply(NULL, "OK", 0);
    delay_ms(100);

    while(!(gpio_read(TOUCH)))
    {
        air_quality_new = gpio_read(Air_Q);                                                             // ���»�ȡ��������Ϣ
        human_active_new = gpio_read(MAN_S);
        temperature_new = ReadTemperature_1();

        if(air_quality_new != air_quality_old)                                                          // �Աȴ��������ݣ������仯���ϱ�
        {
            snprintf(cmd_air_quality, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"AIR%c\":%d}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,air_quality_new);
            send_at_command(cmd_air_quality);
            wait_for_reply(NULL, "OK", 0);
            delay_ms(100);
            air_quality_old = air_quality_new;
        }

        if(human_active_new != human_active_old)
        {
            snprintf(cmd_human_active, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"MAN%c\":%d}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,human_active_new);
            send_at_command(cmd_human_active);
            wait_for_reply(NULL, "OK", 0);
            delay_ms(100);
            human_active_old = human_active_new;
        }

        if(temperature_new != temperature_old)
        {
            snprintf(cmd_temperature, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"TEM%c\":%.2f}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,temperature_new);
            send_at_command(cmd_temperature);
            wait_for_reply(NULL, "OK", 0);
            delay_ms(100);
            temperature_old = temperature_new;
        }

        wait_for_reply_cloud_safety(NULL, NULL, 3000);
        delay_ms(100);
    }
#endif

    gpio_write(BUZZER,0);                                                                               // �رշ�����
    send_at_command("AT+TCMQTTDISCONN\r\n");                                                            // �Ͽ�����
    wait_for_reply(NULL, "OK", 0);
    delay_ms(100);


    extern int show_background;
    show_background = 1;
    change_scheduler_lock(0);
    return;
}

int L610_TXcloud_safety(void)
{
    L610_TXcloud_safety1 = rt_thread_create("L610_TXcloud_safety",
                            L610_TXcloud_safety_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(L610_TXcloud_safety1);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(L610_TXcloud_safety, safety sensor);


/********************************************************************************
    �豸����
********************************************************************************/

void init_sensor(void)
{
    gpio_enable(Air_Q,DIR_IN);
    gpio_enable(SG180,DIR_OUT);
    gpio_enable(BUZZER,DIR_OUT);
    gpio_enable(MAN_S,DIR_IN);
    gpio_enable(TOUCH,DIR_IN);
    
    air_quality_new = gpio_read(Air_Q);
    air_quality_old = air_quality_new;
    human_active_new = gpio_read(MAN_S);
    human_active_old = human_active_new;
    temperature_new = ReadTemperature_1();
    temperature_old = temperature_new;
    return;
}

void Init_DS18B20_1(void)
{
	unsigned char x=0;
	gpio_enable(TEMPE,DIR_OUT);
	DQ_1=1; 				   //DQ_1���ø�
	gpio_write(TEMPE,DQ_1);
	delay_us(80);          //����ʱ
	DQ_1=0; 				   //���͸�λ����
	gpio_write(TEMPE,DQ_1);
	delay_us(800);         //��ʱ��>480us)
	DQ_1=1; 				   //����������
	gpio_write(TEMPE,DQ_1);
	delay_us(50);          //�ȴ���15~60us)

	gpio_enable(TEMPE,DIR_IN);
	DQ_1=gpio_read(TEMPE);
	x=DQ_1; 					//��X��ֵ���жϳ�ʼ����û�гɹ���18B20���ڵĻ�X=0������X=1
	//printk("x=%d\n\r",x);  // ע����ʱ
	delay_us(200);
}


unsigned char ReadOneChar_1(void)  			//�����������ȴӸ������͵�ƽ1us���ϣ���ʹ��������Ϊ�ߵ�ƽ���Ӷ��������ź�
{
	unsigned char i=0; 		//ÿ����������̵ĳ���ʱ��Ϊ60us������������֮�������1us���ϵĸߵ�ƽ�ָ���
	unsigned char dat=0;
	for (i=8;i>0;i--) 		//һ���ֽ���8λ
	{
	    gpio_enable(TEMPE,DIR_OUT);
		DQ_1=1;
		gpio_write(TEMPE,DQ_1);
		delay_us(10);
		DQ_1=0;
		gpio_write(TEMPE,DQ_1);
		dat>>=1;
		DQ_1=1;
		gpio_write(TEMPE,DQ_1);

		gpio_enable(TEMPE,DIR_IN);
		DQ_1=gpio_read(TEMPE);
		if(DQ_1)
		  dat|=0x80;
		delay_us(40);
	}
	return(dat);
}


void WriteOneChar_1(unsigned char dat)
{
	unsigned char i=0; 		//�����ߴӸߵ�ƽ�����͵�ƽ������д��ʼ�źš�15us֮�ڽ�����д��λ�͵��������ϣ�
	gpio_enable(TEMPE,DIR_OUT);
	for(i=8;i>0;i--) 		//��15~60us֮��������߽��в���������Ǹߵ�ƽ��д1����д0������
	{
		DQ_1=0; 				//�ڿ�ʼ��һ��д����ǰ������1us���ϵĸߵ�ƽ�ָ��ڡ�
		gpio_write(TEMPE,DQ_1);
		DQ_1=dat&0x01;
		gpio_write(TEMPE,DQ_1);
		delay_us(50);
		DQ_1=1;
		gpio_write(TEMPE,DQ_1);
		dat>>=1;
	}
	delay_us(40);
}


double ReadTemperature_1(void)
{
    unsigned char tempL=0; 		// ��ȫ�ֱ���
    unsigned char tempH=0;
    unsigned int  sdata;		// ���������¶ȵ���������
    unsigned char xiaoshu1;		// С����һλ
    unsigned char xiaoshu2;		// С���ڶ�λ
    unsigned char xiaoshu;		// ��λС��
    double temperature=0;
	Init_DS18B20_1(); 					//��ʼ��
	WriteOneChar_1(0xcc); 				//���������кŵĲ���
	WriteOneChar_1(0x44); 				//�����¶�ת��
	delay_us(1250);                     //ת����Ҫһ��ʱ�䣬��ʱ
	Init_DS18B20_1(); 					//��ʼ��
	WriteOneChar_1(0xcc); 				//���������кŵĲ���
	WriteOneChar_1(0xbe); 				//���¶ȼĴ�����ͷ����ֵ�ֱ�Ϊ�¶ȵĵ�λ�͸�λ��
	tempL=ReadOneChar_1(); 				//�����¶ȵĵ�λLSB
	tempH=ReadOneChar_1(); 				//�����¶ȵĸ�λMSB
	if(tempH>0x7f)      				//���λΪ1ʱ�¶��Ǹ�
	{
		tempL=~tempL;					//����ת����ȡ����һ
		tempH=~tempH+1;
		fg_1=0;      						//��ȡ�¶�Ϊ��ʱfg_1=0
	}
	sdata = tempL/16+tempH*16;      	//��������
	xiaoshu1 = (tempL&0x0f)*10/16; 		//С����һλ
	xiaoshu2 = (tempL&0x0f)*100/16%10;	//С���ڶ�λ
	xiaoshu=xiaoshu1*10+xiaoshu2; 		//С����λ
	temperature = sdata + xiaoshu*0.01;
	return temperature;
}

//********************************************************************************
//    �ƶ˽���
//********************************************************************************

int wait_for_reply_cloud_safety(const char *verstr, const char *endstr, int waitms)
{
    char tmp[400], *ptr = tmp;
    char *tmp_s = tmp;
    int  rt = 0, tmo;

    memset(tmp, 0, 400);                            // ���������, �������ݹ۲�
    tmo = waitms > 0 ? waitms : 1000;

    if ((endstr == NULL) || (strlen(endstr) == 0))  // û�����ý����ַ���ȫ������
    {
        rt = ls1x_uart_read(devUART1, ptr, 400, tmo);
    }
    else
    {
        while (tmo > 0)
        {
            //  ÿ�ζ�10���ֽ�, ��ʱ2ms: ��UART1���������.   
            int readed = ls1x_uart_read(devUART1, ptr, 10, 2);  // 10 bytes; 2 ms

            if (readed > 0)
            {
                ptr += readed;
                rt  += readed;

                if ((rt >= 400) || strstr(tmp, endstr))         // ���������ַ���
                    break;
            }

            tmo -= 2;
        }
    }

    if (rt <= 2)                                                // �ж��ǲ��ǳ�ʱ�˳�
        return -1;

    /* �����ַ������� "\r\n" ����, ������© */
    if ((tmp[rt-2] != '\r') && (tmp[rt-1] != '\n'))
    {
        rt += ls1x_uart_read(devUART1, ptr, 10, 2);
    }


    int offset=0;
#if (!SIMPLE_SHOW)
    printk(" RX: ");                                            // ������յ�����Ϣ******����г������ƣ���Ҫ�Ľ���������̶�ȡ��
    printk("%s\r\n", tmp);
    if(*(tmp+126))
        printk("%s\r\n", tmp+126);
    if(*(tmp+252))
        printk("%s\r\n", tmp+252);
    if(*(tmp+378))
        printk("%s\r\n", tmp+378);
#endif

    /* ɨ�貢������յ������� */
#if (SIMPLE_DECODE)
    int CMDval=-1,sg180_angle=-1,buzzer_flag=-1;
    int CMDnum = get_command_number_and_value(tmp_s,&CMDval);
    if (CMDval != -1)
    {
        if(CMDnum == 76)
            sg180_angle = CMDval;
        else if(CMDnum == 77)
            buzzer_flag = CMDval;
    }
#else
    int sg180_angle = receive_SG180(tmp_s+100);
    int buzzer_flag = receive_BUZZER(tmp_s+100); 
#endif

    /****  SG180��Ӧ����  ****/
    if(sg180_angle != -1)                                       // ������Ч����Ӧ
    {
        int i = 0 ,num = 350 + sg180_angle*450;
        for(;i<50;i++)  // SG180ת��1S��ָ��λ��
        {
            gpio_write(SG180,1);
            delay_us(num);
            gpio_write(SG180,0);
            delay_us(20000-num);
        }
        if(sg180_angle != SG180_angle)                          // SG180�Ƕȱ䶯�ϱ�
        {
            char cmd_sg180_angle[250];
            char a=92;
            snprintf(cmd_sg180_angle, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"SG%c\":%d}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,sg180_angle);
#if (SIMPLE_SHOW)
            send_at_command_TXcloud(cmd_sg180_angle);           // ����SG180�Ƕȱ䶯
            wait_for_reply_TXcloud(NULL, "OK", 1000);           // L610ȷ��
            wait_for_reply_TXcloud(NULL, NULL, 1000);           // �����ȷ��
            printk("TX:Sg180_angle is %d.\r\n",sg180_angle);
#else
            send_at_command(cmd_sg180_angle);
            wait_for_reply(NULL, "OK", 0);
            delay_ms(100);
#endif
            SG180_angle = sg180_angle;
        }
    }


    /****  BUZZER��Ӧ����  ****/
    if(buzzer_flag != -1)                                       // ������Ч����Ӧ
    {
        gpio_write(BUZZER,buzzer_flag);
        if(buzzer_flag != buzzer_mode)                          // �������䶯�ϱ�
        {
            char cmd_buzzer_flag[250];
            char a=92;
            snprintf(cmd_buzzer_flag, 249, "AT+TCMQTTPUB=\"$thing/up/property/YDTZU7FCM1/Safety_Device_001\",1,\"{%c\"method%c\":%c\"report%c\",%c\"clientToken%c\":%c\"123%c\",%c\"params%c\":{%c\"BUZ%c\":%d}}\"\r\n", a,a,a,a,a,a,a,a,a,a,a,a,buzzer_flag);
#if (SIMPLE_SHOW)
            send_at_command_TXcloud(cmd_buzzer_flag);           // ���ͷ������䶯
            wait_for_reply_TXcloud(NULL, "OK", 1000);           // L610ȷ��
            wait_for_reply_TXcloud(NULL, NULL, 1000);           // �����ȷ��
            printk("TX:Buzzer_mode is %d.\r\n",buzzer_flag);
#else
            send_at_command(cmd_buzzer_flag);
            wait_for_reply(NULL, "OK", 0);
            delay_ms(100);
#endif
            buzzer_mode = buzzer_flag;
        }
    }


    if ((verstr != NULL) && (strlen(verstr) > 0))               // �������ж��ַ�
    {
        if (strstr(tmp, verstr) != NULL)
            return rt;
        else
            return -1;
    }

    return rt;
}

#if (SIMPLE_DECODE)

int get_command_number_and_value(char *tmp_scan,int *val)
{
    int CMDnum = -1;
#if 0
    while(*(tmp_scan+4) != NULL)
    {
        if((*tmp_scan == ',')&&(*(tmp_scan+3) == ','))
        {
            if((*(tmp_scan+1) >= '0')&&(*(tmp_scan+1) <= '9')&&(*(tmp_scan+2) >= '0')&&(*(tmp_scan+2) <= '9'))
            {
                CMDnum = (*(tmp_scan+1)-48)*10 + (*(tmp_scan+2)-48);
                break;
            }
        }
        if((*tmp_scan == ',')&&(*(tmp_scan+4) == ','))
        {
            if((*(tmp_scan+1)>='0')&&(*(tmp_scan+1)<='9')&&(*(tmp_scan+2)>='0')&&(*(tmp_scan+2)<='9')&&(*(tmp_scan+3)>='0')&&(*(tmp_scan+3)<='9'))
            {
                CMDnum = (*(tmp_scan+1)-48)*100 + (*(tmp_scan+2)-48)*10 + (*(tmp_scan+3)-48);
                break;
            }
        }
        tmp_scan++;
    }
    printk("Command number is %d.\n\r",CMDnum);
    if((CMDnum!=76)&&(CMDnum!=77)&&(CMDnum!=102))               // SG180 ���� ����
        return CMDnum;
 #endif
    while(*(tmp_scan) != NULL)
    {
        tmp_scan++;
    }
/*****************����****************/
    if((*(tmp_scan-10)=='S')&&(*(tmp_scan-9)=='G'))
        CMDnum = 76;
    else if((*(tmp_scan-11)=='B')&&(*(tmp_scan-10)=='U')&&(*(tmp_scan-9)=='Z'))         // ����102
        CMDnum = 77;
    else
        return CMDnum;
/************************************/
    *val = *(tmp_scan-6) - 48;
    //printk("Command number is %d.\n\r",CMDnum);
    //printk("Command value is %d.\n\r",*val);

    return CMDnum;
}

#else

int receive_SG180(char *tmp_scan)
{
    int result = -1;
    while(*(tmp_scan+1) != NULL)
    {
        if((*tmp_scan == 'S')&&(*(tmp_scan+1) == 'G'))
        {
            result = *(tmp_scan+4) - 48;
            break;
        }
        tmp_scan++;
    }
#if (SIMPLE_SHOW)
    if (result!=-1)
        printk("RX:sg180_angle is %d\r\n",result);
#endif
    return result;
}

int receive_BUZZER(char *tmp_scan)
{
    int result = -1;
    while(*(tmp_scan+1) != NULL)
    {
        if((*tmp_scan == 'B')&&(*(tmp_scan+1) == 'U')&&(*(tmp_scan+2) == 'Z'))
        {
            result = *(tmp_scan+5) - 48;
            break;
        }
        tmp_scan++;
    }
#if (SIMPLE_SHOW)
    if (result!=-1)
        printk("RX:buzzer_flag is %d\r\n",result);
#endif
    return result;
}

#endif


#include <stdio.h>
#include "ls1b.h"
#include "mips.h"
#include "ls1b_gpio.h"
#include "stdlib.h"
#include "string.h"

#define u8  unsigned char
#define u16 unsigned int

#define BTN1 36
#define BTN2 34
#define BTN3 37
#define BTN4 35

#define BTN5 43
#define BTN6 45
#define BTN7 46
#define BTN8 47
//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"
#define BSP_USE_FB
#define XPT2046_DRV

#ifdef BSP_USE_FB
  #include "ls1x_fb.h"
  #ifdef XPT2046_DRV
    char LCD_display_mode[] = LCD_800x480;
  #elif defined(GT1151_DRV)
    char LCD_display_mode[] = LCD_480x800;
  #else
    #error "��bsp.h��ѡ������ XPT2046_DRV ���� GT1151_DRV"
           "XPT2046_DRV:  ����800*480 �����Ĵ�����."
           "GT1151_DRV:   ����480*800 �����Ĵ�����."
           "�������ѡ��, ע�͵��� error ��Ϣ, Ȼ���Զ���: LCD_display_mode[]"
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// ��������
//-------------------------------------------------------------------------------------------------

int TCSK[24][40]=  //̰��ģʽ�Դ� 24�� 40�� ����ֵ��ͬ��ɫ��ͬ��
{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

const int flg[4][2]={{1,0},{0,1},{0,-1},{-1,0}};//������������仯����
int n=24,m=40;  //n��m���ߵĻ��Χ20��20��+�߿�=���ӻ����ڴ�С
struct xcw{int x,y;}tang[5][15],foodd[2];//�ṹ��tan�洢ÿ���ߵ�����  �ṹ��foodd��¼����ʳ������
int tot[5];     //tot[0123]�������ߵĳ���
int f[5];       //�洢�ߵ�����״̬0123��������   ����flgʹ��
int score;      //�÷�
int food;       //ʳ�����
int ms=1;       //��ʼ--ģʽѡ��
int ans_len=1e9;
int vis[20][20];//��¼ʳ����ڵ�ľ���  ����(foddd[0].x,foddd[0].y)��ʳ����vis[foddd[0].x][foddd[0].y]=1
int start_clock=0;  //��ʼ��

//-------------------------------------------------------------------------------------------------
// ��������
//-------------------------------------------------------------------------------------------------

void init_btn();     //������ʼ��

void put_text(char *s, int i, unsigned coloridx);//�ַ������
char* choosemode(int i);        //�����˵�
void show_menu();               //�˵�
void result(int t);             //���ֹ���-ʤ��
void fnd(int t,int x,int y);    //ײ���ϰ���--ʧ��

void mods(void);            //��ӦģʽС�߳�ʼ��
void draw(void);            //��ʼ��̰�����Դ�ı߿�����
void clear_TCS(void);       //̰�����Դ����
void display_TCS(void);	    //����̰����ģʽ��ʾ
int  check(int x,int y);    //����ĳһ����
void rand_food(void);       //û��ʳ��ʱ���Ͷι
void change(int t);         //û�а��������ߵ�����

int  ads(int x);                            //ȡ����ֵ
int  other(int x);                          //03������12����
int get_food(int x,int y,int xxx,int yyy);  //������������Ѱʳ��
void machine(int t,int x,int y);            //������Ѱ·

void Up_intHandler();       //��
void Down_intHandler();     //��
void Left_intHandler();     //��
void Right_intHandler();    //��

void Up_intHandler_1();       //��
void Down_intHandler_1();     //��
void Left_intHandler_1();     //��
void Right_intHandler_1();    //��
//-------------------------------------------------------------------------------------------------
// ������
//-------------------------------------------------------------------------------------------------

int main(void)
{
    ls1x_drv_init();            		/* Initialize device drivers */
    install_3th_libraries();      		/* Install 3th libraies */
    init_btn();
    
    fb_open();
    fb_cons_clear();
    
    while(1)	
	{
        while(start_clock==0)
		{
			show_menu();
			delay_ms(500);
		}
		while(ms==1)  //1����ģʽ
		{
			display_TCS();
			rand_food();
			change(0);
		}
		while(ms==2)	//2˫�˺���
		{
			display_TCS();	
			rand_food();
			change(0);
			change(1);
		}
		while(ms==3)	//3˫�˶Կ�
		{
			display_TCS();	
			rand_food();
			change(0);
			change(1);
		}
		while(ms==4)	//4�����Կ�����
		{
			display_TCS();
			rand_food();
			change(2),change(3);
		}

		while(ms==5)	//5�˻��Կ�����
		{
			display_TCS();
			rand_food();
			change(0);
			change(2);
		}
		while(ms==6)	//6˫�˶Կ�����
		{
			display_TCS();
			rand_food();
			change(0);
			change(1);
		}
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------
// ����ʵ��
//-------------------------------------------------------------------------------------------------




void init_btn()     //������ʼ��
 {
    // gpioʹ��
     gpio_enable(BTN1, DIR_IN);
     gpio_enable(BTN2, DIR_IN);
     gpio_enable(BTN3, DIR_IN);
     gpio_enable(BTN4, DIR_IN);
     // ���ж�
     ls1x_disable_gpio_interrupt(BTN1);
     ls1x_disable_gpio_interrupt(BTN2);
     ls1x_disable_gpio_interrupt(BTN3);
     ls1x_disable_gpio_interrupt(BTN4);
     // ��װ�ж�����
     ls1x_install_gpio_isr(BTN1, INT_TRIG_EDGE_UP, Up_intHandler, 0);
     ls1x_install_gpio_isr(BTN2, INT_TRIG_EDGE_UP, Down_intHandler, 0);
     ls1x_install_gpio_isr(BTN3, INT_TRIG_EDGE_UP, Left_intHandler, 0);
     ls1x_install_gpio_isr(BTN4, INT_TRIG_EDGE_UP, Right_intHandler, 0);
     //���ж�
     ls1x_enable_gpio_interrupt(BTN1);
     ls1x_enable_gpio_interrupt(BTN2);
     ls1x_enable_gpio_interrupt(BTN3);
     ls1x_enable_gpio_interrupt(BTN4);
     
     // gpioʹ��
     gpio_enable(BTN5, DIR_IN);
     gpio_enable(BTN6, DIR_IN);
     gpio_enable(BTN7, DIR_IN);
     gpio_enable(BTN8, DIR_IN);
     // ���ж�
     ls1x_disable_gpio_interrupt(BTN5);
     ls1x_disable_gpio_interrupt(BTN6);
     ls1x_disable_gpio_interrupt(BTN7);
     ls1x_disable_gpio_interrupt(BTN8);
     // ��װ�ж�����
     ls1x_install_gpio_isr(BTN5, INT_TRIG_EDGE_UP, Up_intHandler_1, 0);
     ls1x_install_gpio_isr(BTN6, INT_TRIG_EDGE_UP, Down_intHandler_1, 0);
     ls1x_install_gpio_isr(BTN7, INT_TRIG_EDGE_UP, Left_intHandler_1, 0);
     ls1x_install_gpio_isr(BTN8, INT_TRIG_EDGE_UP, Right_intHandler_1, 0);
     //���ж�
     ls1x_enable_gpio_interrupt(BTN5);
     ls1x_enable_gpio_interrupt(BTN6);
     ls1x_enable_gpio_interrupt(BTN7);
     ls1x_enable_gpio_interrupt(BTN8);
 }


void put_text(char *s, int i, unsigned coloridx)    //�ַ������
{
    i = i+3;
    fb_put_string_center(400, i*20 ,s , coloridx);
}
char* choosemode(int i) //�����˵�
{
    switch(i)
    {
        case 1: return "����ģʽ";
        case 2: return "˫�˺���";
        case 3: return "˫�˶Կ�";
        case 4: return "�����Կ�--20��";
        case 5: return "�˻��Կ�--20��";
        case 6: return "˫�˶Կ�--20��";
        default:return "����ģʽ";
    }
}
void show_menu()    //�˵�
{
    fb_drawrect(90,40,720,360, 7);
    put_text("---------------------------------------------------------------------------",0,7);
    put_text("��ӭ����̰���ߴ�ð����Ϸ��",1,4);      // ��ɫ
    put_text("��Ϸ˵����",2, 7);                     // ��ɫ
    put_text("ʹ��btn1��btn2ѡ����Ϸģʽ��btn3����ȷ��",3,7);
    put_text("������Ϸ��ʹ�� btn1:���ϡ�btn2:���¡�btn3:����btn4:���� ��̰���߽��вٿأ�",4, 7);
    put_text("---------------------------------------------------------------------------",5,7);
    put_text("ģʽѡ��",6,7);
    put_text("---------------------------------------------------------------------------",7,7);
    int i = 8;
    for(i=8;i<14;i++)     // 8�е�13��
    {
        if(ms == i%7)                   // ģʽ��������Ӧ����ʾ��ɫ
           put_text(choosemode(i%7),i,6);
        else                                          // ������ʾ��ɫ
           put_text(choosemode(i%7),i,7);
    }
    put_text("-----------------------------  ������ ��  --------------------------------",14,7);
}
void result(int t)  //���ֹ���--ʤ��
{
	if(ms==5)           //�˻��Կ�-����
	{
		if(t==0)
			put_text("ʤ��",1,4);
		else
			put_text("ʧ��",1,4);
    }
	else if(ms==4)      //�����Կ�-����
	{
	  if(t==2)
			put_text("������һ�����㹻��ʤ��",1,4);
		else
			put_text("�����˶������㹻��ʤ��",1,4);
    }
	else if(ms==6)      //�Կ�ģʽ-����
	{
	    if(t==1)
            put_text("���һ�����㹻��ʤ��",1,4);
		  else
			put_text("��Ҷ������㹻��ʤ��",1,4);
    }
	else ;
	while(1); //����
}
void fnd(int t,int x,int y)  //ײ���ϰ���--ʧ��
{
	if(check(x,y)) return;   //check(x,y)Ϊ0����ײ
	if(ms==4)        //�����Կ�-����
	{
	    if(t==3)
			put_text("�����˶�ײǽ",1,4);
	    else
			put_text("������һײǽ",1,4);
    }
	else if(ms==3||ms==6)   //�Կ�ģʽ  �Կ�ģʽ-����
    {
	    if(t==1)
			put_text("��Ҷ�ײǽ",1,4);
		else
			put_text("���һײǽ",1,4);
    }
	else
	{
		if(t==2)
			put_text("ʤ��",1,4);
		else
			put_text("ʧ��",1,4);
    }
	score=0;
	while(1);  //����
}



void mods(void)  //��ӦģʽС�߳�ʼ��
{
    int i;
	if(ms!=4)
	{
		tang[0][1].x=1;
		tang[0][1].y=3;
		tang[0][2].x=1;
		tang[0][2].y=2;
		tang[0][tot[0]=3].x=1;
		tang[0][tot[0]=3].y=1;
		f[0]=1;   //���1��ʼ����  ����
		for(i=1;i<=tot[0];i++)
			TCSK[tang[0][i].x][tang[0][i].y]=4;
	}
	if(ms==2||ms==3||ms==6)
	{
		tang[1][1].x=n-2;
		tang[1][1].y=m-4;
		tang[1][2].x=n-2;
		tang[1][2].y=m-3;
		tang[1][tot[1]=3].x=n-2;
		tang[1][tot[1]=3].y=m-2;
		f[1]=2;    //���2��ʼ����  ����
		for(i=1;i<=tot[1];i++)
			TCSK[tang[1][i].x][tang[1][i].y]=5;
	}
	if(ms==5||ms==4)
	{
		tang[2][1].x=n-2;
		tang[2][1].y=m-4;
		tang[2][2].x=n-2;
		tang[2][2].y=m-3;
		tang[2][tot[2]=3].x=n-2;
		tang[2][tot[2]=3].y=m-2;
		f[2]=2;    //����1��ʼ����  ����
		for(i=1;i<=tot[2];i++)
			TCSK[tang[2][i].x][tang[2][i].y]=6;
	}
	if(ms==4)
	{
		tang[3][1].x=1;
		tang[3][1].y=3;
		tang[3][2].x=1;
		tang[3][2].y=2;
		tang[3][tot[3]=3].x=1;
		tang[3][tot[3]=3].y=1;
		f[3]=1;   //����2��ʼ����  ����
		for(i=1;i<=tot[3];i++)
			TCSK[tang[3][i].x][tang[3][i].y]=7;
	}
}
void draw(void)  //��ʼ��̰�����Դ�ı߿�����
{
    int i;
	for(i=0;i<n;i++)
	{
		TCSK[i][0]=1;
		TCSK[i][m-1]=1;
	}
	for(i=0;i<m;i++)
	{
		TCSK[0][i]=1;
		TCSK[n-1][i]=1;
	}
}
void clear_TCS(void)  //̰�����Դ����
{
    int i,j;
	for(i=0;i<n;i++)
		for(j=0;j<m;j++)
			TCSK[i][j]=0;
}
void display_TCS(void)	//����̰����ģʽ��ʾ
{
    unsigned int H,L; //H L ������
    for(H=0;H<n;H++)
	   for(L=0;L<m;L++)
            fb_fillrect(L*20,H*20,(L+1)*20,(H+1)*20,TCSK[H][L]);
}
int check(int x,int y)   //����ĳһ����
{
	if(x<1||x>(n-2)||y<1||y>(m-2))  //�߽��Լ��߽����ⷵ��0
		return 0;
	int t=1;
	int i;
	if(ms!=4)
	    for(i=1;i<=tot[0];i++)   //�������Ϸ���0
	        if(x==tang[0][i].x&&y==tang[0][i].y)
			    {t=0;break;}
	if(ms==2||ms==3||ms==6)
	    for(i=1;i<=tot[1];i++)
	        if(x==tang[1][i].x&&y==tang[1][i].y)
			    {t=0;break;}
	if(ms==5||ms==4)
		for(i=1;i<=tot[2];i++)
			if(x==tang[2][i].x&&y==tang[2][i].y)
					{t=0;break;}
	if(ms==4)
		for(i=1;i<=tot[3];i++)
			if(x==tang[3][i].x&&y==tang[3][i].y)
					{t=0;break;}
	return t;    //��������Щ�㷵��1
}
void rand_food(void)  //û��ʳ��ʱ���Ͷι
{
    int x=2,y=3;
    if(!vis[foodd[0].x][foodd[0].y]) //���0��ʳ�ﲻ����
	{
		//x=(rand()%(n-3))+2,y=(rand()%(m-3))+2; //�������xy����yΪż��
		x=(get_clock_ticks()%(n-3))+2,y=(get_clock_ticks()%(m-3))+2; //�������xy����yΪż��
		while(!check(x,y))
			//x=(rand()%(n-3))+2,y=(rand()%(m-3))+2; //�������Ͷι�ĵ㣬�Ƿ���Ҫ����Ͷι
			x=(get_clock_ticks()%(n-3))+2,y=(get_clock_ticks()%(m-3))+2; //�������xy����yΪż��
		TCSK[x][y]=10;
	    food++;
		vis[x][y]=1;
		foodd[0].x=x;
		foodd[0].y=y;
	}
	if(!vis[foodd[1].x][foodd[1].y])  //�ڶ���ʳ��--ͬ��
	{
		//x=rand()%n,y=rand()%m;
		x=get_clock_ticks()%n,y=get_clock_ticks()%m; //�������xy����yΪż��
		while(!check(x,y))
			//x=rand()%n,y=(rand()%m);
			x=get_clock_ticks()%n,y=get_clock_ticks()%m; //�������xy����yΪż��
		TCSK[x][y]=10;
		food++;
		vis[x][y]=1;
		foodd[1].x=x;
		foodd[1].y=y;
	}
}
void change(int t)  //û�а��������ߵ�����--��t����
{
    int i;
	int x=tang[t][1].x,y=tang[t][1].y;    //x��y����ͷλ�õ�xy
	if(t==2||t==3)                      //2�ź�3�����ǵ���
		machine(t,x,y);                   //������Ѱ·
	x+=flg[f[t]][0],y+=flg[f[t]][1];    //��ͷ(x,y)�����ɱ仯
	for(i=tot[t];i;i--)             //�ߵ�����λ�����ݿ�ˢ��
	   {
	        tang[t][i+1]=tang[t][i];
	        TCSK[tang[t][i].x][tang[t][i].y]=t+4; //������ʾˢ��  t+4��ɫ��ͬ
		}
	TCSK[tang[t][tot[t]+1].x][tang[t][tot[t]+1].y]=0; //ԭ������β��ʧ
	if(vis[x][y])  //����Ե�һ��ʳ��Ļ�
	{
		vis[x][y]=0,score+=(t==0||t==1),food--;
		if(++tot[t]>=10&&((ms==4)||(ms==5)||(ms==6)))  //��һ���Թ���
			result(t);
		TCSK[tang[t][tot[t]].x][tang[t][tot[t]].y]=t+4; //β�ʹ�+1
	}
	fnd(t,x,y);   //һ��ײǽײ����ʧ��
	tang[t][1].x=x;    //����ͷλ�÷�����ȫ�ֱ�����
	tang[t][1].y=y;
	TCSK[tang[t][1].x][tang[t][1].y]=5;  //����ͷ����
}


int ads(int x)  //ȡ����ֵ
{
    return x<0?-x:x; //x�Ƿ�<0,���x<0�Ļ���ִ��:ǰ�����䣬���x��С��0�Ļ���ִ��:��������
}
int other(int x)  //03������12����  ��������
{
	if     (x==0) return 3;
	else if(x==1) return 2;
	else if(x==2) return 1;
	else          return 0;
}
int get_food(int x,int y,int xxx,int yyy)  //������������Ѱʳ��
{
    return ads(x-xxx)+ads(y-yyy);
}
void machine(int t,int x,int y)  //������Ѱ·
{
    int i;
	int foodid,minn=1e9,newf=f[t];//��С�����ʼֵ�ܴ�
	if((get_food(x,y,foodd[0].x,foodd[0].y)<=get_food(x,y,foodd[1].x,foodd[1].y)&&vis[foodd[0].x][foodd[0].y])||!vis[foodd[1].x][foodd[1].y])
		foodid=0;
	else
		foodid=1;
	for(i=0;i<4;i++)
		if(f[t]^other(i))//ȷ���������
		{
			if(check(x+flg[i][0],y+flg[i][1])) //ȷ��ǰ�������ϰ�
			{
				int now=get_food(x+flg[i][0],y+flg[i][1],foodd[foodid].x,foodd[foodid].y); //�����µľ��롪��������һ������һʱ�̾���
				if(now<minn)
					newf=i,minn=now;
			}
		}
	f[t]=newf;
}


 // ����1�жϷ������   ��  mode--
void Up_intHandler()
{
    mips_interrupt_disable();
    if(start_clock==0)
    {
        if(ms>1)
            ms--;
    }
    else
    {
        if(ms==1 && f[0]^0)   f[0]=3;
        if(ms==2 && f[1]^0)   f[1]=3;
        if(ms==3 && f[1]^0)   f[1]=3;
        if(ms==5 && f[0]^0)   f[0]=3;
        if(ms==6 && f[1]^0)   f[1]=3;
    }
    mips_interrupt_enable();
}
 // ����2�жϷ������   ��` mode++
void Down_intHandler()
{
    mips_interrupt_disable();
    if(start_clock==0)
    {
        if(ms<6)
            ms++;
    }
    else
    {
        if(ms==1 && f[0]^3)   f[0]=0;
        if(ms==2 && f[1]^3)   f[1]=0;
        if(ms==3 && f[1]^3)   f[1]=0;
        if(ms==5 && f[0]^3)   f[0]=0;
        if(ms==6 && f[1]^3)   f[1]=0;
    }
    mips_interrupt_enable();
}
 // ����3�жϷ������   ��  ȷ��
void Left_intHandler()
{
    mips_interrupt_disable();
    if(start_clock==0)
    {
        start_clock=1;
        clear_TCS();
        score=0;
        food=0;
        draw();
        mods();
    }
    else
    {
        if(ms==1 && f[0]^1)   f[0]=2;
        if(ms==2 && f[1]^1)   f[1]=2;
        if(ms==3 && f[1]^1)   f[1]=2;
        if(ms==5 && f[0]^1)   f[0]=2;
        if(ms==6 && f[1]^1)   f[1]=2;
    }
    mips_interrupt_enable();
}
// ����4�жϷ������    ��
void Right_intHandler()
{
    mips_interrupt_disable();
    if(start_clock==1)
    {
        if(ms==1 && f[0]^2)   f[0]=1;
        if(ms==2 && f[1]^2)   f[1]=1;
        if(ms==3 && f[1]^2)   f[1]=1;
        if(ms==5 && f[0]^2)   f[0]=1;
        if(ms==6 && f[1]^2)   f[1]=1;
    }
    mips_interrupt_enable();
}
 // ����5�жϷ������   ��  mode--
void Up_intHandler_1()
{
    mips_interrupt_disable();
    if(start_clock==1)
    {
        if(ms==2 && f[0]^0)   f[0]=3;
        if(ms==3 && f[0]^0)   f[0]=3;
        if(ms==6 && f[0]^0)   f[0]=3;
    }
    mips_interrupt_enable();
}
 // ����6�жϷ������   ��  mode++
void Down_intHandler_1()
{
    mips_interrupt_disable();
    if(start_clock==1)
    {
        if(ms==2 && f[0]^3)   f[0]=0;
        if(ms==3 && f[0]^3)   f[0]=0;
        if(ms==6 && f[0]^3)   f[0]=0;
    }
    mips_interrupt_enable();
}
 // ����7�жϷ������   ��  ȷ��
void Left_intHandler_1()
{
    mips_interrupt_disable();
    if(start_clock==1)
    {
        if(ms==2 && f[0]^1)   f[0]=2;
        if(ms==3 && f[0]^1)   f[0]=2;
        if(ms==6 && f[0]^1)   f[0]=2;
    }
    mips_interrupt_enable();
}
// ����8�жϷ������    ��
void Right_intHandler_1()
{
    mips_interrupt_disable();
    if(start_clock==1)
    {
        if(ms==2 && f[0]^2)   f[0]=1;
        if(ms==3 && f[0]^2)   f[0]=1;
        if(ms==6 && f[0]^2)   f[0]=1;
    }
    mips_interrupt_enable();
}

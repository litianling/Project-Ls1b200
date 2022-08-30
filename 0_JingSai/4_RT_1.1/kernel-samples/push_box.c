/*
 * push_box.c
 *
 * created: 2022/4/11
 *  author: 
 */

#include <setjmp.h> /* jmp_buf, setjmp, longjmp */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bsp.h"
#include "ls1b.h"
#include "ls1b_gpio.h"
#include "console.h"
#include "termios.h"
#include "rtconfig.h"
#include "usb.h"

#include <rtthread.h>
#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       0x00100000  //2M
#define THREAD_TIMESLICE        5000
static rt_thread_t push_box1 = RT_NULL;


#define  getch  usb_kbd_getc
#define  _getch usb_kbd_getc
#define putchar fb_cons_putc
#define printf  printk


jmp_buf env1,env2;  //������ת
char a[50][50];
char passward[15];
int  GK,p='1';    //GK�ؿ�ѡ����� p��ѡ�ؿ�����
int  Player_Coordinates_Y,Player_Coordinates_X; //��¼S����ʼλ��  ǰ�к���
int  Box1_Destination_Y,Box1_Destination_X;     //��¼����Ŀ�ĵ�1
int  Box2_Destination_Y,Box2_Destination_X;     //��¼����Ŀ�ĵ�2
int  Box3_Destination_Y,Box3_Destination_X;     //��¼����Ŀ�ĵ�3
int  Box4_Destination_Y,Box4_Destination_X;     //��¼����Ŀ�ĵ�4

void level_pushbox(char k);   //�ؿ�ѡ����
void input_pushbox(char ch);  //���밴���ж��߼�
void supplement_pushbox();    //Ŀ��㲹�亯��
void success_pushbox();       //ͨ��֮������
void output_pushbox();        //��������ʾ�������

//***************************************************************************

char * gets(char *s)
{
    int i;
    for(i=0;i<15;i++)
    {
        if(*s == '\n')
            return s;
        else
            *s++ = getch();
    }
    return s;
}

//***************************************************************************

int push_box_entry()
{
    //system("title ������");
	//system("mode con lines=30 cols=63"); //���ڴ�С
	//HideCursor();
	//system("cls");//��������
	fb_cons_clear();
	printf("\n Sokoban game description: \n \n Arrow keys are WASD \n Wall is # \n Plater is M \n The box is O \n Target is * \n The box in target is @ \n\n Press any key to begin");
    getch(); //�������
	setjmp(env1); //��ת������÷���ֵ���������ж�Դ��
	//system("cls");
	fb_cons_clear();
	printf("Please select the level(1-5): \n");
	setjmp(env2); //��ת������
	GK=_getch();
	printf("%c",GK);
	if ((GK<'0'||GK>'5')&&(GK!='.'))
	    {
	    	printf("\n Select level err,please re-select the level: \n");
		    longjmp(env2,2); //�Ӵ˴���2��λ�ã���ת��env2
		}
	else if(GK>p)
	    {
	    	printf("\n Free prostitution is prohibited, please re-select the level: \n");
		    longjmp(env2,3); //�Ӵ˴���3��λ�ã���ת��env2
		}
    while((GK>='0'&&GK<'6')||GK=='.')
    {
	    level_pushbox(GK);  //�ؿ�ѡ����
        //system("cls");
        fb_cons_clear();
        output_pushbox();  //��ʼ״̬��ʾ
	    while(1)  //����ģ��
	    {
            char ch=_getch();
		    input_pushbox(ch);     //���밴���ж��߼�
		    supplement_pushbox();  //Ŀ��㲹�亯��
		    //system("cls");
		    fb_cons_clear();
		    output_pushbox(); //״̬��ʾˢ��
		    if(a[Box1_Destination_Y][Box1_Destination_X]=='@'&&a[Box2_Destination_Y][Box2_Destination_X]=='@'  //��ʤ����
		     &&a[Box3_Destination_Y][Box3_Destination_X]=='@'&&a[Box4_Destination_Y][Box4_Destination_X]=='@')
		    {
		    	GK++;
		    	if(GK>=p) p=GK; //ѡ������----������
                // Sleep(150);
                delay_ms(150);
                break; //ȷ�ϻ�ʤ������while(1)ѭ��
		    }
	    }
    }
    success_pushbox();//ͨ��֮������
}


void level_pushbox(char GK)  //�ؿ�ѡ����
{
	switch(GK)
	{
		case '1':
			{
				strcpy(a[0],"   ###                                           ");
				strcpy(a[1],"   #*#                                           ");
				strcpy(a[2],"   # #                                           ");
				strcpy(a[3],"####O######                                      ");
				strcpy(a[4],"#*  OM O *#                                      ");
				strcpy(a[5],"#####O#####                                      ");
				strcpy(a[6],"    # #                                          ");
				strcpy(a[7],"    #*#                                          ");
				strcpy(a[8],"    ###                                          ");
				strcpy(a[9],"                                                 ");
				strcpy(a[10],"level 1/5                                       ");
				strcpy(a[11],"                                                ");
				strcpy(a[12],"Press R to re-start                             ");
				strcpy(a[13],"                                                ");
				strcpy(a[14],"                                                ");
	            Player_Coordinates_Y=4,Player_Coordinates_X=5;
	            Box1_Destination_Y=1,Box1_Destination_X=4;
	            Box2_Destination_Y=4,Box2_Destination_X=1;
	            Box3_Destination_Y=4,Box3_Destination_X=9;
	            Box4_Destination_Y=7,Box4_Destination_X=5;
			}
		break;
		case '2':
			{
				strcpy(a[0],"######                                           ");
				strcpy(a[1],"#*   #                                           ");
				strcpy(a[2],"###  #                                           ");
				strcpy(a[3],"#  O ######                                      ");
				strcpy(a[4],"#*  OM O *#                                      ");
				strcpy(a[5],"#####O#####                                      ");
				strcpy(a[6],"    # #                                          ");
				strcpy(a[7],"    #*#                                          ");
				strcpy(a[8],"    ###                                          ");
				strcpy(a[9],"                                                 ");
				strcpy(a[10],"level 2/5                                       ");
				strcpy(a[11],"                                                ");
				strcpy(a[12],"Press R to re-start                             ");
				strcpy(a[13],"                                                ");
				strcpy(a[14],"                                                ");
	            Player_Coordinates_Y=4,Player_Coordinates_X=5;
	            Box1_Destination_Y=1,Box1_Destination_X=1;
	            Box2_Destination_Y=4,Box2_Destination_X=1;
	            Box3_Destination_Y=4,Box3_Destination_X=9;
	            Box4_Destination_Y=7,Box4_Destination_X=5;
			}
		break;
		case '3':
			{
				strcpy(a[0],"  ####                                           ");
				strcpy(a[1],"  #  #                                           ");
				strcpy(a[2],"  #  #                                           ");
				strcpy(a[3],"  #M #                                           ");
				strcpy(a[4],"### ######                                       ");
				strcpy(a[5],"#   O  O*#                                       ");
				strcpy(a[6],"# O*   ###                                       ");
				strcpy(a[7],"#####* O*#                                       ");
				strcpy(a[8],"    ######                                       ");
				strcpy(a[9],"                                                 ");
				strcpy(a[10],"level 3/5                                       ");
				strcpy(a[11],"                                                ");
				strcpy(a[12],"Press R to re-start                             ");
				strcpy(a[13],"                                                ");
				strcpy(a[14],"                                                ");
	            Player_Coordinates_Y=3,Player_Coordinates_X=3;
	            Box1_Destination_Y=6,Box1_Destination_X=3;
	            Box2_Destination_Y=5,Box2_Destination_X=8;
	            Box3_Destination_Y=7,Box3_Destination_X=5;
	            Box4_Destination_Y=7,Box4_Destination_X=8;
			}
		break;
		case '4':
			{
				strcpy(a[0]," ########                                        ");
				strcpy(a[1]," #     ###                                       ");
				strcpy(a[2],"##O###   #                                       ");
				strcpy(a[3],"#M  O  O #                                       ");
				strcpy(a[4],"# **# O ##                                       ");
				strcpy(a[5],"##**#   #                                        ");
				strcpy(a[6]," ########                                        ");
				strcpy(a[7],"                                                 ");
				strcpy(a[8],"                                                 ");
				strcpy(a[9],"                                                 ");
				strcpy(a[10],"level 4/5                                       ");
				strcpy(a[11],"                                                ");
				strcpy(a[12],"Press R to re-start                             ");
				strcpy(a[13],"                                                ");
				strcpy(a[14],"                                                ");
	            Player_Coordinates_Y=3,Player_Coordinates_X=1;
	            Box1_Destination_Y=4,Box1_Destination_X=2;
	            Box2_Destination_Y=4,Box2_Destination_X=3;
	            Box3_Destination_Y=5,Box3_Destination_X=2;
	            Box4_Destination_Y=5,Box4_Destination_X=3;
			}
		break;
		case '5':
			{
				strcpy(a[0],"  ####                                           ");
				strcpy(a[1],"  #**#                                           ");
				strcpy(a[2]," ## *##                                          ");
				strcpy(a[3]," #  O*#                                          ");
				strcpy(a[4],"## O  ##                                         ");
				strcpy(a[5],"#  #OO #                                         ");
				strcpy(a[6],"#  M   #                                         ");
				strcpy(a[7],"########                                         ");
				strcpy(a[8],"                                                 ");
				strcpy(a[9],"                                                 ");
				strcpy(a[10],"level 5/5                                       ");
				strcpy(a[11],"                                                ");
				strcpy(a[12],"Press R to re-start                             ");
				strcpy(a[13],"                                                ");
				strcpy(a[14],"                                                ");
	            Player_Coordinates_Y=6,Player_Coordinates_X=3;
	            Box1_Destination_Y=1,Box1_Destination_X=3;
	            Box2_Destination_Y=1,Box2_Destination_X=4;
	            Box3_Destination_Y=2,Box3_Destination_X=4;
	            Box4_Destination_Y=3,Box4_Destination_X=5;
			}
		break;
		case '0':
			{
				printf("\n Please enter the password to view the customs clearance guide: \n");
				gets(passward);
                if(passward[0]=='L'&&passward[1]=='T'&&passward[2]=='L'&&passward[3]=='1'&&passward[4]=='9'&&passward[5]=='9'   //�ж������Ƿ���ȷ
				 &&passward[6]=='9'&&passward[7]=='1'&&passward[8]=='0'&&passward[9]=='1'&&passward[10]=='5'&&passward[11]==NULL)
				{                                                                      //0����ֻ������һ��������ʾ���ַ������ڴ沢û��ֱ�ӹ�ϵ��   *
		    		strcpy(a[0],"          Customs clearance strategy             ");  //��0��ASCII���й���NULL���˹���������ʹ������ת���ַ�'\0'��*
	    			strcpy(a[1],"level 1/5 :                                      ");  //Ҳ���Խ�һ��������ֵΪNULL����'\0'��Ӧ��ASCII�����ǵ�0�ţ�*
		    		strcpy(a[2],"  SS WW DDD AAAAAA DD WW                         ");  //char���ͱ����Ͼ���int������ʱҲ���Զ�ת��Ϊint���ͣ�      *
    				strcpy(a[3],"level 2/5 :                                      ");  //����'\0'������0��ֵҲǡ����ȡ�����д�������������ͬ     *
	    			strcpy(a[4],"  SS WW DDD AAAAAA DWW DWAA                      ");  //passward[11]==NULL  passward[11]=='\0' passward[11]==0    *
		    		strcpy(a[5],"level 3/5 :                                      ");
			    	strcpy(a[6],"  SSSDDDWDA SSDAWWAASAW AASDDDWDS AAWWWD WWASSSS ");
    				strcpy(a[7],"level 4/5 :                                      ");
	    			strcpy(a[8],"  DDDD SSDDWWA WWAAAA SSASDWDS WAWWDDDD SSAAADDD ");
		    		strcpy(a[9],"  WWAAAA SSASD WDDDDD SASAW DWAAA DDDWW AAAA SSS ");
			    	strcpy(a[10],"  WDDDDDD WAS AAAAA WWDDDD SDS AAAA DDDWW AAAASS");
    				strcpy(a[11],"level 5/5 :                                     ");
	    			strcpy(a[12],"  AWW DDWW SSAASS DDWWW SSSDD WASAAA WWWDDD SAAA");
		    		strcpy(a[13],"  SSDD WW DWA SAWW SSASS DDDWW                  ");
			    	strcpy(a[14],"Press R to re-start                             ");
			    }
			    else
				{
					printf("\n Incorrect password, please re-select the level: \n");
					longjmp(env2,2);
				}

			}
		break;
		case '.':
			{
				printf("\n Please enter password to unlock all levels: \n");
				gets(passward);
                if(passward[0]=='L'&&passward[1]=='T'&&passward[2]=='L'&&passward[3]=='1'&&passward[4]=='9'&&passward[5]=='9'
				 &&passward[6]=='9'&&passward[7]=='1'&&passward[8]=='0'&&passward[9]=='1'&&passward[10]=='5'&&passward[11]==NULL)
				{
					p='5';
		    		printf("\n Password is correct, all levels are unlocked������\n Please select a new level: \n");
		    		longjmp(env2,2);
			    }
			    else
				{
					printf("\n Incorrect password, please re-select the level: \n");
					longjmp(env2,2);
				}

			}
		break;
	}
}


void input_pushbox(char ch)     //���밴���ж��߼�
{
	switch(ch)  //�����ж��߼�
	{
	    case 'w':
		    if(a[Player_Coordinates_Y-1][Player_Coordinates_X]!='#')                 //�ϱ߲���ǽ
		    {
			    if(a[Player_Coordinates_Y-1][Player_Coordinates_X]!='O'&&a[Player_Coordinates_Y-1][Player_Coordinates_X]!='@')      //�ϱ߲�������
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';   //��ǰλ��������ʧ
				   Player_Coordinates_Y--;                              //��������
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';   //�������
			    }
			    else if(a[Player_Coordinates_Y-2][Player_Coordinates_X]!='#'&&a[Player_Coordinates_Y-2][Player_Coordinates_X]!='@'&&a[Player_Coordinates_Y-2][Player_Coordinates_X]!='O')
			    {                                                                    //�ϱ�������  ���ϲ���ǽҲ��������
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';   //ͬ
				   Player_Coordinates_Y--;                              //
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';   //��
				   a[Player_Coordinates_Y-1][Player_Coordinates_X]='O'; //���ӳ��������Ϸ�  ��������ɹ�
				   if(((Player_Coordinates_Y-1==Box1_Destination_Y)&&(Player_Coordinates_X==Box1_Destination_X))
				    ||((Player_Coordinates_Y-1==Box2_Destination_Y)&&(Player_Coordinates_X==Box2_Destination_X))
				    ||((Player_Coordinates_Y-1==Box3_Destination_Y)&&(Player_Coordinates_X==Box3_Destination_X))
					||((Player_Coordinates_Y-1==Box4_Destination_Y)&&(Player_Coordinates_X==Box4_Destination_X)))
				       a[Player_Coordinates_Y-1][Player_Coordinates_X]='@';//�ж������Ƿ���Ŀ���
			    }
		    }
		break;
		case 'a':
		    if(a[Player_Coordinates_Y][Player_Coordinates_X-1]!='#')
		    {
			    if(a[Player_Coordinates_Y][Player_Coordinates_X-1]!='O'&&a[Player_Coordinates_Y][Player_Coordinates_X-1]!='@')
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';
				   Player_Coordinates_X--;
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';
			    }
			    else if(a[Player_Coordinates_Y][Player_Coordinates_X-2]!='#'&&a[Player_Coordinates_Y][Player_Coordinates_X-2]!='@'&&a[Player_Coordinates_Y][Player_Coordinates_X-2]!='O')
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';
				   Player_Coordinates_X--;
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';
				   a[Player_Coordinates_Y][Player_Coordinates_X-1]='O';
			       if(((Player_Coordinates_Y==Box1_Destination_Y)&&(Player_Coordinates_X-1==Box1_Destination_X))
				    ||((Player_Coordinates_Y==Box2_Destination_Y)&&(Player_Coordinates_X-1==Box2_Destination_X))
				    ||((Player_Coordinates_Y==Box3_Destination_Y)&&(Player_Coordinates_X-1==Box3_Destination_X))
					||((Player_Coordinates_Y==Box4_Destination_Y)&&(Player_Coordinates_X-1==Box4_Destination_X)))
				       a[Player_Coordinates_Y][Player_Coordinates_X-1]='@';
			    }
		    }
		break;
		case 's':
		    if(a[Player_Coordinates_Y+1][Player_Coordinates_X]!='#')
		    {
			    if(a[Player_Coordinates_Y+1][Player_Coordinates_X]!='O'&&a[Player_Coordinates_Y+1][Player_Coordinates_X]!='@')
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';
				   Player_Coordinates_Y++;
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';
			    }
			    else if(a[Player_Coordinates_Y+2][Player_Coordinates_X]!='#'&&a[Player_Coordinates_Y+2][Player_Coordinates_X]!='@'&&a[Player_Coordinates_Y+2][Player_Coordinates_X]!='O')
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';
				   Player_Coordinates_Y++;
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';
				   a[Player_Coordinates_Y+1][Player_Coordinates_X]='O';
				   if(((Player_Coordinates_Y+1==Box1_Destination_Y)&&(Player_Coordinates_X==Box1_Destination_X))
				    ||((Player_Coordinates_Y+1==Box2_Destination_Y)&&(Player_Coordinates_X==Box2_Destination_X))
				    ||((Player_Coordinates_Y+1==Box3_Destination_Y)&&(Player_Coordinates_X==Box3_Destination_X))
					||((Player_Coordinates_Y+1==Box4_Destination_Y)&&(Player_Coordinates_X==Box4_Destination_X)))
				       a[Player_Coordinates_Y+1][Player_Coordinates_X]='@';
			    }
			}
		break;
	    case 'd':
		    if(a[Player_Coordinates_Y][Player_Coordinates_X+1]!='#')
		    {
			    if(a[Player_Coordinates_Y][Player_Coordinates_X+1]!='O'&&a[Player_Coordinates_Y][Player_Coordinates_X+1]!='@')
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';
				   Player_Coordinates_X++;
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';
			    }
			    else if(a[Player_Coordinates_Y][Player_Coordinates_X+2]!='#'&&a[Player_Coordinates_Y][Player_Coordinates_X+2]!='@'&&a[Player_Coordinates_Y][Player_Coordinates_X+2]!='O')
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';
				   Player_Coordinates_X++;
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';
				   a[Player_Coordinates_Y][Player_Coordinates_X+1]='O';
				   if(((Player_Coordinates_Y==Box1_Destination_Y)&&(Player_Coordinates_X+1==Box1_Destination_X))
				    ||((Player_Coordinates_Y==Box2_Destination_Y)&&(Player_Coordinates_X+1==Box2_Destination_X))
				    ||((Player_Coordinates_Y==Box3_Destination_Y)&&(Player_Coordinates_X+1==Box3_Destination_X))
					||((Player_Coordinates_Y==Box4_Destination_Y)&&(Player_Coordinates_X+1==Box4_Destination_X)))
				       a[Player_Coordinates_Y][Player_Coordinates_X+1]='@';
			    }
		    }
		break;
		case 'r':longjmp(env1,1); //�Ӵ˴���1��λ�ã���ת��env1
		break;                    //���ܣ����¿�ʼ
		default: ;   //����������������Ӧ
		break;
	}
}


void supplement_pushbox()  //Ŀ��㲹�亯��
{
	if(a[Box1_Destination_Y][Box1_Destination_X]==' ')
	    a[Box1_Destination_Y][Box1_Destination_X]='*';
	if(a[Box2_Destination_Y][Box2_Destination_X]==' ')
	    a[Box2_Destination_Y][Box2_Destination_X]='*';
	if(a[Box3_Destination_Y][Box3_Destination_X]==' ')
	    a[Box3_Destination_Y][Box3_Destination_X]='*';
	if(a[Box4_Destination_Y][Box4_Destination_X]==' ')
	    a[Box4_Destination_Y][Box4_Destination_X]='*';
}


void success_pushbox()  //ͨ��֮������
{
	//system("cls");//��������
	fb_cons_clear();
    printf("\n\n��������ϲͨ�أ�����\n");
    printf("\n ������ LTL19991015 \n");
    //setcolor(12); //ǳ��ɫ����
    float x,y;
    for (y = 1.3f; y > -1.0f; y -= 0.1f)  //������ͼ
    {
        for (x = -1.5f; x < 1.5f; x += 0.05f)
		{
            float a = x * x + y * y - 1;
            putchar(a * a * a - x * x * y * y * y <= 0.0f ? '*' : ' ');
        }
        putchar('\n');
    }
    //setcolor(7); //�ָ����ɫ
}


void output_pushbox()  //��������ʾ�������
{
    int i,j;
	for(i=0;i<15;i++)  //״̬��ʾ puts(a[i])�򵥵������ɸ�����ɫ
    {
	    for(j=0;j<50;j++)
		{
/*
		    if (a[i][j]=='O')
		        setcolor(14); //ǳ��ɫ����
			else if(a[i][j]=='M')
			    setcolor(12); //ǳ��ɫ���
			else if(a[i][j]=='*')
			    setcolor(10); //ǳ��ɫĿ���
			else if(a[i][j]=='@')
			    setcolor(10); //ǳ��ɫ��λ��
			else if(a[i][j]=='#')
			    setcolor(11); //ǳ��ɫǽ��
			else
			    setcolor(15); //ǳ��ɫ����
*/
            printf("%c",a[i][j]);
            //setcolor(7); //�ָ����ɫ
        }
        printf("\n");
    }
}


int push_box(void)
{
    push_box1 = rt_thread_create("push_box",
                            push_box_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(push_box1);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(push_box, push box game);



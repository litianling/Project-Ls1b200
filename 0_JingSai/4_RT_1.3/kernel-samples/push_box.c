/*
 * push_box.c
 *
 * created: 2022/4/11
 *  author: 
 */

#include <setjmp.h> /* jmp_buf, setjmp, longjmp */

#include <stdio.h>
#include <string.h>
#include "bsp.h"
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


jmp_buf env1,env2,env3;  //声明跳转
int  exit_flag = 0;
char a[15][50];
char passward[15];
int  GK,p='1';    //GK关卡选择变量 p可选关卡限制
int  Player_Coordinates_Y,Player_Coordinates_X; //记录S的起始位置  前行后列
int  Box1_Destination_Y,Box1_Destination_X;     //记录箱子目的地1
int  Box2_Destination_Y,Box2_Destination_X;     //记录箱子目的地2
int  Box3_Destination_Y,Box3_Destination_X;     //记录箱子目的地3
int  Box4_Destination_Y,Box4_Destination_X;     //记录箱子目的地4

void level_pushbox(char k);   //关卡选择函数
void input_pushbox(char ch);  //输入按键判断逻辑
void supplement_pushbox();    //目标点补充函数
void success_pushbox();       //通关之后运行
void output_pushbox();        //推箱子显示输出函数

//***************************************************************************

char * gets(char *s)
{
    int i;
    for(i=0;i<11;i++)
    {
        *s = getch();
        printk("%c",*s);
        s++;
    }
    *s = NULL;
    return s;
}

//***************************************************************************

int push_box_entry()
{
    extern int change_scheduler_lock(int val);
    change_scheduler_lock(1);
    exit_flag = 0;  // 不允许退出
    setjmp(env3);   // 设置跳转点
    if(exit_flag)   // 退出程序
    {
        fb_cons_clear();
        change_scheduler_lock(0);
        return 0;
    }
    exit_flag = 1;  // 允许退出
	fb_cons_clear();
	printf("\n Sokoban game description: \n \n Arrow keys are WASD \n Wall is # \n Plater is M \n The box is O \n Target is * \n");
    printf(" The box in target is @ \n\n Press any key to begin");
    getch(); //阻断作用
	setjmp(env1); //跳转到这里【用返回值可以区别中断源】
	//system("cls");
	fb_cons_clear();
	printf("Please select the level(1-5): \n");
	setjmp(env2); //跳转到这里
	GK=_getch();
	printf("%c",GK);
	if ((GK<'0'||GK>'5')&&(GK!='.'))
	    {
	    	printf("\n Select level err,please re-select the level: \n");
		    longjmp(env2,2); //从此处（2号位置）跳转到env2
		}
	else if(GK>p)
	    {
	    	printf("\n Free prostitution is prohibited, please re-select the level: \n");
		    longjmp(env2,3); //从此处（3号位置）跳转到env2
		}
    while((GK>='0'&&GK<'6')||GK=='.')
    {
	    level_pushbox(GK);  //关卡选择函数
        fb_cons_clear();
        output_pushbox();  //初始状态显示
	    while(1)  //运行模块
	    {
            char ch=_getch();
		    input_pushbox(ch);     //输入按键判断逻辑
		    supplement_pushbox();  //目标点补充函数
		    fb_cons_clear();
		    output_pushbox(); //状态显示刷新
		    if(a[Box1_Destination_Y][Box1_Destination_X]=='@'&&a[Box2_Destination_Y][Box2_Destination_X]=='@'  //获胜条件
		     &&a[Box3_Destination_Y][Box3_Destination_X]=='@'&&a[Box4_Destination_Y][Box4_Destination_X]=='@')
		    {
		    	GK++;
		    	if(GK>=p) p=GK; //选关限制----即检查点
                delay_ms(150);
                break; //确认获胜则跳出while(1)循环
		    }
	    }
    }
    success_pushbox();//通关之后运行
    change_scheduler_lock(0);
}


void level_pushbox(char GK)  //关卡选择函数
{
	switch(GK)
	{
		case '1':
			{
				strcpy(a[0],"   ### ");
				strcpy(a[1],"   #*# ");
				strcpy(a[2],"   # # ");
				strcpy(a[3],"####O###### ");
				strcpy(a[4],"#*  OM O *# ");
				strcpy(a[5],"#####O##### ");
				strcpy(a[6],"    # # ");
				strcpy(a[7],"    #*# ");
				strcpy(a[8],"    ### ");
				strcpy(a[9]," ");
				strcpy(a[10],"level 1/5 ");
				strcpy(a[11]," ");
				strcpy(a[12],"Press R to re-start ");
				strcpy(a[13],"Press E to exit ");
				strcpy(a[14]," ");
	            Player_Coordinates_Y=4,Player_Coordinates_X=5;
	            Box1_Destination_Y=1,Box1_Destination_X=4;
	            Box2_Destination_Y=4,Box2_Destination_X=1;
	            Box3_Destination_Y=4,Box3_Destination_X=9;
	            Box4_Destination_Y=7,Box4_Destination_X=5;
			}
		break;
		case '2':
			{
				strcpy(a[0],"###### ");
				strcpy(a[1],"#*   # ");
				strcpy(a[2],"###  # ");
				strcpy(a[3],"#  O ###### ");
				strcpy(a[4],"#*  OM O *# ");
				strcpy(a[5],"#####O##### ");
				strcpy(a[6],"    # # ");
				strcpy(a[7],"    #*# ");
				strcpy(a[8],"    ### ");
				strcpy(a[9]," ");
				strcpy(a[10],"level 2/5 ");
				strcpy(a[11]," ");
				strcpy(a[12],"Press R to re-start ");
				strcpy(a[13],"Press E to exit ");
				strcpy(a[14]," ");
	            Player_Coordinates_Y=4,Player_Coordinates_X=5;
	            Box1_Destination_Y=1,Box1_Destination_X=1;
	            Box2_Destination_Y=4,Box2_Destination_X=1;
	            Box3_Destination_Y=4,Box3_Destination_X=9;
	            Box4_Destination_Y=7,Box4_Destination_X=5;
			}
		break;
		case '3':
			{
				strcpy(a[0],"  #### ");
				strcpy(a[1],"  #  # ");
				strcpy(a[2],"  #  # ");
				strcpy(a[3],"  #M # ");
				strcpy(a[4],"### ###### ");
				strcpy(a[5],"#   O  O*# ");
				strcpy(a[6],"# O*   ### ");
				strcpy(a[7],"#####* O*# ");
				strcpy(a[8],"    ###### ");
				strcpy(a[9]," ");
				strcpy(a[10],"level 3/5 ");
				strcpy(a[11]," ");
				strcpy(a[12],"Press R to re-start ");
				strcpy(a[13],"Press E to exit ");
				strcpy(a[14]," ");
	            Player_Coordinates_Y=3,Player_Coordinates_X=3;
	            Box1_Destination_Y=6,Box1_Destination_X=3;
	            Box2_Destination_Y=5,Box2_Destination_X=8;
	            Box3_Destination_Y=7,Box3_Destination_X=5;
	            Box4_Destination_Y=7,Box4_Destination_X=8;
			}
		break;
		case '4':
			{
				strcpy(a[0]," ######## ");
				strcpy(a[1]," #     ### ");
				strcpy(a[2],"##O###   # ");
				strcpy(a[3],"#M  O  O # ");
				strcpy(a[4],"# **# O ## ");
				strcpy(a[5],"##**#   # ");
				strcpy(a[6]," ######## ");
				strcpy(a[7]," ");
				strcpy(a[8]," ");
				strcpy(a[9]," ");
				strcpy(a[10],"level 4/5 ");
				strcpy(a[11]," ");
				strcpy(a[12],"Press R to re-start ");
				strcpy(a[13],"Press E to exit ");
				strcpy(a[14]," ");
	            Player_Coordinates_Y=3,Player_Coordinates_X=1;
	            Box1_Destination_Y=4,Box1_Destination_X=2;
	            Box2_Destination_Y=4,Box2_Destination_X=3;
	            Box3_Destination_Y=5,Box3_Destination_X=2;
	            Box4_Destination_Y=5,Box4_Destination_X=3;
			}
		break;
		case '5':
			{
				strcpy(a[0],"  #### ");
				strcpy(a[1],"  #**# ");
				strcpy(a[2]," ## *## ");
				strcpy(a[3]," #  O*# ");
				strcpy(a[4],"## O  ## ");
				strcpy(a[5],"#  #OO # ");
				strcpy(a[6],"#  M   # ");
				strcpy(a[7],"######## ");
				strcpy(a[8]," ");
				strcpy(a[9]," ");
				strcpy(a[10],"level 5/5 ");
				strcpy(a[11]," ");
				strcpy(a[12],"Press R to re-start ");
				strcpy(a[13],"Press E to exit ");
				strcpy(a[14]," ");
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
                if(passward[0]=='L'&&passward[1]=='T'&&passward[2]=='L'&&passward[3]=='1'&&passward[4]=='9'&&passward[5]=='9'   //判断密码是否正确
				 &&passward[6]=='9'&&passward[7]=='1'&&passward[8]=='0'&&passward[9]=='1'&&passward[10]=='5'&&passward[11]==NULL)
				{                                                                      //0本身只不过是一个可以显示的字符，与内存并没有直接关系。   *
		    		strcpy(a[0],"          Customs clearance strategy             ");  //在0与ASCII表中关联NULL做了关联，这样使得输入转义字符'\0'，*
	    			strcpy(a[1],"level 1/5 :                                      ");  //也可以将一个变量赋值为NULL。而'\0'对应的ASCII码又是第0号，*
		    		strcpy(a[2],"  SS WW DDD AAAAAA DD WW                         ");  //char类型本质上就是int，运算时也会自动转换为int类型，      *
    				strcpy(a[3],"level 2/5 :                                      ");  //所以'\0'与数字0的值也恰好相等。这里写以下三种情况相同     *
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
		    		printf("\n Password is correct, all levels are unlocked！！！\n Please select a new level: \n");
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


void input_pushbox(char ch)     //输入按键判断逻辑
{
	switch(ch)  //按键判断逻辑
	{
	    case 'w':
		    if(a[Player_Coordinates_Y-1][Player_Coordinates_X]!='#')                 //上边不是墙
		    {
			    if(a[Player_Coordinates_Y-1][Player_Coordinates_X]!='O'&&a[Player_Coordinates_Y-1][Player_Coordinates_X]!='@')      //上边不是箱子
			    {
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';   //当前位置人物消失
				   Player_Coordinates_Y--;                              //人物上移
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';   //人物出现
			    }
			    else if(a[Player_Coordinates_Y-2][Player_Coordinates_X]!='#'&&a[Player_Coordinates_Y-2][Player_Coordinates_X]!='@'&&a[Player_Coordinates_Y-2][Player_Coordinates_X]!='O')
			    {                                                                    //上边是箱子  上上不是墙也不是箱子
				   a[Player_Coordinates_Y][Player_Coordinates_X]=' ';   //同
				   Player_Coordinates_Y--;                              //
				   a[Player_Coordinates_Y][Player_Coordinates_X]='M';   //上
				   a[Player_Coordinates_Y-1][Player_Coordinates_X]='O'; //箱子出现在人上方  推箱操作成功
				   if(((Player_Coordinates_Y-1==Box1_Destination_Y)&&(Player_Coordinates_X==Box1_Destination_X))
				    ||((Player_Coordinates_Y-1==Box2_Destination_Y)&&(Player_Coordinates_X==Box2_Destination_X))
				    ||((Player_Coordinates_Y-1==Box3_Destination_Y)&&(Player_Coordinates_X==Box3_Destination_X))
					||((Player_Coordinates_Y-1==Box4_Destination_Y)&&(Player_Coordinates_X==Box4_Destination_X)))
				       a[Player_Coordinates_Y-1][Player_Coordinates_X]='@';//判断箱子是否在目标点
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
		case 'r':longjmp(env1,1); //从此处（1号位置）跳转到env1
		break;                    //功能：重新开始
		case 'e':longjmp(env3,4); //跳转
		break;                    //功能：退出程序
		default: ;   //按下其他按键不响应
		break;
	}
}


void supplement_pushbox()  //目标点补充函数
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


void success_pushbox()  //通关之后运行
{
	//system("cls");//清屏函数
	fb_cons_clear();
    printf("\n\n !!! Congratulations on customs clearance !!! \n");
    printf("\n password is LTL19991015 \n");
    //setcolor(12); //浅红色爱心
    fb_set_fgcolor(12,1);
    float x,y;
    for (y = 1.3f; y > -1.0f; y -= 0.1f)  //画爱心图
    {
        for (x = -1.5f; x < 1.5f; x += 0.05f)
		{
            float a = x * x + y * y - 1;
            putchar(a * a * a - x * x * y * y * y <= 0.0f ? '*' : ' ');
        }
        putchar('\n');
    }
    //setcolor(7); //恢复深白色
    fb_set_fgcolor(15,1);
}


void output_pushbox()  //推箱子显示输出函数
{
    int i,j;
	for(i=0;i<15;i++)  //状态显示 puts(a[i])简单但，不可附加颜色
    {
	    for(j=0;(j<50) && (a[i][j]!=NULL);j++)
		{
		    if (a[i][j]=='O')
                fb_set_fgcolor(14,1);   //浅黄色箱子
			else if(a[i][j]=='M')
                fb_set_fgcolor(12,1);   //浅红色玩家
			else if(a[i][j]=='*')
                fb_set_fgcolor(10,1);   //浅绿色目标点
			else if(a[i][j]=='@')
                fb_set_fgcolor(10,1);   //浅绿色归位点
			else if(a[i][j]=='#')
                fb_set_fgcolor(11,1);   //浅青色墙壁
			else
                fb_set_fgcolor(15,1);   //浅白色其他
            printf("%c",a[i][j]);
            fb_set_fgcolor(15,1);       //恢复深白色
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

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(push_box, push box game);



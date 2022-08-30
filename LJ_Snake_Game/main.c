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
    #error "在bsp.h中选择配置 XPT2046_DRV 或者 GT1151_DRV"
           "XPT2046_DRV:  用于800*480 横屏的触摸屏."
           "GT1151_DRV:   用于480*800 竖屏的触摸屏."
           "如果都不选择, 注释掉本 error 信息, 然后自定义: LCD_display_mode[]"
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// 变量定义
//-------------------------------------------------------------------------------------------------

int TCSK[24][40]=  //贪吃模式显存 24行 40列 （数值不同颜色不同）
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

const int flg[4][2]={{1,0},{0,1},{0,-1},{-1,0}};//下右左上坐标变化规律
int n=24,m=40;  //n和m是蛇的活动范围20行20列+边框=可视化窗口大小
struct xcw{int x,y;}tang[5][15],foodd[2];//结构体tan存储每条蛇的坐标  结构体foodd记录两个食物坐标
int tot[5];     //tot[0123]是四条蛇的长度
int f[5];       //存储蛇的运行状态0123下右左上   搭配flg使用
int score;      //得分
int food;       //食物个数
int ms=1;       //初始--模式选择
int ans_len=1e9;
int vis[20][20];//记录食物存在点的矩阵  例如(foddd[0].x,foddd[0].y)有食物则vis[foddd[0].x][foddd[0].y]=1
int start_clock=0;  //起始锁

//-------------------------------------------------------------------------------------------------
// 函数声明
//-------------------------------------------------------------------------------------------------

void init_btn();     //按键初始化

void put_text(char *s, int i, unsigned coloridx);//字符串输出
char* choosemode(int i);        //辅助菜单
void show_menu();               //菜单
void result(int t);             //积分够了-胜利
void fnd(int t,int x,int y);    //撞到障碍物--失败

void mods(void);            //对应模式小蛇初始化
void draw(void);            //初始化贪吃蛇显存的边框数据
void clear_TCS(void);       //贪吃蛇显存清除
void display_TCS(void);	    //按照贪吃蛇模式显示
int  check(int x,int y);    //检验某一个点
void rand_food(void);       //没有食物时随机投喂
void change(int t);         //没有按键按下蛇的运行

int  ads(int x);                            //取绝对值
int  other(int x);                          //03交换、12交换
int get_food(int x,int y,int xxx,int yyy);  //辅助机器人找寻食物
void machine(int t,int x,int y);            //机器人寻路

void Up_intHandler();       //上
void Down_intHandler();     //下
void Left_intHandler();     //左
void Right_intHandler();    //右

void Up_intHandler_1();       //上
void Down_intHandler_1();     //下
void Left_intHandler_1();     //左
void Right_intHandler_1();    //右
//-------------------------------------------------------------------------------------------------
// 主程序
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
		while(ms==1)  //1单人模式
		{
			display_TCS();
			rand_food();
			change(0);
		}
		while(ms==2)	//2双人合作
		{
			display_TCS();	
			rand_food();
			change(0);
			change(1);
		}
		while(ms==3)	//3双人对抗
		{
			display_TCS();	
			rand_food();
			change(0);
			change(1);
		}
		while(ms==4)	//4机机对抗抢分
		{
			display_TCS();
			rand_food();
			change(2),change(3);
		}

		while(ms==5)	//5人机对抗抢分
		{
			display_TCS();
			rand_food();
			change(0);
			change(2);
		}
		while(ms==6)	//6双人对抗抢分
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
// 函数实现
//-------------------------------------------------------------------------------------------------




void init_btn()     //按键初始化
 {
    // gpio使能
     gpio_enable(BTN1, DIR_IN);
     gpio_enable(BTN2, DIR_IN);
     gpio_enable(BTN3, DIR_IN);
     gpio_enable(BTN4, DIR_IN);
     // 关中断
     ls1x_disable_gpio_interrupt(BTN1);
     ls1x_disable_gpio_interrupt(BTN2);
     ls1x_disable_gpio_interrupt(BTN3);
     ls1x_disable_gpio_interrupt(BTN4);
     // 安装中断向量
     ls1x_install_gpio_isr(BTN1, INT_TRIG_EDGE_UP, Up_intHandler, 0);
     ls1x_install_gpio_isr(BTN2, INT_TRIG_EDGE_UP, Down_intHandler, 0);
     ls1x_install_gpio_isr(BTN3, INT_TRIG_EDGE_UP, Left_intHandler, 0);
     ls1x_install_gpio_isr(BTN4, INT_TRIG_EDGE_UP, Right_intHandler, 0);
     //开中断
     ls1x_enable_gpio_interrupt(BTN1);
     ls1x_enable_gpio_interrupt(BTN2);
     ls1x_enable_gpio_interrupt(BTN3);
     ls1x_enable_gpio_interrupt(BTN4);
     
     // gpio使能
     gpio_enable(BTN5, DIR_IN);
     gpio_enable(BTN6, DIR_IN);
     gpio_enable(BTN7, DIR_IN);
     gpio_enable(BTN8, DIR_IN);
     // 关中断
     ls1x_disable_gpio_interrupt(BTN5);
     ls1x_disable_gpio_interrupt(BTN6);
     ls1x_disable_gpio_interrupt(BTN7);
     ls1x_disable_gpio_interrupt(BTN8);
     // 安装中断向量
     ls1x_install_gpio_isr(BTN5, INT_TRIG_EDGE_UP, Up_intHandler_1, 0);
     ls1x_install_gpio_isr(BTN6, INT_TRIG_EDGE_UP, Down_intHandler_1, 0);
     ls1x_install_gpio_isr(BTN7, INT_TRIG_EDGE_UP, Left_intHandler_1, 0);
     ls1x_install_gpio_isr(BTN8, INT_TRIG_EDGE_UP, Right_intHandler_1, 0);
     //开中断
     ls1x_enable_gpio_interrupt(BTN5);
     ls1x_enable_gpio_interrupt(BTN6);
     ls1x_enable_gpio_interrupt(BTN7);
     ls1x_enable_gpio_interrupt(BTN8);
 }


void put_text(char *s, int i, unsigned coloridx)    //字符串输出
{
    i = i+3;
    fb_put_string_center(400, i*20 ,s , coloridx);
}
char* choosemode(int i) //辅助菜单
{
    switch(i)
    {
        case 1: return "单人模式";
        case 2: return "双人合作";
        case 3: return "双人对抗";
        case 4: return "机机对抗--20分";
        case 5: return "人机对抗--20分";
        case 6: return "双人对抗--20分";
        default:return "单人模式";
    }
}
void show_menu()    //菜单
{
    fb_drawrect(90,40,720,360, 7);
    put_text("---------------------------------------------------------------------------",0,7);
    put_text("欢迎来到贪吃蛇大冒险游戏！",1,4);      // 红色
    put_text("游戏说明：",2, 7);                     // 白色
    put_text("使用btn1、btn2选择游戏模式，btn3进行确认",3,7);
    put_text("进入游戏后使用 btn1:向上、btn2:向下、btn3:向左、btn4:向右 对贪吃蛇进行操控！",4, 7);
    put_text("---------------------------------------------------------------------------",5,7);
    put_text("模式选择：",6,7);
    put_text("---------------------------------------------------------------------------",7,7);
    int i = 8;
    for(i=8;i<14;i++)     // 8行到13行
    {
        if(ms == i%7)                   // 模式和行数对应，显示黄色
           put_text(choosemode(i%7),i,6);
        else                                          // 否则，显示白色
           put_text(choosemode(i%7),i,7);
    }
    put_text("-----------------------------  李天凌 制  --------------------------------",14,7);
}
void result(int t)  //积分够了--胜利
{
	if(ms==5)           //人机对抗-抢分
	{
		if(t==0)
			put_text("胜利",1,4);
		else
			put_text("失败",1,4);
    }
	else if(ms==4)      //机机对抗-抢分
	{
	  if(t==2)
			put_text("机器人一积分足够，胜利",1,4);
		else
			put_text("机器人二积分足够，胜利",1,4);
    }
	else if(ms==6)      //对抗模式-抢分
	{
	    if(t==1)
            put_text("玩家一积分足够，胜利",1,4);
		  else
			put_text("玩家二积分足够，胜利",1,4);
    }
	else ;
	while(1); //结束
}
void fnd(int t,int x,int y)  //撞到障碍物--失败
{
	if(check(x,y)) return;   //check(x,y)为0则相撞
	if(ms==4)        //机机对抗-抢分
	{
	    if(t==3)
			put_text("机器人二撞墙",1,4);
	    else
			put_text("机器人一撞墙",1,4);
    }
	else if(ms==3||ms==6)   //对抗模式  对抗模式-抢分
    {
	    if(t==1)
			put_text("玩家二撞墙",1,4);
		else
			put_text("玩家一撞墙",1,4);
    }
	else
	{
		if(t==2)
			put_text("胜利",1,4);
		else
			put_text("失败",1,4);
    }
	score=0;
	while(1);  //结束
}



void mods(void)  //对应模式小蛇初始化
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
		f[0]=1;   //玩家1初始方向  向右
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
		f[1]=2;    //玩家2初始方向  向左
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
		f[2]=2;    //电脑1初始方向  向左
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
		f[3]=1;   //电脑2初始方向  向右
		for(i=1;i<=tot[3];i++)
			TCSK[tang[3][i].x][tang[3][i].y]=7;
	}
}
void draw(void)  //初始化贪吃蛇显存的边框数据
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
void clear_TCS(void)  //贪吃蛇显存清除
{
    int i,j;
	for(i=0;i<n;i++)
		for(j=0;j<m;j++)
			TCSK[i][j]=0;
}
void display_TCS(void)	//按照贪吃蛇模式显示
{
    unsigned int H,L; //H L 块坐标
    for(H=0;H<n;H++)
	   for(L=0;L<m;L++)
            fb_fillrect(L*20,H*20,(L+1)*20,(H+1)*20,TCSK[H][L]);
}
int check(int x,int y)   //检验某一个点
{
	if(x<1||x>(n-2)||y<1||y>(m-2))  //边界以及边界上外返回0
		return 0;
	int t=1;
	int i;
	if(ms!=4)
	    for(i=1;i<=tot[0];i++)   //在蛇身上返回0
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
	return t;    //都不在这些点返回1
}
void rand_food(void)  //没有食物时随机投喂
{
    int x=2,y=3;
    if(!vis[foodd[0].x][foodd[0].y]) //如果0号食物不见了
	{
		//x=(rand()%(n-3))+2,y=(rand()%(m-3))+2; //随机产生xy坐标y为偶数
		x=(get_clock_ticks()%(n-3))+2,y=(get_clock_ticks()%(m-3))+2; //随机产生xy坐标y为偶数
		while(!check(x,y))
			//x=(rand()%(n-3))+2,y=(rand()%(m-3))+2; //检验随机投喂的点，是否需要重新投喂
			x=(get_clock_ticks()%(n-3))+2,y=(get_clock_ticks()%(m-3))+2; //随机产生xy坐标y为偶数
		TCSK[x][y]=10;
	    food++;
		vis[x][y]=1;
		foodd[0].x=x;
		foodd[0].y=y;
	}
	if(!vis[foodd[1].x][foodd[1].y])  //第二个食物--同上
	{
		//x=rand()%n,y=rand()%m;
		x=get_clock_ticks()%n,y=get_clock_ticks()%m; //随机产生xy坐标y为偶数
		while(!check(x,y))
			//x=rand()%n,y=(rand()%m);
			x=get_clock_ticks()%n,y=get_clock_ticks()%m; //随机产生xy坐标y为偶数
		TCSK[x][y]=10;
		food++;
		vis[x][y]=1;
		foodd[1].x=x;
		foodd[1].y=y;
	}
}
void change(int t)  //没有按键按下蛇的运行--第t条蛇
{
    int i;
	int x=tang[t][1].x,y=tang[t][1].y;    //x和y是蛇头位置的xy
	if(t==2||t==3)                      //2号和3号蛇是电脑
		machine(t,x,y);                   //机器人寻路
	x+=flg[f[t]][0],y+=flg[f[t]][1];    //蛇头(x,y)按规律变化
	for(i=tot[t];i;i--)             //蛇的身子位置数据库刷新
	   {
	        tang[t][i+1]=tang[t][i];
	        TCSK[tang[t][i].x][tang[t][i].y]=t+4; //身子显示刷新  t+4颜色不同
		}
	TCSK[tang[t][tot[t]+1].x][tang[t][tot[t]+1].y]=0; //原来的蛇尾消失
	if(vis[x][y])  //如果吃掉一个食物的话
	{
		vis[x][y]=0,score+=(t==0||t==1),food--;
		if(++tot[t]>=10&&((ms==4)||(ms==5)||(ms==6)))  //有一方吃够分
			result(t);
		TCSK[tang[t][tot[t]].x][tang[t][tot[t]].y]=t+4; //尾巴处+1
	}
	fnd(t,x,y);   //一方撞墙撞蛇则失败
	tang[t][1].x=x;    //把蛇头位置反馈到全局变量中
	tang[t][1].y=y;
	TCSK[tang[t][1].x][tang[t][1].y]=5;  //新蛇头出现
}


int ads(int x)  //取绝对值
{
    return x<0?-x:x; //x是否<0,如果x<0的话就执行:前面的语句，如果x不小于0的话就执行:后面的语句
}
int other(int x)  //03交换、12交换  对立方向
{
	if     (x==0) return 3;
	else if(x==1) return 2;
	else if(x==2) return 1;
	else          return 0;
}
int get_food(int x,int y,int xxx,int yyy)  //辅助机器人找寻食物
{
    return ads(x-xxx)+ads(y-yyy);
}
void machine(int t,int x,int y)  //机器人寻路
{
    int i;
	int foodid,minn=1e9,newf=f[t];//最小距离初始值很大
	if((get_food(x,y,foodd[0].x,foodd[0].y)<=get_food(x,y,foodd[1].x,foodd[1].y)&&vis[foodd[0].x][foodd[0].y])||!vis[foodd[1].x][foodd[1].y])
		foodid=0;
	else
		foodid=1;
	for(i=0;i<4;i++)
		if(f[t]^other(i))//确保不是身后
		{
			if(check(x+flg[i][0],y+flg[i][1])) //确保前方不是障碍
			{
				int now=get_food(x+flg[i][0],y+flg[i][1],foodd[foodid].x,foodd[foodid].y); //计算新的距离————下一方向下一时刻距离
				if(now<minn)
					newf=i,minn=now;
			}
		}
	f[t]=newf;
}


 // 按键1中断服务程序   上  mode--
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
 // 按键2中断服务程序   下` mode++
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
 // 按键3中断服务程序   左  确认
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
// 按键4中断服务程序    右
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
 // 按键5中断服务程序   上  mode--
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
 // 按键6中断服务程序   下  mode++
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
 // 按键7中断服务程序   左  确认
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
// 按键8中断服务程序    右
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

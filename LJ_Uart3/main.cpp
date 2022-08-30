/*
 * 本代码实现一个 UART3 的驱动程序
 */

#include "bsp.h"
#include "mips.h"
#include "stdio.h"
#include "libc/uart3.c"

//#define TEST_POINTER
//#define TEST_NOASSIGN
//#define TEST_MATCHING
//#define TEST_RETURN_NUMBER
//#define TEST_INTEGER_D
//#define TEST_INTEGER_O
//#define TEST_INTEGER_X
//#define TEST_FLOAT
//#define TEST_CHAR
//#define TEST_STRING
#define TEST_SCAN

int main()
{
    int     a1 = 0 ,a2 = 0,a3 = 0 ;
    float   b1 = 0 ,b2 = 0,b3 = 0 ;
    char    c1 ='0',c2 ='0' ;
    int     *p = NULL;
    char    string1[10],string2[10];
    uart3_initialize(115200,8,'N',1);   //初始化串口
#ifdef TEST_POINTER
        a1 = 1;a2 = 2;a3 = 3;
        iprintf(" &a1=%p\r\n &a2=%p\r\n &a3=%p\r\n",&a1,&a2,&a3);
#endif
    for(int i=0;i<5;i++)
    {
#ifdef TEST_POINTER
        scanf("%p",&p);
        iprintf("p=%p \r\n",p);
        iprintf("*p=%d \r\n",*p);
#endif
#ifdef TEST_NOASSIGN
        printf("hello UART5 %d %d \n",a1,a2);
        iprintf("hello UART3 %d %d \r\n",a1,a2);
        scanf("%*d %d",&a1,&a2);
#endif
#ifdef TEST_MATCHING
        printf("hello UART5 %d %d \n",a1,a2);
        iprintf("hello UART3 %d %d \r\n",a1,a2);
        scanf("input is %d,%d",&a1,&a2);
#endif
#ifdef TEST_RETURN_NUMBER
        printf("hello UART5 %d %d \n",a1,a2);
        iprintf("hello UART3 %d %d \r\n",a1,a2);
        scanf("%d %n",&a1,&a2);
#endif
#ifdef TEST_INTEGER_D
        printf("hello UART5 %d %d %d \n",a1,a2,a3);
        iprintf("hello UART3 %d %d %d \r\n",a1,a2,a3);
        scanf("%2d %3d",&a1,&a2);
        a3 = a1 + a2;
#endif
#ifdef TEST_INTEGER_O
        printf("hello UART5 %d %d %d \n",a1,a2,a3);
        iprintf("hello UART3 %d %d %d \r\n",a1,a2,a3);
        scanf("%o %o",&a1,&a2);
        a3 = a1 + a2;
#endif
#ifdef TEST_INTEGER_X
        printf("hello UART5 %d %d %d \n",a1,a2,a3);
        iprintf("hello UART3 %d %d %d \r\n",a1,a2,a3);
        scanf("%x %x",&a1,&a2);
        a3 = a1 + a2;
#endif
#ifdef TEST_FLOAT
        printf("hello UART5 %f %f %f \n",b1,b2,b3);
        iprintf("hello UART3 %f %f %f \r\n",b1,b2,b3);
        scanf("%2f %3f",&b1,&b2);
        b3 = b1 + b2;
#endif
#ifdef TEST_CHAR
        printf("hello UART5 %c %c \n",c1,c2);
        iprintf("hello UART3 %c %c \r\n",c1,c2);
        scanf("%c %c",&c1,&c2);
#endif
#ifdef TEST_STRING
        printf("hello UART5 %s %s \n",string1,string2);
        iprintf("hello UART3 %s %s \r\n",string1,string2);
        scanf("%s %s",&string1,&string2);
#endif
#ifdef TEST_SCAN
        printf("hello UART5 %s %s \n",string1,string2);
        iprintf("hello UART3 %s %s \r\n",string1,string2);
        scanf("%[^a] %[^ ]",&string1,&string2);
#endif
    }
    printf("hello UART5");
}

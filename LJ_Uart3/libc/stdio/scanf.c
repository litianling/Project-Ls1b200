/*
 * scanf.c
 *
 * created: 2021/10/26
 *  author: 
 */

#include "bsp.h"
#include "scanf.h"

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>

#define FL_NOASSIGN         (1U <<  0U)  
#define FL_WIDTHSPEC        (1U <<  1U)  
#define FL_SHORT            (1U <<  2U)  
#define FL_LONG             (1U <<  3U)  
#define FL_LONGDOUBLE       (1U <<  4U)  
#define FL_NEGATIVE         (1U <<  5U)

#define SUPPORT_NOASSIGN
#define SUPPORT_RETURN_NUMBER
#define SUPPORT_INTEGER
#define SUPPORT_FLOAT
#define SUPPORT_CHAR
#define SUPPORT_STRING
#define SUPPORT_SCAN

// 分配输入数据缓存区（数据流最大空间），并初始化
#define buf_size    50
unsigned char buf_in[buf_size] = {0};

//清楚数据缓存区
void clean_buf()
{
    int i=0;
    for(;i<buf_size;i++)
        buf_in[i]=0;
}


int _doscan(const char *format, va_list ap)
{
    int             flags;          // 标志
    int             kind;           // 数据类型
    register int    ic = 0;         // 从数据缓存数组读取到的字符
    int             nrchars = 0;    // 已读取字符数
    int             done = 0;       // 完成项目数(已经输入几个变量)
    unsigned        precision = buf_size;  // 精度（实数小数点位数，整数宽度），默认最大

    int             base;           // 整数进制
    long            val=0;          // 一个无符号整数值
    register char   *str;           // 临时指针

#ifdef SUPPORT_FLOAT
    double          val_f1=0;       // 一个浮点数
    double          val_f2=0;       // 一个浮点数
    register int    ic_buf = 0;     // 从数据缓存数组读取到的字符
    int             nrchars_buf = 0;// 已读取字符数
    int             nrchars_precision_buf = 0;  //用来控制浮点数精度
#endif

#ifdef SUPPORT_SCAN
    char            char_buf = 0;   // 扫描模式结束符
#endif



    if (!*format) return 0;     // 控制流为空，立即结束
    
    while (1)
    {
        if (isspace(*format))   // 如果控制流开头就是没用的空格
        {
            while (isspace(*format))
                format++;       // 跳过控制流那些空白符号
        }
        if (!*format) break;    // 控制流为空，立即结束


        ic = buf_in[nrchars];   // 从数据流读取一个字符
        nrchars++;
        while (isspace (ic))    // 数据流跳过空白符号
        {
            ic = buf_in[nrchars];
            nrchars++;
        }
        if (ic == 0) break;      // 除了空格之外没输入，直接退出
        else  ;                  // 否则，数据流保留读取到的第一个数据


        if (*format != '%') break;   // 如果控制流检测到的不是百分号，出错跳出去
        else format++;               // 如果控制流是%，准备读取%之后的内容
            
        
        flags = 0;
        if (*format == '*')     // 如果检测到"*"符号，只读取不存储
        {
            format++;
            flags |= FL_NOASSIGN;// 只标志位变化（按位或，置一）
        }

#ifdef SUPPORT_NOASSIGN
        if (isdigit (*format))  // 如果控制流的是数字，分配精度
        {                       //整数位数，实数为小数点后位数
            flags |= FL_WIDTHSPEC;
            for (precision = 0; isdigit (*format);)
                precision = precision * 10 + *format++ - '0';
        }
#endif

        switch (*format)        // 如果在控制流检测到特殊说明符h,l,L
        {                       // 则利用按位或运算将对应标志位置一
            case 'h': flags |= FL_SHORT;        format++; break;
            case 'l': flags |= FL_LONG;         format++; break;
            case 'L': flags |= FL_LONGDOUBLE;   format++; break;
        }
        


        kind = *format;     // 控制流接下来肯定是数据类型

#ifdef SUPPORT_RETURN_NUMBER
        if(kind=='n')       // 数据类型是返回已经读取字符数
        {
            if (!(flags & FL_NOASSIGN))
            {
                if (flags & FL_SHORT)
                    *va_arg(ap, short *) = (short) --nrchars;     //获取可变参数的当前参数，返回指定类型并将指针指向下一参数
                else if (flags & FL_LONG)
                    *va_arg(ap, long *) = (long) --nrchars;
                else
                    *va_arg(ap, int *) = (int) --nrchars;
            }
            while(!isspace(ic)) // 抹除非规范数据
            {
                nrchars++;
                ic = buf_in[nrchars];
            }
        }
#endif

//八进制、十进制、十六进制整数、指针
#ifdef SUPPORT_INTEGER
        else if(kind=='o' || kind=='d' || kind=='x' || kind=='X' || kind=='p' )
        {
            if(ic=='-')     // 先判断输入数据的正负
            {
                flags |= FL_NEGATIVE;     // 如果为负数则将符号标志置一
                ic = buf_in[nrchars];
                nrchars++;
            }
            int width = 0;  //用width与precision控制读入数据的位数
            if (kind=='o')  //八进制整数
            {
                base=8;
                while(ic>=48 && ic<=55 && width<precision)
                {
                    val = val*base + (ic-48);
                    ic = buf_in[nrchars];
                    nrchars++;
                    width++;
                }
            }
            else if (kind=='d') //十进制整数
            {
                base=10;
                while(isdigit(ic) && width<precision)
                {
                    val = val*base + (ic-48);
                    ic = buf_in[nrchars];
                    nrchars++;
                    width++;
                }
            }
            else    //十六进制整数
            {
                base=16;
                while((isdigit (ic) || ( ic>64 && ic<71 ) || ( ic>96 && ic<103 )) && width<precision)
                {
                    if(isdigit (ic))
                        val = val*base + (ic-48);
                    else if( ic>64 && ic<71 )
                        val = val*base + (ic-65+10);
                    else
                        val = val*base + (ic-97+10);
                    ic = buf_in[nrchars];
                    nrchars++;
                    width++;
                }
            }
            width = 0;  // 数据转换完毕后将width归零
            if(flags & FL_NEGATIVE) // 按位与运算，如果符号标志位为1则将正数val取反
                val = -1*val;
            if(!(flags & FL_NOASSIGN))  // 不是只读模式，才会输出
            {                           // 根据特殊说明符，附加数据类型
                if (flags & FL_LONG)
                    *va_arg(ap, unsigned long *) = (unsigned long) val;
                else if (flags & FL_SHORT)
                    *va_arg(ap, unsigned short *) = (unsigned short) val;
                else
                    *va_arg(ap, unsigned *) = (unsigned) val;
            }
            val = 0;
            while(!isspace(ic)) //  抹除非规范数据
            {
                nrchars++;
                ic = buf_in[nrchars];
            }
        }
#endif

#ifdef SUPPORT_FLOAT
        else if(kind=='e'||kind=='E'||kind=='f'||kind=='F'||kind=='g'||kind=='G')
        {   // 如果输入的是浮点数
            if(ic=='-') // 先判断正负
            {
                flags |= FL_NEGATIVE;
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(isdigit(ic)) // 然后处理整数部分的数据转换
            {
                val_f1 = val_f1 *10 + (ic-48);
                ic = buf_in[nrchars];
                nrchars++;
            }
            if(ic=='.') // 如果识别到小数点
            {
                nrchars_precision_buf = nrchars; //记录下小数点后一位位置，用于辅助精度控制
                ic = buf_in[nrchars];
                nrchars++;
                while(isdigit(ic))  // 一直向后扫描，直至小数部分结束
                {
                    ic = buf_in[nrchars];
                    nrchars++;
                }
                ic_buf=ic;                      // 记录当前扫描的位置
                nrchars_buf=nrchars;            // 将当前位置存储在缓存区，以便之后恢复
                if(nrchars > nrchars_precision_buf + precision + 1)
                    nrchars = nrchars_precision_buf + precision - 1 ;
                else                // 根据（精度/小数长度）更改小数部分数据类型转换的起点
                    nrchars = nrchars - 2;
                ic = buf_in[nrchars];
                nrchars--;
                while(isdigit(ic))  // 小数部分数据类型转换，与整数不同为从后向前迭代
                {
                    val_f2 = val_f2 *0.1 + (ic-48)*0.1;
                    ic = buf_in[nrchars];
                    nrchars--;
                }
                ic=ic_buf;          // 恢复之前记录的小数结束的位置与数据
                nrchars=nrchars_buf;
            }
            val_f2 = val_f2 + val_f1; // 小数部分与整数部分合一
            if(flags & FL_NEGATIVE)             // 根据符号标志位更改数据正负
                val_f2 = -1 * val_f2;
            if(!(flags & FL_NOASSIGN)) //不是只读取不存储才输出
            {
                if (flags & FL_LONGDOUBLE)
                    *va_arg(ap, long double *) = (long double) val_f2;
                else if (flags & FL_LONG)
                    *va_arg(ap, double *) = (double) val_f2;
                else
                    *va_arg(ap, float *) = (float) val_f2;
            }
            while(!isspace(ic)) // 抹除非规范数据
            {
                nrchars++;
                ic = buf_in[nrchars];
            }
            val_f1 = 0;     // 整数与小数状态初始化
            val_f2 = 0;     // 以便输入下一个数据
        }
#endif

#ifdef SUPPORT_CHAR
        else if(kind=='c')  // 字符类型（非空）
        {
            while(isspace(ic)) // 消除字符串的空白
            {
                ic = buf_in[nrchars];
                nrchars++;
            }
            if(!(flags & FL_NOASSIGN))  // 不是只读取不存储才输出
                *va_arg(ap, char *) = (char) ic;
        }
#endif

#ifdef SUPPORT_STRING
        else if(kind=='s')  // 字符串类型，遇到空格结束
        {
            str = va_arg(ap, char *); // 这里十分巧妙，区分str指针加一与可变参数列表指针加一
            while(isspace(ic))      // 消除空格
            {
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(ic!=0 && ic!=32 && ic!=10 && ic!=13) // 遇到空、空格、回车、换行结束
            {
                *str++ = (char) ic;
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(*str!=0)  //擦除原先字符串保存的多余的内容，以便对字符串重复赋值
                *str++=0;
        }
#endif

#ifdef SUPPORT_SCAN
        else if(kind=='[' && *(format+1)=='^' && *(format+3)==']')  //扫描模式输入字符串
        {
            if(*(format+2)!=' ') // 不是空格则作为结束符，是空格不修改扫描到换行才结束
                char_buf =  *(format+2);
            format = format + 3;
            str = va_arg(ap, char *); // 同上用于字符串连续输入
            while(isspace(ic))  // 清除数据流的空格
            {
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(ic!=0 && ic!=char_buf && ic!=10 && ic!=13) //遇到空、扫描结束字符、回车、换行结束
            {
                *str++ = (char) ic;
                ic = buf_in[nrchars];
                nrchars++;
            }
            char_buf = 0;
            while(*str!=0)  //擦除原先字符串保存的多余的内容
                *str++=0;
        }
#endif

        else done--; // 数据类型不满足以上所有情况，说明没输出
        precision = buf_size ; // 每次输入数据结束都重置精度为默认最大值
        if (!(flags & FL_NOASSIGN) && kind != 'n') done++;
        format++;
    }
    return  done ;
}


int scanf(const char *format, ...)
{
    uart3_initialize(115200,8,'N',1); // 初始化串口三
    uart3_read(buf_in,buf_size);    // 从串口读取数据到缓存区
    va_list ap;             // 定义可变参数列表
    int retval;
    va_start(ap, format);   // 获取可变参数列表的第一个参数的地址
    retval = _doscan(format, ap);
    va_end(ap);             // 清空va_list可变参数列表
    clean_buf();            // 清除数据缓存区
    return retval;
}


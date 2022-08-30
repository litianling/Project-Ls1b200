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

// �����������ݻ����������������ռ䣩������ʼ��
#define buf_size    50
unsigned char buf_in[buf_size] = {0};

//������ݻ�����
void clean_buf()
{
    int i=0;
    for(;i<buf_size;i++)
        buf_in[i]=0;
}


int _doscan(const char *format, va_list ap)
{
    int             flags;          // ��־
    int             kind;           // ��������
    register int    ic = 0;         // �����ݻ��������ȡ�����ַ�
    int             nrchars = 0;    // �Ѷ�ȡ�ַ���
    int             done = 0;       // �����Ŀ��(�Ѿ����뼸������)
    unsigned        precision = buf_size;  // ���ȣ�ʵ��С����λ����������ȣ���Ĭ�����

    int             base;           // ��������
    long            val=0;          // һ���޷�������ֵ
    register char   *str;           // ��ʱָ��

#ifdef SUPPORT_FLOAT
    double          val_f1=0;       // һ��������
    double          val_f2=0;       // һ��������
    register int    ic_buf = 0;     // �����ݻ��������ȡ�����ַ�
    int             nrchars_buf = 0;// �Ѷ�ȡ�ַ���
    int             nrchars_precision_buf = 0;  //�������Ƹ���������
#endif

#ifdef SUPPORT_SCAN
    char            char_buf = 0;   // ɨ��ģʽ������
#endif



    if (!*format) return 0;     // ������Ϊ�գ���������
    
    while (1)
    {
        if (isspace(*format))   // �����������ͷ����û�õĿո�
        {
            while (isspace(*format))
                format++;       // ������������Щ�հ׷���
        }
        if (!*format) break;    // ������Ϊ�գ���������


        ic = buf_in[nrchars];   // ����������ȡһ���ַ�
        nrchars++;
        while (isspace (ic))    // �����������հ׷���
        {
            ic = buf_in[nrchars];
            nrchars++;
        }
        if (ic == 0) break;      // ���˿ո�֮��û���룬ֱ���˳�
        else  ;                  // ����������������ȡ���ĵ�һ������


        if (*format != '%') break;   // �����������⵽�Ĳ��ǰٷֺţ���������ȥ
        else format++;               // �����������%��׼����ȡ%֮�������
            
        
        flags = 0;
        if (*format == '*')     // �����⵽"*"���ţ�ֻ��ȡ���洢
        {
            format++;
            flags |= FL_NOASSIGN;// ֻ��־λ�仯����λ����һ��
        }

#ifdef SUPPORT_NOASSIGN
        if (isdigit (*format))  // ����������������֣����侫��
        {                       //����λ����ʵ��ΪС�����λ��
            flags |= FL_WIDTHSPEC;
            for (precision = 0; isdigit (*format);)
                precision = precision * 10 + *format++ - '0';
        }
#endif

        switch (*format)        // ����ڿ�������⵽����˵����h,l,L
        {                       // �����ð�λ�����㽫��Ӧ��־λ��һ
            case 'h': flags |= FL_SHORT;        format++; break;
            case 'l': flags |= FL_LONG;         format++; break;
            case 'L': flags |= FL_LONGDOUBLE;   format++; break;
        }
        


        kind = *format;     // �������������϶�����������

#ifdef SUPPORT_RETURN_NUMBER
        if(kind=='n')       // ���������Ƿ����Ѿ���ȡ�ַ���
        {
            if (!(flags & FL_NOASSIGN))
            {
                if (flags & FL_SHORT)
                    *va_arg(ap, short *) = (short) --nrchars;     //��ȡ�ɱ�����ĵ�ǰ����������ָ�����Ͳ���ָ��ָ����һ����
                else if (flags & FL_LONG)
                    *va_arg(ap, long *) = (long) --nrchars;
                else
                    *va_arg(ap, int *) = (int) --nrchars;
            }
            while(!isspace(ic)) // Ĩ���ǹ淶����
            {
                nrchars++;
                ic = buf_in[nrchars];
            }
        }
#endif

//�˽��ơ�ʮ���ơ�ʮ������������ָ��
#ifdef SUPPORT_INTEGER
        else if(kind=='o' || kind=='d' || kind=='x' || kind=='X' || kind=='p' )
        {
            if(ic=='-')     // ���ж��������ݵ�����
            {
                flags |= FL_NEGATIVE;     // ���Ϊ�����򽫷��ű�־��һ
                ic = buf_in[nrchars];
                nrchars++;
            }
            int width = 0;  //��width��precision���ƶ������ݵ�λ��
            if (kind=='o')  //�˽�������
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
            else if (kind=='d') //ʮ��������
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
            else    //ʮ����������
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
            width = 0;  // ����ת����Ϻ�width����
            if(flags & FL_NEGATIVE) // ��λ�����㣬������ű�־λΪ1������valȡ��
                val = -1*val;
            if(!(flags & FL_NOASSIGN))  // ����ֻ��ģʽ���Ż����
            {                           // ��������˵������������������
                if (flags & FL_LONG)
                    *va_arg(ap, unsigned long *) = (unsigned long) val;
                else if (flags & FL_SHORT)
                    *va_arg(ap, unsigned short *) = (unsigned short) val;
                else
                    *va_arg(ap, unsigned *) = (unsigned) val;
            }
            val = 0;
            while(!isspace(ic)) //  Ĩ���ǹ淶����
            {
                nrchars++;
                ic = buf_in[nrchars];
            }
        }
#endif

#ifdef SUPPORT_FLOAT
        else if(kind=='e'||kind=='E'||kind=='f'||kind=='F'||kind=='g'||kind=='G')
        {   // ���������Ǹ�����
            if(ic=='-') // ���ж�����
            {
                flags |= FL_NEGATIVE;
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(isdigit(ic)) // Ȼ�����������ֵ�����ת��
            {
                val_f1 = val_f1 *10 + (ic-48);
                ic = buf_in[nrchars];
                nrchars++;
            }
            if(ic=='.') // ���ʶ��С����
            {
                nrchars_precision_buf = nrchars; //��¼��С�����һλλ�ã����ڸ������ȿ���
                ic = buf_in[nrchars];
                nrchars++;
                while(isdigit(ic))  // һֱ���ɨ�裬ֱ��С�����ֽ���
                {
                    ic = buf_in[nrchars];
                    nrchars++;
                }
                ic_buf=ic;                      // ��¼��ǰɨ���λ��
                nrchars_buf=nrchars;            // ����ǰλ�ô洢�ڻ��������Ա�֮��ָ�
                if(nrchars > nrchars_precision_buf + precision + 1)
                    nrchars = nrchars_precision_buf + precision - 1 ;
                else                // ���ݣ�����/С�����ȣ�����С��������������ת�������
                    nrchars = nrchars - 2;
                ic = buf_in[nrchars];
                nrchars--;
                while(isdigit(ic))  // С��������������ת������������ͬΪ�Ӻ���ǰ����
                {
                    val_f2 = val_f2 *0.1 + (ic-48)*0.1;
                    ic = buf_in[nrchars];
                    nrchars--;
                }
                ic=ic_buf;          // �ָ�֮ǰ��¼��С��������λ��������
                nrchars=nrchars_buf;
            }
            val_f2 = val_f2 + val_f1; // С���������������ֺ�һ
            if(flags & FL_NEGATIVE)             // ���ݷ��ű�־λ������������
                val_f2 = -1 * val_f2;
            if(!(flags & FL_NOASSIGN)) //����ֻ��ȡ���洢�����
            {
                if (flags & FL_LONGDOUBLE)
                    *va_arg(ap, long double *) = (long double) val_f2;
                else if (flags & FL_LONG)
                    *va_arg(ap, double *) = (double) val_f2;
                else
                    *va_arg(ap, float *) = (float) val_f2;
            }
            while(!isspace(ic)) // Ĩ���ǹ淶����
            {
                nrchars++;
                ic = buf_in[nrchars];
            }
            val_f1 = 0;     // ������С��״̬��ʼ��
            val_f2 = 0;     // �Ա�������һ������
        }
#endif

#ifdef SUPPORT_CHAR
        else if(kind=='c')  // �ַ����ͣ��ǿգ�
        {
            while(isspace(ic)) // �����ַ����Ŀհ�
            {
                ic = buf_in[nrchars];
                nrchars++;
            }
            if(!(flags & FL_NOASSIGN))  // ����ֻ��ȡ���洢�����
                *va_arg(ap, char *) = (char) ic;
        }
#endif

#ifdef SUPPORT_STRING
        else if(kind=='s')  // �ַ������ͣ������ո����
        {
            str = va_arg(ap, char *); // ����ʮ���������strָ���һ��ɱ�����б�ָ���һ
            while(isspace(ic))      // �����ո�
            {
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(ic!=0 && ic!=32 && ic!=10 && ic!=13) // �����ա��ո񡢻س������н���
            {
                *str++ = (char) ic;
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(*str!=0)  //����ԭ���ַ�������Ķ�������ݣ��Ա���ַ����ظ���ֵ
                *str++=0;
        }
#endif

#ifdef SUPPORT_SCAN
        else if(kind=='[' && *(format+1)=='^' && *(format+3)==']')  //ɨ��ģʽ�����ַ���
        {
            if(*(format+2)!=' ') // ���ǿո�����Ϊ���������ǿո��޸�ɨ�赽���вŽ���
                char_buf =  *(format+2);
            format = format + 3;
            str = va_arg(ap, char *); // ͬ�������ַ�����������
            while(isspace(ic))  // ����������Ŀո�
            {
                ic = buf_in[nrchars];
                nrchars++;
            }
            while(ic!=0 && ic!=char_buf && ic!=10 && ic!=13) //�����ա�ɨ������ַ����س������н���
            {
                *str++ = (char) ic;
                ic = buf_in[nrchars];
                nrchars++;
            }
            char_buf = 0;
            while(*str!=0)  //����ԭ���ַ�������Ķ��������
                *str++=0;
        }
#endif

        else done--; // �������Ͳ������������������˵��û���
        precision = buf_size ; // ÿ���������ݽ��������þ���ΪĬ�����ֵ
        if (!(flags & FL_NOASSIGN) && kind != 'n') done++;
        format++;
    }
    return  done ;
}


int scanf(const char *format, ...)
{
    uart3_initialize(115200,8,'N',1); // ��ʼ��������
    uart3_read(buf_in,buf_size);    // �Ӵ��ڶ�ȡ���ݵ�������
    va_list ap;             // ����ɱ�����б�
    int retval;
    va_start(ap, format);   // ��ȡ�ɱ�����б�ĵ�һ�������ĵ�ַ
    retval = _doscan(format, ap);
    va_end(ap);             // ���va_list�ɱ�����б�
    clean_buf();            // ������ݻ�����
    return retval;
}


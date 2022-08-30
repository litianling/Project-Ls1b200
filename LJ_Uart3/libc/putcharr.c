#include "uart3.h"


int _putchar1(int ch) //这里的ch是character的缩写，是字符的意思，这里的ch值应该是需要显示的单个字符对应的ASC码值
{
    uart3_write((unsigned char*)&ch,1);  //强制类型转换, 指针变量的 地址数据 不变, 影响的只是指针的寻址(偏移)      经典操作：取地址再指针
    return 0;
}


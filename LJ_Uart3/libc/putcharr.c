#include "uart3.h"


int _putchar1(int ch) //�����ch��character����д�����ַ�����˼�������chֵӦ������Ҫ��ʾ�ĵ����ַ���Ӧ��ASC��ֵ
{
    uart3_write((unsigned char*)&ch,1);  //ǿ������ת��, ָ������� ��ַ���� ����, Ӱ���ֻ��ָ���Ѱַ(ƫ��)      ���������ȡ��ַ��ָ��
    return 0;
}


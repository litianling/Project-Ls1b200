#include "uart3.h"

int getch(int ch) //�����ch��character����д�����ַ�����˼�������chֵӦ������Ҫ��ʾ�ĵ����ַ���Ӧ��ASC��ֵ
{
    uart3_read((unsigned char*)&ch,1);  //ǿ������ת��, ָ������� ��ַ���� ����, Ӱ���ֻ��ָ���Ѱַ(ƫ��)      ���������ȡ��ַ��ָ��
    return 0;
}


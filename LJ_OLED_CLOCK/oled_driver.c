/*
 * oled_driver.c
 *
 * created: 2021/11/5
 *  author:
 */
#include "oled_driver.h"
#include "ls1x_i2c_bus.h"
#include "picture.h"
#include "chars.h"
//-------------------------------------------------------------------------------------------------
// IIC基本函数
//-------------------------------------------------------------------------------------------------


void Write_IIC_Command(unsigned char IIC_Command){
    //start the transimition
    ls1x_i2c_send_start(busI2C0, Addr);
    //ask the device and config to write mode
    ls1x_i2c_send_addr(busI2C0, Addr, rw);
    //get the command need to be sent
    cmd_buf[1] = IIC_Command;
    //send the command
    ls1x_i2c_write_bytes(busI2C0, (uint8_t*)cmd_buf, 2);
    //close the trasmition
    ls1x_i2c_send_stop(busI2C0, Addr);
}

void Write_IIC_Data(unsigned char IIC_Data){
    //start the transmition
    ls1x_i2c_send_start(busI2C0, Addr);
    //ask for the write and wait for reply
    ls1x_i2c_send_addr(busI2C0, Addr, rw);
    //get the data to sent
    data_buf[1] = IIC_Data;
    //sent the data
    ls1x_i2c_write_bytes(busI2C0, (uint8_t*)data_buf, 2);
    //data sented close the transmition
    ls1x_i2c_send_stop(busI2C0, Addr);
}

void Initial_M096128x64_ssd1306()
{
	Write_IIC_Command(0xAE);   //display off
	Write_IIC_Command(0x20);	//Set Memory Addressing Mode
	Write_IIC_Command(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	Write_IIC_Command(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	Write_IIC_Command(0xc8);	//Set COM Output Scan Direction
	Write_IIC_Command(0x00);//---set low column address
	Write_IIC_Command(0x10);//---set high column address
	Write_IIC_Command(0x40);//--set start line address
	Write_IIC_Command(0x81);//--set contrast control register
	Write_IIC_Command(0xdf);
	Write_IIC_Command(0xa1);//--set segment re-map 0 to 127
	Write_IIC_Command(0xa6);//--set normal display
	Write_IIC_Command(0xa8);//--set multiplex ratio(1 to 64)
	Write_IIC_Command(0x3F);//
	Write_IIC_Command(0xa4);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	Write_IIC_Command(0xd3);//-set display offset
	Write_IIC_Command(0x00);//-not offset
	Write_IIC_Command(0xd5);//--set display clock divide ratio/oscillator frequency
	Write_IIC_Command(0xf0);//--set divide ratio
	Write_IIC_Command(0xd9);//--set pre-charge period
	Write_IIC_Command(0x22); //
	Write_IIC_Command(0xda);//--set com pins hardware configuration
	Write_IIC_Command(0x12);
	Write_IIC_Command(0xdb);//--set vcomh
	Write_IIC_Command(0x20);//0x20,0.77xVcc
	Write_IIC_Command(0x8d);//--set DC-DC enable
	Write_IIC_Command(0x14);//
	Write_IIC_Command(0xaf);//--turn on oled panel
}

//-------------------------------------------------------------------------------------------------
// IIC图片显示
//-------------------------------------------------------------------------------------------------

void Picture(int i)
{
  unsigned char y;
  unsigned char x;
  for(y=0;y<8;y++)
    {
      Write_IIC_Command(0xb0+y);
      Write_IIC_Command(0x0);
      Write_IIC_Command(0x10);

      for(x=0;x<128;x++)
			{
				if(i==1)
					Write_IIC_Data(show[y][x]);
				else if(i==2)
					Write_IIC_Data(biaoqinbao[y][x]);
				else ;
			}
    }
}

void fill_picture(unsigned char fill_Data)
{
    unsigned char m;
    unsigned char n;
	for(m=0;m<8;m++)
	{
		Write_IIC_Command(0xb0+m);	//page0-page1
		Write_IIC_Command(0x00);		//low column start address
		Write_IIC_Command(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				Write_IIC_Data(fill_Data);
			}
	}
}

//-------------------------------------------------------------------------------------------------
// IIC字符显示
//-------------------------------------------------------------------------------------------------

void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr)
{
	unsigned char nomber=0;
	int i;
	nomber=chr-' ';//得到偏移后的值即ASC码偏移量  设置空格为0号字符
	if(x>127)	//如果超出这一行自动跳转到下一行（+2）
	{
		x=0;
		y=y+2;
	}
   Write_IIC_Command(0xb0+y);
   Write_IIC_Command(0x00+x%16); //低四位横坐标
   Write_IIC_Command(0x10+x/16);//高四位横坐标
	for(i=0;i<8;i++)
		Write_IIC_Data(L8H16[nomber*2][i]);

   Write_IIC_Command(0xb0+y+1);
   Write_IIC_Command(0x00+x%16);
   Write_IIC_Command(0x10+x/16);
	for(i=0;i<8;i++)
		Write_IIC_Data(L8H16[nomber*2+1][i]);

}

void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr)
{
	unsigned char i=0;
	while (chr[i]!='\0')	//不是字符串的结束则一直循环
	{
		OLED_ShowChar(x,y,chr[i]);	//在x，y处显示字符
		x+=8;												//x=x+8 列地址加8准备显示下一字符
		if(x>120)										//位置不够显示当前字符，去下一行显示
		{
			x=0;
			y+=2;
		}
		i++;	//扫描下一字符
	}
}

void OLED_ShowString_Short(unsigned char  x,unsigned char  y, unsigned char  *chr,unsigned char  l)
{
	unsigned char i=0;
	while (chr[i]!='\0'&&i<l)	//不是字符串并且小于长度
	{
		OLED_ShowChar(x,y,chr[i]);	//在x，y处显示字符
		x+=8;												//x=x+8 列地址加8准备显示下一字符
		if(x>120)										//位置不够显示当前字符，去下一行显示
		{
			x=0;
			y+=2;
		}
		i++;	//扫描下一字符
	}
}



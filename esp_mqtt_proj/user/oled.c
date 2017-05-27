/*
 * oled.c
 *
 *  Created on: 2017年5月11日
 *      Author: zhangtian
 */

#include <c_types.h>
#include <osapi.h>
#include <driver/i2c_master.h>
#include "oled.h"
#include "codetab.h"

void ICACHE_FLASH_ATTR
Delay_ms(int ms)
{
    ms *= 10;
    while(ms--);
}

static bool ICACHE_FLASH_ATTR
I2C_WriteByte(uint8_t addr,uint8_t data)
{
	// iic进入发送状态
	i2c_master_start();

	// iic写入从机“写操作”访问地址，如果没有收到应答信息则停止iic通信，并返回错误标志
	i2c_master_writeByte((uint8)(OLED_ADDRESS << 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}
	i2c_master_writeByte(addr);
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}
	i2c_master_writeByte(data);
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}
	i2c_master_stop();
}

void ICACHE_FLASH_ATTR
WriteCmd(unsigned char I2C_Command)//D′?üá?
{
	I2C_WriteByte(0x00, I2C_Command);
}
void ICACHE_FLASH_ATTR
WriteDat(unsigned char I2C_Data)//D′êy?Y
{
	I2C_WriteByte(0x40, I2C_Data);
}
void ICACHE_FLASH_ATTR
OLED_Init(void)
{
	Delay_ms(100); //?aà?μ??óê±oü??òa

	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //áá?èμ÷?ú 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}

void ICACHE_FLASH_ATTR
OLED_SetPos(unsigned char x, unsigned char y) //éè???eê?μ?×?±ê
{
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void ICACHE_FLASH_ATTR
OLED_Fill(unsigned char fill_Data)//è??áì?3?
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
        {
            WriteDat(fill_Data);
        }
	}
}

void ICACHE_FLASH_ATTR
OLED_CLS(void)//???á
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          :
// Parameters     : none
// Description    : ??OLED′óDY???D??D?
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_ON(void)
{
	WriteCmd(0X8D);  //éè??μ?oé±?
	WriteCmd(0X14);  //?a??μ?oé±?
	WriteCmd(0XAF);  //OLED??D?
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : è?OLEDDY?? -- DY???￡ê???,OLED1|o?2?μ?10uA
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_OFF(void)
{
	WriteCmd(0X8D);  //éè??μ?oé±?
	WriteCmd(0X10);  //1?±?μ?oé±?
	WriteCmd(0XAE);  //OLEDDY??
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          :
// Parameters     : x,y -- ?eê?μ?×?±ê(x:0~127, y:0~7); ch[] -- òa??ê?μ?×?·?′?; TextSize -- ×?·?′óD?(1:6*8 ; 2:8*16)
// Description    : ??ê?codetab.h?Dμ?ASCII×?·?,óD6*8oí8*16?é????
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          :
// Parameters     : x,y -- ?eê?μ?×?±ê(x:0~127, y:0~7); N:oo×??úcodetab.h?Dμ??÷òy
// Description    : ??ê?codetab.h?Dμ?oo×?,16*16μ??ó
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=24*N;
	OLED_SetPos(x , y);
//	for(wm = 0;wm < 12;wm++)
//	{
//		WriteDat(qrs12x12[adder]);
//		adder += 1;
//	}
//	OLED_SetPos(x,y + 1);
//	for(wm = 0;wm < 12;wm++)
//	{
//		WriteDat(qrs12x12[adder]);
//		adder += 1;
//	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          :
// Parameters     : x0,y0 -- ?eê?μ?×?±ê(x0:0~127, y0:0~7); x1,y1 -- ?eμ???????(?áê?μ?)μ?×?±ê(x1:1~128,y1:1~8)
// Description    : ??ê?BMP??í?
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}



/*
 * alarm.c
 *
 *  Created on: 2017Äê5ÔÂ7ÈÕ
 *      Author: zhangtian
 */

#include "alarm.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "c_types.h"
#include "string.h"
#include "at24c.h"

ALARM Alarm[ALARM_SIZE] = {0};
u8 Alarm_size=0;

int ICACHE_FLASH_ATTR
str2int(char* str)
{
	u8 len = strlen(str), i;
	u32 a = 0;
	for(i=0; i<len; i++)
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			a *= 10;
			a += (str[i] - '0');
		}
		else
		{
			a = -1;
			break;
		}
	}
	return a;
}
void ICACHE_FLASH_ATTR
update_at24c()
{
	u8 i;
	u8 temp[3];
	uint16 addr = 0;
	os_printf("start update\n");
	os_printf("oldAlarm_size:%d\n", Alarm_size);
	at24c_writeAcrossPages(0,&Alarm_size, 1, 1);
	at24c_readBytes(0,&Alarm_size,1);
	os_printf("Alarm_size:%d\n", Alarm_size);
	for(i=0;i<Alarm_size;i++)
	{
		addr = (i+1)*AT24C_PAGESIZE;
		temp[0] = Alarm[i].hour;
		temp[1] = Alarm[i].min;
		temp[2] = Alarm[i].options.info;
		os_printf("%d %d %d\n", temp[0], temp[1], temp[2]);
		at24c_writeAcrossPages(addr, temp, 3, 1);
	}
	os_printf("over update\n");
}

void ICACHE_FLASH_ATTR
reload_at24c()
{
	u8 i=0;
	u8 temp[3];
	uint16 addr = 0;
	os_printf("start reload\n");
	if(at24c_readByte(0,&Alarm_size))
	{
		for(i=0;i<Alarm_size;i++)
		{
			addr = (i+1)*AT24C_PAGESIZE;

			if(at24c_readByte(addr,temp)&&at24c_readNextByte(temp+1)&&at24c_readNextByte(temp+2))
			{
				Alarm[i].hour = temp[0];
				Alarm[i].min = temp[1];
				Alarm[i].options.info = temp[2];
				os_printf("reload: %d %d %d\n", temp[0], temp[1], temp[2]);
			}

		}
	}
	os_printf("ovre reload\n");
}

u8 ICACHE_FLASH_ATTR
Do_Order(char* cmd)
{
	u8 o_flag = 0, i,j;
	char *temp;
	int t_h, t_m;
	if (*cmd == 'A')
		o_flag = 1;
	else if(*cmd == 'D')
		o_flag = 2;
	if(o_flag == 1)
	{
		os_printf("ORder: ADD\n");
		if(Alarm_size > ALARM_SIZE)
		{
			os_printf("Size over\n");
			return 0;
		}
		temp = strtok(cmd+2, " ");
		os_printf("temp:%s\n", temp);
		if(temp)
		{
			t_h = str2int(temp);
			os_printf("t_h:%d\n", t_h);
			if (t_h<0 || t_h>23)
			{
				return 0;
			}
			temp = strtok(NULL, " ");
			t_m = str2int(temp);
			os_printf("t_m:%d\n", t_m);
			if(t_m<0 || t_m>60)
			{
				return 0;
			}
			temp = strtok(NULL, " ");
			os_printf("op:%s\n", temp);
			if(strlen(temp) != 8)
			{
				return 0;
			}
			Alarm[Alarm_size].hour = t_h;
			Alarm[Alarm_size].min = t_m;
			Alarm[Alarm_size].options.bit.x1 = temp[0] - '0';
			Alarm[Alarm_size].options.bit.x2 = temp[1] - '0';
			Alarm[Alarm_size].options.bit.x3 = temp[2] - '0';
			Alarm[Alarm_size].options.bit.x4 = temp[3] - '0';
			Alarm[Alarm_size].options.bit.x5 = temp[4] - '0';
			Alarm[Alarm_size].options.bit.x6 = temp[5] - '0';
			Alarm[Alarm_size].options.bit.x7 = temp[6] - '0';
			Alarm[Alarm_size].options.bit.doAc = temp[7] - '0';
			os_printf("%d,%d,%d\n", Alarm[Alarm_size].hour,Alarm[Alarm_size].min,Alarm[Alarm_size].options.info);
			Alarm_size ++;
			update_at24c();
			return 1;
		}
	}
	else if(o_flag == 2)
	{
		temp = strtok(cmd+2, " ");
		t_h = str2int(temp);
		if (t_h<0 || t_h>23)
		{
			return 0;
		}
		temp = strtok(NULL, " ");
		t_m = str2int(temp);
		if(t_m<0 || t_m>60)
		{
			return 0;
		}
		temp = strtok(NULL, " ");
		if(strlen(temp) != 8)
		{
			return 0;
		}

		for(i=0;i<Alarm_size;i++)
		{
			if (t_h == Alarm[i].hour && t_m == Alarm[i].min)
			{
				for(j=i; j<Alarm_size - 1;j++)
				{
					Alarm[j] = Alarm[j+1];
				}
				Alarm_size --;
				update_at24c();
				return 1;
			}
		}
	}
	return 0;
}

bool  ICACHE_FLASH_ATTR
alarm_if_time_arri(uint8 hour, uint8 min, uint8 wday)
{
	uint8 i;
	for (i = 0; i < ALARM_SIZE; i++)
	{
		if (hour == Alarm[i].hour && min == Alarm[i].min)
		{
			if ((Alarm[i].options.info >> wday) ^ 0x01)
			{
				return true;
			}
		}
	}
	return false;
}


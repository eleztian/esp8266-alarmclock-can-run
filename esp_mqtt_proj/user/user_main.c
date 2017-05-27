/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "c_types.h"
#include "math.h"

#include "red_avoidance.h"
#include "at24c.h"
#include "my_mqtt.h"
#include "ds3231.h"
#include "time.h"
#include "motor.h"
#include "oled.h"
//#define ESSID		"ALARM-CLOCK"
//#define PASSWORD	"password"
#define ESSID		"essid"
#define PASSWORD	"password"

os_timer_t rssi_timer;
os_timer_t SetdsTimer,ShowdsTime;
os_timer_t motor_call_timer;

//时间各要素存储变量，从0起，分别为秒、分、时、星期几、每月的几日、月、年，以及温度整数部分和小数部分的一位存储变量
uint8 inttemp[1],floattemp[1];

//定义字符串变量存贮从sntp真实时间中截取到的秒、分、时、星期几、每月的几日、月、年
char* chsec,chmin,chhour,chwday,chmday,chmon,chyear;

struct tm timedate;

char distance[16] = {0};
double doub_distance = 0;
uint8 red_flag = 0;

uint8 motor_flag = 0;

void ICACHE_FLASH_ATTR
alarm_run_go()
{
	static uint8 static_f=0;
	if(red_flag && static_f)
	{
		motor_turn_left();
		static_f=0;
	}
	else
	{
		if (!static_f)
		{
			motor_go_ahead();
			static_f=1;
		}
	}
}

void ICACHE_FLASH_ATTR
alarm_run_black()
{
	static uint8 static_f=0;
	if(red_flag && static_f)
	{
		motor_turn_right();
		static_f=0;
	}
	else
	{
		if (!static_f)
		{
			motor_go_back();
			static_f=1;
		}
	}
}

void motor_call_cb()
{
	if (motor_flag == 0)
	{
		motor_stop();
		motor_flag = 3;
	}
	else if (motor_flag == 1)
	{
		alarm_run_go();

	}
	else if (motor_flag == 2)
	{
		if ( doub_distance - 0.2 > 0.01)
		{
			alarm_run_black();
		}
		else
		{
			motor_flag = 0;
		}
	}

}



void get_rssi_timer_cb();
//当前时间及温度显示回调函数
static  void ICACHE_FLASH_ATTR TimeShow()
{
	char sen_str[125] = {0};
	static uint8 alarm_flag = 0, alarm_flag2 = 0, alarm_flag3 = 0;
	//获取当前时间及温度整数和小数部分s
	//system_uart_de_swap();
	if(ds3231_getTime(&timedate))
	{
		ds3231_getTempInteger(inttemp);
		ds3231_getTempFloat(floattemp);
		os_sprintf(sen_str, "20%02d,%02d,%02d  %d", timedate.tm_year,timedate.tm_mon,timedate.tm_mday, timedate.tm_wday);
		OLED_ShowStr(2, 0, sen_str, 1);
		get_rssi_timer_cb();
		//如果温度整数部分小于128，则为正温度，否则为负温度 ，因为其分辨率为0.25，所以小数部分要乘以个0.25
		if(inttemp[0]<128)
			os_sprintf(sen_str, "%02d.%02dC", inttemp[0],floattemp[0]*25);
			//os_sprintf(sen_str, "(%02d,%02d,%02d,%02d,%02d,%02d,%d,%02d.%02d,%4s)",timedate.tm_year,timedate.tm_mon,timedate.tm_mday,timedate.tm_hour,timedate.tm_min,timedate.tm_sec,timedate.tm_wday,inttemp[0],floattemp[0]*25, distance);
		else
			os_sprintf(sen_str, "%02d.%02dC", inttemp[0]-128,floattemp[0]*25);
			//os_sprintf(sen_str, "(%02d,%02d,%02d,%02d,%02d,%2d,%d,%02d.%02d,%4s)",timedate.tm_year,timedate.tm_mon,timedate.tm_mday,timedate.tm_hour,timedate.tm_min,timedate.tm_sec,timedate.tm_wday,inttemp[0]-128,floattemp[0]*25, distance);
		OLED_ShowStr(90, 0, sen_str, 1);
		//uart0_sendStr(sen_str);
		os_sprintf(sen_str, "%02d:%02d:%02d",timedate.tm_hour,timedate.tm_min,timedate.tm_sec);
		OLED_ShowStr(30, 3, sen_str, 2);
		os_sprintf(sen_str, "%4sm",distance);
		OLED_ShowStr(30, 5, sen_str, 2);
	//	// 产生闹钟信号 持续1min
		if (alarm_if_time_arri(timedate.tm_hour, timedate.tm_min, timedate.tm_wday))
		{
			if (! alarm_flag2)
			{
				motor_go_ahead();
//				os_timer_disarm(&motor_call_timer);
//				os_timer_setfn(&motor_call_timer, motor_call_cb, NULL);
//				os_timer_arm(&motor_call_timer, 100, 1);
				alarm_flag2 = 1;
				motor_flag = 1;
			}
		}
		else
		{
			motor_flag = 2;
			alarm_flag = 0;
			if (motor_flag == 3)
			{
				alarm_flag2 = 0;
				os_timer_disarm(&motor_call_timer);
			}
		}
//		闹铃时间到 且当前没有其他到达的闹铃时间
//		if (alarm_flag && !alarm_flag2)
//		{
//			alarm_run_go();
//			alarm_flag2 = 1;
//			alarm_flag3 = 1;
//		}
//		if (!alarm_flag && alarm_flag2)
//			motor_stop();
//		if (alarm_flag3 && !alarm_flag)
//		{
//			if ((doub_distance - 0.2) > 0.01)
//				alarm_run_black();
//			else
//			{
//				motor_stop();
//				alarm_flag3 = 0;
//			}
//		}

	}
	else
		os_printf("get time error\n");
	//system_uart_swap();
}

//获取当前网络时钟，并设置到DS3231中
static  void ICACHE_FLASH_ATTR SetDsTime()
{
	uint32_t time = sntp_get_current_timestamp();

	if(time>0)
	{
		os_timer_disarm(&SetdsTimer);
	    os_printf("time:%d\r\n",time);
	    os_printf("date:%s\r\n",sntp_get_real_time(time));//传入time参数给sntp_get_real_time

		char* tttt="Wed Dec 07 16:34:45 2016";

	    os_strcpy(tttt,sntp_get_real_time(time));

		//因为后期使用的os_strncpy函数不能自动将字符串中的\0字符串结束符号一并拷贝进来，所以这里根据年月日的长度构造函数，并初始化后各位有\0符号，不至于函数崩溃
		char chsec[3]={""};
		char chmin[3]={""};
		char chhour[3]={""};
		char chwday[4]={""};
		char chmday[3]={""};
		char chmon[4]={""};
		char chyear[5]={""};

	    //待截取字符串指针名即为字符串的起始地址，移动多少个后即为第I位的起始地址，也就是截取的起点
		os_strncpy(chsec,tttt+17,2);
	    os_strncpy(chmin,tttt+14,2);
	    os_strncpy(chhour,tttt+11,2);
	    os_strncpy(chwday,tttt,3);
	    os_strncpy(chmday,tttt+8,2);
	    os_strncpy(chmon,tttt+4,3);
	    os_strncpy(chyear,tttt+20,4);

	    os_printf("\r\nSubdate:%s，%s，%s，%s，%s，%s-----%s\r\n",chyear,chmon,chmday,chhour,chmin,chsec,chwday);//传入time参数给sntp_get_real_time

	    //赋值给时间设置变量
	    timedate.tm_year=(chyear[2]-48)*10+(chyear[3]-48);

	    if(strcmp(chmon,"Jan")==0)
	    	timedate.tm_mon=1;
	    else if(strcmp(chmon,"Feb")==0)
	    	timedate.tm_mon=2;
	    else if(strcmp(chmon,"Mar")==0)
	    	timedate.tm_mon=3;
	    else if(strcmp(chmon,"Apr")==0)
	    	timedate.tm_mon=4;
	    else if(strcmp(chmon,"May")==0)
	    	timedate.tm_mon=5;
	    else if(strcmp(chmon,"Jun")==0)
	    	timedate.tm_mon=6;
	    else if(strcmp(chmon,"Jul")==0)
	    	timedate.tm_mon=7;
	    else if(strcmp(chmon,"Aug")==0)
	    	timedate.tm_mon=8;
	    else if(strcmp(chmon,"Sep")==0)
	    	timedate.tm_mon=9;
	    else if(strcmp(chmon,"Oct")==0)
	    	timedate.tm_mon=10;
	    else if(strcmp(chmon,"Nov")==0)
	    	timedate.tm_mon=11;
	    else if(strcmp(chmon,"Dec")==0)
	    	timedate.tm_mon=12;

	    timedate.tm_mday=(chmday[0]-48)*10+(chmday[1]-48);

	    timedate.tm_hour=(chhour[0]-48)*10+(chhour[1]-48);

	    timedate.tm_min=(chmin[0]-48)*10+(chmin[1]-48);

	    timedate.tm_sec=(chsec[0]-48)*10+(chsec[1]-48);

	    if(strcmp(chwday,"Mon")==0)
	    	timedate.tm_wday=1;
	    else if(strcmp(chwday,"Tue")==0)
	    	timedate.tm_wday=2;
	    else if(strcmp(chwday,"Wed")==0)
	    	timedate.tm_wday=3;
	    else if(strcmp(chwday,"Thu")==0)
	    	timedate.tm_wday=4;
	    else if(strcmp(chwday,"Fri")==0)
	    	timedate.tm_wday=5;
	    else if(strcmp(chwday,"Sat")==0)
	    	timedate.tm_wday=6;
	    else if(strcmp(chwday,"Sun")==0)
	    	timedate.tm_wday=7;

	    os_printf("\r\nSet Time:%d,%d,%d,%d,%d,%d-----%d\r\n",timedate.tm_year,timedate.tm_mon,timedate.tm_mday,timedate.tm_hour,timedate.tm_min,timedate.tm_sec,timedate.tm_wday);

	    ds3231_setTime(timedate);
	}

}


void ICACHE_FLASH_ATTR
printFloat(float val, char *buff) {
   char smallBuff[16];
   int val1 = (int) val;
   unsigned int val2;
   if (val < 0) {
      val2 = (int) (-1000.0 * val) % 1000;
   } else {
      val2 = (int) (1000.0 * val) % 1000;
   }
   if (val2 < 10) {
      os_sprintf(smallBuff, "%i.0%u", val1, val2);
   } else {
      os_sprintf(smallBuff, "%i.%u", val1, val2);
   }
   os_strncpy(buff, smallBuff, 16);
}

void ICACHE_FLASH_ATTR
get_rssi_timer_cb()
{
	static double last_d;
	sint8 rssi =  wifi_station_get_rssi();
	double d = pow(10, (-rssi -74.0)/ (30.0));
	doub_distance = last_d * 0.3 + d;
	printFloat(doub_distance, distance);
	last_d = doub_distance;
}

void wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
	    //联网后初始化SNTP
		sntp_init();
		//启动mqtt连接
		mqtt_Start();
		// 20s 获取一次网络时间并更新DS3231的时间。
		os_timer_disarm(&SetdsTimer);
		os_timer_setfn(&SetdsTimer, SetDsTime, NULL);
		os_timer_arm(&SetdsTimer, 20000, 1);
		// 1s 采集一次距离信息
		os_timer_setfn(&rssi_timer, get_rssi_timer_cb, NULL);
		os_timer_arm(&rssi_timer, 1000, 1);
	} else {
		INFO("Wifi: DisConnected\r\n");
	}
}


void user_init(void)
{
	uart_init(115200, 115200);
//	UART_SetPrintPort(UART1);

	// 红外初始化
//	red_avoidance_init();

	motor_init();
	motor_stop();
	os_delay_us(600);
	//Esp8266的I2c引脚接口初始化函数
	i2c_master_gpio_init();

	os_timer_disarm(&ShowdsTime);
	os_timer_setfn(&ShowdsTime, TimeShow, NULL);
	os_timer_arm(&ShowdsTime, 1000, 1);
	//reload_at24c();	//加载闹钟数据
	//设置SNTP服务器
    sntp_setservername(0,"us.pool.ntp.org");
    sntp_setservername(1,"ntp.sjtu.edu.cn");
    sntp_setservername(2,"0.cn.pool.ntp.org");
    //延时3秒，待初始化完成
	os_delay_us(60000);

	//设置DS3231控制寄存器的CONV位为1，将温度强制转为数字码
	ds3231_setConv(0x20);
	OLED_Init();
	OLED_Fill(0x00);

	mqtt_Init();
	WIFI_Connect(ESSID, PASSWORD, wifiConnectCb);
}

uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


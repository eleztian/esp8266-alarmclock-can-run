/*
 * alarm.h
 *
 *  Created on: 2017年5月7日
 *      Author: zhangtian
 */

#ifndef ESP_MQTT_PROJ_INCLUDE_ALARM_H_
#define ESP_MQTT_PROJ_INCLUDE_ALARM_H_
#include "c_types.h"

#define ALARM_SIZE		5

typedef struct alarm{
	u8 hour;
	u8 min;
	/*  */
	union replay{
		u8 info;
		struct {
			u8 x1	: 1;	/* 星期1 */
			u8 x2	: 1;
			u8 x3	: 1;
			u8 x4	: 1;
			u8 x5	: 1;
			u8 x6	: 1;
			u8 x7	: 1;
			u8 doAc	: 1;	/* 是否跑动 */
		}bit;
	}options;
}ALARM;

enum{
	ADD = 1,
	DELETE,
}ALARM_ORDER;


void  alarm_GPIO_Init();

bool  alarm_if_time_arri(uint8 hour, uint8 min, uint8 wday);

u8 Do_Order(char* cmd);

#endif /* ESP_MQTT_PROJ_INCLUDE_ALARM_H_ */

/*
 * motor.h
 *
 *  Created on: 2017Äê5ÔÂ10ÈÕ
 *      Author: zhangtian
 */

#ifndef ESP_MQTT_PROJ_USER_MOTOR_H_
#define ESP_MQTT_PROJ_USER_MOTOR_H_

#include "pwm.h"

#define PRIV_PARAM_START_SEC		0x3C
#define PRIV_PARAM_SAVE     		0

#define PWM_CHANNEL 4


/*Definition of GPIO PIN params, for GPIO initialization*/
#define PWM_1_OUT_IO_MUX 	PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM 	15
#define PWM_1_OUT_IO_FUNC  	FUNC_GPIO15

#define PWM_2_OUT_IO_MUX 	PERIPHS_IO_MUX_MTCK_U
#define PWM_2_OUT_IO_NUM 	13
#define PWM_2_OUT_IO_FUNC  	FUNC_GPIO13

#define PWM_3_OUT_IO_MUX 	PERIPHS_IO_MUX_GPIO4_U
#define PWM_3_OUT_IO_NUM 	4
#define PWM_3_OUT_IO_FUNC  	FUNC_GPIO4

#define PWM_4_OUT_IO_MUX 	PERIPHS_IO_MUX_GPIO5_U
#define PWM_4_OUT_IO_NUM 	5
#define PWM_4_OUT_IO_FUNC  	FUNC_GPIO5


#define MOTOR_1_H		0
#define MOTOR_1_B		1
#define MOTOR_2_H		2
#define MOTOR_2_B		3


struct motor_saved_param {
    uint32  pwm_period;
    uint32  pwm_duty[PWM_CHANNEL];
};

void motor_init(void);
uint32 motor_get_duty(uint8 channel);
void motor_set_duty(uint32 duty, uint8 channel);
uint32 motor_get_period(void);
void motor_set_period(uint32 period);
void motor_go_ahead();
void motor_go_back();
void motor_turn_right();
void motor_turn_left();
void motor_stop();

#endif /* ESP_MQTT_PROJ_USER_MOTOR_H_ */

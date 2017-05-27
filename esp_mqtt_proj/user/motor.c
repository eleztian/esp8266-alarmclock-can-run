/*
 * motor.c
 *
 *  Created on: 2017Äê5ÔÂ10ÈÕ
 *      Author: zhangtian
 */
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "motor.h"
#include "pwm.h"

struct motor_saved_param motor_param;

/******************************************************************************
 * FunctionName : user_light_get_duty
 * Description  : get duty of each channel
 * Parameters   : uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
motor_get_duty(uint8 channel)
{
    return motor_param.pwm_duty[channel];
}

/******************************************************************************
 * FunctionName : user_light_set_duty
 * Description  : set each channel's duty params
 * Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
 *                uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
motor_set_duty(uint32 duty, uint8 channel)
{
    if (duty != motor_param.pwm_duty[channel]) {
        pwm_set_duty(duty, channel);

        motor_param.pwm_duty[channel] = pwm_get_duty(channel);
    }
}

/******************************************************************************
 * FunctionName : user_light_get_period
 * Description  : get pwm period
 * Parameters   : NONE
 * Returns      : uint32 : pwm period
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
motor_get_period(void)
{
    return motor_param.pwm_period;
}

/******************************************************************************
 * FunctionName : user_light_set_duty
 * Description  : set pwm frequency
 * Parameters   : uint16 freq : 100hz typically
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
motor_set_period(uint32 period)
{
    if (period != motor_param.pwm_period) {
        pwm_set_period(period);

        motor_param.pwm_period = pwm_get_period();
    }
}

void ICACHE_FLASH_ATTR
motor_restart(void)
{
	spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
	spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
	    		(uint32 *)&motor_param, sizeof(struct motor_saved_param));

	pwm_start();
}


void ICACHE_FLASH_ATTR
motor_init()
{

	uint32 io_info[][3] = {
						  {PWM_1_OUT_IO_MUX,PWM_1_OUT_IO_FUNC,PWM_1_OUT_IO_NUM},
						  {PWM_2_OUT_IO_MUX,PWM_2_OUT_IO_FUNC,PWM_2_OUT_IO_NUM},
						  {PWM_3_OUT_IO_MUX,PWM_3_OUT_IO_FUNC,PWM_3_OUT_IO_NUM},
						  {PWM_4_OUT_IO_MUX,PWM_4_OUT_IO_FUNC,PWM_4_OUT_IO_NUM},
					  };

	uint32 pwm_duty_init[PWM_CHANNEL] = {0};
	motor_param.pwm_period = 1000;
	/*PIN FUNCTION INIT FOR PWM OUTPUT*/
	pwm_init(motor_param.pwm_period,  pwm_duty_init ,PWM_CHANNEL,io_info);
	os_printf("MOTOR_1_H: %d \r\n",motor_param.pwm_duty[MOTOR_1_H]);
	os_printf("MOTOR_1_B: %d \r\n",motor_param.pwm_duty[MOTOR_1_B]);
	os_printf("MOTOR_2_H: %d \r\n",motor_param.pwm_duty[MOTOR_2_H]);
	os_printf("MOTOR_2_B: %d \r\n",motor_param.pwm_duty[MOTOR_2_B]);
	os_printf("MOTOR PARAM: P: %d \r\n",motor_param.pwm_period);
//		uint32 motor_init_target[8]={0};
//		os_memcpy(motor_init_target,motor_param.pwm_duty,sizeof(motor_param.pwm_duty));

//		motor_set_aim(
//					motor_init_target[MOTOR_1_H],
//					motor_init_target[MOTOR_1_B],
//					motor_init_target[MOTOR_2_H],
//					motor_init_target[MOTOR_2_B],
//					motor_param.pwm_period);
	set_pwm_debug_en(0);//disable debug print in pwm driver
	os_printf("PWM version : %08x \r\n",get_pwm_version());
	pwm_set_duty(0, 0);
	pwm_set_duty(0, 1);
	pwm_set_duty(0, 2);
	pwm_set_duty(0, 3);
	pwm_start();
}


void ICACHE_FLASH_ATTR
motor_go_ahead()
{
	motor_set_duty(100,0);
	motor_set_duty(20000,1);

	motor_set_duty(100,2);
	motor_set_duty(20000,3);
	pwm_start();
}

void ICACHE_FLASH_ATTR
motor_go_back()
{
	motor_set_duty(20000,0);
	motor_set_duty(100,1);

	motor_set_duty(20000,2);
	motor_set_duty(100,3);
	pwm_start();
}

void ICACHE_FLASH_ATTR
motor_turn_right()
{
	motor_set_duty(100,0);
	motor_set_duty(20000,1);

	motor_set_duty(20000,2);
	motor_set_duty(100,3);
	pwm_start();
}

void ICACHE_FLASH_ATTR
motor_turn_left()
{
	motor_set_duty(20000,0);
	motor_set_duty(100,1);

	motor_set_duty(100,2);
	motor_set_duty(20000,3);
	pwm_start();
}


void ICACHE_FLASH_ATTR
motor_stop()
{
	pwm_set_duty(0,0);
	pwm_set_duty(0,1);

	pwm_set_duty(0,2);
	pwm_set_duty(0,3);
	pwm_start();
}



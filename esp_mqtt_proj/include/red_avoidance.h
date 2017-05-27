/*
 * red_voiddance.h
 *
 *  Created on: 2017Äê5ÔÂ24ÈÕ
 *      Author: zhang
 */

#ifndef ESP_MQTT_PROJ_INCLUDE_RED_AVOIDANCE_H_
#define ESP_MQTT_PROJ_INCLUDE_RED_AVOIDANCE_H_

#include "driver/gpio16.h"

#define red_avoidance_init	gpio16_input_conf
#define red_read			gpio16_input_get

#endif /* ESP_MQTT_PROJ_INCLUDE_RED_AVOIDANCE_H_ */

/*
 * my_mqtt.c
 *
 *  Created on: 2017年5月7日
 *      Author: zhangtian
 */


#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "my_mqtt.h"
#include "alarm.h"
extern ALARM Alarm[ALARM_SIZE];
extern u8 Alarm_size;
MQTT_Client mqttClient;

void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");
	MQTT_Subscribe(client, "alarm", 0);
	//MQTT_Publish(client, "/mqtt/topic/2", "hello2", 6, 2, 0);

}

void mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);
	int i;
	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;
	//TODO: 数据处理
	// 响应命令 A H M XINQI
	//		   D H M
	//system_uart_de_swap();
	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
	if(topicBuf[0] == '$')	//收到命令
	{
		topicBuf[3] = 's';
		topicBuf[4] = 'p';
		if (Do_Order(dataBuf))
		{
			MQTT_Publish(client, topicBuf, "OK",strlen(dataBuf), 0, 5);
		}
		else
			MQTT_Publish(client, topicBuf, "ERROR",strlen(dataBuf), 0, 5);
		reload_at24c();
		for(i=0; i<Alarm_size; i++)
		{
			os_printf("\n%d,%d,%d\n", Alarm[i].hour,Alarm[i].min,Alarm[i].options.info);
		}
	}

	os_delay_us(10000);
	//system_uart_de_swap();
	os_free(topicBuf);
	os_free(dataBuf);
}

void ICACHE_FLASH_ATTR
mqtt_Init()
{
	CFG_Load();

	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);

	//MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
}

void ICACHE_FLASH_ATTR
mqtt_Start(void)
{
	MQTT_Connect(&mqttClient);
}


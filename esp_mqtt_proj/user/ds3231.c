//////////////////////////////////////////////////
//  I2C接口DS3231模块的驱动程序 .
//////////////////////////////////////////////////

#include <c_types.h>
#include <osapi.h>
#include <driver/i2c_master.h>

#include "ds3231.h"
#include "time.h"

// 普通数字格式转化为符合DS3231寄存器数据位要求的二进制编码
static uint8 ICACHE_FLASH_ATTR decToBcd(uint8 dec) {
  return(((dec / 10) * 16) + (dec % 10));
}


// 符合DS3231寄存器数据位要求的二进制编码转化为普通编码
static uint8 ICACHE_FLASH_ATTR bcdToDec(uint8 bcd) {
  return(((bcd / 16) * 10) + (bcd % 16));
}

// 通过iic接口向DS3231发送数据，逻辑参考文档中的写多个字节时序,先写入设备地址后写入寄存器地址及各个字节数据，成功后返回true
static bool ICACHE_FLASH_ATTR ds3231_send(uint8 *data, uint8 len) {

	int loop;

	// iic进入发送状态
	i2c_master_start();

	// iic写入从机“写操作”访问地址，如果没有收到应答信息则停止iic通信，并返回错误标志
	i2c_master_writeByte((uint8)(DS3231_ADDR << 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}


	// iic向DS3231写入数据，如果没有收到应答信息则停止iic通信，并返回错误标志,完成一个地址写入后地址加1继续写入
	for (loop = 0; loop < len; loop++) {
		i2c_master_writeByte(data[loop]);
		if (!i2c_master_checkAck()) {
			i2c_master_stop();
			return false;
		}
	}
	// 停止iic通信
	i2c_master_stop();

    //返回正确标志
	return true;


}

// 通过iic接口从DS323读取数据，逻辑参考文档中的读多个字节时序，在写入设备地址和寄存器地址的基础上，写入读设备地址及各个字节数据，成功后返回true
static bool ICACHE_FLASH_ATTR ds3231_recv(uint8 *data, uint8 len) {

	int loop;

	// iic进入发送状态
	i2c_master_start();

	// iic写入“读操作”访问地址，如果没有收到应答信息则停止iic通信，并返回错误标志
	i2c_master_writeByte((uint8)((DS3231_ADDR << 1) | 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	// iic读DS3231数据，并返回应答信息
	for (loop = 0; loop < len; loop++) {
		data[loop] = i2c_master_readByte();
		if (loop < (len - 1)) i2c_master_send_ack(); else i2c_master_send_nack();
	}

	// 停止iic通信
	i2c_master_stop();

	//返回正确标志
	return true;

}


//设置控制寄存器CONV位为1时强制将温度转换为数字码，uint值为ox20
bool ICACHE_FLASH_ATTR ds3231_setConv(uint8 data) {

	uint8 convdata[2];

	convdata[0] = DS3231_ADDR_CONTROL;
	convdata[1] = data;
	return ds3231_send(convdata, 2);

}


// DS3231设置初始时间，依次写入计时寄存器中的时间设置地址以及秒、分、时、星期、日期、月和年，设置成功后返回true
bool ICACHE_FLASH_ATTR ds3231_setTime(struct tm timedate) {

	uint8 bcddata[8];
	bcddata[0] = DS3231_ADDR_TIME;
	bcddata[1] = decToBcd(timedate.tm_sec);
	bcddata[2] = decToBcd(timedate.tm_min);
	bcddata[3] = decToBcd(timedate.tm_hour);
	bcddata[4] = decToBcd(timedate.tm_wday);
	bcddata[5] = decToBcd(timedate.tm_mday);
	bcddata[6] = decToBcd(timedate.tm_mon);
	bcddata[7] = decToBcd(timedate.tm_year);

	return ds3231_send(bcddata, 8);

}



////DS3231获取温度值的整数部分，成功返回true
bool ICACHE_FLASH_ATTR ds3231_getTempInteger(uint8 *temp) {
	uint8 data[1];
	data[0] = DS3231_ADDR_TEMP;
	if (ds3231_send(data, 1) && ds3231_recv(data, 1)) {
		temp[0] = data[0];
		return true;
	}
	return false;
}

////DS3231获取温度值的小数部分，成功返回true
bool ICACHE_FLASH_ATTR ds3231_getTempFloat(uint8 *temp) {
	uint8 data[1];
	data[0] = DS3231_ADDR_TEMP+1;
	if (ds3231_send(data, 1) && ds3231_recv(data, 1)) {
		temp[0] = data[0]>>6;
		return true;
	}
	return false;
}

//DS3231读取模块中的当前时间，成功返回true
bool ICACHE_FLASH_ATTR ds3231_getTime(struct tm *timedate) {

	int loop;
	uint8 getdata[7];

	// 写入读取地址
	getdata[0] = DS3231_ADDR_TIME;

	//写入成功则继续，否则返回错误标识
	if (!ds3231_send(getdata, 1)) {
		return false;
	}

	// 读取成功则继续，否则返回错误标识
	if (!ds3231_recv(getdata, 7)) {
		return false;
	}

	// 存入变量
	timedate->tm_sec = bcdToDec(getdata[0]);
	timedate->tm_min  = bcdToDec(getdata[1]);
	if (getdata[2] & DS3231_12HOUR_FLAG) {
		// 12小时制
		timedate->tm_hour = bcdToDec(getdata[2] & DS3231_12HOUR_MASK);
		// 是否下午
		if (getdata[2] & DS3231_PM_FLAG) timedate->tm_hour += 12;
	} else {
		// 24小时制
		timedate->tm_hour = bcdToDec(getdata[2]);
	}
	timedate->tm_wday = bcdToDec(getdata[3]);
	timedate->tm_mday = bcdToDec(getdata[4]);
	timedate->tm_mon  = bcdToDec(getdata[5] & DS3231_MONTH_MASK);
	timedate->tm_year = bcdToDec(getdata[6]);

	return true;

}










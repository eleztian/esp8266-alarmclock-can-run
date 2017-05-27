//////////////////////////////////////////////////
// I2C接口DS3231模块的驱动程序 ，实现对模块的时间设置、读取以及温度读取.
//////////////////////////////////////////////////

#ifndef __DS3231_H__
#define __DS3231_H__
#include "time.h"

//以下（地址）定义见DS3231技术说明文档

//DS3231作为IIC从机的设备地址
#define DS3231_ADDR 0x68
//#define DS3231_ADDR 0xd0

//DS3231计时寄存器中时间设置起始地址
#define DS3231_ADDR_TIME    0x00
//DS3231计时寄存器中闹钟1设置起始地址
#define DS3231_ADDR_ALARM1  0x07
//DS3231计时寄存器中闹钟2设置起始地址
#define DS3231_ADDR_ALARM2  0x0b
//DS3231计时寄存器中控制寄存器地址
#define DS3231_ADDR_CONTROL 0x0e
//DS3231计时寄存器中状态寄存器地址
#define DS3231_ADDR_STATUS  0x0f
//DS3231计时寄存器中老化寄存器地址
#define DS3231_ADDR_AGING   0x10
//DS3231计时寄存器中温度寄存器起始地址
#define DS3231_ADDR_TEMP    0x11

#define DS3231_CTRL_ALARM_INTS    0x04
#define DS3231_CTRL_ALARM2_INT    0x02
#define DS3231_CTRL_ALARM1_INT    0x01

#define DS3231_ALARM_NONE 0
#define DS3231_ALARM_1    1
#define DS3231_ALARM_2    2
#define DS3231_ALARM_BOTH 3

#define DS3231_ALARM1_EVERY_SECOND         0
#define DS3231_ALARM1_MATCH_SEC            1
#define DS3231_ALARM1_MATCH_SECMIN         2
#define DS3231_ALARM1_MATCH_SECMINHOUR     3
#define DS3231_ALARM1_MATCH_SECMINHOURDAY  4
#define DS3231_ALARM1_MATCH_SECMINHOURDATE 5

#define DS3231_ALARM2_EVERY_MIN         0
#define DS3231_ALARM2_MATCH_MIN         1
#define DS3231_ALARM2_MATCH_MINHOUR     2
#define DS3231_ALARM2_MATCH_MINHOURDAY  3
#define DS3231_ALARM2_MATCH_MINHOURDATE 4

#define DS3231_ALARM_WDAY   0x40
#define DS3231_ALARM_NOTSET 0x80

//DS3231中12小时制标识和标记、PM下午标识和 月标记
#define DS3231_12HOUR_FLAG 0x40
#define DS3231_12HOUR_MASK 0x1f
#define DS3231_PM_FLAG     0x20
#define DS3231_MONTH_MASK  0x1f


//设置控制寄存器CONV位为1时强制将温度转换为数字码
bool ICACHE_FLASH_ATTR ds3231_setConv(uint8 data);

//DS3231设置初始时间，也就是初始化DS3231中的时间，相当于我们在对时
bool ICACHE_FLASH_ATTR ds3231_setTime(struct tm timedate);

//DS3231获取温度值的整数部分
bool ICACHE_FLASH_ATTR ds3231_getTempInteger(uint8 *temp);
//DS3231获取温度值的小数部分
bool ICACHE_FLASH_ATTR ds3231_getTempFloat(uint8 *temp);

//DS3231读取模块中的当前时间
bool ICACHE_FLASH_ATTR ds3231_getTime(struct tm *timedate);

#endif

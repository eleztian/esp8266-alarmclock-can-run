//////////////////////////////////////////////////
//  I2C�ӿ�DS3231ģ����������� .
//////////////////////////////////////////////////

#include <c_types.h>
#include <osapi.h>
#include <driver/i2c_master.h>

#include "ds3231.h"
#include "time.h"

// ��ͨ���ָ�ʽת��Ϊ����DS3231�Ĵ�������λҪ��Ķ����Ʊ���
static uint8 ICACHE_FLASH_ATTR decToBcd(uint8 dec) {
  return(((dec / 10) * 16) + (dec % 10));
}


// ����DS3231�Ĵ�������λҪ��Ķ����Ʊ���ת��Ϊ��ͨ����
static uint8 ICACHE_FLASH_ATTR bcdToDec(uint8 bcd) {
  return(((bcd / 16) * 10) + (bcd % 16));
}

// ͨ��iic�ӿ���DS3231�������ݣ��߼��ο��ĵ��е�д����ֽ�ʱ��,��д���豸��ַ��д��Ĵ�����ַ�������ֽ����ݣ��ɹ��󷵻�true
static bool ICACHE_FLASH_ATTR ds3231_send(uint8 *data, uint8 len) {

	int loop;

	// iic���뷢��״̬
	i2c_master_start();

	// iicд��ӻ���д���������ʵ�ַ�����û���յ�Ӧ����Ϣ��ֹͣiicͨ�ţ������ش����־
	i2c_master_writeByte((uint8)(DS3231_ADDR << 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}


	// iic��DS3231д�����ݣ����û���յ�Ӧ����Ϣ��ֹͣiicͨ�ţ������ش����־,���һ����ַд����ַ��1����д��
	for (loop = 0; loop < len; loop++) {
		i2c_master_writeByte(data[loop]);
		if (!i2c_master_checkAck()) {
			i2c_master_stop();
			return false;
		}
	}
	// ֹͣiicͨ��
	i2c_master_stop();

    //������ȷ��־
	return true;


}

// ͨ��iic�ӿڴ�DS323��ȡ���ݣ��߼��ο��ĵ��еĶ�����ֽ�ʱ����д���豸��ַ�ͼĴ�����ַ�Ļ����ϣ�д����豸��ַ�������ֽ����ݣ��ɹ��󷵻�true
static bool ICACHE_FLASH_ATTR ds3231_recv(uint8 *data, uint8 len) {

	int loop;

	// iic���뷢��״̬
	i2c_master_start();

	// iicд�롰�����������ʵ�ַ�����û���յ�Ӧ����Ϣ��ֹͣiicͨ�ţ������ش����־
	i2c_master_writeByte((uint8)((DS3231_ADDR << 1) | 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	// iic��DS3231���ݣ�������Ӧ����Ϣ
	for (loop = 0; loop < len; loop++) {
		data[loop] = i2c_master_readByte();
		if (loop < (len - 1)) i2c_master_send_ack(); else i2c_master_send_nack();
	}

	// ֹͣiicͨ��
	i2c_master_stop();

	//������ȷ��־
	return true;

}


//���ÿ��ƼĴ���CONVλΪ1ʱǿ�ƽ��¶�ת��Ϊ�����룬uintֵΪox20
bool ICACHE_FLASH_ATTR ds3231_setConv(uint8 data) {

	uint8 convdata[2];

	convdata[0] = DS3231_ADDR_CONTROL;
	convdata[1] = data;
	return ds3231_send(convdata, 2);

}


// DS3231���ó�ʼʱ�䣬����д���ʱ�Ĵ����е�ʱ�����õ�ַ�Լ��롢�֡�ʱ�����ڡ����ڡ��º��꣬���óɹ��󷵻�true
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



////DS3231��ȡ�¶�ֵ���������֣��ɹ�����true
bool ICACHE_FLASH_ATTR ds3231_getTempInteger(uint8 *temp) {
	uint8 data[1];
	data[0] = DS3231_ADDR_TEMP;
	if (ds3231_send(data, 1) && ds3231_recv(data, 1)) {
		temp[0] = data[0];
		return true;
	}
	return false;
}

////DS3231��ȡ�¶�ֵ��С�����֣��ɹ�����true
bool ICACHE_FLASH_ATTR ds3231_getTempFloat(uint8 *temp) {
	uint8 data[1];
	data[0] = DS3231_ADDR_TEMP+1;
	if (ds3231_send(data, 1) && ds3231_recv(data, 1)) {
		temp[0] = data[0]>>6;
		return true;
	}
	return false;
}

//DS3231��ȡģ���еĵ�ǰʱ�䣬�ɹ�����true
bool ICACHE_FLASH_ATTR ds3231_getTime(struct tm *timedate) {

	int loop;
	uint8 getdata[7];

	// д���ȡ��ַ
	getdata[0] = DS3231_ADDR_TIME;

	//д��ɹ�����������򷵻ش����ʶ
	if (!ds3231_send(getdata, 1)) {
		return false;
	}

	// ��ȡ�ɹ�����������򷵻ش����ʶ
	if (!ds3231_recv(getdata, 7)) {
		return false;
	}

	// �������
	timedate->tm_sec = bcdToDec(getdata[0]);
	timedate->tm_min  = bcdToDec(getdata[1]);
	if (getdata[2] & DS3231_12HOUR_FLAG) {
		// 12Сʱ��
		timedate->tm_hour = bcdToDec(getdata[2] & DS3231_12HOUR_MASK);
		// �Ƿ�����
		if (getdata[2] & DS3231_PM_FLAG) timedate->tm_hour += 12;
	} else {
		// 24Сʱ��
		timedate->tm_hour = bcdToDec(getdata[2]);
	}
	timedate->tm_wday = bcdToDec(getdata[3]);
	timedate->tm_mday = bcdToDec(getdata[4]);
	timedate->tm_mon  = bcdToDec(getdata[5] & DS3231_MONTH_MASK);
	timedate->tm_year = bcdToDec(getdata[6]);

	return true;

}










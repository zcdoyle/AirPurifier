/*************************************************
Copyright: SmartLight
Author: albert
Date: 2016-02-01
Description：EEPROM配置结构
**************************************************/

#ifndef EEPROM_STRUCT_H
#define EEPROM_STRUCT_H

#include <stdint.h>

//设备类型
typedef enum DeviceType
{
    LIGHT = 0,
    ENVIRONMENT = 1,
    HUMAN = 2,
    SOUND = 3,
    CAR = 4,
}DeviceType;

#pragma pack(1)

//前一天时间
typedef struct EEPROMTime{
    uint16_t year;
    uint16_t monAndDay;
    uint32_t time;
}EEPROMTime;

//硬件身份信息
typedef struct EEPROMHardware{
    uint32_t id;
    uint32_t reserve;
}EEPROMHardware;

//硬件虚拟IP
typedef struct EEPROMVirtualIP{
    uint8_t ip1;
    uint8_t ip2;
    uint8_t ip3;
    uint8_t ip4;
    uint16_t port1;
    uint16_t port2;
}EEPROMVirtualIP;

//后台公网域名
typedef struct EEPROMDomain{
    u_char domain[64];
}EEPROMDomain;

//路灯状态配置信息
typedef struct EEPROMLigConf{
    uint8_t elecInterval;
    uint8_t total;
    uint16_t voltageThreashold;
    uint16_t currentThreadhold;
    uint8_t pwmInterval;
    uint8_t pwmFrequency;
}EEPROMLigConf;

//环境状态配置信息
typedef struct EEPROMEnvConf{
	uint32_t number;
    uint8_t interval;
    uint8_t temperature;
    uint16_t pm2p5;
}EEPROMEnvConf;

//车辆状态配置信息
typedef struct EEPROMCarConf{
	uint32_t number;
	uint8_t interval;
    uint16_t carThreadhold;
    uint8_t reserve;
}EEPROMCarConf;

//人群状态配置信息
typedef struct EEPROMHumConf{
	uint32_t number;
	uint8_t interval;
	uint16_t noiseThreadhold;
    uint8_t reserve;
}EEPROMHumConf;

//语音状态配置信息
typedef struct EEPROMSound{
	uint32_t number;
	uint8_t interval;
    uint16_t soundThreadhold;
    uint8_t reserve;
}EEPROMSound;

//工控机后台控制切换
typedef struct EEPROMController{
	uint8_t switcher;
    uint8_t reserve1;
    uint8_t reserve2;
    uint8_t reserve3;
    uint8_t reserve4;
    uint8_t reserve5;
    uint8_t reserve6;
    uint8_t reserve7;
}EEPROMController;

//灯控输出端口1至6
typedef struct EEPROMTimingEnable{
    uint8_t enable1;
    uint8_t enable2;
    uint8_t enable3;
    uint8_t enable4;
    uint8_t enable5;
    uint8_t enable6;
    uint8_t reserve1;
    uint8_t showDevice;
}EEPROMTimingEnable;

//灯控输出端口 不区分PWM还是继电器
typedef struct EEPROMPWMTiming{
	uint8_t port;
	uint8_t power;
	uint16_t startTime;
	uint16_t lastMin;
	uint16_t reserve;
}EEPROMPWMTimingEnable;


////本地字处理配置信息
//typedef struct EEPROMLocal{
//	uint8_t localEnable;
//	uint32_t device;
//	uint8_t lastMin;
//	uint16_t reserve;
//}EEPROMLocal;

#pragma pack()

#endif

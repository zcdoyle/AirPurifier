/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：帧结构
**************************************************/

#ifndef FRAME_STRUCT_H
#define FRAME_STRUCT_H

#include <stdint.h>

static const uint16_t HeaderLength = 27;                                    //帧头长度
static const uint32_t Header = 0xEB90146F;                                  //帧头

typedef uint64_t DEVID;
typedef uint16_t FRAMECOUNT;                                                //帧计数

//消息类型
typedef enum MessageType
{
    CONFIRM = 0x00,
    OPEN_CTRL = 0x01,
    MODE_CTRL = 0x02,
    TIME_CTRL = 0x03,
    CHILDLOCK_CTRL = 0x04,
    STATUS_MSG = 0x05,
    SENSOR_MSG = 0x06,
    ERROR_MSG = 0x07,
    UPDATE_CTRL = 0x08,
    DEVID_MSG = 0x09,
    ERRORREMINDER_CTRL = 0x0a,
    HEART_MSG = 0x0b,
}MessageType;

#pragma pack(1)

//确认帧
typedef struct ContentConfirm{
    uint16_t seq;
}ContentConfirm;

//控制开关帧
typedef struct ContentOpen{
    unsigned isopen:1;
    unsigned res:7;
}ContentOpen;

//设置运行模式
typedef struct ContentMode{
    uint8_t mode;
}ContentMode;

//设置设备定时时间
typedef struct ContentTime{
    uint8_t time;
}ContentTime;

//设置儿童锁，运行异常提醒
typedef struct ContentChildlock{
    unsigned click:1;
    unsigned res:7;
}ContentChildlock;

//设置儿童锁，运行异常提醒
typedef struct ContentErrorreminder{
    unsigned ermd:1;
    unsigned res:7;
}ContentErrorreminder;

typedef struct ContentHeart{
    uint8_t res;
}ContentHeart;

//设备运行状态
typedef struct ContentStatus{
    unsigned isopen:1;
    unsigned mode:4;
    unsigned click:1;
    unsigned ermd:1;
    unsigned res:1;
    uint16_t time; //剩余定时时长
    uint16_t ver;
    uint32_t totm;
    uint32_t topur;
}ContentStatus;

//传感器采样值
typedef struct ContentSensor{
    uint16_t hcho;
    uint16_t pm;
    uint16_t temp;
    uint16_t humi;
}ContentSensor;

//设备故障情况
typedef struct ContentError{
    unsigned fsc:1;
    unsigned ibc:1;
    unsigned ibe:1;
    unsigned uve:1;
    unsigned res:4;
}ContentError;

//设置固件更新
typedef struct ContentUpdate{
    uint8_t update;
}ContentUpdate;

//设置固件更新
typedef struct ContentDevID{
    uint8_t res;
}ContentDevID;

//信息字内容
typedef union MessageContent{
    ContentConfirm confirm;
    ContentOpen open;
    ContentMode mode;
    ContentTime time;
    ContentChildlock childlock;
    ContentErrorreminder errorreminder;
    ContentStatus status;
    ContentSensor sensor;
    ContentError error;
    ContentUpdate update;
    ContentDevID devid;
    ContentHeart heart;
}MessageContent;

//帧头格式
typedef struct FrameHeader{
    uint32_t head; //帧头
    unsigned len:11; //帧长
    unsigned ver:5;  //版本
    uint8_t dev;  //设备类型
    uint8_t type; //帧类型
    unsigned olen:11; //加密前长度
    unsigned enc:1;  //是否加密
    unsigned res1:4; //保留字段1
    uint8_t res2; //保留字段2
    uint16_t seq; //帧序号
    uint64_t hard; //发送方的硬件编号
    uint16_t headerCheck; //帧头CRC16校验
    uint32_t messageCheck; //数据段CRC32校验
}FrameHeader;

//信息字
typedef struct FrameMessage{
    MessageContent content;
}FrameMessage;

#pragma pack()

#endif

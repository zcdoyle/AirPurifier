//
// Created by albert on 12/16/15.
//

#ifndef TCPSERVER_TCPCODEC_H
#define TCPSERVER_TCPCODEC_H

//前端设备解码编码器
#include <string.h>
#include <stdio.h>

#include <time.h>
#include <sys/time.h>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Mutex.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "aes.h"
#include "crc.h"

unsigned char key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};	// 密钥

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

//设备运行状态
typedef struct ContentStatus{
    unsigned isopen:1;
    unsigned mode:2;
    unsigned wspd:2;
    unsigned click:1;
    unsigned ermd:1;
    unsigned res:1;
    uint16_t time; //剩余定时时长
    uint16_t ver;
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

uint64_t dev_id = 0xaaaaaaaaaaaa; //source number

using boost::shared_ptr;
using boost::function;
using namespace muduo::net;
using namespace muduo;


class TCPCodec :boost::noncopyable
{
public:
    TCPCodec() : frameCount_(0)
    {

    }
    //处理收到字节流
    void onMessage(const TcpConnectionPtr& conn, Buffer *buf, Timestamp receiveTime)
    {
        while(buf->readableBytes() >= sizeof(int64_t))
        {
            const void* data = buf->peek();
            int32_t header = *(int32_t*)data;
            int16_t length = *((int16_t *)data + 2);
            if(header != Header)
            {
                //非法帧头
                LOG_ERROR << "Invade header" << conn;
                skipWrongFrame(buf);
                break;
            }
            else if(length < 0 || length > 2047)
            {
                //非法帧长度
                LOG_ERROR << "Invade length" << length;
                skipWrongFrame(buf);
                break;
            }
            else if(buf->readableBytes() >= (uint16_t)length)
            {
                //帧头
                shared_ptr<FrameHeader> frameHeader(new FrameHeader);
                memcpy(get_pointer(frameHeader), data, sizeof(FrameHeader));

                //帧信息字
                shared_ptr<u_char > message((u_char *)malloc(length - HeaderLength));
                memcpy(get_pointer(message), (u_char *)data + HeaderLength, length - HeaderLength);

                //解密这一帧
                if(frameHeader->enc == 1)
                {
                    Decrypt(get_pointer(message), key, length-HeaderLength);
                }
                size_t messageLenth = frameHeader->olen;

    //            char headerLine[256];
    //            sprintf(headerLine, "Header: Header:%08x,length:%d,ver:%d,dev:%d,Type:%d,olen:%d,enc:%d,res1:%d,res2:%d,frameCount:%d,hard:%d,headerCheck:%4x,messageCheck:%8x",
    //                    frameHeader->head, frameHeader->len, frameHeader->ver,frameHeader->dev,frameHeader->type, frameHeader->olen,frameHeader->enc,frameHeader->res1,frameHeader->res2,frameHeader->seq,frameHeader->hard, frameHeader->headerCheck,frameHeader->messageCheck);
    //            LOG_DEBUG << headerLine;
                //检查帧头CRC16校验信息
                uint16_t crc16 = CRC16((u_char *)get_pointer(frameHeader), HeaderLength-6);
                //检查数据CRC32校验信息
                uint32_t crc32 = CRC32(get_pointer(message),messageLenth);

                if(crc16 != frameHeader->headerCheck)
                {
                    char crclg[256];
                    sprintf(crclg,"headerCheck:%4x receive:%4x",crc16,frameHeader->headerCheck);
                    LOG_WARN << crclg <<" Header CRC16 error";
                    skipWrongFrame(buf);
                    break;
                }

                else if(crc32 != frameHeader->messageCheck)
                {
                    char crclg[256];
                    sprintf(crclg,"messageCheck:%4x receive:%4x",crc32,frameHeader->messageCheck);
                    LOG_WARN << crc32 <<" Message CRC32 error";
                    skipWrongFrame(buf);
                    break;
                }

                else
                {
                    //打印这一帧，调试用
                    printFrame("Receive", get_pointer(frameHeader), get_pointer(message), messageLenth);
                    buf->retrieve(length);
                    handleReceiveFrame(conn, frameHeader, message, receiveTime);
                }
            }
            else
            {
                break;
            }
        }
    }


    //组帧
    void send(const TcpConnectionPtr& conn, uint16_t totalLength, uint16_t type, u_char * message, uint64_t devid)
    {
        int mLength=totalLength - HeaderLength; //信息字加密前的长度

        FrameHeader sendFrame;
        sendFrame.head = Header;        
        sendFrame.ver = 0;
        sendFrame.dev = 0;
        sendFrame.type = type;

        sendFrame.olen = mLength;
        //sendFrame.enc = 1;
        sendFrame.enc = 0;
        sendFrame.res1 = 0;
        sendFrame.res2 = 0;
        sendFrame.seq = frameCount_++;
        
        sendFrame.hard = devid;

        //对message加密
        if(sendFrame.enc == 1)
        {
            int encryLength = computeEncryptedSize(mLength);   //信息字加密后的长度
            sendFrame.len = encryLength + HeaderLength;
            shared_ptr<u_char > encryMessage((u_char *)malloc(encryLength));
            expandText(message, get_pointer(encryMessage), mLength, encryLength);
            Encrypt(get_pointer(encryMessage), key, encryLength);

            //计算CRC16,CRC32校验
            sendFrame.headerCheck = CRC16((u_char *)&sendFrame, HeaderLength-6);
            sendFrame.messageCheck = CRC32(message,mLength);

            Buffer buf;
            buf.append((void *)&sendFrame, sizeof(FrameHeader));
            buf.append(get_pointer(encryMessage), encryLength);
            conn->send(&buf);
        }

        else
        {
            sendFrame.len = totalLength;

            //计算CRC16,CRC32校验
            sendFrame.headerCheck = CRC16((u_char *)&sendFrame, HeaderLength-6);
            sendFrame.messageCheck = CRC32(message,mLength);

            Buffer buf;
            buf.append((void *)&sendFrame, sizeof(FrameHeader));
            buf.append(message, mLength);
            conn->send(&buf);
        }

        //打印帧内容，调试用
        printFrame("Send",&sendFrame, message, mLength);
    }

private:

    /*************************************************
    Description:    打印帧，调试用
    Calls:          TCPCodec
    Input:          tag: 打印帧类型
                    frame: 帧头指针
                    message: 帧消息字指针
    Output:         无
    Return:         无
    *************************************************/
    void printFrame(std::string tag,FrameHeader *frameHeader, u_char* message, size_t messageLen)
    {
        char headerLine[256];
        sprintf(headerLine, "%s Header: Header:%08x,length:%d,ver:%d,dev:%d,Type:%d,olen:%d,enc:%d,res1:%d,res2:%d,frameCount:%d,hard:%lx,headerCheck:%4x,messageCheck:%8x", tag.c_str(),
                frameHeader->head, frameHeader->len, frameHeader->ver,frameHeader->dev,frameHeader->type, frameHeader->olen,frameHeader->enc,frameHeader->res1,frameHeader->res2,frameHeader->seq, frameHeader->hard,frameHeader->headerCheck,frameHeader->messageCheck);

        std::string mess = tag;
        mess += " Message: ";
        char messageLine[4];
        for(size_t i = 0; i < messageLen; i++)
        {
            sprintf(messageLine, "%02x ", message[i]);
            mess.append(messageLine);
        }

        LOG_DEBUG << headerLine;
        LOG_DEBUG << mess;
    }


    /*************************************************
    Description:    跳过错误数据帧
    Calls:          TCPCodec::onMessage
    Input:          buf: 接收缓冲区指针
    Output:         无
    Return:         无
    *************************************************/
    void skipWrongFrame(Buffer *buf)
    {
        //丢弃第一个字节，并循环查找帧头
        buf->retrieveInt8();
        while(buf->readableBytes() >= sizeof(int32_t))
        {
            if(*(int32_t *)(buf->peek()) == Header) //找到帧头，跳出循环
                break;
            else
                buf->retrieveInt8(); //如果还不是帧头，移动到下一个字节继续寻找
        }
    }


    void handleReceiveFrame(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char> message, Timestamp timestamp)
    {
        switch(frameHeader->type)
        {
            case OPEN_CTRL:
                sendConfirmFrame(conn, frameHeader);
                break;
            case MODE_CTRL:
                sendConfirmFrame(conn, frameHeader);
                break;
            case TIME_CTRL:
                sendConfirmFrame(conn, frameHeader);
                break;
            case CHILDLOCK_CTRL:
                sendConfirmFrame(conn, frameHeader);
                break;
            case ERRORREMINDER_CTRL:
                sendConfirmFrame(conn, frameHeader);
                break;
            default:
                break;
        }
    }

    void sendConfirmFrame(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader)
    {
        u_char message[2]; //confirm frame 16 bit
        FrameMessage frameMessage;
        frameMessage.content.confirm.seq = frameHeader->seq;
        LOG_DEBUG<<frameHeader->seq;
        memcpy(message, &frameMessage, 2);
        uint16_t ConfirmFrameLength = HeaderLength + 2;
        send(conn, ConfirmFrameLength, CONFIRM, message, dev_id++);
    }


    volatile uint32_t frameCount_;
    MutexLock timeMutex_;
};


#endif //TCPSERVER_TCPCODEC_H

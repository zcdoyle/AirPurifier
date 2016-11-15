#include "codec.h"

#include <time.h>
#include <sys/time.h>
#include <muduo/base/Logging.h>
#include <muduo/base/LogFile.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <stdio.h>
#include<stdlib.h>
#define random(x) (rand()%x)

using namespace muduo;
using namespace muduo::net;
using boost::shared_ptr;
using boost::bind;


class TimerClient : boost::noncopyable
{
public:
    TimerClient(EventLoop* loop, const InetAddress& serverAddr, int processNo)
            : client_(loop, serverAddr, "TimerClient"),
              loop_(loop),
              processNo_(processNo)
    {
        loop_->runEvery(2, bind(&TimerClient::sendPeriodical, this));
        client_.setConnectionCallback(bind(&TimerClient::onConnection, this, _1));
        client_.setConnectionCallback(bind(&TimerClient::onConnection, this, _1));
        client_.setMessageCallback(bind(&TCPCodec::onMessage, &codec_, _1, _2, _3));
        client_.enableRetry();
    }

    void connect()
    {
        client_.connect();
    }



private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
                 << conn->peerAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");
        if (conn->connected())
        {
            connection_ = conn;
            sendDevidMessage();
            sendStatusMessage();
            //sendErrorMessage();
            //sendSensorMessage();
        }
        else
        {
            connection_.reset();
        }
    }


    void sendPeriodical()
    {
        if(count_.get() < 60)
        {
              //sendSensorMessage();
        }
        else
        {
            loop_->quit();
        }
    }

    void sendDevidMessage()
    {
        uint16_t MessageLength = 1;
        uint16_t totalLength = HeaderLength + MessageLength;
        unsigned char message[MessageLength];
        memset(message, 0, sizeof(message));

        FrameMessage msg;
        msg.content.devid.res = 0;
        memcpy(message, &msg, MessageLength);
        char headerLine[256];
        sprintf(headerLine, "devid:%lx",dev_id);
        LOG_DEBUG << headerLine;
        codec_.send(connection_, totalLength, DEVID_MSG, message, dev_id);
    }

    void sendStatusMessage()
    {
        uint16_t MessageLength = 5;
        uint16_t totalLength = HeaderLength + MessageLength;
        unsigned char message[MessageLength];
        memset(message, 0, sizeof(message));

        FrameMessage msg;
        msg.content.status.isopen = 0;
        msg.content.status.mode = 0;
        msg.content.status.wspd = 0;
        msg.content.status.click = 0;
        msg.content.status.ermd = 0;
        msg.content.status.res = 0;
        msg.content.status.time = 0;
        msg.content.status.ver = 1;
        memcpy(message, &msg, MessageLength);
        char headerLine[256];
        sprintf(headerLine, "devid:%lx",dev_id);
        LOG_DEBUG << headerLine;
        codec_.send(connection_, totalLength, STATUS_MSG, message, dev_id);
    }

    void sendSensorMessage()
    {
        uint16_t MessageLength = 8;
        uint16_t totalLength = HeaderLength + MessageLength;
        unsigned char message[MessageLength];
        memset(message, 0, sizeof(message));

        FrameMessage msg;
        msg.content.sensor.hcho = random(100);
        msg.content.sensor.pm = random(100);
        msg.content.sensor.temp = random(100);
        msg.content.sensor.humi = random(100);
        memcpy(message, &msg, MessageLength);
        char headerLine[256];
        sprintf(headerLine, "devid:%lx",dev_id);
        LOG_DEBUG << headerLine;
        codec_.send(connection_, totalLength, SENSOR_MSG, message, dev_id);
    }

    void sendErrorMessage()
    {
        uint16_t MessageLength = 1;
        uint16_t totalLength = HeaderLength + MessageLength;
        unsigned char message[MessageLength];
        memset(message, 0, sizeof(message));

        FrameMessage msg;
        msg.content.error.fsc = 0;
        msg.content.error.ibc = 0;
        msg.content.error.ibe = 0;
        msg.content.error.uve = 0;
        msg.content.error.res = 0;
        memcpy(message, &msg, MessageLength);
        char headerLine[256];
        sprintf(headerLine, "devid:%lx",dev_id);
        LOG_DEBUG << headerLine;
        codec_.send(connection_, totalLength,ERROR_MSG, message, dev_id);
    }

//    //发送错误帧，测试容错能力
//    void testErrorFrame()
//    {
//        u_char errorFrame1[11];
//        Buffer errorbuf1;
//        errorbuf1.append((void *)errorFrame1, sizeof(errorFrame1));
//        connection_->send(&errorbuf1);

//        sleep(1);

//        u_char errorFrame2[4];
//        uint32_t header = Header;
//        memcpy(errorFrame2, &header, 4);
//        Buffer errorbuf2;
//        errorbuf2.append((void *)errorFrame2, sizeof(errorFrame2));
//        connection_->send(&errorbuf2);

//        sleep(2);

//        u_char errorFrame3[59];
//        Buffer errorbuf3;
//        errorbuf3.append((void *)errorFrame3, sizeof(errorFrame3));
//        connection_->send(&errorbuf3);
//        sendTiming();
//    }

    AtomicInt32 count_;
    TcpClient client_;
    TCPCodec codec_;
    EventLoop *loop_;
    int processNo_;
    TcpConnectionPtr connection_;

};

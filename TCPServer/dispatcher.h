/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：消息分发器，根据设备发来不同的帧做对应的处理
**************************************************/

#ifndef TCPSERVER_DISPATCHER_H
#define TCPSERVER_DISPATCHER_H

#include "TCPCodec.h"
#include "FrameStruct.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TimerId.h>
#include <muduo/net/EventLoop.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

using muduo::net::EventLoop;
using muduo::net::TimerId;
using muduo::MutexLock;
using muduo::MutexLockGuard;
using muduo::net::TcpConnectionPtr;
using muduo::Timestamp;
using std::map;
using boost::function;
using boost::bind;
using boost::weak_ptr;

const static int RETRY_NUMBER = 3;

class Dispatcher : boost::noncopyable
{
public:
    typedef function<void (const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char>& message)> MessageCallback;
    typedef function<void (const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader)> DevIDCallback;

    /*************************************************
    Description:    Dispatcher构造函数，设置时间循环和编码解码
    Calls:          TCPServer::TCPServer
    Input:          loop: 时间循环指针
                    tc: 编码解码器
                    timeoutSec：超时重发秒数
    Output:         无
    Return:         无
    *************************************************/
    explicit Dispatcher(EventLoop *loop,TCPCodec &tc, int timeoutSec) :
                        loop_(loop),tcpCodec_(tc), timeoutSec_(timeoutSec){}

    /*************************************************
    Description:    设置各类消息的回调函数，用于消息分发
    Calls:          TCPServer::TCPServer
    Input:          各类回调函数
    Output:         无
    Return:         无
    *************************************************/
    inline void setCallbacks(const MessageCallback& statusCb, const MessageCallback& sensorCb, const MessageCallback& errorCb, const DevIDCallback& devidCb, const DevIDCallback& heartCb)
    {
        statusCallback_ = statusCb;
        sensorCallback_ = sensorCb;
        errorCallback_ = errorCb;
        devidCallback_ = devidCb;
        heartCallback_ = heartCb;
    }

    /*************************************************
    Description:    发送消息
    Calls:          TCPServer::
    Input:          conn: TCP连接
                    totalLength: 帧总长度
                    type: 帧类型
                    frameHeader:帧头
                    message: 帧消息字
    Output:         无
    Return:         无
    *************************************************/
    inline void send(const TcpConnectionPtr& conn, uint16_t totalLength, MessageType type, u_char * message, DEVID devid)
    {
        uint16_t seq = getAndSetFrameCount(devid, 1); //帧计数加1
        tcpCodec_.send(conn, totalLength, type, seq, message);
    }

    void sendForTimer(weak_ptr<TcpConnection> weakTcpPtr, uint16_t totalLength, MessageType type, shared_ptr<u_char> message,
                             DEVID devid, FRAMECOUNT count, shared_ptr<int> retryCount, function<void ()> retryExceedHandler);

    void onStringMessage(const TcpConnectionPtr&, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char>& message, Timestamp);
    void sendConfirmFrame(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader);
    void sendConfirmHeartFrame(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader);
    uint16_t getAndSetFrameCount(DEVID devid, uint16_t increment);

    void setTimer(DEVID devid, TcpConnectionPtr& conn, uint16_t totalLength, MessageType type, u_char * message, function<void ()> retryExceedHandler);

    bool cancelTimer(DEVID devid, FRAMECOUNT count);

private:
    void confirm(DEVID devid, shared_ptr<u_char>& message);

    void statusMessage(const TcpConnectionPtr&, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char>& message);
    void sensorMessage(const TcpConnectionPtr&, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char>& message);
    void errorMessage(const TcpConnectionPtr&, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char>& message);
    void devidMessage(const TcpConnectionPtr&, shared_ptr<FrameHeader>& frameHeader);
    void heartMessage(const TcpConnectionPtr&, shared_ptr<FrameHeader>& frameHeader);

    EventLoop* loop_;
    TCPCodec& tcpCodec_;
    int timeoutSec_;

    //以帧计数区分的定时器
    MutexLock confirmMutex_;
    map<DEVID, map<FRAMECOUNT, TimerId> > confirmTimer_;


    //帧计数器
    MutexLock frameCountMutex_;
    map<DEVID , FRAMECOUNT> frameCountMap_;

    MessageCallback statusCallback_;
    MessageCallback sensorCallback_;
    MessageCallback errorCallback_;
    DevIDCallback devidCallback_;
    DevIDCallback heartCallback_;
};


#endif //TCPSERVER_DISPATCHER_H

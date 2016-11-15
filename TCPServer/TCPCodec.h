/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description: 编码解码TCP信息帧
**************************************************/

#ifndef TCPSERVER_TCPCODEC_H
#define TCPSERVER_TCPCODEC_H

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
#include <muduo/net/Endian.h>

#include "FrameStruct.h"


using boost::shared_ptr;
using boost::function;
using muduo::MutexLock;
using muduo::MutexLockGuard;
using muduo::net::TcpConnectionPtr;
using muduo::net::TcpConnection;
using muduo::Timestamp;
using muduo::net::Buffer;

class Dispatcher;

class TCPCodec :boost::noncopyable
{
public:
    friend class Dispatcher;

    typedef function<void (const TcpConnectionPtr&, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char>& message, Timestamp)> StringMessageCallback;

    /*************************************************
    Description:    TCPCodec构造函数，设置解码完成的回调函数
    Calls:          TCPServer::TCPServer
    Input:          cb: dispatcher回调函数
    Output:         无
    Return:         无
    *************************************************/
    explicit  TCPCodec(const StringMessageCallback& cb):
        dispatcherCallback_(cb)
    {}

    void onMessage(const TcpConnectionPtr& conn, Buffer *buf, Timestamp receiveTime);

private:

    void send(TcpConnectionPtr conn, uint16_t totalLength, uint16_t type,uint16_t seq, u_char * message);

    void printFrame(std::string tag,FrameHeader *frame, u_char* message, size_t messageLen);
    void skipWrongFrame(Buffer *buf);

    StringMessageCallback dispatcherCallback_;
};


#endif //TCPSERVER_TCPCODEC_H

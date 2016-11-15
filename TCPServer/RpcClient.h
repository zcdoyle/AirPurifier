/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-23
Description: RPC客户端
**************************************************/

#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/protorpc/RpcChannel.h>
#include <boost/bind.hpp>

#include "AirPurifier.ProtoMessage.pb.h"


using namespace muduo::net;
using namespace AirPurifier;

class RpcClient : boost::noncopyable
{
public:
    RpcClient(EventLoop* loop, const InetAddress& serverAddr)
        : loop_(loop),
          client_(loop, serverAddr, "RpcClient"),
          channel_(new RpcChannel),
          stub_(get_pointer(channel_))
    {
        client_.setConnectionCallback(
                    boost::bind(&RpcClient::onConnection, this, _1));
        client_.setMessageCallback(
                    boost::bind(&RpcChannel::onMessage, get_pointer(channel_), _1, _2, _3));
        client_.enableRetry();
    }

    void connect()
    {
        client_.connect();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        if (conn->connected())
        {
            channel_->setConnection(conn);
            LOG_INFO << "RpcServer Connnect successfully";
        }
        else
            LOG_ERROR << "RpcServer Connection lost";
    }

    EventLoop* loop_;
    TcpClient client_;
    RpcChannelPtr channel_;
public:
    MySQLService::Stub stub_;
};


#endif

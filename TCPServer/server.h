/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：TCP 收发模块
**************************************************/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "TCPCodec.h"
#include "protobuf/ProtobufCodec.h"
#include "protobuf/AirPurifier.ProtoMessage.pb.h"
#include "dispatcher.h"
#include "MessageHandler.h"
#include "configuration.h"
#include "RpcClient.h"
#include "JsonHandler.h"
#include "json/JsonCodec.h"
#include "MessageConstructor.h"

#include <muduo/base/Logging.h>
#include <muduo/base/LogFile.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/ThreadLocalSingleton.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/protorpc/RpcChannel.h>
#include <hiredis/hiredis.h>
#include <hiredis-vip/hircluster.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>
#include <stdio.h>

using namespace muduo::net;
using namespace AirPurifier;
using boost::shared_ptr;
using boost::bind;
using std::map;
using std::vector;
using std::istream;
using std::stringstream;
using muduo::ThreadLocalSingleton;
using muduo::LogFile;
using muduo::Logger;
using muduo::TimeZone;


class TCPServer : boost::noncopyable
{
    typedef shared_ptr<TcpClient> TcpClientPtr;
    typedef vector<ProtoMessage> Messages;
    typedef ThreadLocalSingleton<map<ConnectionType, TcpConnectionPtr> > LocalConnections;
    typedef ThreadLocalSingleton<map<ConnectionType, TcpClientPtr> > LocalClients;
    typedef ThreadLocalSingleton<map<ConnectionType, Messages> > UnSendMessages;

public:
    TCPServer(EventLoop* loop, const Configuration &config);
    void sendProtoMessage(ProtoMessage message, ConnectionType type);
    void start();

    /*************************************************
    Description:    根据设备id得到TCP连接
    Input:          devid：设备id
    Output:         无
    Return:         该设备对应的TCP连接
    *************************************************/
    inline TcpConnectionPtr getDevConnection(DEVID devid)
    {
        MutexLockGuard lock(devConnMutex_);
        map<DEVID, TcpConnectionPtr>::const_iterator it = devToConn_.find(devid);
        if(it != devToConn_.end())
            return it->second;
        else
            return TcpConnectionPtr(NULL);
    }


    /***************************************************
    Description:    发送信息帧，并设置以帧计数为标记的定时器
    Input:          conn：TCP连接
    Output:         无
    Return:         无
    ***************************************************/
    inline void sendWithTimer(DEVID devid, TcpConnectionPtr conn, MessageType type, uint16_t totalLength, shared_ptr<u_char> message)
    {
        weak_ptr<TcpConnection> weakTcpPtr(conn);
        function<void ()> retryExceedHandler = bind(&TCPServer::retryExceedMaxNumer, this, weakTcpPtr);
        dispatcher_.setTimer(devid, conn, totalLength, type, get_pointer(message), retryExceedHandler);
    }

    inline void retryExceedMaxNumer(weak_ptr<TcpConnection> weakConn)
    {
        LOG_DEBUG << "retryExceed! ";
        TcpConnectionPtr conn(weakConn.lock());
        if(conn == 0 || conn.use_count()==0)
            return;
        if (conn)
        {
            clearredis(conn);
            conn->forceClose();
        }
    }

    /***************************************************
    Description:    收到devid帧更新设备连接信息
    Input:          devId：设备id
                    type：设备类型
                    devVec:设备唯一号数组
                    conn：对应设备的TCP连接
    Output:         无
    Return:         设备id
    ***************************************************/
    inline void updateConnectionInfo(const TcpConnectionPtr& conn,DEVID devid)
    {
        MutexLockGuard lock(devConnMutex_);

        map<DEVID, TcpConnectionPtr>::iterator devToConnIt = devToConn_.find(devid);
            if(devToConnIt != devToConn_.end())
        {
            TcpConnectionPtr conn_tmp = devToConnIt->second;
            LOG_DEBUG<<"new conn: "<<((conn_tmp != conn)?true:false);
            if(conn_tmp != conn)
            {
                conn_tmp->forceClose();
                connHasDev_[conn] = devid;
                devToConn_[devid] = conn;
                LOG_DEBUG<<"new conn saved";
            }
                else
            {
            //如果原来的连接和新的连接相等，就不更新
            }
        }
        else
        {
            connHasDev_[conn] = devid;
            devToConn_[devid] = conn;
        }
    }

    /***************************************************
    Description:    收到heart帧更新设备连接信息
    Input:          devId：设备id
                    type：设备类型
                    devVec:设备唯一号数组
                    conn：对应设备的TCP连接
    Output:         无
    Return:         设备id
    ***************************************************/
    /*inline void updateConnectionInfo(const TcpConnectionPtr& conn,DEVID devid)
    {
        MutexLockGuard lock(devConnMutex_);
        map<TcpConnectionPtr, DEVID>::iterator connIt = connHasDev_.find(conn);
        if(connIt == connHasDev_.end())
            connHasDev_[conn] = devid;

        map<DEVID, TcpConnectionPtr>::iterator devToConnIt = devToConn_.find(devid);
        if(devToConnIt == devToConn_.end())
            devToConn_[devid] = conn;
    }*/
    void clearConnectionInfo(const weak_ptr<TcpConnection> &weakConn);
    void clearConnectionInfo_nodelredis(const weak_ptr<TcpConnection> &weakConn);
    void clearredis(const weak_ptr<TcpConnection> &weakConn);

    Dispatcher dispatcher_;
    redisContext* redisConn_;
    redisClusterContext* redisConnClu_;

private:
    void onServerConnection(const TcpConnectionPtr& conn);
    void onJsonConnection(const TcpConnectionPtr& conn);
    void onMySQLProxyConnection(const TcpConnectionPtr& conn);
    void onHBaseProxyConnection(const TcpConnectionPtr& conn);
    void connectRedis();
    void connectRedisClu();
    void threadInit(EventLoop* loop);

    const Configuration& config_;                               //配置内容
    EventLoop* loop_;                                           //时间循环
    TcpServer server_;                                          //tcp消息服务器
    TcpServer jsonMessageServer_;                               //json消息服务器
    TCPCodec tcpCodec_;                                         //tcp帧编码解码器
    ProtobufCodec protoCodec_;                                  //protobuf编码解码器
    RpcClient rpcClient_;                                       //RPC客户端
    MessageHandler messageHandler_;                             //tcp消息处理对象
    JsonHandler jsonHandler_;                                   //json消息处理对象
    JsonCodec jsonCodec_;                                       //json编码解码器

    MutexLock devConnMutex_;                                    //连接共享变量互斥锁
    map<DEVID, TcpConnectionPtr> devToConn_;                    //设备唯一号 -> TCP连接的映射
    map<TcpConnectionPtr, DEVID > connHasDev_;                  //TCP连接 -> 设备唯一号的映射
};

#endif



/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：TCP 收发模块
**************************************************/


#include "server.h"
#include "http.h"
#include "JsonHandler.h"
#include <sstream>

class RedisReply
{
public:
    explicit RedisReply(redisReply* reply): reply_(reply) {}

    ~RedisReply()
    {
        freeReplyObject(reply_);
    }

private:
    redisReply* reply_;
};
/***************************************************
Description:    TCPServer构造函数，初始化各个模块
Input:          loop：时间循环
                config：配置内容
Output:         无
Return:         无
***************************************************/
TCPServer::TCPServer(EventLoop* loop, const Configuration &config)
            : config_(config),
              loop_(loop),
              server_(loop, InetAddress(config.listenPort_), "TCPServer"),
              jsonMessageServer_(loop, InetAddress(config.jsonListenPort_), "JSONMessageServer"),
              dispatcher_(loop, tcpCodec_, config.timeout_sec_),
              tcpCodec_(bind(&Dispatcher::onStringMessage, &dispatcher_, _1, _2, _3, _4)),
              protoCodec_(),
              rpcClient_(loop, InetAddress(config.MySQLProxyAddress_, config.MySQLRPCPort_)),
              messageHandler_(this, &rpcClient_),
              jsonHandler_(&rpcClient_, this, loop),
              jsonCodec_(bind(&JsonHandler::onJsonMessage, jsonHandler_, _1, _2))
{
    server_.setConnectionCallback(bind(&TCPServer::onServerConnection, this, _1));
    server_.setMessageCallback(bind(&TCPCodec::onMessage, &tcpCodec_, _1, _2, _3)); //消息编码回调
    server_.setThreadNum(config.threadNum_); //设定线程数

    jsonMessageServer_.setConnectionCallback(bind(&TCPServer::onJsonConnection, this, _1));
    jsonMessageServer_.setMessageCallback(bind(&JsonCodec::onMessage, &jsonCodec_, _1, _2, _3));

    //设置各类处理信息的回调函数,派发器利用这些回调函数分发不同消息
    dispatcher_.setCallbacks(bind(&MessageHandler::onStatusMessage, &messageHandler_, _1, _2, _3),
                             bind(&MessageHandler::onSensorMessage, &messageHandler_, _1, _2, _3),
                             bind(&MessageHandler::onErrorMessage, &messageHandler_, _1, _2, _3),
                             bind(&MessageHandler::onDevidMessage, &messageHandler_, _1, _2),
                             bind(&MessageHandler::onHeartMessage, &messageHandler_, _1, _2));

}

/***************************************************
Description:    连接Redis数据库
Calls:          TCPServer::start()
Input:          无
Output:         无
Return:         无
***************************************************/
void TCPServer::connectRedis()
{
    redisConn_  = redisConnect(config_.RedisAddress_,config_.RedisPort_);
    if(redisConn_ != NULL && redisConn_->err)
    {
        LOG_FATAL << "redis connect error" << redisConn_->errstr;
        return;
    }
    redisReply *reply;
    reply= (redisReply* )redisCommand(redisConn_, "AUTH %s", config_.RedisPassword_);
    if (reply->type == REDIS_REPLY_ERROR)
        LOG_FATAL << "redis authoricate error" << redisConn_->errstr;

    LOG_INFO << "Redis connect successful";
}

/***************************************************
Description:    连接Redis cluster数据库
Calls:          TCPServer::start()
Input:          无
Output:         无
Return:         无
***************************************************/
void TCPServer::connectRedisClu()
{
    redisConnClu_ = redisClusterConnect("101.200.181.238:6380,101.200.181.238:6381,101.200.184.135:6380,101.200.184.135:6381,123.57.224.166:6380,123.57.224.166:6381",HIRCLUSTER_FLAG_NULL);
    if(redisConn_ == NULL || redisConn_->err)
    {
        LOG_FATAL << "rediscluster connect error" << redisConn_->errstr;
        return;
    }

    LOG_INFO << "Rediscluster connect successful";
}

/***************************************************
Description:    启动TCPServer
Input:          无
Output:         无
Return:         无
***************************************************/
void TCPServer::start()
{
    //初始化各线程
    server_.setThreadInitCallback(bind(&TCPServer::threadInit, this, _1));
    server_.start(); //start以后就利用各个注册的回调函数工作

    jsonMessageServer_.start();

    connectRedis();
    connectRedisClu();
    //rpcClient_.connect(); //TODO:RPCClient

//    Http::post(config_.smsAddress_, config_.smsPort_, config_.smsPage, "msg=TCPServer模块启动 【】");
}

/***************************************************
Description:    处理新的TCP连接或断开
Input:          无
Output:         无
Return:         无
***************************************************/
void TCPServer::onServerConnection(const TcpConnectionPtr& conn)
{
    conn->setTcpNoDelay(true);
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected())
    {
        //usleep(1000000);  //防止上次连接刚断开马上又连上导致数据库异步写入冲突的问题
    }
    else
    {
        weak_ptr<TcpConnection> weakTcpPtr(conn);
        clearConnectionInfo(weakTcpPtr);
    }
}

void TCPServer::clearConnectionInfo(const weak_ptr<TcpConnection> &weakConn)
{
    //清除连接信息
    TcpConnectionPtr conn(weakConn.lock());
    MutexLockGuard lock(devConnMutex_);
    map<TcpConnectionPtr, DEVID>::iterator connIt = connHasDev_.find(conn);
    if(connIt != connHasDev_.end())
    {
        DEVID id = connIt->second;

        //clear map
        map<DEVID, TcpConnectionPtr>::iterator devToConnIt = devToConn_.find(id);
        if(devToConnIt != devToConn_.end())
        {
            if(devToConnIt->second == conn)
                devToConn_.erase(devToConnIt);
        }
        connHasDev_.erase(connIt);
    }
}

void TCPServer::clearConnectionInfo_nodeldev(const weak_ptr<TcpConnection> &weakConn)
{
    //清除连接信息
    TcpConnectionPtr conn(weakConn.lock());
    MutexLockGuard lock(devConnMutex_);
    map<TcpConnectionPtr, DEVID>::iterator connIt = connHasDev_.find(conn);
    if(connIt != connHasDev_.end())
    {
        connHasDev_.erase(connIt);
    }
}

void TCPServer::clearredis(const weak_ptr<TcpConnection> &weakConn)
{
    //清除redis连接信息
    TcpConnectionPtr conn(weakConn.lock());
    map<TcpConnectionPtr, DEVID>::iterator connIt = connHasDev_.find(conn);
    if(connIt != connHasDev_.end())
    {
        DEVID id = connIt->second;

        //clear redis
        char command[256];
        sprintf(command, "HMSET STATUS%lx switch %d", id, -1);
        LOG_DEBUG<<command;
        RedisReply reply_status((redisReply*)redisCommand(redisConn_,command));
        RedisReply replyclu((redisReply*)redisClusterCommand(redisConnClu_,command));
        LOG_DEBUG<<"clear redis success";
    }
}

/***************************************************
Description:    处理新的MySQLPRoxy连接或者连接断开情况
Input:          conn：TCP连接
Output:         无
Return:         无
***************************************************/
void TCPServer::onMySQLProxyConnection(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        conn->setTcpNoDelay(true);
        LOG_INFO << "Connect to MySQLProxy successfully";
        LocalConnections::instance()[MySQL] = conn;
        //发送以前发送失败的消息
        for(Messages::iterator it = UnSendMessages::instance()[MySQL].begin();
            it != UnSendMessages::instance()[MySQL].end();
            ++it)
        {
            protoCodec_.send(conn, *it);
            //防止过快发送
            usleep(1);
        }
        UnSendMessages::instance()[MySQL].clear();
    }
    else
    {
        LOG_ERROR << "Lost connection to MySQLProxy";
    }
}

/***************************************************
Description:    处理新的json连接或者连接断开情况
Input:          conn：TCP连接
Output:         无
Return:         无
***************************************************/
void TCPServer::onJsonConnection(const TcpConnectionPtr &conn)
{
    LOG_DEBUG <<"JSONConnection: " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if(!conn->connected())
        jsonCodec_.cleanup(conn);
}


/***************************************************
Description:    处理新的HBaseProxy连接或者连接断开情况
Input:          conn：TCP连接
Output:         无
Return:         无
***************************************************/
void TCPServer::onHBaseProxyConnection(const TcpConnectionPtr &conn)
{
    if(conn->connected())
    {
        conn->setTcpNoDelay(true);
        LOG_INFO << "Connect to HBaseProxy successfully";
        LocalConnections::instance()[HBase] = conn;
        //发送以前发送失败的消息
        for(Messages::iterator it = UnSendMessages::instance()[HBase].begin();
            it != UnSendMessages::instance()[HBase].end();
            ++it)
        {
            protoCodec_.send(conn, *it);
            //防止过快发送
            usleep(1);
        }
        UnSendMessages::instance()[HBase].clear();
    }
    else
    {
        LOG_ERROR << "Lost connection to HBaseProxy";
    }
}

/***************************************************
Description:    向数据库代理模块（MySQLProxy或者HBaseProxy）发送信息 利用protobuf向数据库发消息
Calls:          MessageHandler::
Input:          messageToSend：待发送的protobuf message
                type：连接类型
Output:         无
Return:         无
***************************************************/
void TCPServer::sendProtoMessage(ProtoMessage messageToSend, ConnectionType type)
{
    TcpConnectionPtr dbProxyConn = LocalConnections::instance()[type];
    if(dbProxyConn->connected())
    {
        protoCodec_.send(dbProxyConn, messageToSend);
    }
    else
    {
        //保存发送失败的消息
        LOG_INFO << "Save unsend MySQL messages to UnSendMessages";
        UnSendMessages::instance()[type].push_back(messageToSend);
    }
}


/***************************************************
Description:    各线程初始化，每个线程各有一个MySQL客户端和HBase客户端
Calls:          TCPServer::start()
Input:          loop：事件循环
Output:         无
Return:         无
***************************************************/
void TCPServer::threadInit(EventLoop* loop)
{
//    //连接MySQL代理服务器
//    TcpClientPtr mysqlProxyPtr(new TcpClient(loop, InetAddress(config_.MySQLProxyAddress_, config_.MySQLProxyPort_), "MySQLProxy"));
//    LocalClients::instance()[MySQL] = mysqlProxyPtr;
//    mysqlProxyPtr->setConnectionCallback(bind(&TCPServer::onMySQLProxyConnection, this, _1));
//    mysqlProxyPtr->enableRetry();
//    mysqlProxyPtr->connect();
//    UnSendMessages::instance()[MySQL].clear();

//    //连接Hbase代理服务器
//    TcpClientPtr hbaseProxyPtr(new TcpClient(loop, InetAddress(config_.HBaseProxyAddress_, config_.HBaseProxyPort_), "HBaseProxy"));
//    LocalClients::instance()[HBase] = hbaseProxyPtr;
//    hbaseProxyPtr->setConnectionCallback(
//            bind(&TCPServer::onHBaseProxyConnection, this, _1));
//    hbaseProxyPtr->enableRetry();
//    hbaseProxyPtr->connect();
//    UnSendMessages::instance()[HBase].clear();
//    LOG_INFO << "Thread init succssful";
}






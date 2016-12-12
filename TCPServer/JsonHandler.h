/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：处理JSON消息
**************************************************/
#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <muduo/base/Logging.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpClient.h>
#include "RpcClient.h"
#include "FrameStruct.h"

using namespace muduo::net;
using muduo::Logger;
using muduo::Timestamp;
using namespace rapidjson;


class TCPServer;
class MySQLRpcParam;

class JsonHandler
{
public:
    explicit JsonHandler(RpcClient* rpc, TCPServer* tcpServer, EventLoop* loop):
        rpcClient_(rpc),
        tcpServer_(tcpServer),
        loop_(loop){}

    void onJsonMessage(const TcpConnectionPtr& conn, const Document &jsonObject);
    void getRedisDateTime(char* timestr);
    void updateStatusSwitchDatainRedis(uint64_t DeviceID, uint32_t val, char* timeStr);
    void updateStatusModeDatainRedis(uint64_t DeviceID, uint32_t val, char* timeStr);
    void updateStatusTimingDatainRedis(uint64_t DeviceID, uint32_t val, char* timeStr);
    void updateStatusChildDatainRedis(uint64_t DeviceID, uint32_t val, char* timeStr);
    void updateStatusErrorDatainRedis(uint64_t DeviceID, uint32_t val, char* timeStr);
    void clearRedis(DEVID devid);

private:
    /*************************************************
    Description:    生成Json回复字符串
    Calls:          JsonHandler:
    Input:          s：字符buffer
                    result：结果字段
                    msg：内容字段
    Output:         无
    Return:         无
    *************************************************/
    inline const char* setResultJson(StringBuffer& s,bool result, std::string& msg)
    {
        Writer<StringBuffer> writer(s);
        writer.StartObject();
        writer.String("result");
        writer.Bool(result);
        writer.String("msg");
        writer.String(msg.c_str());
        writer.EndObject();
        return s.GetString();
    }

    /*************************************************
    Description:    发送Json字符串作为响应值
    Calls:          JsonHandler:
    Input:          s：字符buffer
                    result：结果字段
                    msg：内容字段
    Output:         无
    Return:         无
    *************************************************/
    inline void returnJsonResult(const TcpConnectionPtr& conn, bool result, std::string msg = "")
    {
        StringBuffer s;
        if(conn->connected())
        {
            conn->send(setResultJson(s, result, msg));
            conn->shutdown();
        }
    }

    bool getConnbyDevID(const TcpConnectionPtr& jsonConn, TcpConnectionPtr& conn, DEVID devid);

    void openControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject);
    void modeControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject);
    void timeControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject);
    void childlockControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject);
    void errorreminderControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject);
    void updateControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject);


    EventLoop* loop_;
    RpcClient* rpcClient_;
    TCPServer* tcpServer_;
    //MutexLock timeMutex_;

    const static char* OpenControl;
    const static char* ModeControl;
    const static char* TimeControl;
    const static char* ChildLockControl;
    const static char* ErrorReminderControl;
    const static char* UpdateControl;
};

#endif

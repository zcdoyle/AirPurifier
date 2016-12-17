/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：处理JSON消息
**************************************************/

#include "JsonHandler.h"
#include "protobuf/AirPurifier.ProtoMessage.pb.h"
#include "MessageConstructor.h"
#include "MessageHandler.h"
#include "server.h"
#include <stdlib.h>

using namespace AirPurifier;

const char* JsonHandler::OpenControl = "switch";
const char* JsonHandler::ModeControl = "mode";
const char* JsonHandler::TimeControl = "timing";
const char* JsonHandler::ChildLockControl = "child_lock";
const char* JsonHandler::ErrorReminderControl = "error_reminder";
const char* JsonHandler::UpdateControl = "operation_type";

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
void JsonHandler::getRedisDateTime(char* timestr)
{
//    //时间函数不是线程安全的, 利用互斥锁保护
//    MutexLockGuard lock(timeMutex_);

    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep);

    sprintf(timestr, "%04d%02d%02d%02d%02d", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min);
}
void JsonHandler::updateStatusSwitchDatainRedis(DEVID DeviceID, uint32_t val, char* timeStr)
{
    char command[256];
    sprintf(command, "HMSET STATUS%lx switch %d time %s",
            DeviceID, val, timeStr);
    LOG_DEBUG<<command;
    RedisReply reply((redisReply*)redisCommand(tcpServer_->redisConn_,command));
}
void JsonHandler::updateStatusModeDatainRedis(DEVID DeviceID, uint32_t val, char* timeStr)
{
    char command[256];
    sprintf(command, "HMSET STATUS%lx mode %d time %s",
            DeviceID, val, timeStr);
    LOG_DEBUG<<command;
    RedisReply reply((redisReply*)redisCommand(tcpServer_->redisConn_,command));
}
void JsonHandler::updateStatusTimingDatainRedis(DEVID DeviceID, uint32_t val, char* timeStr)
{
    char command[256];
    sprintf(command, "HMSET STATUS%lx timing %d time %s",
            DeviceID, val, timeStr);
    LOG_DEBUG<<command;
    RedisReply reply((redisReply*)redisCommand(tcpServer_->redisConn_,command));
}
void JsonHandler::updateStatusChildDatainRedis(DEVID DeviceID, uint32_t val, char* timeStr)
{
    char command[256];
    sprintf(command, "HMSET STATUS%lx childlock %d time %s",
            DeviceID, val, timeStr);
    LOG_DEBUG<<command;
    RedisReply reply((redisReply*)redisCommand(tcpServer_->redisConn_,command));
}
void JsonHandler::updateStatusErrorDatainRedis(DEVID DeviceID, uint32_t val, char* timeStr)
{
    char command[256];
    sprintf(command, "HMSET STATUS%lx errorreminder %d time %s",
            DeviceID, val, timeStr);
    LOG_DEBUG<<command;
    RedisReply reply((redisReply*)redisCommand(tcpServer_->redisConn_,command));
}
void JsonHandler::clearRedis(DEVID DeviceID)
{
    //clear redis
    char command[256];
    sprintf(command, "HMSET STATUS%lx switch %d", DeviceID, -1);
    LOG_DEBUG<<command;
    RedisReply reply_status((redisReply*)redisCommand(tcpServer_->redisConn_,command));
    RedisReply replyclu((redisReply*)redisClusterCommand(tcpServer_->redisConnClu_,command));
    LOG_DEBUG<<"clear redis success";
}
/*************************************************
Description:    根据JSON对象发送不同类型的数据
Calls:          JsonHandler:
Input:          conn TCP连接
                jsonObject json对象
Output:         无
Return:         无
*************************************************/
void JsonHandler::onJsonMessage(const TcpConnectionPtr& conn, const Document &jsonObject)
{
    if(jsonObject.HasMember(OpenControl))
    {
        openControl(conn, jsonObject);
    }
    if(jsonObject.HasMember(ModeControl))
    {
        modeControl(conn, jsonObject);
    }
    if(jsonObject.HasMember(TimeControl))
    {
        timeControl(conn, jsonObject);
    }
    if(jsonObject.HasMember(ChildLockControl))
    {
        childlockControl(conn, jsonObject);
    }
    if(jsonObject.HasMember(ErrorReminderControl))
    {
        errorreminderControl(conn, jsonObject);
    }
    if(jsonObject.HasMember(UpdateControl))
    {
        updateControl(conn, jsonObject);
    }
    if(!jsonObject.HasMember(OpenControl)&&!jsonObject.HasMember(ModeControl)&&!jsonObject.HasMember(TimeControl)&&!jsonObject.HasMember(ChildLockControl)&&!jsonObject.HasMember(ErrorReminderControl)&&!jsonObject.HasMember(UpdateControl))
    {
        return returnJsonResult(conn, false, "unkonw message");
    }
}

/*************************************************
Description:    根据设备id获取TCP连接
Calls:          JsonHandler:
Input:          jsonConn：json的TCP连接
                conn：需要获取的TCP连接
                devid：需要获取的设备号
Output:         无
Return:         是否获取成功
*************************************************/
bool JsonHandler::getConnbyDevID(const TcpConnectionPtr& jsonConn, TcpConnectionPtr& conn, DEVID devid)
{
    conn = tcpServer_->getDevConnection(devid);
    if(get_pointer(conn) == NULL)
    {
        LOG_INFO << "no connection found";
        clearRedis(devid);
        returnJsonResult(jsonConn, false, "device not connected to server");
        return false;
    }
    return true;
}

/*************************************************
Description:    开关控制
Calls:          JsonHandler:
Input:          jsonConn：json的TCP连接
                jsonObjcet：json对象

Output:         无
Return:         无
*************************************************/
void JsonHandler::openControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject)
{
    //解析json串
    int opencontrol = jsonObject[OpenControl].GetInt();
    DEVID devid = strtoul(jsonObject["device_id"].GetString(),NULL,16);

    //received json log
    char jsonstr[256];
    sprintf(jsonstr, "OpenControl: device_id => %lx, switch => %d",
            devid, opencontrol);
    LOG_DEBUG<<jsonstr;

    //get conn by devid
    TcpConnectionPtr conn;
    if(getConnbyDevID(jsonConn,conn,devid))
    {
        returnJsonResult(jsonConn, true);
        MessageType type;
        type = OPEN_CTRL;

        int MessageLength = 1;
        shared_ptr<u_char> message((u_char*)malloc(MessageLength)); //构造控制开关帧，需要1byte空间
        MessageConstructor::openControl(get_pointer(message),opencontrol);
//        FrameMessage msg;
//        msg.content.open.isopen = opencontrol;
//        msg.content.open.res = 0;
//        memcpy(get_pointer(message), &msg, sizeof(msg));

        function<void()> sendCb = bind(&TCPServer::sendWithTimer, tcpServer_, devid, conn, type, HeaderLength+MessageLength, message);
        conn->getLoop()->runInLoop(sendCb);

        //update redis TODO: delete after hardware is ready
        //char timeStr[16];
        //getRedisDateTime(timeStr);
        //updateStatusSwitchDatainRedis(devid,opencontrol,timeStr);
    }
}

void JsonHandler::modeControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject)
{
    //解析json串
    int modecontrol = jsonObject[ModeControl].GetInt();
    DEVID devid = strtoul(jsonObject["device_id"].GetString(),NULL,16);

    //received json log
    char jsonstr[256];
    sprintf(jsonstr, "ModeControl: device_id => %lx, mode => %d",
            devid, modecontrol);
    LOG_DEBUG<<jsonstr;

    //get conn by devid
    TcpConnectionPtr conn;
    if(getConnbyDevID(jsonConn,conn,devid))
    {
        returnJsonResult(jsonConn, true);
        MessageType type;
        type = MODE_CTRL;

        int MessageLength = 1;
        shared_ptr<u_char> message((u_char*)malloc(MessageLength)); //构造控制模式帧，需要1byte空间
        MessageConstructor::modeControl(get_pointer(message),modecontrol);

        function<void()> sendCb = bind(&TCPServer::sendWithTimer, tcpServer_, devid, conn, type, HeaderLength+MessageLength, message);
        conn->getLoop()->runInLoop(sendCb);

        //update redis TODO: delete after hardware is ready
        //char timeStr[16];
        //getRedisDateTime(timeStr);
        //updateStatusModeDatainRedis(devid,modecontrol,timeStr);
    }
}

void JsonHandler::timeControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject)
{
    //解析json串
    int timecontrol = jsonObject[TimeControl].GetInt();
    DEVID devid = strtoul(jsonObject["device_id"].GetString(),NULL,16);

    //received json log
    char jsonstr[256];
    sprintf(jsonstr, "TimeControl: device_id => %lx, timing => %d",
            devid, timecontrol);
    LOG_DEBUG<<jsonstr;

    //get conn by devid
    TcpConnectionPtr conn;
    if(getConnbyDevID(jsonConn,conn,devid))
    {
        returnJsonResult(jsonConn, true);
        MessageType type;
        type = TIME_CTRL;

        int MessageLength = 1;
        shared_ptr<u_char> message((u_char*)malloc(MessageLength)); //构造控制定时帧，需要1byte空间
        MessageConstructor::timeControl(get_pointer(message),timecontrol/3600);

        function<void()> sendCb = bind(&TCPServer::sendWithTimer, tcpServer_, devid, conn, type, HeaderLength+MessageLength, message);
        conn->getLoop()->runInLoop(sendCb);

        //update redis TODO: delete after hardware is ready
        //char timeStr[16];
        //getRedisDateTime(timeStr);
        //updateStatusTimingDatainRedis(devid,timecontrol,timeStr);
    }
}
void JsonHandler::childlockControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject)
{
    //解析json串
    int childlockcontrol = jsonObject[ChildLockControl].GetInt();
    DEVID devid = strtoul(jsonObject["device_id"].GetString(),NULL,16);

    //received json log
    char jsonstr[256];
    sprintf(jsonstr, "ChildLockControl: device_id => %lx, childlock => %d",
            devid, childlockcontrol);
    LOG_DEBUG<<jsonstr;

    //get conn by devid
    TcpConnectionPtr conn;
    if(getConnbyDevID(jsonConn,conn,devid))
    {
        returnJsonResult(jsonConn, true);
        MessageType type;
        type = CHILDLOCK_CTRL;

        int MessageLength = 1;
        shared_ptr<u_char> message((u_char*)malloc(MessageLength)); //构造控制设置帧，需要1byte空间
        MessageConstructor::childlockControl(get_pointer(message),childlockcontrol);

        function<void()> sendCb = bind(&TCPServer::sendWithTimer, tcpServer_, devid, conn, type, HeaderLength+MessageLength, message);
        conn->getLoop()->runInLoop(sendCb);

        //update redis TODO: delete after hardware is ready
        //char timeStr[16];
        //getRedisDateTime(timeStr);
        //updateStatusChildDatainRedis(devid,childlockcontrol,timeStr);
    }
}
void JsonHandler::errorreminderControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject)
{
    //解析json串
    int errorremindercontrol = jsonObject[ErrorReminderControl].GetInt();
    DEVID devid = strtoul(jsonObject["device_id"].GetString(),NULL,16);

    //received json log
    char jsonstr[256];
    sprintf(jsonstr, "ErrorReminderControl: device_id => %lx, errorreminder => %d",
            devid, errorremindercontrol);
    LOG_DEBUG<<jsonstr;

    //get conn by devid
    TcpConnectionPtr conn;
    if(getConnbyDevID(jsonConn,conn,devid))
    {
        returnJsonResult(jsonConn, true);
        MessageType type;
        type = ERRORREMINDER_CTRL;

        int MessageLength = 1;
        shared_ptr<u_char> message((u_char*)malloc(MessageLength)); //构造控制设置帧，需要1byte空间
        MessageConstructor::errorreminderControl(get_pointer(message),errorremindercontrol);

        function<void()> sendCb = bind(&TCPServer::sendWithTimer, tcpServer_, devid, conn, type, HeaderLength+MessageLength, message);
        conn->getLoop()->runInLoop(sendCb);

        //update redis TODO: delete after hardware is ready
        //char timeStr[16];
        //getRedisDateTime(timeStr);
        //updateStatusErrorDatainRedis(devid,errorremindercontrol,timeStr);
    }
}
void JsonHandler::updateControl(const TcpConnectionPtr& jsonConn, const Document& jsonObject)
{
    //解析json串
    int updatecontrol = jsonObject[UpdateControl].GetInt();
    DEVID devid = strtoul(jsonObject["device_id"].GetString(),NULL,16);

    //received json log
    char jsonstr[256];
    sprintf(jsonstr, "UpdateControl: device_id => %lx, update => %d",
            devid, updatecontrol);
    LOG_DEBUG<<jsonstr;

    //get conn by devid
    TcpConnectionPtr conn;
    if(getConnbyDevID(jsonConn,conn,devid))
    {
        returnJsonResult(jsonConn, true);
        MessageType type;
        type = UPDATE_CTRL;

        int MessageLength = 1;
        shared_ptr<u_char> message((u_char*)malloc(MessageLength)); //构造控制更新帧，需要1byte空间
        MessageConstructor::updateControl(get_pointer(message),updatecontrol);

        function<void()> sendCb = bind(&TCPServer::sendWithTimer, tcpServer_, devid, conn, type, HeaderLength+MessageLength, message);
        conn->getLoop()->runInLoop(sendCb);
    }
}

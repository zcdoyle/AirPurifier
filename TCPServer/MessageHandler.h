/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：消息处理器，完成具体消息的操作
**************************************************/

#ifndef TCPSERVER_MESSAGEHANDLER_H
#define TCPSERVER_MESSAGEHANDLER_H
#include "TCPCodec.h"
#include "protobuf/AirPurifier.ProtoMessage.pb.h"
#include "dispatcher.h"

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

using namespace muduo::net;
using namespace AirPurifier;
using boost::shared_ptr;
using boost::bind;
using std::map;
using std::vector;
using std::istream;
using std::stringstream;

//连接的类型，分为MySQL和HBase两种连接，交给不同模块完成数据存取
typedef enum connectionType
{
    MySQL = 0,
    HBase,
}ConnectionType;

class TCPServer;
class RpcClient;

//MySQL RPC的封装类，用于传递参数
class MySQLRpcParam
{
public:
    TcpConnectionPtr conn;
};

class MessageHandler
{
public:
    explicit MessageHandler(TCPServer *tcpserver, RpcClient* rpcClient):
        tcpserver_(tcpserver),
        rpcClient_(rpcClient) {}

    void onStatusMessage(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char> message);
    void onSensorMessage(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char> message);
    void onErrorMessage(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader, shared_ptr<u_char> message);
    void onDevidMessage(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader);
    void onHeartMessage(const TcpConnectionPtr& conn, shared_ptr<FrameHeader>& frameHeader);

    void updateStatusDatainRedis(uint64_t DeviceID, uint32_t switchStatus, uint32_t modeStatus, uint32_t timing, uint32_t ver, uint32_t childLock, uint32_t errorReminder, uint32_t totm, uint32_t topur, char* timeStr);
    void updateSensorDatainRedis(uint64_t DeviceID, uint16_t hcho, uint16_t pm2p5, uint16_t temperature, uint16_t humidity, char* timeStr);
    void updateErrorDatainRedis(uint64_t DeviceID, uint32_t fsc, uint32_t ibc, uint32_t ibe, uint32_t uve, char* timeStr);
    void updateIpDatainRedis(uint64_t DeviceID, char* ipstr, char* timeStr);

private:

    void getMySQLDateTime(char* date, char* timestr);
    void getHBaseDateTime(char* date, char* timestr);
    void getRedisDateTime(char* timestr);

    void initializeHBaseProto(ProtoMessage& protoMessage, MessageType type, DEVID devId);

    TCPServer* tcpserver_;
    RpcClient* rpcClient_;
    MutexLock timeMutex_;
};


#endif //TCPSERVER_MESSAGEHANDLER_H

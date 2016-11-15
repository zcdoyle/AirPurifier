/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-23
Description：读取配置文件
**************************************************/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define LINELENGTH 512

using std::ifstream;

class Configuration
{
public:
    Configuration();
    char MySQLProxyAddress_[LINELENGTH];           //MySQLProxy地址
    int MySQLProxyPort_;                           //MySQLProxy端口
    int MySQLRPCPort_;                             //MySQL RPC端口
    char HBaseProxyAddress_[LINELENGTH];           //HBaseProxy地址
    int HBaseProxyPort_;                           //MySQLProxy端口
    char RedisAddress_[LINELENGTH];                //Redis服务器地址
    int RedisPort_;                                //Redis服务器端口
    char RedisPassword_[LINELENGTH];               //Redis服务器登录密码
    int listenPort_;                               //Redis服务器地址
    int jsonListenPort_;                           //Json消息端口
    char smsAddress_[LINELENGTH];                  //短信服务地址
    char smsPort_[LINELENGTH];                     //短信服务端口
    char smsPage[LINELENGTH];                      //短信服务页面
    uint32_t serverAddr_;                          //服务器信宿地址
    int timeout_sec_;                              //超时重发时长（秒）
    int threadNum_;                                //线程数
    bool isLogInFile_;                             //是否记录日志文件
    bool isDaemonize_;                             //是否daemonize
private:
    void setValue(char* line, int& item, const char* name);
    void setValue(char* line, char* item, const char* name);
    void setValue(char* line, bool& item, const char* name);
    void setServerAddr(char* line);
};


#endif

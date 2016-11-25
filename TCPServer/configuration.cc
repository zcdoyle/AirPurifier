/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-23
Description：读取配置文件
**************************************************/

#include "configuration.h"

/*************************************************
Description:    设置整数配置项
Calls:          Configuration::Configuration
Input:          line：字符串
                item：要设置的项目
                name：项目名
Output:         无
Return:         无
*************************************************/
void Configuration::setValue(char* line, int& item, const char* name)
{
    char* position;
    if((position = strstr(line, name)) != NULL)
    {
        position += strlen(name) + 1;
        item = atoi(position);
    }
}

/*************************************************
Description:    设置整数配置项
Calls:          Configuration::Configuration
Input:          line：字符串
                item：要设置的项目
                name：项目名
Output:         无
Return:         无
*************************************************/
void Configuration::setValue(char* line, char* item, const char* name)
{
    char* position;
    if((position = strstr(line, name)) != NULL)
    {
        position += strlen(name) + 1;
        strcpy(item, position);
    }
}

/*************************************************
Description:    设置布尔值配置项
Calls:          Configuration::Configuration
Input:          line：字符串
                item：要设置的项目
                name：项目名
Output:         无
Return:         无
*************************************************/
void Configuration::setValue(char* line, bool& item, const char* name)
{
    char* position;
    if((position = strstr(line, name)) != NULL)
    {
        position += strlen(name) + 1;
        if(strcmp(position, "y") == 0 || strcmp(position, "yes") == 0)
            item = true;
        else
            item = false;
    }
}

/*************************************************
Description:    设置服务器信宿地址
Calls:          Configuration::Configuration
Input:          line：字符串
Output:         无
Return:         无
*************************************************/
void Configuration::setServerAddr(char* line)
{
    int serverAddr;
    char* position;
    if((position = strstr(line, "destination_address")) != NULL)
    {
        position += strlen("destination_address") + 1;
        serverAddr = atoi(position);
        serverAddr_ = serverAddr;
        serverAddr_ |= 0xF0000000;
    }
}


/*************************************************
Description:    读取配置文件，初始化配置项
Calls:          Configuration::Configuration
Input:          line：字符串
Output:         无
Return:         无
*************************************************/
Configuration::Configuration()
{
    //ifstream configFile("./TCPServer.cfg", std::ios::in);
//    ifstream configFile("/home/zcdoyle/pro/air/RemoteControl/TCPServer/build/bin/TCPServer.cfg", std::ios::in);
    ifstream configFile("/mnt/AirPurifier/TCPServer/build/bin/TCPServer.cfg", std::ios::in);
    char line[LINELENGTH];

    //逐行读入配置文件
    while(configFile.getline(line, sizeof(line)))
    {
        if (strstr(line, "#") != NULL) //注释行
            continue;
        setValue(line, MySQLProxyAddress_, "MySQLProxyAddress");
        setValue(line, MySQLProxyPort_, "MySQLProxyPort");
        setValue(line, MySQLRPCPort_, "MySQLRPCPort");
        setValue(line, HBaseProxyAddress_, "HBaseProxyAddress");
        setValue(line, HBaseProxyPort_, "HBaseProxyPort");
        setValue(line, RedisAddress_, "RedisAddress");
        setValue(line, RedisPort_, "RedisPort");
        setValue(line, RedisPassword_, "RedisPassword");
        setValue(line, listenPort_, "listenPort");
        setValue(line, jsonListenPort_, "jsonListenPort");
        setValue(line, smsAddress_, "smsAddress");
        setValue(line, smsPort_, "smsPort");
        setValue(line, smsPage, "smsPage");
        setServerAddr(line);
        setValue(line, timeout_sec_, "timeout_retry_second");
        setValue(line, threadNum_, "threadNumber");
        setValue(line, isLogInFile_, "log_in_file");
        setValue(line, isDaemonize_, "daemonize");
    }
}

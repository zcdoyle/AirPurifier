/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：程序入口
**************************************************/

#include "server.h"
#include "configuration.h"
#include "daemonize.h"
#include <muduo/base/TimeZone.h>


//日志文件
boost::scoped_ptr<LogFile> g_logFile;


/*************************************************
Description:    flush函数
Input:          无
Output:         无
Return:         无
*************************************************/
void flushFunc()
{
    g_logFile->flush();
}


/*************************************************
Description:    添加记录函数
Input:          无
Output:         无
Return:         无
*************************************************/
void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
    //马上flush,测试用
    flushFunc();
}

/*************************************************
Description:    程序主入口
Input:          无
Output:         无
Return:         无
*************************************************/
int main(int argc, char* argv[])
{
    //读取配置
    Configuration config;
    
    //日志记录
    Logger::setLogLevel(Logger::DEBUG);
    Logger::setTimeZone(TimeZone(8 * 3600, "China"));
    if(config.isLogInFile_)
    {
        //创建日志文件并设置记录和flush函数
        g_logFile.reset(new muduo::LogFile(::basename("TCPServer"), 500000*1000)); //每个日志文件最大500MB
        Logger::setOutput(outputFunc);
        Logger::setFlush(flushFunc);
    }

    //后台运行
    if(config.isDaemonize_)
        daemonize();

    //开始事件循环
    EventLoop loop;
    TCPServer server(&loop, config);
    server.start();
    loop.loop();

    return 0;
}

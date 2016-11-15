#include "timerClient.h"
#include "configuration.h"
#include <muduo/base/TimeZone.h>
#include "daemonize.h"


//日志文件
boost::scoped_ptr<LogFile> g_logFile;

//flush函数
void flushFunc()
{
    g_logFile->flush();
}

//添加记录函数
void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
    //马上flush，测试用
    flushFunc();
}


void createTCPClient(Configuration *config, int processNo)
{
//    daemonize();
    EventLoop loop;
    InetAddress serverAddr(config->address, config->port);
//    定时发送消息,测试并发
    TimerClient client(&loop, serverAddr, processNo);
    client.connect();
    loop.loop();
}


int main()
{
    //读取配置文件
    Configuration config;
    readConfiguration(&config);

    Logger::setLogLevel(Logger::DEBUG);
    Logger::setTimeZone(TimeZone(8 * 3600, "China"));
    if(config.isLogInFile)
    {
        //创建日志文件并设置记录和flush函数
        g_logFile.reset(new muduo::LogFile(::basename("TCPClient"), 500000*1000)); //每个日志文件最大500MB
        Logger::setOutput(outputFunc);
        Logger::setFlush(flushFunc);
    }
    //多进程模拟并发
    if(config.processNum > 0)
    {
        for(int i = 0; i < config.processNum - 1; i++)
        {
            pid_t pid = fork();
            if(pid == 0)
            {
                //子进程
                LOG_INFO << "pid = " << getpid();
                createTCPClient(&config, i + 1);
                return 0;
            }
        }
        //父进程
        LOG_INFO << "pid = " << getpid();
        createTCPClient(&config, 0);
        return 0;
    }
}

/*************************************************
Copyright: SmartLight
Author: albert
Date: 2015-12-09
Description: json编码解码器，基于RapidJson实现
**************************************************/

#ifndef TCPSERVER_JSONCODEC_H
#define TCPSERVER_JSONCODEC_H

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using std::map;
using std::string;
using boost::weak_ptr;

class JsonCodec {
public:
    typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
                                  const rapidjson::Document &document)> StringMessageCallback;
    typedef enum ParseStatus
    {
        START,
        END,
    }ParseStatus;

    explicit JsonCodec(const StringMessageCallback &cb) : messageCallback_(cb) {}

    /***************************************************
    Description:    接收到TCP信息，基于状态机获取完整的json串
                    此函数作为json解析模块的wrapper,试图找出一个完整的json串，然后给由rapidjson解析。
                    此处不考虑字符串的错误情况，rapidjson处理异常。
    Input:          conn：TCP连接
                    buf：TCP连接的buffer
                    receiveTime：接收时间戳
    Output:         无
    Return:         无
    ***************************************************/
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp receiveTime)
    {
        TcpConnection* conPtr = get_pointer(conn);
        if (stackLevel_.find(conPtr) == stackLevel_.end())
            stackLevel_[conPtr] = 0;
        if (status_.find(conPtr) == status_.end())
            status_[conPtr] = END;

        while(buf->readableBytes() > 0)
        {
            char c = buf->readInt8();
            switch(c)
            {
                case '{':
                    if(stackLevel_[conPtr] == 0 && status_[conPtr] == END)
                    {
                        //开始json串
                        status_[conPtr] = START;
                    }
                    stackLevel_[conPtr]++;
                    receiveString_[conPtr] += c;
                    break;

                case '}':
                    --stackLevel_[conPtr];
                    receiveString_[conPtr] += c;
                    if(stackLevel_[conPtr] == 0 && status_[conPtr] == START)
                    {
                        //取出一个完整的json串,开始解析
                        rapidjson::Document document;
                        document.Parse(receiveString_[conPtr].c_str());
                        messageCallback_(conn, document);
                        receiveString_[conPtr]="";
                        status_[conPtr] = END;
                    }
                    break;

                default:
                    receiveString_[conPtr] += c;
                    break;
            }
        }
    }

    /***************************************************
    Description:    清理连接信息
    Input:          conn：TCP连接
                    buf：TCP连接的buffer
                    receiveTime：接收时间戳
    Output:         无
    Return:         无
    ***************************************************/
    void cleanup(const muduo::net::TcpConnectionPtr& conn)
    {
        TcpConnection* conPtr = get_pointer(conn);

        stackLevel_.erase(conPtr);
        status_.erase(conPtr);
        receiveString_.erase(conPtr);
    }

private:
    StringMessageCallback messageCallback_;

    map<TcpConnection*, unsigned int> stackLevel_;  //临时字符串，保存未完全接收的json串
    map<TcpConnection*, ParseStatus> status_;       //记录json串的DOM深度
    map<TcpConnection*, string> receiveString_;     //解析状态量
};


#endif //TCPSERVER_JSONCODEC_H

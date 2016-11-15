/*************************************************
Copyright: SmartLight
Author: albert
Date: 2016-02-15
Description：用于发送HTTP请求，目前只实现POST方法
**************************************************/

#ifndef HTTP_H
#define HTTP_H

#include <string>

using std::string;

class Http
{
public:
    static int post(string host,string port, string page, const string& data);
    static int post(string host,string port, string page, const string& data, string& reponse_data);
};


#endif

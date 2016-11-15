/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：生成各类帧
**************************************************/
#ifndef TCPSERVER_MESSAGECONSTRUCTOR_H
#define TCPSERVER_MESSAGECONSTRUCTOR_H

#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include "TCPCodec.h"

//构造发送帧
class MessageConstructor
{
public:
    static void confirmFrame(u_char* message, uint16_t frameCount);
    static void openControl(u_char* message,int opencontrol);
    static void modeControl(u_char* message,int modecontrol);
    static void timeControl(u_char* message,int timecontrol);
    static void childlockControl(u_char* message,int childlockcontrol);
    static void errorreminderControl(u_char* message,int erroremindercontrol);
    static void updateControl(u_char* message,int updatecontrol);
};


#endif //TCPSERVER_MESSAGECONSTRUCTOR_H

/*************************************************
Copyright: RemoteControl_AirPurifier
Author: zcdoyle
Date: 2016-06-13
Description：生成各类帧
**************************************************/

#include "MessageConstructor.h"


/*************************************************
Description:    确认回复帧
Input:          message：需要填入帧内容的空间
                frameCount：确认帧计数
Output:         确认信息字帧
Return:         无
*************************************************/
void MessageConstructor::confirmFrame(u_char* message, uint16_t frameCount)
{
    FrameMessage frameMessage;
    frameMessage.content.confirm.seq = frameCount;
    memcpy(message, &frameMessage, sizeof(frameMessage));
}

/*************************************************
Description:    设备开关控制信息帧
Input:          message：需要填入帧内容的空间
                opencontrol：开关
Output:         设备开关控制帧
Return:         无
*************************************************/
void MessageConstructor::openControl(u_char* message,int opencontrol)
{
    FrameMessage msg;
    msg.content.open.isopen = opencontrol;
    msg.content.open.res = 0;
    memcpy(message, &msg, sizeof(msg));
}

/*************************************************
Description:    设备模式控制信息帧
Input:          message：需要填入帧内容的空间
                opencontrol：模式
Output:         设备模式控制帧
Return:         无
*************************************************/
void MessageConstructor::modeControl(u_char* message,int modecontrol)
{
    FrameMessage msg;
    msg.content.mode.mode = modecontrol;

    memcpy(message, &msg, sizeof(msg));
}

void MessageConstructor::timeControl(u_char* message,int timecontrol)
{
    FrameMessage msg;
    msg.content.time.time = timecontrol;

    memcpy(message, &msg, sizeof(msg));
}

void MessageConstructor::childlockControl(u_char* message,int childlockcontrol)
{
    FrameMessage msg;
    msg.content.childlock.click = childlockcontrol;
    msg.content.childlock.res = 0;
    memcpy(message, &msg, sizeof(msg));
}

void MessageConstructor::errorreminderControl(u_char* message,int erroremindercontrol)
{
    FrameMessage msg;
    msg.content.errorreminder.ermd = erroremindercontrol;
    msg.content.errorreminder.res = 0;
    memcpy(message, &msg, sizeof(msg));
}

void MessageConstructor::updateControl(u_char* message,int updatecontrol)
{
    FrameMessage msg;
    msg.content.update.update = updatecontrol;

    memcpy(message, &msg, sizeof(msg));
}


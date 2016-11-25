#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <fstream>
#include <muduo/base/Logging.h>
#define LINELENGTH 512

using std::ifstream;

typedef struct configuration
{
    char address[LINELENGTH];
    uint16_t port;
    int processNum;
    bool isLogInFile;
}Configuration;


void readConfiguration(Configuration *config)
{
    ifstream configFile("/mnt/AirPurifier/TCPClient/build/bin/TCPClient.cfg", std::ios::in);
    char *position;
    char line[LINELENGTH];

    //逐行读入配置文件
    while(configFile.getline(line, sizeof(line)))
    {
        if((position = strstr(line, "#")) !=NULL)
        {
            continue;
        }
        else if((position = strstr(line,"address")) != NULL)
        {
            position += strlen("address") + 1;
            strcpy(config->address, position);
        }
        else if((position = strstr(line,"port")) != NULL)
        {
            position += strlen("port") + 1;
            config->port = static_cast<uint16_t>(atoi(position));
        }
        else if((position = strstr(line,"processNumber")) != NULL)
        {
            position += strlen("processNumber") + 1;
            config->processNum = static_cast<uint16_t>(atoi(position));
        }
        else if((position = strstr(line,"log_in_file")) != NULL)
        {
            position += strlen("log_in_file") + 1;
            if(strcmp(position, "y") == 0 || strcmp(position, "yes") == 0)
            {
                config->isLogInFile = true;
            }
            else
            {
                config->isLogInFile = false;
            }
        }
    }
}

#endif

//
//  PingTest.cpp
//  LearnCPlaus
//
//  Created by javalong on 2017/6/1.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>


class PingTest
{
public:
    int ping(int count, int interval/*S*/, int timeout/*S*/, const char *dest, unsigned int packetSize)
    {
        count    = count    <= 0 ? 2 : count;
        interval = interval <= 0 ? 1 : interval;
        timeout  = timeout  <= 0 ? 4 : timeout;
        
        if (NULL == dest || 0 == strlen(dest))
        {
            //        struct in_addr _addr;
        }
        
        char cmd[256] = {0};
        if (strlen(dest) > 200)
        {
            std::cout<<"Error: 域名太长了\n";
            return -1;
        }
        
        int index = snprintf(cmd, 256, "ping -c %d -i %d -t %d", count, interval, timeout);
        if (index < 0 || index >= 256)
        {
            std::cout<<"Error: cmd越界\n";
            return -1;
        }
        
        int tempLen = 0;
        if (packetSize > 0 )
        {
            tempLen = snprintf((char *)&cmd[index], 256 - index, "-s %u %s", packetSize, dest);
        } else
        {
            tempLen = snprintf((char *)&cmd[index], 256 - index, " %s", dest);
        }
        if (tempLen < 0 || tempLen >= 256)
        {
            std::cout<<"Error: cmd越界\n";
            return -1;
        }
        std::cout<<"cmd: " << cmd << "\n";
        
        FILE *pp = popen(cmd, "r");
        if (!pp)
        {
            std::cout<<"Error: 文件操作失败\n";
            return -1;
        }
        std::string pingresult;
        char line[512] = {0};
        while (fgets(line, sizeof(line), pp) != NULL)
        {
            pingresult.append(line, strlen(line));
            //        std::cout <<"ping的结果为：" << line << "\n";
        }
        pclose(pp);
        
        std::cout <<"ping的结果为：" << pingresult << "\n";
        
        return 1;
    }
    
    int testImp()
    {
        std::string host = "www.qq.com";
        return ping(0, 0, 0, host.c_str(), 0);
    }
};

//
//  SockTest.cpp
//  LearnCPlaus
//
//  Created by javalong on 2017/6/2.
//  Copyright © 2017年 javalong. All rights reserved.
//


/**
 头文件：#include <sys/types.h>   #include <sys/socket.h>
 
 定义函数：int sendto(int s, const void * msg, int len, unsigned int flags, const struct sockaddr * to, int tolen);
 
 函数说明：sendto() 用来将数据由指定的socket 传给对方主机. 参数s 为已建好连线的socket, 如果利用UDP协议则不需经过连线操作. 参数msg 指向欲连线的数据内容, 参数flags 一般设0, 详细描述请参考send(). 参数to 用来指定欲传送的网络地址, 结构sockaddr 请参考bind(). 参数tolen 为sockaddr 的结果长度.
 
 返回值：成功则返回实际传送出去的字符数, 失败返回－1, 错误原因存于errno 中.
 
 错误代码：
 1、EBADF 参数s 非法的socket 处理代码.
 2、EFAULT 参数中有一指针指向无法存取的内存空间.
 3、WNOTSOCK canshu s 为一文件描述词, 非socket.
 4、EINTR 被信号所中断.
 5、EAGAIN 此动作会令进程阻断, 但参数s 的soket 为补课阻断的.
 6、ENOBUFS 系统的缓冲内存不足.
 7、EINVAL 传给系统调用的参数不正确.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <netdb.h>

using namespace std;

class SockTest
{
private:
    void testGetaddrinfo()
    {
        struct addrinfo hints;
        struct addrinfo *res, *cur;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family   = AF_INET;
        hints.ai_flags    = AI_PASSIVE;
        hints.ai_protocol = 0;
        hints.ai_socktype = SOCK_STREAM;
        
        int ret = getaddrinfo("www.qq.com", NULL, &hints, &res);
        if (ret == -1)
        {
            perror("getaddrinfo");
            return;
        }
        
        for (cur = res; cur != NULL; cur = cur->ai_next)
        {
            struct sockaddr_in *addr = (struct sockaddr_in *)cur->ai_addr;
            cout << "IP:" << inet_ntoa((*addr).sin_addr) << "\n";
        }
    }
    
    void testSock()
    {
        int sockfd;
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            perror("socket");
            return;
        }
        
        struct sockaddr_in addr;
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(2345);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
        {
            perror("connect");
            return;
        }
        
        socklen_t addrLen = sizeof(addr);
        char buffer[256];
        ssize_t len;
        while (1)
        {
            cout << "\n请输入需要发送的消息，按回车键结束输入......" << "\n";
            
            bzero(buffer, sizeof(buffer));
            
            /*从标准输入设备取得字符串*/
            len = read(STDIN_FILENO, buffer, sizeof(buffer));
            
            /*将字符串传送给server端*/
            sendto(sockfd, buffer, len, 0, (struct sockaddr*)&addr, addrLen);
            
            bzero(buffer, sizeof(buffer));
            
            /*接收server端返回的字符串*/
            len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addrLen);
            
            //显示client 端的网络地址
            printf("receive from %s\n ", inet_ntoa(addr.sin_addr));
            cout << "接收到：" << buffer << " 长度为：" << len << "\n";
        }
    }
public:
    void testImp()
    {
        testGetaddrinfo();
        testSock();
    }
};

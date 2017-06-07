//
//  IcmpTest.cpp
//  LearnCPP
//
//  Created by javalong on 2017/6/2.
//  Copyright © 2017年 javalong. All rights reserved.
//
#include <stdio.h>
#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

using namespace std;

#define MAXBUFSIZE      4096

static int DATALEN = 56;        /* data that goes with ICMP echo request */
static const int IP_HEADER_LEN = 20;
static const int ICMP_HEADER_LEN = 8;

class IcmpTest
{
private:
    int nsent_;
    int sockfd_;
    struct sockaddr          sendaddr_;
    struct sockaddr          recvaddr_;
    
    static uint16_t in_cksum(uint16_t* _addr, int _len)
    {
        int             nleft = _len;
        uint32_t        sum = 0;
        uint16_t*        w = _addr;
        uint16_t        answer = 0;
        
        while (nleft > 1)  {
            sum += *w++;
            nleft -= 2;
        }
        
        /* 4mop up an odd byte, if necessary */
        if (nleft == 1) {
            *(unsigned char*)(&answer) = *(unsigned char*)w;
            sum += answer;
        }
        
        /* 4add back carry outs from top 16 bits to low 16 bits */
        sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
        sum += (sum >> 16);         /* add carry */
        answer = ~sum;              /* truncate to 16 bits */
        return (answer);
    }
    
    static char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
    {
        static char str[128];
        
        switch (sa->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)sa;
                if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
                {
                    return NULL;
                }
                return (str);
            }
            
#ifdef  IPV6
            
            case AF_INET6:
            {
                struct sockaddr_in6* sin6 = (struct sockaddr_in6*)sa;
                
                if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
                return (NULL);
                
                return (str);
            }
            
#endif
            
            
            
#ifdef  AF_UNIX
            
            case AF_UNIX:
            {
                struct sockaddr_un* unp = (struct sockaddr_un*)sa;
                
                /* OK to have no pathname bound to the socket: happens on
                 every connect() unless client calls bind() first. */
                if (unp->sun_path[0] == 0)
                strcpy(str, "(no pathname bound)");
                else
                snprintf(str, sizeof(str), "%s", unp->sun_path);
                
                return (str);
            }
            
#endif
            
            
            
#ifdef  HAVE_SOCKADDR_DL_STRUCT
            
            case AF_LINK: {
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)sa;
                
                if (sdl->sdl_nlen > 0)
                snprintf(str, sizeof(str), "%*s", sdl->sdl_nlen, &sdl->sdl_data[0]);
                else
                snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
                
                return (str);
            }
            
#endif
            
            default:
            snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d", sa->sa_family, salen);
            return (str);
        }
        
        return (NULL);
    }
    
    static char* Sock_ntop_host(const struct sockaddr* sa, socklen_t salen) {
        char* ptr;
        
        if ((ptr = sock_ntop_host(sa, salen)) == NULL)
        {
            printf("sock_ntop_host error,errno");
        }
        
        return (ptr);
    }
    
    static struct addrinfo *host_serv(const char *host, const char *serv, int family, int socktype)
    {
        int n;
        struct addrinfo hints, *res;
        bzero(&hints, sizeof(struct addrinfo));
        hints.ai_flags = AI_CANONNAME; /* always return canonical name */
        hints.ai_family = family; /* 0, AF_INET, AF_INET6, etc. */
        hints.ai_socktype = socktype; /* 0, SOCK_STREAM, SOCK_DGRAM, etc. */
        
        if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        {
            printf("host_serv error\n");
            return NULL;
        }
        return (res);
    }
    
    int prepareSendAddr(const char *dest)
    {
        struct addrinfo *ai;
        char *h;
        const char *host = dest;
        
        ai = host_serv(host, NULL, 0, 0);
        
        if (NULL == ai)
        {
            printf("prepareSendAddr error\n");
            return -1;
        }
        
        h = Sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
        
        printf("Sock_ntop_host:%s\n",h);
        
        if (ai->ai_family != AF_INET)
        {
            printf("error:ai->ai_family != AF_INET");
            return -1;
        }
        
        memcpy(&sendaddr_, ai->ai_addr, sizeof(struct sockaddr));
        
        freeaddrinfo(ai);
        
        return 0;
    }
    
    int begin(const char* dest)
    {
        int res = prepareSendAddr(dest);
        if (res != 0)
        {
            printf("prepareSendAddr error\n");
            return res;
        }
        
        int sockfd = socket(sendaddr_.sa_family, SOCK_DGRAM, IPPROTO_ICMP);
        sockfd_ = sockfd;
        if (sockfd < 0 )
        {
            printf("socket 创建 error\n");
            return -1;
        }
        
        int size = 60 * 1024;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
        
//        int _only = 0;
//        int ipv6onlyres = setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &_only, sizeof(_only));
//        if (ipv6onlyres != 0)
//        {
//            printf("set ipv6only failed\n");
//        }
        
//        static const int noblock = 1;
//        int nobres = ioctl(sockfd, FIONBIO, (u_long*)&noblock);
//        
//        if (nobres != 0)
//        {
//            printf("set nonblock socket error\n");
//            return -1;
//        }
        
//        int ret = fcntl(sockfd, F_GETFL, 0);
//        if(ret >= 0)
//        {
//            long flags = ret | O_NONBLOCK;
//            ret = fcntl(sockfd, F_SETFL, flags);
//        }
//        if (ret != 0)
//        {
//            printf("set nonblock socket error\n");
//            return -1;
//        }

        
        return 0;
    }
    
    void end(int sockfd)
    {
        if (sockfd >= 0)
        {
            ::close(sockfd);
        }
    }
    
    static void tv_sub(struct timeval* _out, struct timeval* _in)
    {
        if ((_out->tv_usec -= _in->tv_usec) < 0) {     /* out -= in */
            --_out->tv_sec;
            _out->tv_usec += 1000000;
        }
        
        _out->tv_sec -= _in->tv_sec;
    }

    void proc_v4(char* _ptr, ssize_t _len, struct msghdr* _msg, struct timeval* _tvrecv)
    {
        int     icmplen;
        double      rtt;
        struct icmp* icmp;
        struct timeval*  tvsend;
        icmp = (struct icmp*) _ptr;
        
        if ((icmplen = (int)_len - IP_HEADER_LEN) < ICMP_HEADER_LEN)
        {
            printf("receive malformed icmp packet");
            return;             /* malformed packet */
        }
        
        printf("icmp->icmp_type=%d,is equal with ICMP_ECHOREPLY:%d\n", icmp->icmp_type, icmp->icmp_type == ICMP_ECHOREPLY);
        
        if (icmplen < ICMP_HEADER_LEN + sizeof(struct timeval))
        {
            printf("not enough data to compute RTT");
            return;         /* not enough data to use */
        }
        
        tvsend = (struct timeval*)(&_ptr[ICMP_MINLEN]);
        printf("before ntohl tvsend sec=%ld, nsec=%d; tvrecv sec=%ld, usec=%d\n", tvsend->tv_sec
                , tvsend->tv_usec, _tvrecv->tv_sec, _tvrecv->tv_usec);
        
        tvsend->tv_sec = ntohl(tvsend->tv_sec);
        tvsend->tv_usec = ntohl(tvsend->tv_usec);
        
        printf("tvsend sec=%ld, nsec=%d; tvrecv sec=%ld, usec=%d\n", tvsend->tv_sec
                , tvsend->tv_usec, _tvrecv->tv_sec, _tvrecv->tv_usec);
        
        tv_sub(_tvrecv, tvsend);
        rtt = _tvrecv->tv_sec * 1000.0 + _tvrecv->tv_usec / 1000.0;
        
        if (rtt < 10000.0 && rtt > 0.0)
        {
//            vecrtts_.push_back(rtt);
        } else
        {
            printf("rtt = %f is illegal.receive %d bytes from %s", rtt, icmplen, Sock_ntop_host(&recvaddr_, sizeof(recvaddr_)));
        }
        
        char tempbuff[1024] = {0};
        snprintf(tempbuff, 1024, "%d bytes from %s: seq=%d,  rtt=%f ms\n", icmplen, Sock_ntop_host(&recvaddr_, sizeof(recvaddr_)),
                 ntohs(icmp->icmp_seq), rtt);
        printf("%s", (char*)tempbuff);
//        pingresult_.append(tempbuff);
    }
    
    void preparePacket(char* _sendbuffer, int& _len)
    {
        printf("===准备发送ICMP包===\n");
        char    sendbuf[MAXBUFSIZE];
        memset(sendbuf, 0, MAXBUFSIZE);
        struct icmp* icmp;
        icmp = (struct icmp*) sendbuf;
        icmp->icmp_type = ICMP_ECHO;      // 0字节的赋值，值为8
        icmp->icmp_code = 0;              // 1字节的赋值，值为0
        icmp->icmp_id = getpid() & 0xffff;/* ICMP ID field is 16 bits */ // 4字节到5字节的赋值
        icmp->icmp_seq = htons(nsent_++);                                // 6字节到7字节的赋值
        /**
         void *memset(void *s,int c,size_t n)
         总的作用：将已开辟内存空间 s 的首 n 个字节的值设为值 c。
         */
        memset(&sendbuf[ICMP_MINLEN], 0xa5, DATALEN);   /* fill with pattern */ // 8字节到63字节的赋值，值为0xa5
        
        struct timeval now;
        (void)gettimeofday(&now, NULL);
        now.tv_usec = htonl(now.tv_usec);
        now.tv_sec = htonl(now.tv_sec);
        bcopy((void*)&now, (void*)&sendbuf[ICMP_MINLEN], sizeof(now)); //sizeof(now)等于16，则8字节到23字节的赋值，值为now数据
        _len = ICMP_MINLEN + DATALEN;        /* checksum ICMP header and data */
        icmp->icmp_cksum = 0;
        icmp->icmp_cksum = in_cksum((u_short*) icmp, _len);// 2字节到3字节的赋值，值为把0字节到63字节的值加起来，在进行运算得来
        memcpy(_sendbuffer, sendbuf, _len);
    }
    
    ssize_t send()
    {
        char sendbuffer[MAXBUFSIZE];
        memset(sendbuffer, 0, MAXBUFSIZE);
        int len = 0;
        preparePacket(sendbuffer, len);
        
        ssize_t sendLen = sendto(sockfd_, sendbuffer, len, 0, &sendaddr_, sizeof(sendaddr_));
        
        printf("sockfd= %d sendLen==%ld\n", sockfd_, sendLen);

        return sendLen;
    }
    
    int recv()
    {
        char            recvbuf[MAXBUFSIZE];
        char            controlbuf[MAXBUFSIZE];
        memset(recvbuf, 0, MAXBUFSIZE);
        memset(controlbuf, 0, MAXBUFSIZE);
        
        struct msghdr   msg = {0};
        struct iovec    iov = {0};
        
        
        iov.iov_base       = recvbuf;
        iov.iov_len        = sizeof(recvbuf);
        msg.msg_name       = &recvaddr_;
        msg.msg_namelen    = sizeof(recvaddr_);
        msg.msg_iov        = &iov;
        msg.msg_iovlen     = 1;
        msg.msg_control    = controlbuf;
        msg.msg_controllen = sizeof(controlbuf);
        
        int n = (int)recvmsg(sockfd_, &msg, 0);

        printf("sockfd= %d after recvmsg() n =%d\n", sockfd_, (int)n);
        
        if (n < 0)
        {
            return -1;
        }
        
        struct timeval  tval;
        
        gettimeofday(&tval, NULL);
        
        printf("gettimeofday sec=%ld,usec=%d\n", tval.tv_sec, tval.tv_usec);
        
        proc_v4(recvbuf + IP_HEADER_LEN, n, &msg, &tval);
        
        return n;
    }

    
    int ping(int count, int interval/*S*/, int timeout/*S*/, const char *dest, unsigned int packetSize)
    {
        count    = count    <= 0 ? 2 : count;
        interval = interval <= 0 ? 1 : interval;
        timeout  = timeout  <= 0 ? 4 : timeout;
        
        int readcount = count;
        int sendcount = count;
        
        if (NULL == dest || 0 == strlen(dest))
        {
            //        struct in_addr _addr;
        }
        if (begin(dest) == -1)
        {
            end(sockfd_);
            return -1;
        }
       
        printf("===测试icmp开始===\n\n");
        
        while (readcount > 0)
        {
            printf("\n===第%d包===\n", (count - readcount + 1));
            send();
            sendcount--;

            recv();
            readcount --;
        }
        
        printf("\n\n===测试icmp完成===\n");
        
        return 0;
    }
    
    void testPreparePacket()
    {
        char sendbuffer[MAXBUFSIZE];
        memset(sendbuffer, 0, MAXBUFSIZE);
        int len = 0;
        preparePacket(sendbuffer, len);
        
        printf("前%d个字节为:\n", len);
        for(int i = 0; i < len; i++)
        {
            printf("%x、", sendbuffer[i]);
            if (i == 0 || i == 1 || i == 3 || i == 5 || i == 7  || i == 23 || i == 63)
            {
                printf("\n");
            }
        }
        printf("\n");
    }
    
    void testPrepareSendAddr()
    {
        string host = "www.qq.com";
        prepareSendAddr(host.c_str());
    }
    
    void testPing()
    {
        std::string host = "www.qq.com";//"www.marsopen.cn";//"127.0.0.1";//"www.baidu.com";//
        ping(0, 0, 0, host.c_str(), 0);
    }
    
public:
    void testImp()
    {
//        testPreparePacket();
//        testPrepareSendAddr();
        testPing();
    }
};

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

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include "socketselect.h"

using namespace std;

/*
 * Interface Control Message Protocol Definitions.
 * Per RFC 792, September 1981.
 */

/*
 * Internal of an ICMP Router Advertisement
 */
struct icmp_ra_addr {
    u_int32_t ira_addr;
    u_int32_t ira_preference;
};

/*
 * Structure of an icmp header.
 */
struct icmp {
    u_char    icmp_type;        /* type of message, see below */         // 1Byte
    u_char    icmp_code;        /* type sub code */                      // 1Byte
    u_short    icmp_cksum;        /* ones complement cksum of struct */  // 2Byte
    union {                                                              // 4Byte
        u_char ih_pptr;            /* ICMP_PARAMPROB */
        struct in_addr ih_gwaddr;    /* ICMP_REDIRECT */
        struct ih_idseq {
            n_short    icd_id;
            n_short    icd_seq;
        } ih_idseq;
        int ih_void;
        
        /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
        struct ih_pmtu {
            n_short ipm_void;
            n_short ipm_nextmtu;
        } ih_pmtu;
        
        struct ih_rtradv {
            u_char irt_num_addrs;
            u_char irt_wpa;
            u_int16_t irt_lifetime;
        } ih_rtradv;
    } icmp_hun;
#define    icmp_pptr    icmp_hun.ih_pptr
#define    icmp_gwaddr    icmp_hun.ih_gwaddr
#define    icmp_id        icmp_hun.ih_idseq.icd_id
#define    icmp_seq    icmp_hun.ih_idseq.icd_seq
#define    icmp_void    icmp_hun.ih_void
#define    icmp_pmvoid    icmp_hun.ih_pmtu.ipm_void
#define    icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu
#define    icmp_num_addrs    icmp_hun.ih_rtradv.irt_num_addrs
#define    icmp_wpa    icmp_hun.ih_rtradv.irt_wpa
#define    icmp_lifetime    icmp_hun.ih_rtradv.irt_lifetime
    union {                                                         //20Byte
        struct id_ts {
            n_time its_otime;
            n_time its_rtime;
            n_time its_ttime;
        } id_ts;
        struct id_ip  {
            struct ip idi_ip;
            /* options and then 64 bits of data */
        } id_ip;
        struct icmp_ra_addr id_radv;
        u_int32_t id_mask;
        char    id_data[1];
    } icmp_dun;
#define    icmp_otime    icmp_dun.id_ts.its_otime
#define    icmp_rtime    icmp_dun.id_ts.its_rtime
#define    icmp_ttime    icmp_dun.id_ts.its_ttime
#define    icmp_ip        icmp_dun.id_ip.idi_ip
#define    icmp_radv    icmp_dun.id_radv
#define    icmp_mask    icmp_dun.id_mask
#define    icmp_data    icmp_dun.id_data
};

/*
 * Lower bounds on packet lengths for various types.
 * For the error advice packets must first insure that the
 * packet is large enough to contain the returned ip header.
 * Only then can we do the check to see if 64 bits of packet
 * data have been returned, since we need to check the returned
 * ip header length.
 */
#define    ICMP_MINLEN    8                /* abs minimum */
#define    ICMP_TSLEN    (8 + 3 * sizeof (n_time))    /* timestamp */
#define    ICMP_MASKLEN    12                /* address mask */
#define    ICMP_ADVLENMIN    (8 + sizeof (struct ip) + 8)    /* min */
#ifndef _IP_VHL
#define    ICMP_ADVLEN(p)    (8 + ((p)->icmp_ip.ip_hl << 2) + 8)
/* N.B.: must separately check that ip_hl >= 5 */
#else
#define    ICMP_ADVLEN(p)    (8 + (IP_VHL_HL((p)->icmp_ip.ip_vhl) << 2) + 8)
/* N.B.: must separately check that header length >= 5 */
#endif

/*
 * Definition of type and code field values.
 */
#define    ICMP_ECHOREPLY        0        /* echo reply */
#define    ICMP_UNREACH        3        /* dest unreachable, codes: */
#define        ICMP_UNREACH_NET    0        /* bad net */
#define        ICMP_UNREACH_HOST    1        /* bad host */
#define        ICMP_UNREACH_PROTOCOL    2        /* bad protocol */
#define        ICMP_UNREACH_PORT    3        /* bad port */
#define        ICMP_UNREACH_NEEDFRAG    4        /* IP_DF caused drop */
#define        ICMP_UNREACH_SRCFAIL    5        /* src route failed */
#define        ICMP_UNREACH_NET_UNKNOWN 6        /* unknown net */
#define        ICMP_UNREACH_HOST_UNKNOWN 7        /* unknown host */
#define        ICMP_UNREACH_ISOLATED    8        /* src host isolated */
#define        ICMP_UNREACH_NET_PROHIB    9        /* prohibited access */
#define        ICMP_UNREACH_HOST_PROHIB 10        /* ditto */
#define        ICMP_UNREACH_TOSNET    11        /* bad tos for net */
#define        ICMP_UNREACH_TOSHOST    12        /* bad tos for host */
#define        ICMP_UNREACH_FILTER_PROHIB 13        /* admin prohib */
#define        ICMP_UNREACH_HOST_PRECEDENCE 14        /* host prec vio. */
#define        ICMP_UNREACH_PRECEDENCE_CUTOFF 15    /* prec cutoff */
#define    ICMP_SOURCEQUENCH    4        /* packet lost, slow down */
#define    ICMP_REDIRECT        5        /* shorter route, codes: */
#define        ICMP_REDIRECT_NET    0        /* for network */
#define        ICMP_REDIRECT_HOST    1        /* for host */
#define        ICMP_REDIRECT_TOSNET    2        /* for tos and net */
#define        ICMP_REDIRECT_TOSHOST    3        /* for tos and host */
#define    ICMP_ALTHOSTADDR    6        /* alternate host address */
#define    ICMP_ECHO        8        /* echo service */
#define    ICMP_ROUTERADVERT    9        /* router advertisement */
#define        ICMP_ROUTERADVERT_NORMAL     0  /* normal advertisement */
#define        ICMP_ROUTERADVERT_NOROUTE_COMMON 16 /* selective routing */
#define    ICMP_ROUTERSOLICIT    10        /* router solicitation */
#define    ICMP_TIMXCEED        11        /* time exceeded, code: */
#define        ICMP_TIMXCEED_INTRANS    0        /* ttl==0 in transit */
#define        ICMP_TIMXCEED_REASS    1        /* ttl==0 in reass */
#define    ICMP_PARAMPROB        12        /* ip header bad */
#define        ICMP_PARAMPROB_ERRATPTR 0        /* error at param ptr */
#define        ICMP_PARAMPROB_OPTABSENT 1        /* req. opt. absent */
#define        ICMP_PARAMPROB_LENGTH 2            /* bad length */
#define    ICMP_TSTAMP        13        /* timestamp request */
#define    ICMP_TSTAMPREPLY    14        /* timestamp reply */
#define    ICMP_IREQ        15        /* information request */
#define    ICMP_IREQREPLY        16        /* information reply */
#define    ICMP_MASKREQ        17        /* address mask request */
#define    ICMP_MASKREPLY        18        /* address mask reply */
#define    ICMP_TRACEROUTE        30        /* traceroute */
#define    ICMP_DATACONVERR    31        /* data conversion error */
#define    ICMP_MOBILE_REDIRECT    32        /* mobile host redirect */
#define    ICMP_IPV6_WHEREAREYOU    33        /* IPv6 where-are-you */
#define    ICMP_IPV6_IAMHERE    34        /* IPv6 i-am-here */
#define    ICMP_MOBILE_REGREQUEST    35        /* mobile registration req */
#define    ICMP_MOBILE_REGREPLY    36        /* mobile registration reply */
#define    ICMP_SKIP        39        /* SKIP */
#define    ICMP_PHOTURIS        40        /* Photuris */
#define        ICMP_PHOTURIS_UNKNOWN_INDEX    1    /* unknown sec index */
#define        ICMP_PHOTURIS_AUTH_FAILED    2    /* auth failed */
#define        ICMP_PHOTURIS_DECRYPT_FAILED    3    /* decrypt failed */

#define    ICMP_MAXTYPE        40

#define    ICMP_INFOTYPE(type) \
((type) == ICMP_ECHOREPLY || (type) == ICMP_ECHO || \
(type) == ICMP_ROUTERADVERT || (type) == ICMP_ROUTERSOLICIT || \
(type) == ICMP_TSTAMP || (type) == ICMP_TSTAMPREPLY || \
(type) == ICMP_IREQ || (type) == ICMP_IREQREPLY || \
(type) == ICMP_MASKREQ || (type) == ICMP_MASKREPLY)

//#endif /* _NETINET_IP_ICMP_H_ */

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
    SocketSelectBreaker     readwrite_breaker_;
    
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
        
        int _only = 0;
        int ipv6onlyres = setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &_only, sizeof(_only));
        if (ipv6onlyres != 0)
        {
            printf("set ipv6only failed\n");
        }
        
//        static const int noblock = 1;
//        int nobres = ioctl(sockfd, FIONBIO, (u_long*)&noblock);
//        
//        if (nobres != 0)
//        {
//            printf("set nonblock socket error\n");
//            return -1;
//        }
        
        int ret = fcntl(sockfd, F_GETFL, 0);
        if(ret >= 0)
        {
            long flags = ret | O_NONBLOCK;
            ret = fcntl(sockfd, F_SETFL, flags);
        }
        if (ret != 0)
        {
            printf("set nonblock socket error\n");
            return -1;
        }

        
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
       
        printf("===测试icmp开始===\n");
        
        while (readcount > 0)
        {
            send();
            sendcount--;

            recv();
            readcount --;
        }
        
        printf("===测试icmp完成===\n");
        
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
        std::string host = "www.marsopen.cn";//"127.0.0.1";//"www.qq.com";//
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

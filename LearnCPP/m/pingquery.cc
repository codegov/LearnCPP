// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * pingquery.cc
 *
 *  Created on: 2014年6月18日
 *      Author: wutianqiang
 */

#include "pingquery.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//#include "mars/comm/xlogger/xlogger.h"
//#include "mars/comm/network/getgateway.h"
//#include "mars/comm/socket/socketselect.h"
//#include "mars/comm/socket/socket_address.h"
//#include "mars/sdt/constants.h"

//#include "netchecker_trafficmonitor.h"

using namespace mars::sdt;

#define TRAFFIC_LIMIT_RET_CODE (INT_MIN)

static void clearPingStatus(struct PingStatus& _ping_status) {
    _ping_status.res.clear();
    _ping_status.loss_rate = 0.0;
    _ping_status.maxrtt = 0.0;
    _ping_status.minrtt = 0.0;
    _ping_status.avgrtt = 0.0;
    memset(_ping_status.ip, 0, 16);
}

// APPLE
#include    <netinet/ip.h>
#include    <sys/time.h>
#include    <sys/un.h>
#include    <arpa/inet.h>
#include    <signal.h>
#include    <netinet/in_systm.h>
#include    <netinet/ip.h>
#include    <sys/types.h>
#include    <time.h>
#include    <sys/socket.h>
#include    <netdb.h>

#include <sys/ioctl.h>

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include    "mars/comm/objc/ip_icmp.h"
#else
#include    <netinet/ip_icmp.h>
#endif

//#include "mars/comm/time_utils.h"  // comm/utils.h
#define MAXBUFSIZE      4096

static int DATALEN = 56;        /* data that goes with ICMP echo request */
static const int IP_HEADER_LEN = 20;
static const int ICMP_HEADER_LEN = 8;

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

    if ((ptr = sock_ntop_host(sa, salen)) == NULL) {
//        xerror2(TSF"sock_ntop_host error,errno=%0", errno); /* inet_ntop() sets errno */
    }

    return (ptr);
}
static void Gettimeofday(struct timeval* tv, void* foo) {
    if (gettimeofday(tv, (struct timezone*)foo) == -1) {
//        xerror2(TSF"gettimeofday error");
    }

    return;
}
static int Sendto(int fd, const void* ptr, size_t nbytes, int flags, const struct sockaddr* sa, socklen_t salen) {
//    xdebug_function();
    int len = 0;

    if ((len = (int)sendto(fd, ptr, nbytes, flags, sa, salen)) != (ssize_t) nbytes) {
//        xerror2(TSF"sendto: uncomplete packet");
    }

    return len;
}


static struct addrinfo* Host_serv(const char* host, const char* serv, int family, int socktype) {
    int n;
    struct addrinfo hints, *res;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME;  /* always return canonical name */
    hints.ai_family = family;   /* 0, AF_INET, AF_INET6, etc. */
    hints.ai_socktype = socktype;   /* 0, SOCK_STREAM, SOCK_DGRAM, etc. */

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
//        xerror2(TSF"host_serv error for %0, %1: %2",
//                ((host == NULL) ? "(no hostname)" : host),
//                ((serv == NULL) ? "(no service name)" : serv),
//                gai_strerror(n));
        return NULL;
    }

    return (res);       /* return pointer to first on linked list */
}


static int Socket(int family, int type, int protocol) {
    int n;

    if ((n = socket(family, type, protocol)) < 0) {
//        xerror2(TSF"socket error");
    }

    return (n);
}


static uint16_t in_cksum(uint16_t* _addr, int _len) {
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

static void tv_sub(struct timeval* _out, struct timeval* _in) {
    if ((_out->tv_usec -= _in->tv_usec) < 0) {     /* out -= in */
        --_out->tv_sec;
        _out->tv_usec += 1000000;
    }

    _out->tv_sec -= _in->tv_sec;
}

void PingQuery::proc_v4(char* _ptr, ssize_t _len, struct msghdr* _msg, struct timeval* _tvrecv) {
    int     icmplen;
    double      rtt;
    struct icmp* icmp;
    struct timeval*  tvsend;
    icmp = (struct icmp*) _ptr;

    if ((icmplen = (int)_len - IP_HEADER_LEN) < ICMP_HEADER_LEN) {
        printf("receive malformed icmp packet");
        return;             /* malformed packet */
    }

    // if (icmp->icmp_type == ICMP_ECHOREPLY)
    //  {
    printf("icmp->icmp_type=%d,is equal with ICMP_ECHOREPLY:%d\n", icmp->icmp_type, icmp->icmp_type == ICMP_ECHOREPLY);

    if (icmplen < ICMP_HEADER_LEN + sizeof(struct timeval)) {
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

    if (rtt < 10000.0 && rtt > 0.0) {
        vecrtts_.push_back(rtt);
    } else {
        printf("rtt = %f is illegal.receive %d bytes from %s\n", rtt, icmplen, Sock_ntop_host(&recvaddr_, sizeof(recvaddr_)));
    }

    char tempbuff[1024] = {0};
    snprintf(tempbuff, 1024, "%d bytes from %s: seq=%d,  rtt=%f ms\n",
             icmplen, Sock_ntop_host(&recvaddr_, sizeof(recvaddr_)),
             ntohs(icmp->icmp_seq), rtt);
    printf("%s", (char*)tempbuff);
    pingresult_.append(tempbuff);
    //   }
}


int PingQuery::__prepareSendAddr(const char* _dest) {
    struct addrinfo* ai;
    char* h;
    const char* host = _dest;

    ai = Host_serv(host, NULL, 0, 0);

    if (NULL == ai) return -1;

    h = Sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
    printf("PING %s (%s): %d data bytes\n", (ai->ai_canonname ? ai->ai_canonname : h), h, DATALEN);

    if (ai->ai_family != AF_INET) {
        printf("unknown address family %d\n", ai->ai_family);
        return -1;
    }

    memcpy(&sendaddr_, ai->ai_addr, sizeof(struct sockaddr));
//    printf("m_sendAddr=%s", socket_address(&sendaddr_).ip());
    freeaddrinfo(ai);  // 閲婃斁addrinfo鍐呴儴瀛楁malloc鐨勫唴瀛橈紙鐢眊etaddrinfo鍑芥暟鍐呴儴浜х敓锛�
    return 0;
}

int PingQuery::__initialize(const char* _dest) {
    if (-1 == __prepareSendAddr(_dest)) return -1;;

    sockfd_ = Socket(sendaddr_.sa_family, SOCK_DGRAM/*SOCK_RAW*/, IPPROTO_ICMP);

    if (sockfd_ < 0) return -1;

    int size = 60 * 1024;       /* OK if setsockopt fails */
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    // make nonblock socket
    
//    int _only = 0;
//    int ipv6onlyres = setsockopt(sockfd_, IPPROTO_IPV6, IPV6_V6ONLY, &_only, sizeof(_only));
//    if (ipv6onlyres != 0)
//    {
//        printf("set ipv6only failed\n");
//    }
    
    static const int noblock = 1;
    int nobres = ioctl(sockfd_, FIONBIO, (u_long*)&noblock);
    if (nobres != 0)
    {
        printf("set nonblock socket error\n");
        return -1;
    }
    
    return 0;
}
void PingQuery::__deinitialize() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
    }
}
int PingQuery::__recv() {
    char            recvbuf[MAXBUFSIZE];
    char            controlbuf[MAXBUFSIZE];
    memset(recvbuf, 0, MAXBUFSIZE);
    memset(controlbuf, 0, MAXBUFSIZE);

    struct msghdr   msg = {0};
    struct iovec    iov = {0};


    iov.iov_base = recvbuf;
    iov.iov_len = sizeof(recvbuf);
    msg.msg_name = &recvaddr_;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = controlbuf;

    msg.msg_namelen = sizeof(recvaddr_);
    msg.msg_controllen = sizeof(controlbuf);

    int n = (int)recvmsg(sockfd_, &msg, 0);

//    if (NULL != traffic_monitor_) {
//        if (traffic_monitor_->recvLimitCheck(n)) {
//            xwarn2(TSF"limitCheck,recv Size=%0", n);
//            return TRAFFIC_LIMIT_RET_CODE;
//        }
//    }

    printf("after recvmsg() n =%d\n", (int)n);

    if (n < 0) {
        return -1;
    }

    struct timeval  tval;

    Gettimeofday(&tval, NULL);

    printf("gettimeofday sec=%ld,usec=%d\n", tval.tv_sec, tval.tv_usec);

    proc_v4(recvbuf + IP_HEADER_LEN, n, &msg, &tval);  // 杩欎釜闀垮害n锛屽寘鍚�20涓瓧鑺傜殑ip澶�

    return n;
}

int PingQuery::__send()
{
    char sendbuffer[MAXBUFSIZE];
    memset(sendbuffer, 0, MAXBUFSIZE);
    int len = 0;
    __preparePacket(sendbuffer, len);

//    if (NULL != traffic_monitor_) {
//        if (traffic_monitor_->sendLimitCheck(len)) {
//            xwarn2(TSF"limitCheck!!len=%0", len);
//            return TRAFFIC_LIMIT_RET_CODE;
//        }
//    }

    int sendLen = Sendto(sockfd_, sendbuffer, len, 0, &sendaddr_, sizeof(sendaddr_));
    sendtimes_++;

    return sendLen;
}

void PingQuery::__preparePacket(char* _sendbuffer, int& _len)
{
    printf("准备发送包\n");
    char    sendbuf[MAXBUFSIZE];
    memset(sendbuf, 0, MAXBUFSIZE);
    struct icmp* icmp;
    icmp = (struct icmp*) sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = getpid() & 0xffff;/* ICMP ID field is 16 bits */
    icmp->icmp_seq = htons(nsent_++);
    memset(&sendbuf[ICMP_MINLEN], 0xa5, DATALEN);   /* fill with pattern */

    struct timeval now;
    (void)gettimeofday(&now, NULL);
    printf("gettimeofday now sec=%ld, nsec=%d\n", now.tv_sec, now.tv_usec);
    now.tv_usec = htonl(now.tv_usec);
    now.tv_sec = htonl(now.tv_sec);
    bcopy((void*)&now, (void*)&sendbuf[ICMP_MINLEN], sizeof(now));
    _len = ICMP_MINLEN + DATALEN;        /* checksum ICMP header and data */
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((u_short*) icmp, _len);
    memcpy(_sendbuffer, sendbuf, _len);
}

void PingQuery::__onAlarm()
{
    readwrite_breaker_.Break();
}

uint64_t PingQuery::gettickcount()
{
    static mach_timebase_info_data_t timebase_info = {0};
    
    // Convert to nanoseconds - if this is the first time we've run, get the timebase.
    if (timebase_info.denom == 0 )
    {
        (void) mach_timebase_info(&timebase_info);
    }
    
    // Convert the mach time to milliseconds
    uint64_t mach_time = mach_absolute_time();
    uint64_t millis = (mach_time * timebase_info.numer) / (timebase_info.denom * 1000000);
    return millis;
}

// return :-1 no send all packets
//         0 send all packets
int PingQuery::__runReadWrite(int& _errcode)
{
    unsigned long timeout_point = timeout_ * 1000 + gettickcount();
    unsigned long send_next = 0;

    while (readcount_ > 0)
    {
        bool    should_send = false;

        if (send_next <= gettickcount() && sendcount_ > 0)
        {
            send_next = gettickcount() + interval_ * 1000;
//            alarm_.Cancel();
//            alarm_.Start(interval_ * 1000);  // m_interval*1000 convert m_interval from s to ms
            should_send = true;
        }

        SocketSelect sel(readwrite_breaker_, true);
        sel.PreSelect();
        sel.Read_FD_SET(sockfd_);
        sel.Exception_FD_SET(sockfd_);

        if (sendcount_ > 0) sel.Write_FD_SET(sockfd_);

        long timeoutMs = timeout_point - gettickcount();

        if (timeoutMs < 0) {
            return -1;  //设置的超时时间内没有收完所有的包
        }

        printf("阻塞：%ld", timeoutMs);
        int retsel = sel.Select((int)timeoutMs);
        if (retsel < 0)
        {
            printf("retSel<0");
            _errcode = sel.Errno();
            return -1;
        }
        printf("完成、");

        if (sel.IsException()) {
//            printf("socketselect exception");
////            _errcode = socket_error(sockfd_);
            return -1;
        }

        if (sel.Exception_FD_ISSET(sockfd_)) {
//            _errcode = socket_error(sockfd_);

            return -1;
        }

        if (sel.Write_FD_ISSET(sockfd_) && should_send)
        {
            printf("sockfd=%d 发送%d包......\n", sockfd_, sendcount_);
            int sendLen = __send();

            if (TRAFFIC_LIMIT_RET_CODE == sendLen)
            {
                printf("发送%d包失败\n", sendcount_);
                return TRAFFIC_LIMIT_RET_CODE;
            }
            if (sendLen < 0)
            {
                printf("发送%d包失败\n", sendcount_);
//                _errcode = socket_error(sockfd_);
            }
            printf("发送%d包完成\n", readcount_);
            sendcount_--;
        }

        if (sel.Read_FD_ISSET(sockfd_) && readcount_ > 0)
        {
            printf("接收%d包......\n", readcount_);
            if (TRAFFIC_LIMIT_RET_CODE == __recv())
            {
                printf("接收%d包失败\n", readcount_);
                readcount_--;
                return TRAFFIC_LIMIT_RET_CODE;
            }
            printf("接收%d包完成\n\n", readcount_);
            readcount_--;
        }
    }

    return 0;
}

int PingQuery::RunPingQuery(int _querycount, int _interval/*S*/, int _timeout/*S*/, const char* _dest, unsigned int _packet_size) {
//    xassert2(_querycount >= 0);
//    printf("dest=%s", _dest);

    if (_querycount <= 0)
        _querycount = 4;

    if (_interval <= 0)
        _interval = 1;

    if (_timeout <= 0)
        _timeout = 5;

    if (_packet_size >= ICMP_MINLEN && _packet_size <= MAXBUFSIZE/*4096*/) {  // packetSize is the length of ICMP packet, include ICMP header,but not include IP header
        DATALEN = _packet_size - ICMP_MINLEN;
    }

    if (NULL == _dest || 0 == strlen(_dest)) {
//        struct  in_addr _addr;
//        int ret = getdefaultgateway(&_addr);
//
//        if (-1 == ret) {
//            xerror2(TSF"get default gateway error.");
//            return -1;
//        }
//
//        _dest = inet_ntoa(_addr);
//
//        if (NULL == _dest || 0 == strlen(_dest)) {
//            xerror2(TSF"ping dest host is NULL.");
//            return -1;
//        }
//
//        xinfo2(TSF"get default gateway: %0", _dest);
    }

    sendcount_ = _querycount;
    readcount_ = _querycount;
    interval_ = _interval;
    timeout_  = _timeout;

    if (-1 == __initialize(_dest)) {
        __deinitialize();
        return -1;
    }

    int errcode;
    int ret = __runReadWrite(errcode);
    __deinitialize();

    //  if(0 >= m_vecRTTs.size())  return -1;
    return ret;
}

int PingQuery::GetPingStatus(struct PingStatus& _ping_status) {
//    clearPingStatus(_ping_status);
//    int size = (int)vecrtts_.size();
//    const char* pingIP = socket_address(&sendaddr_).ip();
//    printf("pingIP=%s", pingIP);
//
//    if (pingIP != NULL) {
//        strncpy(_ping_status.ip, pingIP, 16);
//    } else {
//        xerror2(TSF"pingIP==NULL");
//    }
//
//    xinfo2(TSF"getPingStatus():size = %0; m_readCount=%1,m_sendTimes=%2", size, readcount_, sendtimes_);
//
//    _ping_status.loss_rate = (1 - (double)size / sendtimes_) < 0 ? 0 : (1 - (double)size / sendtimes_);
//    char temp[1024] = {0};
//    snprintf(temp, 1024, "\n%d packets transmitted,%d packets received,lossRate=%f%%.\n ", sendtimes_, size, _ping_status.loss_rate * 100.0);
//    pingresult_.append(std::string(temp));
//
//    if (size > 0) {
//        _ping_status.minrtt = vecrtts_.at(0);
//        _ping_status.maxrtt = vecrtts_.at(0);
//        std::vector<double>::iterator iter = vecrtts_.begin();
//        double sum = 0.0;
//
//        for (; iter != vecrtts_.end(); ++iter) {
//            if (_ping_status.minrtt > *iter)
//                _ping_status.minrtt = *iter;
//
//            if (_ping_status.maxrtt < *iter)
//                _ping_status.maxrtt = *iter;
//
//            sum += *iter;
//        }
//
//        _ping_status.avgrtt = sum / size;
//    } else {
//        _ping_status.res = pingresult_;
//        return -1;
//    }
//
//    memset(temp, 0, 1024);
//    snprintf(temp, 1024, " MaxRTT=%f ms, MinRTT=%f ms, AverageRTT=%f ms", _ping_status.maxrtt, _ping_status.minrtt, _ping_status.avgrtt);
//    pingresult_.append(std::string(temp));
//    _ping_status.res = pingresult_;
    return 0;
}


int doPing(const std::string& _destaddr, std::string& _real_pingip, std::string& _resultstr, unsigned int _packet_size, int _pingcount, int _interval/*s*/, int _timeout/*s*/) {
    PingQuery pingObj;
    int ret = pingObj.RunPingQuery(_pingcount, _interval, _timeout, _destaddr.c_str(), _packet_size);

    if (ret != 0) {
        printf("ret=%d", ret);
        return ret;
    }

    return ret;
//    struct PingStatus ping_status;
//
//    int ret2 = pingObj.GetPingStatus(ping_status);
//
//
//    _real_pingip.clear();
//
//    _real_pingip.append(ping_status.ip);
//
//    _resultstr.clear();
//
//    _resultstr.append(ping_status.res);
//
//    printf("realPingIP=%s,resultStr=%s,destAddr=%s", _real_pingip.c_str(), _resultstr.c_str(), _destaddr.c_str());
//
//    return ret2;
}



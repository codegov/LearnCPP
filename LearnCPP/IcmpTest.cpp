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
#include <unistd.h>
#include <sys/time.h>
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
    
    void preparePacket(char* _sendbuffer, int& _len)
    {
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
    
public:
    void testImp()
    {
        char sendbuffer[MAXBUFSIZE];
        memset(sendbuffer, 0, MAXBUFSIZE);
        int len = 0;
        preparePacket(sendbuffer, len);
    }
};

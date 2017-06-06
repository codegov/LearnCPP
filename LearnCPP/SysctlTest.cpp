//
//  SysctlTest.cpp
//  LearnCPP
//
//  Created by javalong on 2017/6/5.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/route.h>
#include <netinet/in.h>

class SysctlTest
{
#define ROUNDUP(a) ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define SUCCESS (0)
#define FAILED  (-1)
    
public:
    int getdefaultgateway(struct in_addr * addr)
    {
#if 0
        /* net.route.0.inet.dump.0.0 ? */
        int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_DUMP, 0, 0/*tableid*/};
#endif
        /* net.route.0.inet.flags.gateway */
        int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_FLAGS, RTF_GATEWAY};
        size_t l;
        char * buf, * p;
        struct rt_msghdr * rt;
        struct sockaddr * sa;
        struct sockaddr * sa_tab[RTAX_MAX];
        int i;
        int r = FAILED;
        if(sysctl(mib, sizeof(mib)/sizeof(int), 0, &l, 0, 0) < 0)
        {
            return FAILED;
        }
        if(l>0)
        {
            buf = (char *)malloc(l);
            if(sysctl(mib, sizeof(mib)/sizeof(int), buf, &l, 0, 0) < 0)
            {
                free(buf);
                return FAILED;
            }
            for(p=buf; p<buf+l; p+=rt->rtm_msglen)
            {
                rt = (struct rt_msghdr *)p;
                sa = (struct sockaddr *)(rt + 1);
                for(i=0; i<RTAX_MAX; i++)
                {
                    if(rt->rtm_addrs & (1 << i))
                    {
                        sa_tab[i] = sa;
                        sa = (struct sockaddr *)((char *)sa + ROUNDUP(sa->sa_len));
                    } else
                    {
                        sa_tab[i] = NULL;
                    }
                }
                if( ((rt->rtm_addrs & (RTA_DST|RTA_GATEWAY)) == (RTA_DST|RTA_GATEWAY)) && sa_tab[RTAX_DST]->sa_family == AF_INET)
                {
                    //              && sa_tab[RTAX_GATEWAY]->sa_family == AF_INET) {
                    if(((struct sockaddr_in *)sa_tab[RTAX_DST])->sin_addr.s_addr == 0)
                    {
                        *addr = ((struct sockaddr_in *)(sa_tab[RTAX_GATEWAY]))->sin_addr;
                        r = SUCCESS;
                        break;
                    }
                }
            }//for
            free(buf);
        }//if(l>0)
        return r;
    }
    
    int getdefaultgateway6(struct in6_addr * addr)
    {
        /* net.route.0.inet6.flags.gateway */
        int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET6, NET_RT_FLAGS, RTF_GATEWAY};
        size_t l;
        char * buf, * p;
        struct rt_msghdr * rt;
        struct sockaddr * sa;
        struct sockaddr * sa_tab[RTAX_MAX];
        int i;
        int r = FAILED;
        if(sysctl(mib, sizeof(mib)/sizeof(int), 0, &l, 0, 0) < 0)
        {
            return FAILED;
        }
        if(l>0)
        {
            buf = (char * )malloc(l);
            if(sysctl(mib, sizeof(mib)/sizeof(int), buf, &l, 0, 0) < 0)
            {
                free(buf);
                return FAILED;
            }
            for(p=buf; p<buf+l; p+=rt->rtm_msglen)
            {
                rt = (struct rt_msghdr *)p;
                sa = (struct sockaddr *)(rt + 1);
                for(i=0; i<RTAX_MAX; i++)
                {
                    if(rt->rtm_addrs & (1 << i))
                    {
                        sa_tab[i] = sa;
                        sa = (struct sockaddr *)((char *)sa + ROUNDUP(sa->sa_len));
                    } else
                    {
                        sa_tab[i] = NULL;
                    }
                }
                if( ((rt->rtm_addrs & (RTA_DST|RTA_GATEWAY)) == (RTA_DST|RTA_GATEWAY)) && sa_tab[RTAX_DST]->sa_family == AF_INET6)
                {
                    //               && sa_tab[RTAX_GATEWAY]->sa_family == AF_INET6) {
                    if(IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6 *)sa_tab[RTAX_DST])->sin6_addr))
                    {
                        *addr = ((struct sockaddr_in6 *)(sa_tab[RTAX_GATEWAY]))->sin6_addr;
                        r = SUCCESS;
                        break;
                    }
                }
            }//for
            free(buf);
        }//if(l>0)
        return r;
    }
    
    void testImp()
    {
        struct  in_addr _addr;
        int ret = getdefaultgateway(&_addr);
        
        if (-1 == ret)
        {
            printf("get default gateway error.");
        }
    }
};

//
//  ConditionTest.cpp
//  LearnCPP
//
//  Created by javalong on 2017/6/16.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include "ConditionTest.hpp"
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>
#include <iostream>

static pthread_t       t_thread;
static pthread_cond_t  t_cond;
static pthread_mutex_t t_mutes;
static unsigned char   t_flag = 1;

void* ConditionTest::thr_fn(void *arg)
{
    struct timeval  now;
    struct timespec outtime;
    pthread_mutex_lock(&t_mutes);
    while (t_flag)
    {
        printf("thread sleep 2s\n");
        gettimeofday(&now, NULL);
        outtime.tv_sec  = now.tv_sec + 2;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&t_cond, &t_mutes, &outtime);
    }
    pthread_mutex_unlock(&t_mutes);
    printf("thread exit\n");
    return 0;
}

void ConditionTest::testCond()
{
    char c;
    pthread_mutex_init(&t_mutes, NULL);
    pthread_cond_init(&t_cond, NULL);
    if (0 != pthread_create(&t_thread, NULL, thr_fn, NULL))
    {
        printf("error when create pthread, %d\n", errno);
        return;
    }
    
    while ((c = getchar()) != 'q');
    printf("Now terminate the thread!\n");
    t_flag = 0;
    pthread_mutex_lock(&t_mutes);
    pthread_cond_signal(&t_cond);
    pthread_mutex_unlock(&t_mutes);
    printf("Wait for thread to exit\n");
//    pthread_join(t_thread, NULL);
    printf("Bye\n");
}

void ConditionTest::testImp()
{
    testCond();
}

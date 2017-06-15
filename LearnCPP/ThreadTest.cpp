//
//  ThreadTest.cpp
//  LearnCPP
//
//  Created by javalong on 2017/6/13.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>

using namespace std;

static pthread_t       t_thread;
static pthread_cond_t  t_cond;
static pthread_mutex_t t_mutes;
static unsigned char   t_flag = 1;

#define            LOOPCONSTANT     1000
#define            THREADS          3

//static pthread_mutex_t    t_mutex;// = PTHREAD_MUTEX_INITIALIZER;
static int                t_i,t_j,t_k,t_l;

static pthread_mutex_t    t_mutex = PTHREAD_MUTEX_INITIALIZER;

class ThreadTest
{
public:
    static void *thr_fn(void *arg)
    {
        struct timeval  now;
        struct timespec outtime;
        pthread_mutex_lock(&t_mutes);
        while (t_flag)
        {
            printf("thread sleep now\n");
            gettimeofday(&now, NULL);
            outtime.tv_sec  = now.tv_sec + 2;
            outtime.tv_nsec = now.tv_usec * 1000;
            int i = pthread_cond_timedwait(&t_cond, &t_mutes, &outtime);
            printf("i====%d\n", i);
            if (i == 0) break;
        }
        pthread_mutex_unlock(&t_mutes);
        printf("thread exit\n");
        return 0;
    }
    
    void testCond()
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
        //        pthread_join(t_thread, NULL);
        printf("Bye\n");
    }
    
    static void checkResults(const char *string, int rc)
    {
        if (rc) printf("Error on : %s, rc=%d", string, rc);
    }
    static void *threadfunc(void *parm)
    {
        int   rc;
        printf("Entered thread %s\n", parm);
        for (int loop = 0; loop < LOOPCONSTANT; ++loop)
        {
            rc = pthread_mutex_lock(&t_mutex);
            checkResults("pthread_mutex_lock()\n", rc);
            /* Perform some not so important processing */
            t_i++, t_j++, t_k++, t_l++;
            rc = pthread_mutex_unlock(&t_mutex);
            checkResults("pthread_mutex_unlock()\n", rc);
            /* This work is not too important. Also, we just released a lock       
             and would like to ensure that other threads get a chance in       
             a more co-operative manner. This is an admittedly contrived       
             example with no real purpose for doing the sched_yield().       
             */
            sched_yield();
        }
        printf("Finished thread\n");
        return NULL;
    }
    
    void testSched_yield()
    {
        pthread_attr_t attr_;
        string str = "1231231231";
        int res = pthread_attr_init(&attr_);
        res = pthread_attr_setstack(&attr_, &str, str.length());
        pthread_attr_destroy(&attr_);
        
        pthread_t threadid[THREADS];
        int       rc   = 0;
        int       loop = 0;
        rc = pthread_mutex_lock(&t_mutex);
        checkResults("pthread_mutex_lock()\n", rc);
        printf("Creating %d threads\n", THREADS);
        for (loop = 0; loop < THREADS; ++loop)
        {
            string str = to_string(loop + 1);
            rc = pthread_create(&threadid[loop], NULL, threadfunc, &str);
            checkResults("pthread_create()\n", rc);
        }
        sleep(1);
        rc = pthread_mutex_unlock(&t_mutex);
        checkResults("pthread_mutex_unlock()\n", rc);
        
        printf("Wait for results\n");
        for (loop = 0; loop < THREADS; ++loop)
        {
            rc = pthread_join(threadid[loop], NULL);
            checkResults("pthread_join()\n", rc);
        }
        pthread_mutex_destroy(&t_mutex);
        printf("Main completed\n");
    }
    
    void testImp()
    {
//        testCond();
        testSched_yield();
    }
};

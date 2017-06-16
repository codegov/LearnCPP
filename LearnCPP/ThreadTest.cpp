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

#define            LOOPCONSTANT     1000
#define            THREADS          3

static int                t_i,t_j,t_k,t_l;
static pthread_mutex_t    t_mutex = PTHREAD_MUTEX_INITIALIZER;

class ThreadTest
{
public:
    static void checkResults(const char *string, int rc)
    {
        if (rc) printf("Error on : %s, rc=%d", string, rc);
    }
    static void *threadfunc(void *parm)
    {
        int   rc;
        uint64_t id = (uint64_t)pthread_self();
        //sizeof(pthread_t) 是指针大小
        printf("Entered thread %s %llu sizeof:%lu %lu \n", parm, id, sizeof(pthread_t), sizeof(uint64_t));
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
        printf("Finished thread %llu\n", id);
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
        testSched_yield();
    }
};

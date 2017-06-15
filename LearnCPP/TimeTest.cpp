//
//  TimeTest.cpp
//  LearnCPlaus
//
//  Created by javalong on 2017/6/2.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <mach/mach_time.h>

using namespace std;

class TimeTest
{
public:
    
    void normalUse()
    {
        //使用该方法就可以检测出调用delay()函数所使用的时间
        struct timeval start;
        struct timeval end;
        unsigned long diff;
        gettimeofday(&start, NULL);
        cout << "开始执行函数delay()"<< "\n";
        delay();
        cout << "结束执行函数delay()"<< "\n";
        gettimeofday(&end, NULL);
        
        diff = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        
        cout << "执行函数delay()花了" << diff << "微秒\n";
    }
    
    void delay()
    {
        sleep(1);
    }
    
    void showCurTime(long time)
    {
        char time_string[30];
        struct tm *ptm;
        ptm = localtime(&time);
        strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);
        printf("当前时间：%s\n", time_string);
    }
    
    void testMach()
    {
        static mach_timebase_info_data_t timebase_info = {0};
        
        // Convert to nanoseconds（微秒） - if this is the first time we've run, get the timebase.
        if (timebase_info.denom == 0)
        {
            (void)mach_timebase_info(&timebase_info);
        }
        
        // Convert the mach time to milliseconds
        uint64_t mach_time = mach_absolute_time();
        uint64_t millis = (mach_time * timebase_info.numer) / (timebase_info.denom * 1000000);
        // CPU 滴答时间就是CPU运行时间，也就是开机时间了
        printf("mach_absolute_time(): %lld毫秒 %lld \n", millis, millis/(60 * 1000));
        showCurTime(millis);
    }
    
    void testImp()
    {
        struct timeval tv;
        struct timezone tz;
        
        gettimeofday(&tv, &tz);
        
        cout << "秒：" << tv.tv_sec <<" 微妙：" << tv.tv_usec << "\n";
        cout << "时差：" << tz.tz_minuteswest << " type of DST correction :" << tz.tz_dsttime << "\n";
        
        printf("gettimeofday():%ld秒 %d微秒 ", tv.tv_sec, tv.tv_usec);
        showCurTime(tv.tv_sec);
        testMach();
        sleep(1);
        testMach();
        sleep(1);
        testMach();
        normalUse();
    }
};

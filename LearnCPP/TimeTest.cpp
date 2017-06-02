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
    
    void testImp()
    {
        struct timeval tv;
        struct timezone tz;
        
        gettimeofday(&tv, &tz);
        
        std::cout << "秒：" << tv.tv_sec <<" 微妙：" << tv.tv_usec << "\n";
        std::cout << "时差：" << tz.tz_minuteswest << " type of DST correction :" << tz.tz_dsttime << "\n";
        
        char time_string[30];
        struct tm *ptm;
        ptm = localtime(&tv.tv_sec);
        strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);
        long milliseconds = tv.tv_usec/1000;
        
        printf("当前时间：%s.%3ld\n", time_string, milliseconds);
        
        normalUse();
    }
};

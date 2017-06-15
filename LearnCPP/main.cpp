//
//  main.cpp
//  LearnCPlaus
//
//  Created by javalong on 2017/6/1.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include <iostream>
#include "PingTest.cpp"
#include "ClassTest.cpp"
#include "TimeTest.cpp"
#include "SockTest.cpp"
#include "UnionTest.cpp"
#include "IcmpTest.cpp"
#include "SysctlTest.cpp"
#include "pingquery.h"
#include "OtherTest.h"
#include "ThreadTest.cpp"
#include "BoostTest.hpp"


int main(int argc, const char * argv[])
{
//    BoostTest().testImp();
    ThreadTest().testImp();
//    SysctlTest().testImp();
//    IcmpTest().testImp();
//    UnionTest().testImp();
//    ClassTest().testImp();
//    PingTest().testImp();
//    TimeTest().testImp();
//    SockTest().testImp();
    
//    std::string host = "www.qq.com";
//    mars::sdt::PingQuery pingObj;
//    int ret = pingObj.RunPingQuery(0, 0, 0, host.c_str(), 0);
//    if (ret != 0) {
//        printf("ret=%d", ret);
//        return ret;
//    }
    
    
    
    return 0;
}

